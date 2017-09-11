
//The AVR hardware clears the global interrupt flag in SREG before entering an interrupt vector. Thus, normally interrupts will remain disabled inside the handler until the handler exits, where the RETI instruction (that is emitted by the compiler as part of the normal function epilogue for an interrupt handler) will eventually re-enable further interrupts. For that reason, interrupt handlers normally do not nest. For most interrupt handlers, this is the desired behaviour, for some it is even required in order to prevent infinitely recursive interrupts (like UART interrupts, or level-triggered external interrupts). In rare circumstances though it might be desired to re-enable the global interrupt flag as early as possible in the interrupt handler, in order to not defer any other interrupt more than absolutely needed. This could be done using an sei() instruction right at the beginning of the interrupt handler, but this still leaves few instructions inside the compiler-generated function prologue to run with global interrupts disabled.

/**
 * An Mirf example which copies back the data it recives.
 *
 * Pins:
 * Hardware SPI:
 * MISO -> 12
 * MOSI -> 11
 * SCK -> 13
 *
 * Configurable:
 * CE -> 10
 * CSN -> 9
 *
 */

#include <avr/wdt.h>
// include the library code:
#include <OneWire.h>
#include <SPI.h>

#include "RF24.h"
#include "printf.h"
#include "nRF24L01.h"

#define POWER_USAGE_MONITOR

//PINS SETUP
// from SPI.h
// MOSI: pin 11
// MISO: pin 12
// SCK: pin 13
#define MAINS_CIRCUIT_1_ON_DETECTOR_PIN A1
#define MAINS_CIRCUIT_2_ON_DETECTOR_PIN A3
#define TEMPERATURE_PIN A5
#define LED1_PIN A0         
#define LED2_PIN 8         
#define TONE_PIN 7
#define LED_MODE_BUTTON_PIN 6

#define MAINS_SENSOR_READ_DELAY 100

//constants
#define TEMP_PACKET             3
#define TEMP_PACKET_POWER_BOX   10
#define POWER_ON_PACKET         11
#define ANSW_NONE 0
#define NO_MORE_ITEMS 255
#define REQUEST_TOKEN 254
#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

#define LEDS_MODE_TXRX 1
#define LEDS_MODE_POWER_STATUS 2

//to test if our receiver is working and getting sth
unsigned long lastAnyRadioResponse = 0;

unsigned long previousMillisLed1=0;
unsigned long previousMillisLed2=0;
unsigned long previousMillisTemp=0;
unsigned long previousMillisTXRX=0;
unsigned long lastMainsControllCircuitActiveTime_1=0;
unsigned long lastMainsControllCircuitActiveTime_2=0;
unsigned int ledsMode;
//bits are 1 in case given circuit is on
unsigned int circuitsState = 0;

//button handling
int lastButtonState;
unsigned long lastButtonDebounceTime;
int lastButtonStateReading;

boolean waitForConversion = false;
boolean receivedRequest = false;
float temp;
byte msgType = 0;
// temperature read related:
byte ti;
byte tdata[12];
byte taddr[8];
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long intervalLED1 = 1000;           // interval at which to blink (milliseconds)
unsigned long intervalLED2 = 1000;           // interval at which to blink (milliseconds)
unsigned long intervalBeep = 6000;
unsigned long previousMillisCircuit_1;
unsigned long intervalCircuit_1;
unsigned long previousMillisCircuit_2;
unsigned long intervalCircuit_2;
int answerType = ANSW_NONE;
long lastMainsSensor_1_ReadTime = millis();
long lastMainsSensor_2_ReadTime = 0;

//initialize one wire connection to Dallas temperature sensor
OneWire ds(TEMPERATURE_PIN);

// Set up nRF24L01 radio on SPI bus plus pins ce = 10  & csn = 9
RF24 radio(10,9);

// Radio pipe addresses for the 2 nodes to communicate.
//Tx,Rx
const uint64_t InfoToSensorStationPipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};
const uint64_t SensorToInfoStationPipes[3] = { 0xF0F0F0F0D2LL, 0xF0F0F0F0E1LL};

const uint64_t InfoToPowerStationPipes[2] = { 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL};
const uint64_t PowerToInfoStationPipes[3] = { 0xF0F0F0F0B4LL, 0xF0F0F0F0C3LL};

void setup() {
  //not needed with optiboot which handles this correctly
  // Clear the reset bit
  //MCUSR &= ~_BV(WDRF);
  // Disable the WDT
  //WDTCSR |= _BV(WDCE) | _BV(WDE); 
  //WDTCSR = 0;

  ledsMode = LEDS_MODE_POWER_STATUS;
  pinMode(MAINS_CIRCUIT_1_ON_DETECTOR_PIN, INPUT);
  pinMode(MAINS_CIRCUIT_2_ON_DETECTOR_PIN, INPUT);
  pinMode(TEMPERATURE_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);         
  pinMode(LED2_PIN, OUTPUT);         
  pinMode(TONE_PIN, OUTPUT);
  pinMode(LED_MODE_BUTTON_PIN, INPUT);
  //initialize com port based communication/debug/log
  //Serial.begin(57600);
  //Serial.println("Infostacja wita! Procedura uruchamiania wykonana!");
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);
  delay(500);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED1_PIN, LOW);
  delay(500);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  
  Serial.begin(57600);
  Serial.println("Starting up ...");
  printf_begin();
  printf("\n\rRF24-InfoStation WELCOME - Starting Radio...\n\r");

  //
  // Setup and configure rf radio
  //
  radio.begin();
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  //setDataRate()  RF24_2MBPS, RF24_250KBPS
  radio.setDataRate(RF24_250KBPS);
  // radio.setPALevel( RF24_PA_MAX );
  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);
  radio.setChannel(95);  
  
  radio.openWritingPipe(PowerToInfoStationPipes[0]);
  radio.openReadingPipe(1,PowerToInfoStationPipes[1]);
  
  radio.startListening();
  radio.printDetails();
    
  //Start the watchdog beast to be hungry after 2 sec
  wdt_reset(); 
  wdt_enable(WDTO_2S);

}

void loop() {
  handleMainsSensor();
  blinkTheLEDLoop(); 
  handleSendReceiveRF24();
  getDS18B22Temperature();
  handleUserButton();
  //if no message from radio since 1 minute - allow reset
  if(millis() - lastAnyRadioResponse < 60000){
    wdt_reset();
  }  
}

void handleMainsSensor(){
    if(millis() - lastMainsSensor_1_ReadTime > MAINS_SENSOR_READ_DELAY){
    lastMainsSensor_1_ReadTime = millis();
    int read1 = 0;
    for(int x = 0; x < 10; x++){
      read1+= analogRead(MAINS_CIRCUIT_1_ON_DETECTOR_PIN);
    }
    read1=(read1/10);
    printf("read1: %d",read1);
    if( read1 > 508){
      circuitsState = circuitsState | 1;
      //if(ledsMode == LEDS_MODE_POWER_STATUS){
        //digitalWrite(LED1_PIN,HIGH);   
        blinkTheLED1(2000);
      //}
    }
    //else {
    //  circuitsState = circuitsState & (!1);
    //  if(ledsMode == LEDS_MODE_POWER_STATUS){
    //    digitalWrite(LED1_PIN,LOW);   
    //  }
    //}
    
    int read2 = 0;
    for(int x = 0; x < 10; x++){
      read2+= analogRead(MAINS_CIRCUIT_2_ON_DETECTOR_PIN);
    }
    read2=read2/10;
    printf("read2: %d",read2);

    if( read2 > 513){
      circuitsState = circuitsState | 2;
      //if(ledsMode == LEDS_MODE_POWER_STATUS){
        //digitalWrite(LED2_PIN,HIGH);   
        blinkTheLED2(2000); 
      //}
    }
    //else {
    //  circuitsState = circuitsState & (!2);
    //  if(ledsMode == LEDS_MODE_POWER_STATUS){
    //    digitalWrite(LED2_PIN,LOW);   
    //  }
    //}
  }
}

void blinkTheLEDLoop(){
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillisLed1 > intervalLED1) {
      digitalWrite(LED1_PIN, LOW);
      circuitsState = circuitsState & (!1);
    }
     if(currentMillis - previousMillisLed2 > intervalLED2) {
      digitalWrite(LED2_PIN, LOW);
      circuitsState = circuitsState & (!2);
    }
}

void blinkTheLED1(int duration){
  intervalLED1 = duration;
  previousMillisLed1 = millis(); 
  digitalWrite(LED1_PIN, HIGH);  
}

void blinkTheLED2(int duration){
  intervalLED2 = duration;
  previousMillisLed2 = millis(); 
  digitalWrite(LED2_PIN, HIGH);  
}

 //Get temperature from DS18B22
void getDS18B22Temperature(){
  //to be frequenty called, e.g. directly from loop
 if(!waitForConversion){ 
   //find a device
   if (!ds.search(taddr)) {
     ds.reset_search();
     return;
   }
   if (OneWire::crc8( taddr, 7) != taddr[7]) {
     return;
   }
   if (taddr[0] != DS18S20_ID && taddr[0] != DS18B20_ID) {
     return;
   }
   ds.reset();
   ds.select(taddr);
   // Start conversion
   ds.write(0x44, 1);
   // Wait some time...
   waitForConversion = true;
   previousMillisTemp = millis();
   } 
 else {
   unsigned long currentMillis = millis();
   //in case of overflow, which will happen every 50 days, just read whaever is there, next read will be correctly waited.
   if((currentMillis - previousMillisTemp > 850) || (currentMillis - previousMillisTemp < 0)) {
      waitForConversion = false;   
      //reset DS sensor
      ds.reset();
      ds.select(taddr);
      // Issue Read scratchpad command
      ds.write(0xBE);
      // Receive 9 bytes
      for ( ti = 0; ti < 9; ti++) {
        tdata[ti] = ds.read();
      }
      // Calculate temperature value
      temp = ( (tdata[1] << 8) + tdata[0] )*0.0625;
      }
   }
 }
 
 void handleSendReceiveRF24(){
  byte data[radio.getPayloadSize()];

  //RECEIVE 
  // If a rf package is available - process it
  if(radio.available()){
     if(ledsMode == LEDS_MODE_TXRX){    
       digitalWrite(LED1_PIN, HIGH);
     }
     boolean readingDone = false;  
     
     while(!readingDone){
       readingDone = radio.read( &data, radio.getPayloadSize() );
       msgType = data[0];
       if( msgType == REQUEST_TOKEN) {
         //received request to send report of mesurements
         receivedRequest = true;
         answerType = ANSW_NONE;
       }
     }
     if(ledsMode == LEDS_MODE_TXRX){ 
       digitalWrite(LED1_PIN, LOW); 
     }
     lastAnyRadioResponse = millis();
  } 
  
  //SEND
  if ( receivedRequest ) {
      switch(answerType) {
      case ANSW_NONE:
          answerType = TEMP_PACKET;
          break;
      case TEMP_PACKET:
          answerType = POWER_ON_PACKET;
          // create and send the data packet
          data[0] = TEMP_PACKET_POWER_BOX;
          dtostrf(temp, 6, 3, (char*)data+1);
          break;
      case POWER_ON_PACKET:
          answerType = NO_MORE_ITEMS;
          data[0] = POWER_ON_PACKET;
          //send both circuit states as bit flags
          data[1] = circuitsState;
          break;
      case NO_MORE_ITEMS:
          answerType = ANSW_NONE;
          data[0] = NO_MORE_ITEMS;
          receivedRequest = false;
          break;
    }

    // Delay just a little bit to let the other unit
    // make the transition to receiver
    delay(40);
    if(ledsMode == LEDS_MODE_TXRX){
      digitalWrite(LED2_PIN, HIGH);     
    }
    //do the transmission
    printf("Now sending: %d...",data[0]);
    // First, stop listening so we can talk.
    radio.stopListening();

    bool ok = radio.write( data, radio.getPayloadSize() );
    // Now, continue listening
    radio.startListening();
    
    if (ok)
      printf("ACK");
    else
      printf("NO_ACK.\n\r");
    if(ledsMode == LEDS_MODE_TXRX){
      digitalWrite(LED2_PIN, LOW);     
    }
    delay(10);
}
  
}

void handleUserButton(){
   int reading = digitalRead(LED_MODE_BUTTON_PIN);
   // If the switch changed, due to noise or pressing:
   if (reading != lastButtonState) {
     // reset the debouncing timer
     lastButtonDebounceTime = millis();
   }
   if ((millis() - lastButtonDebounceTime) > 50) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if(reading == LOW && (reading != lastButtonStateReading)){
      if(ledsMode == LEDS_MODE_TXRX){
        ledsMode = LEDS_MODE_POWER_STATUS;
      } else {
        ledsMode = LEDS_MODE_TXRX;
      }
      blinkTheLED1(1000);   
      blinkTheLED2(1000);   
      tone(TONE_PIN, 3500, 300);
    }
    lastButtonStateReading = reading;
   }
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButtonState = reading;
}


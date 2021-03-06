
//The AVR hardware clears the global interrupt flag in SREG before entering an interrupt vector. Thus, normally interrupts will remain disabled inside the handler until the handler exits, where the RETI instruction (that is emitted by the compiler as part of the normal function epilogue for an interrupt handler) will eventually re-enable further interrupts. For that reason, interrupt handlers normally do not nest. For most interrupt handlers, this is the desired behaviour, for some it is even required in order to prevent infinitely recursive interrupts (like UART interrupts, or level-triggered external interrupts). In rare circumstances though it might be desired to re-enable the global interrupt flag as early as possible in the interrupt handler, in order to not defer any other interrupt more than absolutely needed. This could be done using an sei() instruction right at the beginning of the interrupt handler, but this still leaves few instructions inside the compiler-generated function prologue to run with global interrupts disabled.

/**
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

#define ON_OFF_RECEIVER

//PINS SETUP
// from SPI.h
// MOSI: pin 11
// MISO: pin 12
// SCK: pin 13
#define MAINS_CIRCUIT_1_CTRL_PIN 3
#define MAINS_CIRCUIT_1_ON_DETECTOR_PIN A2
#define MAINS_CIRCUIT_2_CTRL_PIN A3
#define MAINS_CIRCUIT_2_ON_DETECTOR_PIN A4
#define TEMPERATURE_PIN A5
#define LED1_PIN A0         
#define LED2_PIN 8         
#define TONE_PIN 7
#define LED_MODE_BUTTON_PIN 6

#define MAINS_SENSOR_READ_DELAY 1000

//constants
#define TEMP_PACKET             3
#define TEMP_PACKET_POWER_BOX   10
#define POWER_ON_PACKET         11
#define ANSW_NONE 0
#define NO_MORE_ITEMS 255
#define REQUEST_ON_TRIGGER 198
#define REQUEST_ON_TRIGGER_INPUT 199
#define REQUEST_ON_TRIGGER_FINAL 200
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
boolean waitingForFinal = false;
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
long lastMainsSensor_1_ReadTime = 0;
long lastMainsSensor_2_ReadTime = 0;
unsigned long requestOnTriggerToken;
//initialize one wire connection to Dallas temperature sensor
OneWire ds(TEMPERATURE_PIN);

// Set up nRF24L01 radio on SPI bus plus pins ce = 10  & csn = 9
RF24 radio(10,9);

// Radio pipe addresses for the 2 nodes to communicate.
//Tx,Rx
const uint64_t InfoToSensorStationPipes[2] = { 0xF0F0F0F0E5LL, 0xF0F0F0F0D6LL};
const uint64_t SensorToInfoStationPipes[3] = { 0xF0F0F0F0D6LL, 0xF0F0F0F0E5LL};

const uint64_t InfoToPowerStationPipes[2] = { 0xF0F0F0F0C7LL, 0xF0F0F0F0B8LL};
const uint64_t PowerToInfoStationPipes[3] = { 0xF0F0F0F0B8LL, 0xF0F0F0F0C7LL};

void setup() {
  //not needed with optiboot which handles this correctly
  // Clear the reset bit
  //MCUSR &= ~_BV(WDRF);
  // Disable the WDT
  //WDTCSR |= _BV(WDCE) | _BV(WDE); 
  //WDTCSR = 0;

  ledsMode = LEDS_MODE_TXRX;
  pinMode(TEMPERATURE_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);         
  pinMode(LED2_PIN, OUTPUT);         
  pinMode(TONE_PIN, OUTPUT);
  pinMode(MAINS_CIRCUIT_1_CTRL_PIN, OUTPUT);
  digitalWrite(MAINS_CIRCUIT_1_CTRL_PIN, HIGH);
  pinMode(MAINS_CIRCUIT_2_CTRL_PIN, OUTPUT);
  digitalWrite(MAINS_CIRCUIT_2_CTRL_PIN, HIGH);
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
  radio.setChannel(85);  
  
  radio.openWritingPipe(PowerToInfoStationPipes[0]);
  radio.openReadingPipe(1,PowerToInfoStationPipes[1]);
  
  radio.startListening();
  radio.printDetails();
    
  //Start the watchdog beast to be hungry after 2 sec
  //wdt_reset(); 
  //wdt_enable(WDTO_2S);

}

void loop() {
  powerOnCircuitLoop();   
  blinkTheLEDLoop(); 
  handleSendReceiveRF24();
  //getDS18B22Temperature();
  handleUserButton();
  //if no message from radio since 1 minute - allow reset
  //if(millis() - lastAnyRadioResponse < 60000){
  //  wdt_reset();
  //}  
}


void powerOnCircuitLoop(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisCircuit_1 > intervalCircuit_1) {
    //HIGH state disables the realy on Electrodragon module
    digitalWrite(MAINS_CIRCUIT_1_CTRL_PIN, HIGH);
    circuitsState = circuitsState & (~1);
  }
   if(currentMillis - previousMillisCircuit_2 > intervalCircuit_2) {
    digitalWrite(MAINS_CIRCUIT_2_CTRL_PIN, HIGH);
    circuitsState = circuitsState & (~2);
  }
}

void powerOnCircuit_1(int duration){
  intervalCircuit_1 = duration;
  previousMillisCircuit_1 = millis(); 
  //LOW state enables the electrodragon relay module
  digitalWrite(MAINS_CIRCUIT_1_CTRL_PIN, LOW);
}

void powerOnCircuit_2(int duration){
  intervalCircuit_2 = duration;
  previousMillisCircuit_2 = millis(); 
  digitalWrite(MAINS_CIRCUIT_2_CTRL_PIN, LOW);  
}


void blinkTheLEDLoop(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisLed1 > intervalLED1) {
    digitalWrite(LED1_PIN, LOW);
  }
   if(currentMillis - previousMillisLed2 > intervalLED2) {
    digitalWrite(LED2_PIN, LOW);
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
         unsigned long value = 0;
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
       if( msgType == REQUEST_ON_TRIGGER) {
         waitingForFinal = true;
         receivedRequest=true;
         requestOnTriggerToken = millis();
         data[0] = REQUEST_ON_TRIGGER_INPUT;
         data[1] = requestOnTriggerToken & 0xFF;
         data[2] = (requestOnTriggerToken >> 8) & 0xFF;
         data[3] = (requestOnTriggerToken >> 16) & 0xFF;
         data[4] = (requestOnTriggerToken >> 24) & 0xFF;
       }
       if( (msgType == REQUEST_ON_TRIGGER_FINAL) && waitingForFinal){
         waitingForFinal = false;
         value = 0;
         value = data[4];
         value = (value <<8) | data[3];
         value = (value <<8) | data[2];
         value = (value <<8) | data[1];
  
         if(value == ((requestOnTriggerToken >> 3)+15)){
             powerOnCircuit_1(500);
         }  
         //invalidate token - set to sth we did not sent and do not expect
         requestOnTriggerToken = requestOnTriggerToken + 13;
       }
     }
     if(ledsMode == LEDS_MODE_TXRX){ 
       digitalWrite(LED1_PIN, LOW); 
     }
     lastAnyRadioResponse = millis();
  } 
  
  //SEND - disabled
 
  if ( receivedRequest ) {
     receivedRequest = false;

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


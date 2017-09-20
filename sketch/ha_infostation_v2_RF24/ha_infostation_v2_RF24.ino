/*
* 18.08.2012
* Fixed nIRQ pin definition in RF12B library - def.h, if this doesn't help to serve the IRQ correctly, try the lib in original form adjusting hardware to required pin definitions rather than software to hardware layaut.
* Fixed NFFS (fifo select) controll, in construction just pull it high
*/

//Infostation UART conn:
//blue - gnd
//green - rx
//violet - tx

//The AVR hardware clears the global interrupt flag in SREG before entering an interrupt vector. Thus, normally interrupts will remain disabled inside the handler until the handler exits, where the RETI instruction (that is emitted by the compiler as part of the normal function epilogue for an interrupt handler) will eventually re-enable further interrupts. For that reason, interrupt handlers normally do not nest. For most interrupt handlers, this is the desired behaviour, for some it is even required in order to prevent infinitely recursive interrupts (like UART interrupts, or level-triggered external interrupts). In rare circumstances though it might be desired to re-enable the global interrupt flag as early as possible in the interrupt handler, in order to not defer any other interrupt more than absolutely needed. This could be done using an sei() instruction right at the beginning of the interrupt handler, but this still leaves few instructions inside the compiler-generated function prologue to run with global interrupts disabled.

/* 
The LCD circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

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
#define USE_NRF24L01

#include <avr/wdt.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <SPI.h>

#include "RF24.h"
#include "printf.h"
#include "nRF24L01.h"

//#define SENSORSTATION
// OR
#define INFOSTATION

#define PHOTOTRANS_OR_BTN_PIN A1       // PhotoTransistor is connected to Arduino digital pin A1.// GENERAL user button in case of infostation
#define HEATER_BTN_PIN 2
#define TEMPERATURE_PIN A3
#define LED1_PIN A0         
#define LED2_PIN 8
#define TONE_PIN A2
#define MAINS_INDICATOR_PIN 7
#define MAINS_CTRL_PIN 7
#define TEMP_PACKET     3
#define MAINS_PACKET    4
#define MAINS_ORDER 5
#define THERMISTER_PIN A3 //analog read from connection of 10k resistor and 10 thermistor - voltage divider - http://www.hacktronics.com/Tutorials/arduino-thermistor-tutorial
#define TEMP_PACKET_POWER_BOX    10
#define POWER_ON_PACKET 11
#define REC_ALL_START 0
#define ANSW_ALL_START 0
#define REC_SENSORSTATION 1
#define REC_POWER_COTROLLER 2
#define NO_MORE_ITEMS 255
#define REQUEST_TOKEN 254

#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

//to test if our receiver is working and getting sth
unsigned long lastAnyRadioResponse = 0;

int lcdPage = 0;
int lcdPageShowCounter = 0;
//int powerBoxAbsence = 0;
//send nextReceiver related variables
byte powerControllersStatus = 0;
boolean tokenOrOrderMode = true;
int nextReceiver = REC_ALL_START;
int currentReceiver = REC_ALL_START;
int nextAnswer = ANSW_ALL_START;
boolean gotAnswer = false;
boolean mainsRequest = false; 
boolean heatingRequest = false; 
//boolean lastMainsRequest = false;
int lastButton1State = LOW;
int lastButton1StateReading = HIGH;
long lastButton1DebounceTime = 0;

int lastButton2State = LOW;
int lastButton2StateReading = HIGH;
long lastButton2DebounceTime = 0;
//loops timeCounters
unsigned long previousMillisLed1;
unsigned long previousMillisLed2;
unsigned long previousMillisTemp;
unsigned long previousMillisTXRX;
unsigned long previousMillisLCD;
unsigned long previousMillisWS;
unsigned long previousMillisBeep;
boolean waitForConversion = false;
volatile boolean mainsOn = false;
boolean receivedRequest = false;
byte lcdLine1[50] = "-----";
byte lcd2Line1[50] = "-----";
byte TxTypeState = 0;
byte msgType = 0;
// temperature read related:
float temp;
byte ti;
byte tdata[12];
byte taddr[8];
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long intervalLED1 = 1000;           // interval at which to blink (milliseconds)
long intervalLED2 = 1000;           // interval at which to blink (milliseconds)
long intervalBeep = 6000;
//uint16_t RF12BStatus = 0;

//initialize one wire connection to Dallas temperature sensor
OneWire ds(TEMPERATURE_PIN);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A5, A4, 5, 4, 6, 3);

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
  MCUSR &= ~_BV(WDRF);
  // Disable the WDT
  WDTCSR |= _BV(WDCE) | _BV(WDE); 
  WDTCSR = 0;

  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(TONE_PIN, OUTPUT);     
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(PHOTOTRANS_OR_BTN_PIN, INPUT);
  pinMode(HEATER_BTN_PIN, INPUT);  
  pinMode(THERMISTER_PIN, INPUT);
  pinMode(MAINS_INDICATOR_PIN, OUTPUT);  
  //initialize LCD and set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("INFS 5.0");
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);
  delay(500);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED1_PIN, LOW);
  delay(500);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  
  lcd.clear();
  
  //Init Serial comm with Wifly module which will report measured data to Web service and get back controll command
  Serial.begin(9600);
  printf_begin();
  printf("MAIN_STATION-BOOT");

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
  #if defined SENSORSTATION
    radio.openWritingPipe(SensorToInfoStationPipes[0]);
    radio.openReadingPipe(1,SensorToInfoStationPipes[1]);
  #endif
  
  radio.startListening();
  //radio.printDetails();
  
  //Start the watchdog beast to be hungry after 2 sec
  wdt_reset(); 
  wdt_enable(WDTO_2S);

}

void loop() {
  handleUserButton();
  
  handleMainsControll();
  
  handleMainsSensor();
  
  #if defined SENSORSTATION
    get10KThermisterTemp();
  #else
    getDS18B22Temperature(); 
  #endif
  
  blinkTheLEDLoop();
 
  handleLCDRefresh();
 
  handleWebServiceComm();
  
  handleSendReceiveRF24();
  
  //if no message from radio since 1 minute - allow reset
  if(millis() - lastAnyRadioResponse < 60000){
    wdt_reset();
  }
}

void handleMainsControll(){
  #if defined SENSORSTATION
    digitalWrite(MAINS_CTRL_PIN, mainsRequest);
  #endif
}

void handleUserButton(){
  #if defined INFOSTATION
  //////// BUTTON 1 ///////////////////////////////
   int reading = digitalRead(PHOTOTRANS_OR_BTN_PIN);
   // If the switch changed, due to noise or pressing:
   if (reading != lastButton1State) {
     // reset the debouncing timer
     lastButton1DebounceTime = millis();
   }
   if ((millis() - lastButton1DebounceTime) > 50) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if(reading == LOW && (reading != lastButton1StateReading)){
      mainsRequest = !mainsRequest;
      tone(TONE_PIN, 3500, 300);
    }
    lastButton1StateReading = reading;
   }
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButton1State = reading;
  //////// BUTTON 1 ///////////////////////////////
   reading = digitalRead(HEATER_BTN_PIN);
   // If the switch changed, due to noise or pressing:
   if (reading != lastButton2State) {
     // reset the debouncing timer
     lastButton2DebounceTime = millis();
   }
   if ((millis() - lastButton2DebounceTime) > 50) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if(reading == LOW && (reading != lastButton2StateReading)){
      heatingRequest = !heatingRequest;
      tone(TONE_PIN, 3500, 300);
    }
    lastButton2StateReading = reading;
   }
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButton2State = reading;

  #endif
}

void handleMainsSensor(){
  //check on/off 220v sensor 
  #if defined INFOSTATION
  if (mainsOn)
  {
    digitalWrite(MAINS_INDICATOR_PIN,HIGH);
    //let's make some noise
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillisBeep > intervalBeep) {
      previousMillisBeep = currentMillis;
      tone(TONE_PIN, 3000, 75);
     }    
  }else {
    digitalWrite(MAINS_INDICATOR_PIN,LOW);
  }
  #else
  if (mainsOn = (!digitalRead(PHOTOTRANS_OR_BTN_PIN)) )
  { //hi voltage, transistor not conducting - infrared LED not on --> external device OFF
      //let's make some noise
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillisBeep > intervalBeep) {
      previousMillisBeep = currentMillis;
      tone(TONE_PIN, 3000, 50);
     }
  }     
  #endif
}

void handleSendReceiveRF24(){
  byte data[radio.getPayloadSize()];
  
  //RECEIVE 
  // If a rf package is available - process it
  if(radio.available()){
     digitalWrite(LED1_PIN, HIGH);
     boolean readingDone = false;  
     while(!readingDone){
       readingDone = radio.read( &data, radio.getPayloadSize() );
     
       digitalWrite(LED1_PIN, LOW); 
       lastAnyRadioResponse = millis();
       msgType = data[0];
      //Serial.print("RX <--: msgType=");
      //Serial.println(data[0]);
      #if defined SENSORSTATION
        if((msgType == REQUEST_TOKEN) || (msgType == MAINS_ORDER)){
          receivedRequest = true;
        }
        if(msgType == MAINS_ORDER) {
          mainsRequest = data[1];
          nextAnswer = MAINS_ORDER;
        } else {
          nextAnswer = ANSW_ALL_START;
        }    
      #else
        switch(msgType){
          case MAINS_PACKET:
              //remember the remote mains state to present via led state
              mainsOn = data[1];       
          break;
          case TEMP_PACKET: //TODO send data as data (not as stirng) and convert to str locally
              memcpy(lcdLine1, data+1,5);
              lcdLine1[5]='\0';
          break;
          case TEMP_PACKET_POWER_BOX: //TODO Add LCD1 & LCD2 alternation
              memcpy(lcd2Line1, data+1,5);
              lcd2Line1[5]='\0';
          break;
          case POWER_ON_PACKET:
              //remember remote power controllers state and present later on somehowe
              powerControllersStatus = data[1];
          break;
          case NO_MORE_ITEMS:          
            gotAnswer = true;
          break;
        }
     #endif
     }
     //blinkTheLED2(10);
  } //END RECEIVE 

  //SEND
    //if(!Mirf.isSending()) {
      #if defined SENSORSTATION
        if ( receivedRequest ) {
          switch(nextAnswer){
              case ANSW_ALL_START:
                nextAnswer = TEMP_PACKET;
                //break;
              case TEMP_PACKET:
                nextAnswer = MAINS_PACKET;
                data[0] = TEMP_PACKET;
                dtostrf(temp, 6, 3, (char*)data+1);
                break;
              case MAINS_PACKET:
                nextAnswer = NO_MORE_ITEMS;
                data[0] = MAINS_PACKET;
                data[1] = mainsOn;
                break;
              case MAINS_ORDER:
              case NO_MORE_ITEMS:
                nextAnswer = ANSW_ALL_START;
                data[0] = NO_MORE_ITEMS;
                receivedRequest = false;
               break;             
          }
          //Serial.print("TX -->: msgType=");
          //Serial.print(data[0]);
          //Serial.print("...");
          
          // Delay just a little bit to let the other unit
          // make the transition to receiver
          delay(40);
          digitalWrite(LED2_PIN, HIGH);  
          
          //printf("Now sending: %d...",data[0]);
          // First, stop listening so we can talk.
          radio.stopListening();
      
          bool ok = radio.write( data, radio.getPayloadSize() );
          // Now, continue listening
          radio.startListening();
          
          if (ok)
            //printf("ACK");
          else
            //printf("NO_ACK.\n\r");
          digitalWrite(LED2_PIN, LOW);
          delay(10);     
        }            
      #else
        //byte * receiverAddr;
        unsigned long currentMillis = millis();
        if (gotAnswer || (currentMillis - previousMillisTXRX > 500)) {
          //if(gotAnswer == false){
             //if(currentReceiver == REC_POWER_COTROLLER) ++powerBoxAbsence;
             //Serial.println("Timeout waiting for response!");
          //} else {
            //if(currentReceiver == REC_POWER_COTROLLER){ 
            //  powerBoxAbsence = 0;
            //}
          //}
          
          gotAnswer = false;
          previousMillisTXRX = currentMillis; 
          if(tokenOrOrderMode){
            //send request token for specific data collection from selected sensor 
            switch(nextReceiver){
              case REC_ALL_START:
                currentReceiver = REC_ALL_START;
                nextReceiver = REC_SENSORSTATION;
                gotAnswer = true;
                return;
              case REC_SENSORSTATION:
                currentReceiver = REC_SENSORSTATION;
                //if(powerBoxAbsence < 80){
                nextReceiver = REC_POWER_COTROLLER;
                //} 
                radio.openWritingPipe(InfoToSensorStationPipes[0]);
                radio.openReadingPipe(1,InfoToSensorStationPipes[1]);
                break;
              case REC_POWER_COTROLLER:
                currentReceiver = REC_POWER_COTROLLER;
                nextReceiver = REC_SENSORSTATION;
                radio.openWritingPipe(InfoToPowerStationPipes[0]);
                radio.openReadingPipe(1,InfoToPowerStationPipes[1]);
                break; 
            }
            data[0] = REQUEST_TOKEN;
          } else { //ORDER MODE
                currentReceiver = REC_SENSORSTATION;
                radio.openWritingPipe(InfoToSensorStationPipes[0]);
                radio.openReadingPipe(1,InfoToSensorStationPipes[1]);
                data[0] = MAINS_ORDER;
                //send out value of the request
                data[1] = mainsRequest;
                data[2] = heatingRequest;          
          }
          
          tokenOrOrderMode = !tokenOrOrderMode;
          
          //Serial.print("TX -->");
          //Serial.print((char* )receiverAddr);
          //Serial.print(": msgType=");
          //Serial.print(data[0]);
          //Serial.print("...");
          delay(40);
          digitalWrite(LED2_PIN, HIGH);  
    
          //printf("Now sending: %d...",data[0]);
          // First, stop listening so we can talk.
          radio.stopListening();
      
          bool ok = radio.write( data, radio.getPayloadSize() );
          // Now, continue listening
          radio.startListening();
          
          //if (ok)
            //printf("ACK");
          //else
            //printf("NO_ACK.\n\r");
          digitalWrite(LED2_PIN, LOW);
          delay(10);     
        }    
      #endif
      //}
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

 void handleWebServiceComm(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisWS > 5000){
    char tempTXT[8];
    dtostrf(temp, 6, 3, (char*)tempTXT);
    printf("outdoor.temperature=%s&indoor.temperature=%s&pomp.waste.water=%d&pomp.rain.water=%d&pomp.warm.water=%s&temp.warm.water=%s&up.time.main.station=%d",(char*)lcd2Line1,tempTXT,(powerControllersStatus & 0x01),(powerControllersStatus & 0x02),mainsOn?"PRACA":"CZEKA",(char*)lcdLine1,(millis()/1000/60));
    previousMillisWS = currentMillis;
  }   
 } 

 void handleLCDRefresh(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisLCD > 100){
    switch(lcdPage){
      case 0: lcdPageTWTL();      
      break;
      case 1: lcdPageTZPS();
      break;
      case 2: lcdPageUpTime();
      break;
    }
      ++lcdPageShowCounter;
      if(lcdPageShowCounter > 30){
        lcdPage = (lcdPage+1)%3;
        lcdPageShowCounter = 0;  
    }
    previousMillisLCD = currentMillis;
  } 
 }

void  lcdPageTWTL(){
    lcd.setCursor(0,0);
    lcd.print("TW:");
    lcd.print((char*)lcdLine1);
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("TL:");
    lcd.print(temp);
    //lcd.print(" S:");
    //RF12BStatus = *((uint16_t*)(RF12.status())) ;
    //lcd.print(RF12BStatus);
}

void  lcdPageTZPS(){
    lcd.setCursor(0,0);
    lcd.print("TZ:");
    lcd.print((char*)lcd2Line1);
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("POW: ");
    lcd.print(powerControllersStatus & 0x01);
    lcd.print(" ");
    lcd.print(powerControllersStatus & 0x02);
}

void  lcdPageUpTime(){
    lcd.setCursor(0,0);
    lcd.print("Up Time:");
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis()/1000/60);
    lcd.print("m                   ");
}


void get10KThermisterTemp() {
  double Temp;
  int RawADC = analogRead(THERMISTER_PIN);
  // See http://en.wikipedia.org/wiki/Thermistor for explanation of formula
  Temp = log(((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;           // Convert Kelvin to Celcius
  temp = Temp;
  //temp = 19.96;
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

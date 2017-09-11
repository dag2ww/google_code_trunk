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
#include <SPI.h>
#include "dht.h"
#include "RF24.h"
#include "printf.h"
#include "nRF24L01.h"

#define BTN_PIN_1 4
#define BTN_PIN_2 5
#define LED1_PIN 8         
#define LED2_PIN 9         
#define TONE_PIN 2
#define TEMP_PACKET     3
#define MAINS_PACKET    4
#define MAINS_ORDER 5
#define TEMP_PACKET_POWER_BOX    10
#define POWER_ON_PACKET 11
#define REC_ALL_START 0
#define ANSW_ALL_START 0
#define REC_SENSORSTATION 1
#define REC_POWER_COTROLLER 2
#define NO_MORE_ITEMS 255
#define REQUEST_TOKEN 254
#define HUMIDITY_1_PIN 7
#define HUMIDITY_2_PIN 6
#define HUMIDITY_3_PIN 8

dht DHT;


//to test if our receiver is working and getting sth
unsigned long lastAnyRadioResponse = 0;
int dhtReadMode = 2;
int lcdShowMode = 1;
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
unsigned long previousMillisDHT;
unsigned long previousMillisTXRX;
unsigned long previousMillisLCD;
unsigned long previousMillisSerial;
unsigned long previousMillisWS;
unsigned long previousMillisBeep;
boolean waitForConversion = false;
volatile boolean mainsOn = false;
boolean receivedRequest = false;
byte temperature1[50] = "-----";
byte humidity1[50] = "-----";
byte dewPointStr1[50] = "-----";
byte dewPointFastStr1[50] = "-----";
byte temperature2[50] = "-----";
byte humidity2[50] = "-----";
byte dewPointStr2[50] = "-----";
byte dewPointFastStr2[50] = "-----";

byte temperature3[50] = "-----";
byte humidity3[50] = "-----";
byte dewPointStr3[50] = "-----";
byte dewPointFastStr3[50] = "-----";

byte temperatureSR[50] = "-----";
byte humiditySR[50] = "-----";
float humidityF1 = 0;
float humidityF2 = 0;
float humidityF3 = 0;

float temperatureF1 = 0;
float temperatureF2 = 0;
float temperatureF3 = 0;

float humidityAvg = 0;
float temperatureAvg = 0;
int avgCnt = 0;
int i = 0;
int j = 11;
int k = 21;

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

//LiquidCrystal::LiquidCrystal(uint8_t rs,  uint8_t enable,
//			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

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
  pinMode(BTN_PIN_1, INPUT);
  pinMode(BTN_PIN_2, INPUT);

//  pinMode(MAINS_INDICATOR_PIN, OUTPUT);  
  //initialize LCD and set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("WEATHER 5.0");
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
  //radio.begin();
  //radio.setRetries(15,15);
  //radio.setDataRate(RF24_250KBPS);
  //radio.setPayloadSize(8);
  //radio.setChannel(95);  
  //#if defined SENSORSTATION
  //  radio.openWritingPipe(SensorToInfoStationPipes[0]);
  //  radio.openReadingPipe(1,SensorToInfoStationPipes[1]);
  //#endif
  
  //radio.startListening();
  //radio.printDetails();
  
  //Start the watchdog beast to be hungry after 2 sec
  //wdt_reset(); 
  //wdt_enable(WDTO_2S);

}

void loop() {
  handleHumidityTemperature();

  handleLCDRefresh();
  
  handleSerialComm();

  handleUserButton();
      
  //blinkTheLEDLoop();
  
  //handleSendReceiveRF24();
  
  //if no message from radio since 1 minute - allow reset
  //if(millis() - lastAnyRadioResponse < 60000){
  //  wdt_reset();
  //}
}

void button1Pressed(){
  lcdShowMode = ++lcdShowMode;
  lcdShowMode = lcdShowMode % 4;
}

void button2Pressed(){
  dhtReadMode = ++dhtReadMode;
  dhtReadMode = dhtReadMode % 3;
}

void handleUserButton(){
  //////////////////////////BTN 1 //////////////////
  int reading = digitalRead(BTN_PIN_1);
   // If the switch changed, due to noise or pressing:
   if (reading != lastButton1State) {
     // reset the debouncing timer
     lastButton1DebounceTime = millis();
   }
   if ((millis() - lastButton1DebounceTime) > 50) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if(reading == LOW && (reading != lastButton1StateReading)){
      tone(TONE_PIN, 3500, 300);
      button1Pressed();
    }
    lastButton1StateReading = reading;
   }
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButton1State = reading;

  //////////////////////////BTN 2 //////////////////
   reading = digitalRead(BTN_PIN_2);
   // If the switch changed, due to noise or pressing:
   if (reading != lastButton2State) {
     // reset the debouncing timer
     lastButton2DebounceTime = millis();
   }
   if ((millis() - lastButton2DebounceTime) > 50) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if(reading == LOW && (reading != lastButton2StateReading)){
      tone(TONE_PIN, 3500, 300);
      button2Pressed();
    }
    lastButton2StateReading = reading;
   }
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButton2State = reading;

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
              memcpy(temperature1, data+1,5);
              temperature1[5]='\0';
          break;
          case TEMP_PACKET_POWER_BOX: //TODO Add LCD1 & LCD2 alternation
              memcpy(humidity1, data+1,5);
              humidity1[5]='\0';
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
            //send request token for specific data from selected sensor 
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

 void handleLCDRefresh(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisLCD > 100){
    switch(lcdPage){
      case 0: lcdPageTWSR();      
      break;
      case 1: lcdPageTW();      
      break;
      case 2: lcdPageDewPoint();
      break;
      case 3: lcdPageUpTime();
      break;
    }
    lcdPage = lcdShowMode;
    previousMillisLCD = currentMillis;
  } 
 }
  
 void handleSerialComm(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisSerial > 1000){
    printf("T: %s, W: %s\n\r",temperature3, humidity3 );
    previousMillisSerial = currentMillis;
  }    
 }

void lcdPageDewPoint(){
  if(dhtReadMode == 0){
    lcdPageDewPoint1();
  }
  if(dhtReadMode == 1){
    lcdPageDewPoint2();
  }
  if(dhtReadMode == 2){
    lcdPageDewPoint3();
  }
}

void  lcdPageTW(){
  if(dhtReadMode == 0){
    lcdPageTW1();
  }
  if(dhtReadMode == 1){
    lcdPageTW2();
  }
  if(dhtReadMode == 2){
    lcdPageTW3();
  }
}

void  lcdPageDewPoint1(){
    lcd.setCursor(0,0);
    lcd.print("P. Rosy1(C):");
    lcd.print((char*)dewPointFastStr1);
    lcd.setCursor(0, 1);
    lcd.print("                ");
}

void  lcdPageDewPoint2(){
    lcd.setCursor(0,0);
    lcd.print("P. Rosy2(C):");
    lcd.print((char*)dewPointFastStr2);
    lcd.setCursor(0, 1);
    lcd.print("                ");
}

void  lcdPageDewPoint3(){
    lcd.setCursor(0,0);
    lcd.print("P. Rosy3(C):");
    lcd.print((char*)dewPointFastStr3);
    lcd.setCursor(0, 1);
    lcd.print("                ");
}

void  lcdPageTW1(){
    lcd.setCursor(0,0);
    lcd.print("Temp1: ");
    lcd.print((char*)temperature1);
    lcd.print("         ");
    lcd.setCursor(0, 1);
    lcd.print("Wilg1: ");
    lcd.print((char*)humidity1);
    lcd.print("         ");
}

void  lcdPageTW2(){
    lcd.setCursor(0,0);
    lcd.print("Temp2: ");
    lcd.print((char*)temperature2);
    lcd.print("         ");
    lcd.setCursor(0, 1);
    lcd.print("Wilg2: ");
    lcd.print((char*)humidity2);
    lcd.print("         ");
}

void  lcdPageTW3(){
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print((char*)temperature3);
    lcd.print("         ");
    lcd.setCursor(0, 1);
    lcd.print("Wilg: ");
    lcd.print((char*)humidity3);
    lcd.print("         ");
}

void  lcdPageTWSR(){
    lcd.setCursor(0,0);
    lcd.print("TempSR: ");
    lcd.print((temperatureF2+temperatureF1+temperatureF1)/3);
    lcd.print("        ");
    lcd.setCursor(0, 1);
    lcd.print("WilgSR: ");
    lcd.print((humidityF2+humidityF1+humidityF1)/3);
    lcd.print("        ");
}

void  lcdPageUpTime(){
    lcd.setCursor(0,0);
    lcd.print("DHT Read Mode:");
    lcd.print(dhtReadMode);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print("Up Time: ");
    lcd.print(millis()/1000/60);
    lcd.print("m                   ");
}

void handleHumidityTemperature(){
  //unsigned long currentMillis = millis();
  //if((currentMillis - previousMillisDHT > 100) || i < 10 || j < 10){
  /////////////////////////// Sensor 1 /////////////////////
    if(i < 10){  
      int chk = DHT.read11(HUMIDITY_1_PIN);
      if(chk == DHTLIB_OK){
        humidityAvg+=(float)DHT.humidity;
        temperatureAvg+=(float)DHT.temperature;
        ++avgCnt;
      }
      ++i;
    }
    if((avgCnt > 0) && (i == 10)){
      humidityF1 = humidityAvg/avgCnt;
      temperatureF1 = temperatureAvg/avgCnt;
      dtostrf(humidityF1, 6, 3, (char*)humidity1);      
      dtostrf(temperatureF1, 6, 3, (char*)temperature1);     
      dtostrf(dewPointFast(temperatureF1, humidityF1), 6, 3, (char*)dewPointFastStr1);
    }
    if(i == 10){
      humidityAvg = 0;
      temperatureAvg = 0;
      avgCnt = 0;
      j = 0;
      i = 11;
    }

  /////////////////////////// Sensor 2 /////////////////////
    if(j < 10){  
      int chk = DHT.read11(HUMIDITY_2_PIN);
      if(chk == DHTLIB_OK){
        humidityAvg+=(float)DHT.humidity;
        temperatureAvg+=(float)DHT.temperature;
        ++avgCnt;
      }
      ++j;
    }
    if((avgCnt > 0) && (j == 10)){
      humidityF2 = humidityAvg/avgCnt;
      temperatureF2 = temperatureAvg/avgCnt;
      dtostrf(humidityF2, 6, 3, (char*)humidity2);      
      dtostrf(temperatureF2, 6, 3, (char*)temperature2);     
      dtostrf(dewPointFast(temperatureF2, humidityF2), 6, 3, (char*)dewPointFastStr2);
    }
    if(j == 10){
      humidityAvg = 0;
      temperatureAvg = 0;
      avgCnt = 0;
      j = 11;
      k = 0;
    }
    //previousMillisDHT = currentMillis;  
  /////////////////////////// Sensor 3 /////////////////////
    if(k < 10){  
      int chk = DHT.read22(HUMIDITY_3_PIN);
      if(chk == DHTLIB_OK){
        humidityAvg+=(float)DHT.humidity;
        temperatureAvg+=(float)DHT.temperature;
        ++avgCnt;
      }
      ++k;
    }
    if((avgCnt > 0) && (k == 10)){
      humidityF3 = humidityAvg/avgCnt;
      temperatureF3 = temperatureAvg/avgCnt;
      dtostrf(humidityF3, 6, 3, (char*)humidity3);      
      dtostrf(temperatureF3, 6, 3, (char*)temperature3);     
      dtostrf(dewPointFast(temperatureF3, humidityF3), 6, 3, (char*)dewPointFastStr3);
    }
    if(k == 10){
      humidityAvg = 0;
      temperatureAvg = 0;
      avgCnt = 0;
      k = 11;
      i = 0;
    }

}


// delta max = 0.6544 wrt dewPoint()
// 6.9 x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
	double a = 17.271;
	double b = 237.7;
	double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
	double Td = (b * temp) / (a - temp);
	return Td;
}


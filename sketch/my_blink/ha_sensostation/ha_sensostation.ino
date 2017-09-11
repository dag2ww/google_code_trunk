/*
* 18.08.2012
* Fixed nIRQ pin definition in RF12B library - def.h, if this doesn't help to serve the IRQ correctly, try the lib in original form adjusting hardware to required pin definitions rather than software to hardware layaut.
*/

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

// include the library code:
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <SPI.h>
#include <RF12B.h>
#include <rf_packet.h>

//indirectly from RF12B.h
// #define NIRQ_PIN 3
// #define nFFS_PIN 8
// #define CS_PIN 10
// from SPI.h
// MOSI: pin 11
// MISO: pin 12
// SCK: pin 13

#define PHOTO_PIN A1       // PhotoTransistor is connected to Arduino digital pin A1.
#define TEMPERATURE_PIN A3
#define LED1_PIN A0         
#define LED2_PIN 9         
#define TONE_PIN A2
#define MAINS_INDICATOR_PIN 7

#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

unsigned long previousMillisLed1;
unsigned long previousMillisLed2;
unsigned long previousMillisTemp;
unsigned long previousMillisTXRX;
boolean waitForConversion = false;
float temp;
byte lcdLine1[50] = "-----";
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by
// temperature read related:
byte ti;
byte tdata[12];
byte taddr[8];
// ledState used to set the LED
int ledState = LOW;
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long intervalLED1 = 1000;           // interval at which to blink (milliseconds)
long intervalLED2 = 1000;           // interval at which to blink (milliseconds)

//initialize one wire connection to Dallas temperature sensor
OneWire ds(TEMPERATURE_PIN);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(A5, A4, 5, 4, 6, 2);

void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(TONE_PIN, OUTPUT);     
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(PHOTO_PIN, INPUT);
  //debug
  //pinMode(7, OUTPUT);  
  //digitalWrite(7,LOW);
  pinMode(MAINS_INDICATOR_PIN, OUTPUT);  
  //initialize com port based communication/debug/log
  Serial.begin(57600);
  Serial.println("Infostacja wita! Procedura uruchamiania wykonana!");
  //initialize LCD and set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("E-Davinci");
  tone(TONE_PIN, 2000, 100);
  delay(100);
  tone(TONE_PIN, 4000, 200);
  delay(100);  
  tone(TONE_PIN, 2000, 100);
  delay(2000);
  lcd.clear();
  lcd.print("Infostacja v0.2");
  delay(2000);
  lcd.clear();
  //initialize RF12B transceiver
  RF12.begin();
}

void loop() {
  handleMainsSensor();
  
  handleLCDRefresh();

  getDS18B22Temperature();
  
  blinkTheLEDLoop();
  //fadeTheLED();  
  handleSendReceiveRF12BData();
}

void handleMainsSensor(){
    //check on/off 220v sensor
  if (digitalRead(PHOTO_PIN) ) 
  { //hi voltage, transistor not cunducting - infrared LED not on --> external devied OFF
    digitalWrite(MAINS_INDICATOR_PIN,LOW);       
  }else {
    digitalWrite(MAINS_INDICATOR_PIN,HIGH);
  }
}

void handleSendReceiveRF12BData(){
  byte buf[50];
  buf[0] = 'a';
  buf[1] = 'l';
  buf[2] = 'a';
  buf[3] = 'a';
  buf[4] = 'a';
  buf[5] = 'a';
  buf[6] = 'a';
  buf[7] = 'a';
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisTXRX > 1000) {
    // save the last time you blinked the LED 
    previousMillisTXRX = currentMillis;   
    
    // create and send the data packet
    RFPacket packet(buf, 7, 1, 1,2);
    RF12.send(packet, packet.size());
    blinkTheLED1(10);
  }
  
  // If a rf package is available print it
  if (RF12.packetAvailable()) {
    RFPacket p = RF12.recvPacket();
    p.dump();
    if(p.valid()){
      //beep
      tone(TONE_PIN, 1000, 100);
      blinkTheLED2(10);
      p.getData(lcdLine1, p.dataSize()-1);
      lcdLine1[p.dataSize()]='\0';
    }
  }
}

//void fadeTheLED(){
//  // set the brightness of pin:
//  analogWrite(FADE_PIN, brightness);    
//  // change the brightness for next time through the loop:
//  //ghtness = brightness + fadeAmount;
//  //everse the direction of the fading at the ends of the fade: 
//  if (brightness == 0 || brightness == 255) {
//    fadeAmount = -fadeAmount ; 
//  }     
//  // wait for 30 milliseconds to see the dimming effect    
//  delay(30);                            
//}

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
  lcd.setCursor(0,0);
  lcd.print("TW:");
  lcd.print((char*)lcdLine1);
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print("S:");
  lcd.print(millis()/1000);
  lcd.print("s");
  lcd.print(" T:");
  lcd.print(temp);
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

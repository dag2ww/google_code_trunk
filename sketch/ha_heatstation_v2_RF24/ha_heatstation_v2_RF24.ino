
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
#include <OneWire.h>
#include <SPI.h>

#include "RF24.h"
#include "printf.h"
#include "nRF24L01.h"

#define LED1_PIN A5
#define HEATER_PIN A0
#define TEMP_PACKET     3
#define MAINS_PACKET    4
#define MAINS_ORDER 5
#define TEMP_PACKET_POWER_BOX    10
#define POWER_ON_PACKET 11
#define REC_ALL_START 0
#define REC_SENSORSTATION 1
#define REC_POWER_COTROLLER 2
#define NO_MORE_ITEMS 255
#define REQUEST_TOKEN 254

#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

//to test if our receiver is working and getting sth
unsigned long lastAnyRadioResponse = 0;

int nextReceiver = REC_ALL_START;
int currentReceiver = REC_ALL_START;
boolean mainsRequest = false; 
boolean heatingRequest = false; 
//loops timeCounters
unsigned long previousMillisLed1;
unsigned long previousMillisTemp;
boolean receivedRequest = false;
byte msgType = 0;
long intervalLED1 = 1000;           // interval at which to blink (milliseconds)

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
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);  
  //initialize LCD and set up the LCD's number of columns and rows: 
  
  digitalWrite(LED1_PIN, HIGH);
  delay(500);
  digitalWrite(LED1_PIN, LOW);
  delay(500);
  digitalWrite(LED1_PIN, HIGH);
  delay(500);
  digitalWrite(LED1_PIN, LOW);

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
  
  //radio.openWritingPipe(SensorToInfoStationPipes[0]);
  radio.openReadingPipe(1,SensorToInfoStationPipes[1]);
  
  radio.startListening();
  //radio.printDetails();
  
  //Start the watchdog beast to be hungry after 2 sec
  wdt_reset(); 
  wdt_enable(WDTO_2S);

}

void loop() {
  handleMainsControll();
  
  handleSendReceiveRF24();
  
  //if no message from radio since 1 minute - allow reset
  if(millis() - lastAnyRadioResponse < 60000){
    wdt_reset();
  }
}

void handleMainsControll(){
    digitalWrite(HEATER_PIN, heatingRequest);
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
      if(msgType == MAINS_ORDER) {
        heatingRequest = data[2];
      }    
     }
  } //END RECEIVE 
  }

void blinkTheLEDLoop(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisLed1 > intervalLED1) {
    digitalWrite(LED1_PIN, LOW);
  }
}

void blinkTheLED1(int duration){
  intervalLED1 = duration;
  previousMillisLed1 = millis(); 
  digitalWrite(LED1_PIN, HIGH);  
}



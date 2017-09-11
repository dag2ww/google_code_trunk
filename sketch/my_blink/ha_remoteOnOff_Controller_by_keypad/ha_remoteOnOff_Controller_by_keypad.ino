
//The AVR hardware clears the global interrupt flag in SREG before entering an interrupt vector. Thus, normally interrupts will remain disabled inside the handler until the handler exits, where the RETI instruction (that is emitted by the compiler as part of the normal function epilogue for an interrupt handler) will eventually re-enable further interrupts. For that reason, interrupt handlers normally do not nest. For most interrupt handlers, this is the desired behaviour, for some it is even required in order to prevent infinitely recursive interrupts (like UART interrupts, or level-triggered external interrupts). In rare circumstances though it might be desired to re-enable the global interrupt flag as early as possible in the interrupt handler, in order to not defer any other interrupt more than absolutely needed. This could be done using an sei() instruction right at the beginning of the interrupt handler, but this still leaves few instructions inside the compiler-generated function prologue to run with global interrupts disabled.

/**

   Pins:
   Hardware SPI:
   MISO -> 12
   MOSI -> 11
   SCK -> 13

   Configurable:
   CE -> 10
   CSN -> 9

*/

// Design decisions:
// Leds:
// 1. Power LED - yellow, soft
// 2. Identify failure - red, bright - fast double blink (+ sound signal optionally)
// 3. Scanning - built in blue scannning led
// 4. Identify succeeded - green, bright - fast double blink and then gate open

#include <avr/wdt.h>
#include "Keypad.h"
// include the library code:
//#include <OneWire.h>
#include <SPI.h>

#include "RF24.h"
//#include "printf.h"
#include "nRF24L01.h"

#define ON_OFF_CONTEROLLER

//PINS SETUP
// from SPI.h
// MOSI: pin 11
// MISO: pin 12
// SCK: pin 13


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

char lastKey;
//to test if our receiver is working and getting sth
unsigned long lastAnyRadioResponse = 0;

unsigned long previousMillisLed1 = 0;
unsigned long previousMillisLed2 = 0;
unsigned long previousMillisTemp = 0;
unsigned long previousMillisTXRX = 0;
unsigned long lastMainsControllCircuitActiveTime_1 = 0;
unsigned long lastMainsControllCircuitActiveTime_2 = 0;
unsigned int ledsMode;
//bits are 1 in case given circuit is on
unsigned int circuitsState = 0;

//button handling
int lastButtonState;
unsigned long last_send_interval = 0;
unsigned long lastButtonDebounceTime;
int lastButtonStateReading;
unsigned long requestOnTriggerToken;
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

//finger identify variables
byte highbyte = 0;
byte lowbyte = 0;
word checksum = 0;
byte highcheck = 0;
byte lowcheck = 0;

const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {3, 2, 1, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 5, 4}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


//initialize one wire connection to Dallas temperature sensor
//OneWire ds(TEMPERATURE_PIN);

// Set up nRF24L01 radio on SPI bus plus pins ce = 10  & csn = 9
RF24 radio(10, 9);

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
  pinMode(MAINS_CIRCUIT_2_CTRL_PIN, OUTPUT);
  pinMode(LED_MODE_BUTTON_PIN, INPUT);

  //
  // Setup and configure rf radio
  //
  radio.begin();
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);
  //setDataRate()  RF24_2MBPS, RF24_250KBPS
  radio.setDataRate(RF24_250KBPS);
  // radio.setPALevel( RF24_PA_MAX );
  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);
  radio.setChannel(85);

  radio.openWritingPipe(InfoToPowerStationPipes[0]);
  radio.openReadingPipe(1, InfoToPowerStationPipes[1]);

  radio.startListening();
  //radio.printDetails();

  //Start the watchdog beast to be hungry after 2 sec
  //wdt_reset();
  //wdt_enable(WDTO_2S);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  delay(250);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED1_PIN, LOW);


}

void loop() {
  blinkTheLEDLoop();
  handleSendReceiveRF24();
  handleUserButton();
  //if no message from radio since 1 minute - allow reset
  //if(millis() - lastAnyRadioResponse < 60000){
  //  wdt_reset();
  //}
}


void powerOnCircuitLoop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisCircuit_1 > intervalCircuit_1) {
    //HIGH state disables the realy on Electrodragon module
    digitalWrite(MAINS_CIRCUIT_1_CTRL_PIN, HIGH);
    circuitsState = circuitsState & (~1);
  }
  if (currentMillis - previousMillisCircuit_2 > intervalCircuit_2) {
    digitalWrite(MAINS_CIRCUIT_2_CTRL_PIN, HIGH);
    circuitsState = circuitsState & (~2);
  }
}

void powerOnCircuit_1(int duration) {
  intervalCircuit_1 = duration;
  previousMillisCircuit_1 = millis();
  //LOW state enables the electrodragon relay module
  digitalWrite(MAINS_CIRCUIT_1_CTRL_PIN, LOW);
}

void powerOnCircuit_2(int duration) {
  intervalCircuit_2 = duration;
  previousMillisCircuit_2 = millis();
  digitalWrite(MAINS_CIRCUIT_2_CTRL_PIN, LOW);
}


void blinkTheLEDLoop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisLed1 > intervalLED1) {
    digitalWrite(LED1_PIN, LOW);
  }
  if (currentMillis - previousMillisLed2 > intervalLED2) {
    digitalWrite(LED2_PIN, LOW);
  }
}

void blinkTheLED1(int duration) {
  intervalLED1 = duration;
  previousMillisLed1 = millis();
  digitalWrite(LED1_PIN, HIGH);
}

void blinkTheLED2(int duration) {
  intervalLED2 = duration;
  previousMillisLed2 = millis();
  digitalWrite(LED2_PIN, HIGH);
}

//Get temperature from DS18B22
void getDS18B22Temperature() {
  return;
 
}

void handleSendReceiveRF24() {
  byte data[radio.getPayloadSize()];

  //RECEIVE
  // If a rf package is available - process it
  if (radio.available()) {
    if (ledsMode == LEDS_MODE_TXRX) {
      digitalWrite(LED1_PIN, HIGH);
    }
    boolean readingDone = false;

    while (!readingDone) {
      readingDone = radio.read( &data, radio.getPayloadSize() );
      msgType = data[0];
      if ( msgType == REQUEST_ON_TRIGGER_INPUT) {
        //requestOnTriggerToken = (unsigned long)(data[4] << 24) | (data[3] << 16) | (data[2] << 8) | data[1];
        requestOnTriggerToken = 0;
        requestOnTriggerToken = data[4];
        requestOnTriggerToken = (requestOnTriggerToken << 8) | data[3];
        requestOnTriggerToken = (requestOnTriggerToken << 8) | data[2];
        requestOnTriggerToken = (requestOnTriggerToken << 8) | data[1];
        receivedRequest = true;
      }
    }
    if (ledsMode == LEDS_MODE_TXRX) {
      digitalWrite(LED1_PIN, LOW);
    }
    lastAnyRadioResponse = millis();
  }

  //SEND

  if ( receivedRequest ) {
    if (msgType == REQUEST_ON_TRIGGER_INPUT) {
      msgType = ANSW_NONE;
      data[0] = REQUEST_ON_TRIGGER_FINAL;
      requestOnTriggerToken = (requestOnTriggerToken >> 3) + 15;
      data[1] = requestOnTriggerToken & 0xFF;
      data[2] = (requestOnTriggerToken >> 8) & 0xFF;
      data[3] = (requestOnTriggerToken >> 16) & 0xFF;
      data[4] = (requestOnTriggerToken >> 24) & 0xFF;
    } else {
      data[0] = REQUEST_ON_TRIGGER;
    }
    receivedRequest = false;
    // Delay just a little bit to let the other unit
    // make the transition to receiver
    delay(40);
    if (ledsMode == LEDS_MODE_TXRX) {
      digitalWrite(LED2_PIN, HIGH);
    }
    //do the transmission
    //printf("Now sending: %d...",data[0]);
    // First, stop listening so we can talk.
    radio.stopListening();

    bool ok = radio.write( data, radio.getPayloadSize() );
    // Now, continue listening
    radio.startListening();

    //if (ok)
    //  printf("ACK");
    //else
    //  printf("NO_ACK.\n\r");
    //if(ledsMode == LEDS_MODE_TXRX){
    //   digitalWrite(LED2_PIN, LOW);
    // }
    delay(10);
  }
}

void handleUserButton() {
  int reading = digitalRead(LED_MODE_BUTTON_PIN);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastButtonDebounceTime = millis();
  }
  if ((millis() - lastButtonDebounceTime) > 50) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if (reading == LOW && (reading != lastButtonStateReading)) {
      //if finger verification OK --> request ate to be opened
      receivedRequest = checkFinger();
    }
    lastButtonStateReading = reading;
  }
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}

boolean checkFinger() {
  boolean identificationResult = false;
  int retries = 0;
  Serial.begin(9600);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  delay(250);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED1_PIN, LOW);
  String verification = "";
  char key = customKeypad.getKey();

  while (key != '#') {
    key = customKeypad.getKey();
    if (key != lastKey) {
      if (key == '*') {
        verification = "";
      } else {
        //add to verification
        verification = verification+key;
      }
    }
    lastKey = key;
  }

  identificationResult = (verification.equals("1234"));
  if (identificationResult == false) {
    digitalWrite(LED1_PIN, HIGH);
    delay(300);
    digitalWrite(LED1_PIN, LOW);
    delay(100);
  } else {
    digitalWrite(LED2_PIN, HIGH);
    delay(300);
    digitalWrite(LED2_PIN, LOW);
    delay(100);
  }


return identificationResult;
}

boolean processIdentificationResponse() {
  boolean result = false;
  byte inByte = 0;
  //wait for response start byte
  while (waitAndReadSerialByte() != (byte)0x55);
  //0xAA
  waitAndReadSerialByte();
  //dev id
  waitAndReadSerialByte();
  waitAndReadSerialByte();
  // response param
  waitAndReadSerialByte();
  waitAndReadSerialByte();
  waitAndReadSerialByte();
  waitAndReadSerialByte();
  //response (ACK or NACK)
  inByte = waitAndReadSerialByte();
  if (inByte == (byte)0x30) {
    result = true;
  }
  inByte = waitAndReadSerialByte();
  if (inByte == (byte)0x30) {
    result = true;
  }
  // checksum
  waitAndReadSerialByte();
  waitAndReadSerialByte();
  return result;
}

void sendCommand(byte command1, byte param1) {
  valueToWORD(param1); //This value is the parameter being send to the device. 0 will turn the LED off, while 1 will turn it on.
  calcChecksum(command1, highbyte, lowbyte); //This function will calculate the checksum which tells the device that it received all the data
  Serial.write((byte)0x55); //Command start code 1
  Serial.write((byte)0xaa); //Command start code 2
  Serial.write((byte)0x01); // This is the first byte for the device ID. It is the word 0x0001
  Serial.write((byte)0x00); // Second byte of Device ID. Notice the larger byte is first. I'm assuming this is because the datasheet says "Multi-byte item is represented as Little Endian"
  Serial.write((byte)lowbyte); //writing the largest byte of the Parameter
  Serial.write((byte)highbyte); //Writing the second largest byte of the Parameter
  Serial.write((byte)0x00); //The datasheet says the parameter is a DWORD, but it never seems to go over the value of a word
  Serial.write((byte)0x00); //so I'm just sending it a word of data. These are the 2 remaining bytes of the Dword
  Serial.write((byte)command1); //write the command byte
  Serial.write((byte)0x00); //again, the commands don't go over a byte, but it is sent as a word, so I'm only sending a byte
  Serial.write((byte)lowcheck); //Writes the largest byte of the checksum
  Serial.write((byte)highcheck); //writes the smallest byte of the checksum
}

void flushSerialInput() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void consumeResponse() {
  //length of response
  byte i = 12;
  //wait for response
  while (Serial.available() <= 0);
  //read this into dev null
  while (i > 0) {
    //if there were data read, sdecrement todo counter
    if (Serial.read() != -1) {
      --i;
    }
  }
}

byte waitAndReadSerialByte() {
  //wait if the data is not there yet
  while (Serial.available() <= 0);
  //now read a byte and return it
  return lowByte(Serial.read());
}

void valueToWORD(int v) { //turns the word you put into it (the paramter in the code above) to two bytes
  highbyte = highByte(v); //the high byte is the first byte in the word
  lowbyte = lowByte(v); //the low byte is the last byte in the word (there are only 2 in a word)
}

void calcChecksum(byte c, byte h, byte l) {
  checksum = 256 + c + h + l; //adds up all the bytes sent
  highcheck = highByte(checksum); //then turns this checksum which is a word into 2 bytes
  lowcheck = lowByte(checksum);
}


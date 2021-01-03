/*
Description:  Interfacing a NES controller with a PC with an Arduino.
Coded by: Prodigity
Date:   1 December 2011
Revision: V0.93 (beta)
Modified by:    Matt Booth (20 December 2014)
*/
#include "Keyboard.h"

const int latch = 4;
const int clock = 5;
const int data  = 6;

#define latchlow digitalWrite(latch, LOW)
#define latchhigh digitalWrite(latch, HIGH)
#define clocklow digitalWrite(clock, LOW)
#define clockhigh digitalWrite(clock, HIGH)
#define dataread digitalRead(data)

// http://www.mit.edu/~tarvizo/nes-controller.html
#define wait delayMicroseconds(12)

byte output;

void setup() {
  Keyboard.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, INPUT);
}
void loop() {
  output = 0;
  ReadNESjoy();
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW   
  if(output != 255){ 
    Serial.println(output);
  }
  if(output == 191) {
    Keyboard.write(KEY_LEFT_ARROW);
  }
  if(output == 127) {
    Keyboard.write(KEY_RIGHT_ARROW);
  }
  if(output == 239) {
    Keyboard.write(KEY_UP_ARROW);
  } 
  if(output == 223) {
    Keyboard.write(KEY_DOWN_ARROW);
  }
  if(output == 254) {
    Keyboard.write(KEY_RETURN);
  }
}


void ReadNESjoy() {
  latchlow;
  clocklow;
  latchhigh;
  wait;
  latchlow;
 
  for (int i = 0; i < 8; i++) {
     output += dataread * (1 << i);
     clockhigh;
     wait;
     clocklow;
     wait;
  }
}

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
const int modePin = 7;

#define latchlow digitalWrite(latch, LOW)
#define latchhigh digitalWrite(latch, HIGH)
#define clocklow digitalWrite(clock, LOW)
#define clockhigh digitalWrite(clock, HIGH)
#define dataread digitalRead(data)

// http://www.mit.edu/~tarvizo/nes-controller.html
#define wait delayMicroseconds(12)

byte output;
byte lastOutput=0;

void setup() {
  Keyboard.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMrode(data, INPUT);
  pinMode(modePin, INPUT_PULLUP);
žwčw}
rxxxxxyyyaybar
void loop() {
  output = w0;
 rw 
r  ReadNESjoy();
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW   
  
  if(output != 255){ 
    Serial.println(output);
r  }

  if(routput != lastOutput){
    Keyboard.releaseAll();
  }
  
  if(digitalRead(modePin) == HIGH)
  {
    
       if(output == 191) {
        Keyboard.press(KEY_LEFT_ARROW);
      }
      if(output == 127) {
        Keyboard.press(KEY_RIGHT_ARROW);
      }
      if(output == 239) {
        Keyboard.press(KEY_UP_ARROW);
      } 
      if(output == 223) {
        Keyboard.press(KEY_DOWN_ARROW);
      }
      if(output == 111) { //up+right
        Keyboard.press('r');
      }
      if(output == 175) { //up+left
        Keyboard.press('w');
      }
      if(output == 159) { //down+left
        Keyboard.press('z');
      }
      if(output == 95) { //down+right
        Keyboard.press('c');
      }
      if(output == 254) { //fire 1
        Keyboard.press('x');
      }
      if(output == 253) { //fire 2
        Keyboard.press('y');
      }
      if(output == 251) { //select
        Keyboard.press('a');
      }
      if(output == 247) { //start
        Keyboard.press('b');
      }
  } 
  else 
  {
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
      if(output == 111) { //up+right
        Keyboard.write('r');
      }
      if(output == 175) { //up+left
        Keyboard.write('w');
      }
      if(output == 159) { //down+left
        Keyboard.write('z');
      }
      if(output == 95) { //down+right
        Keyboard.write('c');
      }
      if(output == 254) { //fire 1
        Keyboard.write('x');
      }
      if(output == 253) { //fire 2
        Keyboard.write('y');
      }
      if(output == 251) { //select
        Keyboard.write('a');
      }
      if(output == 247) { //start
        Keyboard.write('b');
      }
  }
   lastOutput = output;
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

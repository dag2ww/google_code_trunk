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

// http:www.mit.edu/~tarvizo/nes-controller.html
#define wait delayMicroseconds(12)

byte output;
byte lastOutput=0;

void setup() {
  Keyboard.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  pinMode(latch, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, INPUT);
  pinMode(modePin, INPUT_PULLUP);
}

void loop() {
  output = 0;
  
  ReadNESjoy();         

  if(output != 255){ 
    Serial.println(output);
  }

  if(output != lastOutput){
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
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press(KEY_UP_ARROW);
      }
      if(output == 175) { //up+left
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press(KEY_UP_ARROW);
      }
      if(output == 159) { //down+left
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press(KEY_DOWN_ARROW);
      }
      if(output == 95) { //down+right
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press(KEY_DOWN_ARROW);
      }
      
      if(output == 190) { //left + fire1
        Keyboard.press(KEY_LEFT_ARROW);
          Keyboard.press('a');
      }      if(output == 126) { //right + fire1
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press('a');
      }
      if(output == 238) { //up + fire1
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.press('a');
      } 
      if(output == 222) { //down + fire1
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.press('a');
      }
      if(output == 110) { //up+right + fire1
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.press('a');
      }
      if(output == 174) { //up+left + fire1
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.press('a');
      }
      if(output == 158) { //down+left + fire1
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.press('a');
      }
      if(output == 94) { //down+right + fire1
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.press('a');
      }

      if(output == 189) { //left + fire2
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press('b');
      }
      if(output == 125) { //right + fire2
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press('b');
      }
      if(output == 237) { //up + fire2
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.press('b');
      } 
      if(output == 221) { //down + fire2
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.press('b');
      }
      if(output == 109) { //up+right + fire2
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.press('b');
      }
      if(output == 173) { //up+left + fire2
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press(KEY_UP_ARROW);
        Keyboard.press('b');
      }
      if(output == 157) { //down+left + fire2
        Keyboard.press(KEY_LEFT_ARROW);
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.press('b');
      }
      if(output == 93) { //down+right + fire2
        Keyboard.press(KEY_RIGHT_ARROW);
        Keyboard.press(KEY_DOWN_ARROW);
        Keyboard.press('b');
      }

      if(output == 254) { //fire 1
        Keyboard.press('a');
      }
      if(output == 253) { //fire 2
        Keyboard.press('b');
      }
      if(output == 251) { //select
        Keyboard.press('s');
      }
      if(output == 247) { //start
        Keyboard.press('l');
      }
       if(output == 246) { //start + fire1
        Keyboard.press('x');
      }
      if(output == 250) { //select + fire1
        Keyboard.press('y');
      }
       if(output == 252) { //fire1 + fire2
        Keyboard.press('h');
        Keyboard.press('l');
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
        Keyboard.write('a');
      }
      if(output == 253) { //fire 2
        Keyboard.write('b');
      }
      if(output == 251) { //select
        Keyboard.write('s');
      }
      if(output == 247) { //start
        Keyboard.write('l');
      }
       if(output == 246) { //start + fire1
        Keyboard.write('x');
      }
      if(output == 250) { //select + fire1
        Keyboard.write('y');
      }
       if(output == 252) { //fire1 + fire2
        Keyboard.write('h');
      }
  }
   lastOutput = output;
   delay(5); 
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

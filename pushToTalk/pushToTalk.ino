/*
  Keyboard Message test

  For the Arduino Leonardo and Micro.

  Sends a text string when a button is pressed.

  The circuit:
  - pushbutton attached from pin 4 to +5V
  - 10 kilohm resistor attached from pin 4 to ground

  created 24 Oct 2011
  modified 27 Mar 2012
  by Tom Igoe
  modified 11 Nov 2013
  by Scott Fitzgerald

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/KeyboardMessage
*/

#include "Keyboard.h"

const int buttonPin = 3;          // input pin for pushbutton
const int buttonPin2 = 6;          // input pin for pushbutton
const int buttonPin3 = 5;   
const int powerPin = 14;

int previousButtonState = HIGH;   // for checking the state of a pushButton

int previousButtonState2 = HIGH;   // for checking the state of a pushButton

int previousButtonState3 = HIGH;   // for checking the state of a pushButton

void setup() {
  // make the pushButton pin an input:
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);
    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, HIGH);
  // initialize control over the keyboard:
  Keyboard.begin();
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  int buttonState2 = digitalRead(buttonPin2);
  int buttonState3 = digitalRead(buttonPin3);

  if (buttonState != previousButtonState){
    if (buttonState == LOW) {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('1');
    } else {
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release('1');
    }
    previousButtonState = buttonState;
  }
    if (buttonState2 != previousButtonState2){
    if (buttonState2 == LOW) {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('1');
    } else {
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release('1');
    }
    previousButtonState2 = buttonState2;
  }
    
    if (buttonState3 != previousButtonState3){
    if (buttonState3 == LOW) {
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('1');
    } else {
      Keyboard.release(KEY_LEFT_CTRL);
      Keyboard.release(KEY_LEFT_SHIFT);
      Keyboard.release('1');
    }
  previousButtonState3 = buttonState3;
}
}

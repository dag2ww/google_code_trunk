/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
#include <LowPower.h>
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 10;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);  
  pinMode(16, INPUT);
  delay(10000);   
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
  //delay(2500);               // wait for a second
  LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  if(digitalRead(16) == LOW){
  digitalWrite(led, HIGH);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
  digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
  LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  digitalWrite(led, HIGH);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
  digitalWrite(led, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(100);          
  digitalWrite(led, HIGH);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
  }
}

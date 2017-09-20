/*
  Calibration

 Demonstrates one technique for calibrating sensor input.  The
 sensor readings during the first five seconds of the sketch
 execution define the minimum and maximum of expected values
 attached to the sensor pin.

 The sensor minimum and maximum initial values may seem backwards.
 Initially, you set the minimum high and listen for anything
 lower, saving it as the new minimum. Likewise, you set the
 maximum low and listen for anything higher as the new maximum.

 The circuit:
 * Analog sensor (potentiometer will do) attached to analog input 0
 * LED attached from digital pin 9 to ground

 created 29 Oct 2008
 By David A Mellis
 modified 30 Aug 2011
 By Tom Igoe

 http://www.arduino.cc/en/Tutorial/Calibration

 This example code is in the public domain.

 */

// These constants won't change:
const int sensorPin = A0;    // pin that the sensor is attached to
const int ledPin = 9;        // pin that the LED is attached to

// variables:
int sensorValue = 0;         // the sensor value
int freqValue = 0;

void setup() {
  // turn on LED to signal the start of the calibration period:
  pinMode(13, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(13, LOW);
  freqValue = 23000;
  tone(10, freqValue);
}

void loop() {
  // read the sensor:
  //sensorValue = analogRead(sensorPin);

  // apply the calibration to the sensor reading
  //freqValue = 23000;//map(sensorValue, 0, 1023, 36000, 46000);
  // fade the LED using the calibrated value:
  //analogWrite(ledPin, sensorValue);
  //tone(10, freqValue);
  
}

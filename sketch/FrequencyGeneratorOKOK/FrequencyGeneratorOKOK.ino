/*

 Mimics the fade example but with an extra parameter for frequency. It should dim but with a flicker 
 because the frequency has been set low enough for the human eye to detect. This flicker is easiest to see when 
 the LED is moving with respect to the eye and when it is between about 20% - 60% brighness. The library 
 allows for a frequency range from 1Hz - 2MHz on 16 bit timers and 31Hz - 2 MHz on 8 bit timers. When 
 SetPinFrequency()/SetPinFrequencySafe() is called, a bool is returned which can be tested to verify the 
 frequency was actually changed.
 
 This example runs on mega and uno.
 */

#include <PWM.h>

const int PIEZO_PIN = 11;                // the pin that the piezo driver is attached to
int32_t lastFrequency = 17000; //frequency (in Hz)
unsigned long lastFreqChangeTime = 0;
int diff = 500; //freq change in Hz

void setup()
{
  //Serial.begin(9600);
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe(); 

  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(PIEZO_PIN, lastFrequency);
  
  //if the pin frequency was set successfully, turn pin 13 on
  if(success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);    
  }
  //use this functions instead of analogWrite on 'initialized' pins
  //apply more less about 20% duty cycle for whole frequencies
  pwmWrite(PIEZO_PIN, 40);//0...255
}

void loop()
{
  unsigned long currentTime = millis();
   if(currentTime - lastFreqChangeTime > 700 || currentTime - lastFreqChangeTime < 0){
    if(lastFrequency > 24000 || lastFrequency < 17000){
      diff = diff * -1;
    }
    lastFrequency = lastFrequency + diff;
    lastFreqChangeTime = currentTime;
   bool success = SetPinFrequencySafe(PIEZO_PIN, lastFrequency);
  }
}


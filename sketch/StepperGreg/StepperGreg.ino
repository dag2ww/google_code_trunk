/*
 * MotorKnob
 *
 * A stepper motor follows the turns of a potentiometer
 * (or other sensor) on analog input 0.
 *
 * http://www.arduino.cc/en/Reference/Stepper
 * This example code is in the public domain.
 */

#include <Stepper.h>

// change this to the number of steps on your motor
#define STEPS 100

#define POT_SPEED A1
#define POT_DIR A2
#define BTN_ON_OFF A3
int lastSpeedRead = 0;
int lastDirRead = 0;
int lastIsOnRead = 0;

int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 5, 6, 7, 8);


void setup()
{
  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(30);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(10, HIGH);
  digitalWrite(9, HIGH);

  pinMode(POT_SPEED, INPUT);
  pinMode(POT_DIR, INPUT);
  
  pinMode(BTN_ON_OFF, INPUT_PULLUP);
  digitalWrite(BTN_ON_OFF, HIGH);
  
  lastSpeedRead = analogRead(POT_SPEED);
  lastDirRead = analogRead(POT_DIR);
  
  
}

void loop()
{
  int speedRead = analogRead(POT_SPEED);
  int speed1 = map(speedRead, 0, 1023, -500, 1000);
  stepper.setSpeed(505+speed1);
  int dir = analogRead(POT_DIR) - lastDirRead >= -15 ? 1 : -1; 
  stepper.step(10*dir);

  handleButton();
  
}

void handleButton(){
   buttonState = digitalRead(BTN_ON_OFF);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == LOW) {
      //turn on and off the motor lines
      digitalWrite(10, !digitalRead(10));
      digitalWrite(9, !digitalRead(9));
    }
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState;

}


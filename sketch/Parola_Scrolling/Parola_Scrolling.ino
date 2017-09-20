// Use the Parola library to scroll text on the display
//
// Demonstrates the use of the scrolling function to display text received 
// from the Serial1 interface
//
// User can enter text on the Serial1 monitor and this will display as a
// scrolling message on the display.
// Speed for the display is controlled by a pot on SPEED_IN analog in.
// Scrolling direction is controlled by a switch on DIRECTION_SET digital in.
// Invert ON/OFF is set by a switch on INVERT_SET digital in.
//
// Keyswitch library can be found at http://arduinocode.codeplex.com
//

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#if USE_LIBRARY_SPI
#include <SPI.h>
#endif

// set to 1 if we are implementing the user interface pot, switch, etc
#define	USE_UI_CONTROL	0

#if USE_UI_CONTROL
#include <MD_KeySwitch.h>
#endif

// Turn on debug statements to the Serial1 output
#define  DEBUG  1

#if  DEBUG
#define	PRINT(s, x)	{ Serial1.print(F(s)); Serial1.print(x); }
#define	PRINTS(x)	Serial1.print(F(x))
#define	PRINTX(x)	Serial1.println(x, HEX)
#else
#define	PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif
 
//PRO-MICRO ISP:
#define	MAX_DEVICES	4
#define	CLK_PIN		15
#define	DATA_PIN	16
#define	CS_PIN		10

// HARDWARE SPI
MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


#define	PAUSE_TIME		1000
#define	SPEED_DEADBAND	5

// Scrolling parameters
#if USE_UI_CONTROL
#define	SPEED_IN		A5
#define	DIRECTION_SET	8	// change the effect
#define	INVERT_SET		9	// change the invert

#endif // USE_UI_CONTROL

uint8_t	frameDelay = 50;	// default frame delay value
textEffect_t	scrollEffect = SCROLL_LEFT;

// Global message buffers shared by Serial1 and Scrolling functions
#define	BUF_SIZE	255
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;

#if USE_UI_CONTROL

MD_KeySwitch uiDirection(DIRECTION_SET);
MD_KeySwitch uiInvert(INVERT_SET);

void doUI(void)
{
  // set the speed if it has changed
  {
    int16_t	speed = map(analogRead(SPEED_IN), 0, 1023, 10, 150);

    if ((speed >= ((int16_t)P.getSpeed() + SPEED_DEADBAND)) || 
      (speed <= ((int16_t)P.getSpeed() - SPEED_DEADBAND)))
    {
      P.setSpeed(speed);
      P.setPause(speed);
      frameDelay = speed;
      PRINT("\nChanged speed to ", P.getSpeed());
    }
  }

  if (uiDirection.read() == MD_KeySwitch::KS_PRESS)	// SCROLL DIRECTION
  {
    PRINTS("\nChanging scroll direction");
    scrollEffect = (scrollEffect == SCROLL_LEFT ? SCROLL_RIGHT : SCROLL_LEFT);
    P.setTextEffect(scrollEffect, scrollEffect);
    P.displayReset();
  }

  if (uiInvert.read() == MD_KeySwitch::KS_PRESS)	// INVERT MODE
  {
    PRINTS("\nChanging invert mode");
    P.setInvert(!P.getInvert());
  }
}
#endif // USE_UI_CONTROL

void readSerial1(void)
{
  static uint8_t	putIndex = 0;

  while (Serial1.available())
  {
    newMessage[putIndex] = (char)Serial1.read();
    if ((newMessage[putIndex] == '\n') || (putIndex >= BUF_SIZE-2))	// end of message character or full buffer
    {
      // put in a message separator and end the string
      newMessage[putIndex] = '\0';
      // restart the index for next filling spree and flag we have a message waiting
      putIndex = 0;
      newMessageAvailable = true;
    }
      else
      // Just save the next char in next location
      newMessage[putIndex++];
  }
}

void setup()
{
  Serial1.begin(57600);

#if USE_UI_CONTROL
  uiDirection.begin();
  uiInvert.begin();
  pinMode(SPEED_IN, INPUT);

  doUI();
#endif // USE_UI_CONTROL

  P.begin();
  P.displayClear();
  P.displaySuspend(false);

  P.displayScroll(curMessage, LEFT, scrollEffect, frameDelay);

  strcpy(curMessage, "Hello! Enter new message?");
  newMessage[0] = '\0';
}

void loop() 
{
#if USE_UI_CONTROL
	doUI();
#endif // USE_UI_CONTROL

  readSerial1();
  if (P.displayAnimate()) 
  {
    if (newMessageAvailable)
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    P.displayReset();
  }
}

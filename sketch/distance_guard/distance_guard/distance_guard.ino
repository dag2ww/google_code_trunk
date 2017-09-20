/*
 * Define the pins you want to use as trigger and echo.
 */

#define ECHOPIN 2        // Pin to receive echo pulse
#define TRIGPIN 3        // Pin to send trigger pulse
#define LED_GREEN 4
#define LED_YELLOW 5
#define LED_RED 6
#define BUZZER 7

boolean buzzerOn = false;
/*
 * setup function
 * Initialize the serial line (D0 & D1) at 115200.
 * Then set the pin defined to receive echo in INPUT 
 * and the pin to trigger to OUTPUT.
 */
 
void setup()
{
  Serial.begin(115200);
  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);  
}

/*
 * loop function.
 * 
 */
void loop()
{
  // Start Ranging
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  // Compute distance
  float distance = pulseIn(ECHOPIN, HIGH);
  distance= distance/58;
  if(distance < 15){
     digitalWrite( LED_GREEN, LOW);
     digitalWrite( LED_YELLOW, LOW);    
     digitalWrite( LED_RED, HIGH);
     buzzerOn = true;   
} else if(distance < 30){
     digitalWrite( LED_GREEN, LOW);
     digitalWrite( LED_YELLOW, HIGH);    
     digitalWrite( LED_RED, LOW);
     buzzerOn = false;    
  } else if(distance < 100){
     digitalWrite( LED_GREEN, HIGH);
     digitalWrite( LED_YELLOW, LOW);    
     digitalWrite( LED_RED, LOW);
     buzzerOn = false;    
  } else {
     digitalWrite( LED_GREEN, LOW);
     digitalWrite( LED_YELLOW, LOW);    
     digitalWrite( LED_RED, LOW);
     buzzerOn = false;     
  }
  if(buzzerOn){
    digitalWrite( BUZZER, HIGH);
  }
  delay(100);
  if(buzzerOn){
    digitalWrite( BUZZER, LOW);
  }
  delay(80);
  Serial.println(distance);
}


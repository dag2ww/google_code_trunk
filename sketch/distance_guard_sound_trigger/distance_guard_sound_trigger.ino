/*
 * Define the pins you want to use as trigger and echo.
 */

#define ECHOPIN 2        // Pin to receive echo pulse
#define TRIGPIN 3        // Pin to send trigger pulse
#define PLAY_SWITCH 4
#define LIGHT_SWITCH 5

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
  pinMode(PLAY_SWITCH, OUTPUT);
  pinMode(LIGHT_SWITCH, OUTPUT);
  digitalWrite( PLAY_SWITCH, HIGH);
  digitalWrite( LIGHT_SWITCH, HIGH);
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
  
  if(distance < 80){
     
     digitalWrite( PLAY_SWITCH, LOW);
     delay(200);
     digitalWrite( PLAY_SWITCH, HIGH);
     
     digitalWrite( LIGHT_SWITCH, LOW);
     delay(3000);
     digitalWrite( LIGHT_SWITCH, HIGH);
     
  } 
  delay(100);
  Serial.println(distance);
}

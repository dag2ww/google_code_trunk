

//https://lastminuteengineers.com/sim800l-gsm-module-arduino-tutorial/
//https://arduino.stackexchange.com/questions/29990/sim800-gsm-module-returns-0-on-atcops

#include <M5Stack.h>
#define RX_PIN      16
#define TX_PIN      17
#define RESET_PIN   5  
//Create software serial object to communicate with SIM800L

//https://lastminuteengineers.com/sim800l-gsm-module-arduino-tutorial/
//https://arduino.stackexchange.com/questions/29990/sim800-gsm-module-returns-0-on-atcops

void header(const char *string, uint16_t color){
    M5.Lcd.fillScreen(color);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(TFT_MAGENTA, TFT_BLUE);
    M5.Lcd.fillRect(0, 0, 320, 30, TFT_BLUE);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(string, 160, 3, 4);
}


void setup()
{
 M5.begin();

  header("SIM800L AT command", TFT_BLACK);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.drawString("Please use serial port to Test AT command.",0, 35, 2);
  // Host serial communication
  Serial.begin(115200);   

  // SIM800L serial communication
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);   
  pinMode(RESET_PIN, OUTPUT);

  Serial.println("Initializing..."); 
  delay(1000);

  Serial2.println("AT"); //Once the handshake test is successful, it will back to OK

  Serial.println("Batt status:"); 
 Serial2.println("AT+CBC"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
  
  updateSerial();
    Serial2.print (F("AT+CSCA=\"")); 
    // Information center number, shenzhen unicom   
    Serial2.print ("+48790998250");
    Serial2.print(F("\"\r"));
   updateSerial();

    Serial2.print (F("AT+CSCS=\"GSM\"\r"));
  
 updateSerial();


 
Serial.println("Signal quality:"); 
  Serial2.println("AT+CSQ");
   updateSerial();
  Serial.println("SIM ID (means inserted and init OK):"); 
  Serial2.println("AT+CCID");
   updateSerial();
  Serial.println("Check network registration:"); 
  Serial2.println("AT+CREG?");
 updateSerial();
  Serial.println("Switch to automatic reg:"); 
  Serial2.println("AT+CREG=1");
 updateSerial();


  Serial.println("report mode:"); 
  Serial2.println("AT+COPS=0,2");
 updateSerial();
 
  Serial.println("Network?:"); 
  Serial2.println("AT+COPS?");
 updateSerial();
  Serial.println("List Networks:"); 
  Serial2.println("AT+COPS=?");
   updateSerial();
  Serial2.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  Serial2.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();



}

void loop()
{
  updateSerial();
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    Serial2.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(Serial2.available()) 
  {
    Serial.write(Serial2.read());//Forward what Software Serial received to Serial Port
  }
}

/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>

const char* ssid     = "BTHub4-6TPJ";
const char* password = "649c46a487";

const char* host = "192.168.1.77";
const int httpPort = 8080;

const char* deviceId   = "0001";
const char* sw1Key = "sw1";
const char* sw1ON = "sw1ON";
const char* sw1OFF = "sw1OFF";

const char* sw2Key = "sw2";
const char* sw2ON = "sw2ON";
const char* sw2OFF = "sw2OFF";

const int ledRed = 0;
const int ledBlue = 4;
const int ledStat = 12;

String response = "";

void blinkStat(){
  digitalWrite(ledStat, HIGH);
  delay(100);
  digitalWrite(ledStat, LOW);
  delay(100);
}

void blinkLed(int led){
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);
  delay(100);
}

void setup() {
  pinMode(ledStat, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  
  Serial.begin(115200);
  blinkStat();
  blinkLed(ledRed);
  blinkLed(ledBlue);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.print("Booting deviceId: ");
  Serial.println(ESP.getChipId());
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  blinkStat();
  blinkStat();
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int value = 0;

void processResponse(String resp){
    Serial.print("Processing response: ");
    Serial.println(resp);
    if(resp.indexOf(sw1ON) > -1){
      digitalWrite(ledRed, HIGH);
    }
    if(resp.indexOf(sw1OFF) > -1){
      digitalWrite(ledRed, LOW);      
    }
    if(resp.indexOf(sw2ON) > -1){
      digitalWrite(ledBlue, HIGH);
    }
    if(resp.indexOf(sw2OFF) > -1){
      digitalWrite(ledBlue, LOW);      
    }
}

void loop() {
  delay(2000);
  ++value;
  blinkStat();
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/ctrl/";
  url += deviceId;
  url += "/";
  url += sw1Key;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);

  digitalWrite(ledStat, HIGH);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
           "Host: " + host + "\r\n\r\n");

  delay(100);
  
  response = "";
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
    response = response + line;    
  }
  processResponse(response);
 
  digitalWrite(ledStat, LOW);
  delay(100);
  digitalWrite(ledStat, HIGH);
  
  // We now create a URI for the request
  url = "/ctrl/";
  url += deviceId;
  url += "/";
  url += sw2Key;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(100);

  response = "";
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
    response = response + line;
  }
  processResponse(response);
  
  digitalWrite(ledStat, LOW);

  Serial.println();
  Serial.println("closing connection");
}


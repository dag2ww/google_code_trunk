#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
//for LED status
#include <Ticker.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <DHT.h>
#include <Wire.h>
//#include <Adafruit_BMP085.h>      //http://www.instructables.com/id/Adding-the-BMP180-to-the-ESP8266/
#include <SFE_BMP180.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#define ARDUINOJSON_ENABLE_PROGMEM 0
#define BLYNK_DEBUG           // Comment this out to disable debug and save space
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#define DHTPIN D3
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define BRZEG_ALTITUDE 147.0

Ticker ticker;
String CityID = "7532249"; //Brzeg
String APIKEY = "6a44311c1d307ba90a4a30fac1abce64";
float temperature = 0;
float humidity = 0;
float pressure = 0;
float windKmPerHour = 0;
float cloudyPercent = 0; 
int weatherID = 0;
unsigned long iterations = 2000;
char* weatherUrl ="api.openweathermap.org";
String weatherDescription ="";
String weatherLocation = "";
boolean bmpPresent = false;
int dhtReadErrorCount = 0;
char blynk_token[34] = "22c39700a6ef43ab9abd0a1eabfa50xx";
//flag for saving data
int ticksToRestart = 60;
bool shouldSaveConfig = false;


// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor for normal 16mhz Arduino
//DHT dht(DHTPIN, DHTTYPE);
// NOTE: For working with a faster chip, like an Arduino Due or Teensy, you
// might need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// Example to initialize DHT sensor for Arduino Due:
DHT dht(DHTPIN, DHTTYPE);

SFE_BMP180 bmp;

SimpleTimer timer;

BlynkTimer blynkTimer; // Create a Timer object called "timer"! 

void setup() {
  delay(3000);
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  Serial.println("Starting...");    //read configuration from FS json

  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);

    // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.1, tickLed);
  
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here

  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("ESPAutoConfig", "edavinci")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  
  ticker.detach();
  //turn LED off
  digitalWrite(BUILTIN_LED, HIGH);
  //read updated parameters

  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  dht.begin();

  //SDA, SCL
  Wire.pins(D1, D2);
  Wire.begin(D1, D2);
  Wire.setClock(400000);
  if (!bmp.begin()) {
    Serial.println("No BMP180 / BMP085 found");
  } else {
    bmpPresent = true;
  }
  
  Blynk.config(blynk_token);
  Blynk.connect();

  blynkTimer.setInterval(2500L, blynkPush);

}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void tickLed()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

void handleWeatherdisplay()
{
 if(iterations >= 30)//We check for updated weather forecast once every hour
 {
   getWeatherData();
   printWeatherIcon(weatherID);
   iterations = 0;   
 }

 sendTemperatureToNextion();
 
 sendHumidityToNextion();
 
 sendPressureToNextion();

 sendCloudsToNextion();

 sendWindToNextion();

 iterations++;
}

// This function will run every time Blynk connection is established
BLYNK_CONNECTED() {
    ticksToRestart = 60;
    // Request Blynk server to re-send latest values for all pins
    Blynk.syncAll();
}

void blynkPush()
{
  if(!Blynk.connected()){
    --ticksToRestart;
  }
  
  if(ticksToRestart <=0){
    ESP.restart();
  }
  
  float h = dht.readHumidity();
  //humidity = h;
  // Read temperature as Celsius
  float t = dht.readTemperature();
  //temperature = t;

  //t = ((int) (t * 10) / 10.0);
  //h = ((int) (h * 10) / 10.0);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    dhtReadErrorCount += 1;
    if(dhtReadErrorCount >= 10) {
      Blynk.virtualWrite(V1, -127);
      Blynk.virtualWrite(V2, -127);
      dhtReadErrorCount = 0;     
    }    
  } else {
    Blynk.virtualWrite(V1, t);
    Blynk.virtualWrite(V2, h); 
    double dp = dewPoint(t,h);
    Blynk.virtualWrite(V4, dp); 
    dhtReadErrorCount = 0;  
    Serial.println("Temp: "+String(t)+" Humidity: "+String(h)+".");
  }
  
  Wire.setClock(400000);
  Wire.begin(D1, D2);
  if (!bmp.begin()) {
    Serial.println("No BMP180 / BMP085 found");
  } else {
    bmpPresent = true;
  }

  if(bmpPresent) {
    readPressure();
  } 
    
  tickLed();
  handleWeatherdisplay();
}

void readPressure(){

    char status;
  double T,P,p0,a;

  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(BRZEG_ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(BRZEG_ALTITUDE*3.28084,0);
  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.


   status = bmp.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = bmp.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = bmp.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = bmp.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = bmp.sealevel(P,BRZEG_ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Blynk.virtualWrite(V3, p0);
          //pressure = p0;
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = bmp.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");

}

double dewPoint(double celsius, double humidity)
{
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

        // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558 - T);
}


void getWeatherData() //client function to send/receive GET request data.
{
  String result ="";
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(weatherUrl, httpPort)) {
    Serial.println("Failed to connect to weather server");
        return;
    }
      // We now create a URI for the request
    String url = "/data/2.5/forecast?id="+CityID+"&units=metric&cnt=1&APPID="+APIKEY;

       // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + weatherUrl + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server
    while(client.available()) {
        result = client.readStringUntil('\r');
    }

result.replace('[', ' ');
result.replace(']', ' ');

char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';

StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);
if (!root.success())
{
  Serial.println("parseObject() failed");
}

String location = root["city"]["name"];
String temp = root["list"]["main"]["temp"];
String weather = root["list"]["weather"]["main"];
String description = root["list"]["weather"]["description"];
String idString = root["list"]["weather"]["id"];
String timeS = root["list"]["dt_txt"];
String pressuree = root["list"]["main"]["pressure"];
String humid = root["list"]["main"]["humidity"];
String windMeterPerSec = root["list"]["wind"]["speed"];
String cloudsPercent = root["list"]["clouds"]["all"];

weatherID = idString.toInt();
temperature = temp.toFloat();
humidity = humid.toInt();
pressure = pressuree.toFloat();
windKmPerHour = (windMeterPerSec.toFloat() / 1000.0) * 60.0 * 60.0;
cloudyPercent = cloudsPercent.toFloat();
endNextionCommand(); //We need that in order the nextion to recognise the first command after the serial print

}

void showConnectingIcon()
{
  Serial.println();
  String command = "weatherIcon.pic=3";
  Serial.print(command);
  endNextionCommand();
}

void sendHumidityToNextion()
{
  endNextionCommand();
  String command = "humidity.txt=\""+String(humidity,1)+" %\"";
  Serial.print(command);
  endNextionCommand();
}

void sendTemperatureToNextion()
{
  endNextionCommand();
  String command = "temperature.txt=\""+String(temperature,1)+" st.C\"";
  Serial.print(command);
  endNextionCommand();
}

void sendPressureToNextion()
{
  endNextionCommand();
  String command = "pressure.txt=\""+String(pressure,1)+" hPa\"";
  Serial.print(command);
  endNextionCommand();
}

void sendCloudsToNextion()
{
  String command = "clouds.txt=\""+String(cloudyPercent,1)+" %\"";
  Serial.print(command);
  endNextionCommand();
}

void sendWindToNextion()
{
  String command = "wind.txt=\""+String(windKmPerHour,1)+" Km/h\"";
  Serial.print(command);
  endNextionCommand();
}

void endNextionCommand()
{
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void printWeatherIcon(int id)
{
 switch(id)
 {
  case 800: drawClearWeather(); break;
  case 801: drawFewClouds(); break;
  case 802: drawFewClouds(); break;
  case 803: drawCloud(); break;
  case 804: drawCloud(); break;
  
  case 200: drawThunderstorm(); break;
  case 201: drawThunderstorm(); break;
  case 202: drawThunderstorm(); break;
  case 210: drawThunderstorm(); break;
  case 211: drawThunderstorm(); break;
  case 212: drawThunderstorm(); break;
  case 221: drawThunderstorm(); break;
  case 230: drawThunderstorm(); break;
  case 231: drawThunderstorm(); break;
  case 232: drawThunderstorm(); break;

  case 300: drawLightRain(); break;
  case 301: drawLightRain(); break;
  case 302: drawLightRain(); break;
  case 310: drawLightRain(); break;
  case 311: drawLightRain(); break;
  case 312: drawLightRain(); break;
  case 313: drawLightRain(); break;
  case 314: drawLightRain(); break;
  case 321: drawLightRain(); break;

  case 500: drawLightRainWithSunOrMoon(); break;
  case 501: drawLightRainWithSunOrMoon(); break;
  case 502: drawLightRainWithSunOrMoon(); break;
  case 503: drawLightRainWithSunOrMoon(); break;
  case 504: drawLightRainWithSunOrMoon(); break;
  case 511: drawLightRain(); break;
  case 520: drawModerateRain(); break;
  case 521: drawModerateRain(); break;
  case 522: drawHeavyRain(); break;
  case 531: drawHeavyRain(); break;

  case 600: drawLightSnowfall(); break;
  case 601: drawModerateSnowfall(); break;
  case 602: drawHeavySnowfall(); break;
  case 611: drawLightSnowfall(); break;
  case 612: drawLightSnowfall(); break;
  case 615: drawLightSnowfall(); break;
  case 616: drawLightSnowfall(); break;
  case 620: drawLightSnowfall(); break;
  case 621: drawModerateSnowfall(); break;
  case 622: drawHeavySnowfall(); break;

  case 701: drawFog(); break;
  case 711: drawFog(); break;
  case 721: drawFog(); break;
  case 731: drawFog(); break;
  case 741: drawFog(); break;
  case 751: drawFog(); break;
  case 761: drawFog(); break;
  case 762: drawFog(); break;
  case 771: drawFog(); break;
  case 781: drawFog(); break;

  default:break; 
 }
}

void drawFog()
{
  String command = "weatherIcon.pic=13";
  Serial.print(command);
  endNextionCommand();
}

void drawHeavySnowfall()
{
  String command = "weatherIcon.pic=8";
  Serial.print(command);
  endNextionCommand();
}

void drawModerateSnowfall()
{
  String command = "weatherIcon.pic=8";
  Serial.print(command);
  endNextionCommand();
}

void drawLightSnowfall()
{
  String command = "weatherIcon.pic=11";
  Serial.print(command);
  endNextionCommand();
}

void drawHeavyRain()
{
  String command = "weatherIcon.pic=10";
  Serial.print(command);
  endNextionCommand();
}

void drawModerateRain()
{
  String command = "weatherIcon.pic=6";
  Serial.print(command);
  endNextionCommand();
}

void drawLightRain()
{
  String command = "weatherIcon.pic=6";
  Serial.print(command);
  endNextionCommand();
}

void drawLightRainWithSunOrMoon()
{
  String command = "weatherIcon.pic=7";
  Serial.print(command);
  endNextionCommand(); 
}
void drawThunderstorm()
{
  String command = "weatherIcon.pic=3";
  Serial.print(command);
  endNextionCommand();
}

void drawClearWeather()
{
  String command = "weatherIcon.pic=4";
  Serial.print(command);
  endNextionCommand();
}

void drawCloud()
{
  String command = "weatherIcon.pic=9";
  Serial.print(command);
  endNextionCommand();
}

void drawFewClouds()
{
  String command = "weatherIcon.pic=5";
  Serial.print(command);
  endNextionCommand(); 
}  
  void loop()
{
  Blynk.run(); // Initiates Blynk
  timer.run(); // Initiates SimpleTimer  
  blynkTimer.run();
}

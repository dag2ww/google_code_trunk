// M5Stack nixie tube clock : 2019.06.16 macsbug
// https://macsbug.wordpress.com/2019/06/16/m5stack-nixie-tube-clock/
// RTC DS3231 : https://wiki.52pi.com/index.php/Raspberry_Pi_Super_Capacitor_RTC(English)
// RTClib : https://www.arduinolibraries.info/libraries/rt-clib
#include <M5Stack.h>
#include "M5StackUpdater.h"
#include <WiFi.h>
#include <SPI.h> 
#include "RTClib.h"
RTC_DS3231 rtc;
#include "vfd_18x34.c"   // font 18px34
#include "vfd_35x67.c"   // font 35x67
#include "vfd_70x134.c"  // font 70px134
#include "apple_35x41.c" // icon 35px41 
const uint8_t rtc_sda = 5, rtc_sck = 13;
const char *ssid = "Elektro";
const char *pass = "2020Dancewicz2020";
uint32_t targetTime = 0; // for next 1 second timeout
const uint8_t*n[] = { // vfd font 18x34
  vfd_18x34_0,vfd_18x34_1,vfd_18x34_2,vfd_18x34_3,vfd_18x34_4,
  vfd_18x34_5,vfd_18x34_6,vfd_18x34_7,vfd_18x34_8,vfd_18x34_9 };
const uint8_t*m[] = { // vfd font 35x67
  vfd_35x67_0,vfd_35x67_1,vfd_35x67_2,vfd_35x67_3,vfd_35x67_4,
  vfd_35x67_5,vfd_35x67_6,vfd_35x67_7,vfd_35x67_8,vfd_35x67_9,
  vfd_35x67_q,vfd_35x67_n };
  const uint8_t*b[] = { // vfd font 70x134
  vfd_70x134_0,vfd_70x134_1,vfd_70x134_2,vfd_70x134_3,vfd_70x134_4,
  vfd_70x134_5,vfd_70x134_6,vfd_70x134_7,vfd_70x134_8,vfd_70x134_9,
  vfd_70x134_q,vfd_70x134_n };
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
uint16_t yy;
uint8_t mn, dd, hh, mm, ss;
uint8_t md = 2; // mode 1(yyyy_mmdd_hhmmss),2(mmdd_hh_mmss),3(mmdd_ss_hhmm)
  
void setup() { 
  M5.begin();
  Serial.begin(115200);
  Wire.begin(rtc_sda,rtc_sck);delay(10); //GLOVE A : SDA,SCL
  if(digitalRead(39)==0){updateFromFS(SD);ESP.restart();}
  rtc.begin(); 
  M5.Lcd.setBrightness(255);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillRect(0, 0, 319, 239, BLACK);
  M5.Lcd.fillRect(1, 1, 317, 236, BLACK);
  M5.Lcd.setTextSize(2);
  esp_timer_init();
  wifi_setup(); // wifi conection, NTP read, Save NTP to RTC
}
 
void loop() {
  M5.update();
  if (targetTime < esp_timer_get_time()/1000 ){
    targetTime = esp_timer_get_time()/1000 + 1000;
    DateTime now = rtc.now(); // time geting from RTC
    if (now.year() == 2165){  // rtc check
      ss++; if (ss == 60) { ss = 0; mm++;
           if (mm == 60) { mm = 0; hh++;
            if (hh == 24) { hh = 0; dd++; //
      }}}
    }else{
    yy = now.year(); mn = now.month(); dd = now.day();
    hh = now.hour(); mm = now.minute(); ss = now.second();
    Serial.printf("From RTC: %d %d %d %d %d %d\n",yy,mn,dd,hh,mm,ss);
    }
  }
  if(M5.BtnC.wasPressed()){            // mode change
   if (md == 3){md = 1;M5.Lcd.fillRect(1,1,317,236,BLACK);return;}
   if (md == 2){md = 3;M5.Lcd.fillRect(1,1,317,236,BLACK);return;}
   if (md == 1){md = 2;M5.Lcd.fillRect(1,1,317,236,BLACK);return;}
  }
  if ( md == 3 ){ hhmm();}             // yyyy,mm,dd,ss,hh,mm
  if ( md == 2 ){ yyyy_mmdd_hhmmss();} // yyyy,mm,dd,hh,mm,ss
  if ( md == 1 ){ mmss();}             // mm,ss
  periodic_ntp();                      // Set the time by periodic NTP
  delay(80);
}
 
void wifi_setup(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  M5.Lcd.setCursor(20,40);M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print("Connecting to wifi");
  WiFi.begin(ssid,pass);
  int wifi_check = 10; // WiFi count
  M5.Lcd.setCursor(20,80);M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.print("wifi check in progress");
  while (WiFi.status() != WL_CONNECTED){ // WiFI Status
    delay(500); wifi_check--; if (wifi_check < 0){break;}
  }
  if (wifi_check > 0){  // when there is wifi
    M5.Lcd.setCursor(20,120);M5.Lcd.setTextColor(GREEN);
    M5.Lcd.print("wifi connection complete");delay(1000);
    struct tm t;
    configTime(2*3600L,0,"ntp.nict.jp",
      "time.google.com","ntp.jst.mfeed.ad.jp");
    getLocalTime(&t);
    yy = t.tm_year + 1900; mn = t.tm_mon + 1; dd = t.tm_mday;
    hh = t.tm_hour; mm = t.tm_min; ss = t.tm_sec;
    M5.Lcd.setCursor(20,160);M5.Lcd.setTextColor(GREEN);
    M5.Lcd.print("SET UP NTP");delay(1000); 

    rtc.adjust(DateTime(yy,mn,dd,hh,mm,ss));
    DateTime now = rtc.now(); // time geting from RTC
    if (now.year() == 2165){  //
        M5.Lcd.setCursor(20,200);M5.Lcd.setTextColor(RED);
        M5.Lcd.print("RTC FAIL");delay(1000);
        M5.Lcd.fillRect(1, 1, 317, 236, BLACK);
    } else {
    //Serial.printf("%d %d %d %d %d %d\n",yy,mn,dd,hh,mm,ss);
    M5.Lcd.setCursor(20,200);M5.Lcd.setTextColor(BLUE);
    M5.Lcd.print("SET UP RTC");delay(2000);
    M5.Lcd.fillRect(1, 1, 317, 236, BLACK);
    }
  }
  if (wifi_check < 0){   // When there is no wifi
    M5.Lcd.setCursor(20,120);M5.Lcd.setTextColor(ORANGE);
    M5.Lcd.print("without wifi connection");delay(1000);
  }
}
 
void yyyy_mmdd_hhmmss(){
  int y1 = (yy / 1000) % 10; int y2 = (yy / 100) % 10;
  int y3 = (yy /   10) % 10; int y4 = yy % 10;
  int ma = (mn /   10) % 10; int mb = mn % 10;
  int d1 = (dd /   10) % 10; int d2 = dd % 10;
  int h1 = (hh /   10) % 10; int h2 = hh % 10;
  int m1 = (mm /   10) % 10; int m2 = mm % 10;
  int s1 = (ss /   10) % 10; int s2 = ss % 10;
 
  //int p0 = 8; int x0 = 40; int t0 = 22; // icon
  //M5.Lcd.pushImage( p0 + 0*x0, t0, 35,41, (uint16_t *)apple_35x41);
 
  int p1 = 80; int px1 = 40; int py1 = 5;
  M5.Lcd.pushImage( p1 + 0*px1, py1, 35,67, (uint16_t *)m[y1]); 
  M5.Lcd.pushImage( p1 + 1*px1, py1, 35,67, (uint16_t *)m[y2]);
  M5.Lcd.pushImage( p1 + 2*px1, py1, 35,67, (uint16_t *)m[y3]);
  M5.Lcd.pushImage( p1 + 3*px1, py1, 35,67, (uint16_t *)m[y4]);
   
  int p2 = 80; int px2 = 40; int py2 = 76;
  M5.Lcd.pushImage( p2 + 0*px2, py2, 35,67, (uint16_t *)m[ma]);
  M5.Lcd.pushImage( p2 + 1*px2, py2, 35,67, (uint16_t *)m[mb]);
  //M5.Lcd.drawPixel(118,13, ORANGE); M5.Lcd.drawPixel(119,23,ORANGE);
  M5.Lcd.pushImage( p2 + 2*px2, py2, 35,67, (uint16_t *)m[d1]);
  M5.Lcd.pushImage( p2 + 3*px2, py2, 35,67, (uint16_t *)m[d2]);
 
  int p3 = 2; int px3 = 40; int py3 = 150;
  M5.Lcd.pushImage( p3 + 0*px3, py3, 35,67, (uint16_t *)m[h1]);
  M5.Lcd.pushImage( p3 + 1*px3, py3, 35,67, (uint16_t *)m[h2]);
  M5.Lcd.pushImage( p3 + 2*px3, py3, 35,67, (uint16_t *)m[10]);
  M5.Lcd.pushImage( p3 + 3*px3, py3, 35,67, (uint16_t *)m[m1]);
  M5.Lcd.pushImage( p3 + 4*px3, py3, 35,67, (uint16_t *)m[m2]);
  M5.Lcd.pushImage( p3 + 5*px3, py3, 35,67, (uint16_t *)m[10]);
  M5.Lcd.pushImage( p3 + 6*px3, py3, 35,67, (uint16_t *)m[s1]);
  M5.Lcd.pushImage( p3 + 7*px3, py3, 35,67, (uint16_t *)m[s2]);
   
  if ( s1 == 0 && s2 == 0 ){ fade1();}
}
 
void mmss(){
  int ma = (mn / 10) % 10; int mb = mn % 10;
  int d1 = (dd / 10) % 10; int d2 = dd % 10;
  int h1 = (hh / 10) % 10; int h2 = hh % 10;
  int m1 = (mm / 10) % 10; int m2 = mm % 10;
  int s1 = (ss / 10) % 10; int s2 = ss % 10;
 
  int p0 = 8; int x0 = 40; int t0 = 22; // icon
  M5.Lcd.pushImage( p0 + 0*x0, t0, 35,41, (uint16_t *)apple_35x41);
 
  int p2 = 65; int px2 = 40; int py2 = 10;
  M5.Lcd.pushImage( p2 + 0*px2, py2, 35,67, (uint16_t *)m[ma]);
  M5.Lcd.pushImage( p2 + 1*px2, py2, 35,67, (uint16_t *)m[mb]);
  //M5.Lcd.drawPixel(118,13, ORANGE); M5.Lcd.drawPixel(119,23,ORANGE);
  M5.Lcd.pushImage( p2 + 2*px2, py2, 35,67, (uint16_t *)m[d1]);
  M5.Lcd.pushImage( p2 + 3*px2, py2, 35,67, (uint16_t *)m[d2]);
   
  int p3 = 240; int px3 = 40; int py3 = 10;
  M5.Lcd.pushImage( p3 + 0*px3, py3, 35,67, (uint16_t *)m[h1]);
  M5.Lcd.pushImage( p3 + 1*px3, py3, 35,67, (uint16_t *)m[h2]);
   
  int p4 = 2; int px4 = 80; int py4 = 100;
  M5.Lcd.pushImage( p4 + 0*px4   , py4, 70,134, (uint16_t *)b[m1]);
  M5.Lcd.pushImage( p4 + 1*px4 -4, py4, 70,134, (uint16_t *)b[m2]);
  //M5.Lcd.drawPixel(155,150, ORANGE); M5.Lcd.drawPixel(155,190,ORANGE);
  M5.Lcd.fillCircle(156,151,3,ORANGE);M5.Lcd.fillCircle(156,191,3,ORANGE);
  M5.Lcd.fillCircle(156,151,1,YELLOW);M5.Lcd.fillCircle(156,191,1,YELLOW);
  M5.Lcd.pushImage( p4 + 2*px4 +4, py4, 70,134, (uint16_t *)b[s1]);
  M5.Lcd.pushImage( p4 + 3*px4   , py4, 70,134, (uint16_t *)b[s2]);
 
  if ( m1 == 0 && m2 == 0 ){ fade2();}
}
 
void hhmm(){
  int ma = (mn / 10) % 10; int mb = mn % 10;
  int d1 = (dd / 10) % 10; int d2 = dd % 10;
  int h1 = (hh / 10) % 10; int h2 = hh % 10;
  int m1 = (mm / 10) % 10; int m2 = mm % 10;
  int s1 = (ss / 10) % 10; int s2 = ss % 10;
 
  //int p0 = 8; int x0 = 40; int t0 = 22; // icon
  //M5.Lcd.pushImage( p0 + 0*x0, t0, 35,41, (uint16_t *)apple_35x41);
 
  int p2 = 65; int px2 = 40; int py2 = 10;
  M5.Lcd.pushImage( p2 + 0*px2, py2, 35,67, (uint16_t *)m[ma]);
  M5.Lcd.pushImage( p2 + 1*px2, py2, 35,67, (uint16_t *)m[mb]);
  //M5.Lcd.drawPixel(118,13, ORANGE); M5.Lcd.drawPixel(119,23,ORANGE);
  M5.Lcd.pushImage( p2 + 2*px2, py2, 35,67, (uint16_t *)m[d1]);
  M5.Lcd.pushImage( p2 + 3*px2, py2, 35,67, (uint16_t *)m[d2]);
   
  int p3 = 240; int px3 = 40; int py3 = 10;
  M5.Lcd.pushImage( p3 + 0*px3, py3, 35,67, (uint16_t *)m[s1]);
  M5.Lcd.pushImage( p3 + 1*px3, py3, 35,67, (uint16_t *)m[s2]);
   
  int p4 = 2; int px4 = 80; int py4 = 100;
  M5.Lcd.pushImage( p4 + 0*px4   , py4, 70,134, (uint16_t *)b[h1]);
  M5.Lcd.pushImage( p4 + 1*px4 -4, py4, 70,134, (uint16_t *)b[h2]);
  //M5.Lcd.drawPixel( 155,150, ORANGE); M5.Lcd.drawPixel(155,190,ORANGE);
  M5.Lcd.fillCircle(156,151,3,ORANGE);M5.Lcd.fillCircle(156,191,3,ORANGE);
  M5.Lcd.fillCircle(156,151,1,YELLOW);M5.Lcd.fillCircle(156,191,1,YELLOW);
  M5.Lcd.pushImage( p4 + 2*px4 +4, py4, 70,134, (uint16_t *)b[m1]);
  M5.Lcd.pushImage( p4 + 3*px4   , py4, 70,134, (uint16_t *)b[m2]);
 
  if ( h1 == 0 && h2 == 0 ){ fade2();}
}
 
void fade1(){
  int p3 = 2; int px3 = 40; int py3 = 150;
  for ( int i = 0; i < 2; i++ ){
  M5.Lcd.pushImage( p3 + 2*px3, py3, 35,67, (uint16_t *)m[11]);
  M5.Lcd.pushImage( p3 + 5*px3, py3, 35,67, (uint16_t *)m[11]);
  delay(25);
  M5.Lcd.pushImage( p3 + 2*px3, py3, 35,67, (uint16_t *)m[10]);
  M5.Lcd.pushImage( p3 + 5*px3, py3, 35,67, (uint16_t *)m[10]);
  delay(25);
  }
}
 
void fade2(){
  int p3 = 2; int px3 = 40; int py3 = 150;
  for ( int i = 0; i < 2; i++ ){
  M5.Lcd.fillCircle(156,151,3,BLACK);M5.Lcd.fillCircle(156,191,3,BLACK);
  delay(25);
  M5.Lcd.fillCircle(156,151,3,ORANGE);M5.Lcd.fillCircle(156,191,3,ORANGE);
  delay(25);
  }
}
 
void periodic_ntp(){ // Adjust to ntp at hh mm ss
  if ( (hh == 1) & (mm == 0) & (ss == 0)){ // AM 1:00:00
    wifi_setup();
  }
}

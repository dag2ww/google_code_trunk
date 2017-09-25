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

const int PIEZO_PIN = 9;                // the pin that the piezo driver is attached to
//koty: 45 Hz do 64 kHz,
//Psy słyszą dźwięki o znacznie wyższej częstotliwości  47000-65000 herców, co jest cechą osobniczą każdego psa.
/*
 * http://gdynia.pl/pies/poradnik,7147/co-slysza-psy,481789
Badając zdolność do odbierania sygnałów dźwiękowych należy wziąć pod uwagę natężenie, czy intensywność dźwięku oraz jego częstotliwość. Fale dźwiękowe o wysokiej częstotliwości słyszalne są jako dźwięki wysokie, natomiast o niskiej częstotliwości jako dźwięki niskie.
Ostrość słuchu psa w zakresie niskich częstotliwości   jest zbliżona do ludzkiej. Największe różnice w słyszeniu pomiędzy psem i człowiekiem występują na wysokich częstotliwościach. Człowiek może słyszeć dźwięki do  20000 herców . Chcąc skonstruować fortepian wydający dźwięki o tak wysokiej częstotliwości musielibyśmy dodać do instrumentu około 28 klawiszy (3,4 oktawy) dla prawej ręki, a przeciętny człowiek i tak nie byłby w stanie usłyszeć tak wysokich dźwięków. ( poza tym słuch z wiekiem ulega u człowieka uszkodzeniu i w pierwszej kolejności traci on wrażliwość właśnie na wysokie dźwięki). Psy słyszą dźwięki o znacznie wyższej częstotliwości  47000-65000 herców, co jest cechą osobniczą każdego psa. Odwołując się do porównania z klawiaturą fortepianu musielibyśmy dodać jeszcze 48 klawiszy dla prawej ręki. Dźwięki o częstotliwościach w przedziale od 65-2000 herców są mniej więcej tak samo słyszalne dla psów i dla ludzi.
Psy nie potrafią, jak nietoperze, czy delfiny emitować ultradźwięków natomiast słyszą je. Zdolność do słyszenia określonego zakresu dźwięków jest uzasadniona ewolucyjnie . Słuch drapieżników wrażliwy jest na dźwięki, od których zależy ich przetrwanie. Gatunki z rodziny psowatych (wilki, szakale, kojoty, lisy), do której zalicza się również pies domowy, polują na myszy, szczury, małe gryzonie. Dzięki czułym detektorom w postaci uszu słyszą one przenikliwe piski małych zwierząt ukrytych w norkach.  Koty, których przetrwanie w znacznie większym stopniu zależy od polowania na małe gryzonie słyszą dźwięki o częstotliwości 5000-10000 wyższe od tych, które odbierają psy. W rodzinie canidae za najlepiej słyszące uważa się kojoty. Zakres ich słuchu dochodzi do 80000 herców. Człowiek najbardziej wrażliwy jest na zakres dźwięków, który ma decydujące znaczenie w słyszeniu ludzkiej mowy (200-3000 herców). Wrażliwość psich uszu  sprawia, że  wysokie dźwięki wydawane przez wiele urządzeń codziennego domowego użytku (kosiarki, odkurzacze, urządzenia kuchenne wyposażone w wysokoobrotowe silniki),   mogą być  dla nich nie tylko nieprzyjemnym doświadczeniem, ale również sprawiać ból. Dla człowieka są one do zniesienia, ponieważ większości tych dźwięków zwyczajnie nie słyszy.   Pies, podobnie jak człowiek, dysponuje zakresem słuchu wynoszącym osiem i pół oktawy, potrafi rozróżnić dźwięki różniące się o jedną ósmą tonu. Wszystko to sprawia, że aparat słuchowy psa pozwala mu na odróżnianie bardzo złożonych bodźców dźwiękowych np. ludzkiej mowy. Pies potrafi rozróżniać, zapamiętać i kojarzyć konkretne słowa z zachowaniami. Rozróżnienie i zapamiętanie odgłosu silnika samochodu  swojego właściciela czy jego kroków również nie stanowi dla psa problemu.
 */
int32_t lastFrequency = 20000; //frequency (in Hz)
unsigned long lastFreqChangeTime = 0;
int diff = 500; //freq change in Hz

void setup()
{
  pinMode(PIEZO_PIN, OUTPUT);
  digitalWrite(PIEZO_PIN, LOW);
  Serial.begin(9600);
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe(); 

  //sets the frequency for the specified pin
  bool success = SetPinFrequency(PIEZO_PIN, lastFrequency);
  
  //if the pin frequency was set successfully, turn pin 13 on
  if(success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);    
  }

  Serial.println("Sukces?");
  Serial.println(success);
  //use this functions instead of analogWrite on 'initialized' pins
  //apply more less about 20% duty cycle for whole frequencies
  pwmWrite(PIEZO_PIN, 36);//0...255
}

void loop()
{
  unsigned long currentTime = millis();
   if(currentTime - lastFreqChangeTime > 700 || currentTime - lastFreqChangeTime < 0){
    if(lastFrequency > 37000 || lastFrequency < 20000){
      diff = diff * -1;
    }
    lastFrequency = lastFrequency + diff;
    lastFreqChangeTime = currentTime;
    Serial.print("About to change freq:");
    Serial.print(lastFrequency);
    
   bool success = SetPinFrequencySafe(PIEZO_PIN, lastFrequency);
   Serial.println(success);
  }
}


/*----------------------------------------- 
 * ino fajl je rađen za potrebe članka u Svetu Kompjutera.
 * Author: Petrović Dejan
 * Date  : 06 mart 2017
 * Arduino Mega, ekran Nokije 5110 i gomila drugih senzora i modula...
 */
 /*------------------------------
 * Atmosferski nivo CO2..............400ppm
 * Prosecna vrednost CO2 u sobi......350-450ppm
 * Maksimalna prihvatljiva vrednost..1000ppm
 * Opasan nivo CO2..................>2000ppm
 */
#include <DHT.h>                              //DHT biblioteka
#include <OneWire.h>                          //OneWire biblioteka
#include <DallasTemperature.h>                //Dallas biblioteka
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#define mq135Pin A0                           //pin mq135
#define mq2Pin   A1                           //pin mq2
#define rainPin  A2                           //pin rain
#define ntcPin   A3                           //NTC pin
#define DHTPin    2                           //DHT22 pin
#define DallasPin 3                           //Dallas pin
#define UVOutPin A4                           //uv pin
#define Reff3V3Pin A5                         //ref pin
///dugme
int buttonPin = 40;                           //button pin
int relayPin  = 45;                           //relej pin
int state = HIGH;                             //trenutno stanje dugmeta
int reading;                                  //citanje trenutnog stanja dugmeta
int previous = LOW;                           //prethodno citanje stanja
long time = 0;                                //vreme poslednjeg stanja dugmeta
long debounce = 200;                          //vreme za eliminaciju plivajuceg inputa

#define DHTTYPE DHT22
#define TEMPERATURE_PRECISON 9                //Niza rezolucija
#define co2Zero 40                            //kalibracija mq135
int mq2Const=0;
int rainConst=0;
String rainSensor;
int Vout;
int UVindex=0;
float R1 = 10000;                             //otpornost na sobnoj temperaturi 10K ili 10000 oma
float R2, Tk, Tc;
float Ac = 1.009249522e-03, Bc = 2.378405444e-04, Cc = 2.019202697e-07;
DHT dht(DHTPin, DHTTYPE);                     //dht objekat
OneWire oneWire(DallasPin);                   //onewire objekat
DallasTemperature sensors(&oneWire);          //slanje reference dallas biblioteci
Adafruit_BMP280 bmp;                          //bmp objekat
Adafruit_PCD8544 display = Adafruit_PCD8544(8,7,6,5,4);
///// podesavanje vremena prikaza uz millis() funkciju
int period = 27000;
unsigned long vreme1 = 0;
unsigned long vreme2 = 3000;
unsigned long vreme3 = 6000;
unsigned long vreme4 = 9000;
unsigned long vreme5 = 12000;
unsigned long vreme6 = 15000;
unsigned long vreme7 = 18000;
unsigned long vreme8 = 21000;
unsigned long vreme9 = 24000;

void setup(){
  Serial.begin(9600);
  if(!bmp.begin()){                 //provera BMP280 musica
    Serial.println("BMP280 nije nadjen");
    while(1);
    }
  dht.begin();                      //pokrecemo DHT22 objekat
  sensors.begin();                  //isto i sensor objekat
  display.begin();                  //nokijin ekran takodje
  display.setContrast(50);          //kontrast u nasem slucaju
  display.setTextColor(BLACK);
  pinMode(DHTPin, INPUT);           //dodeljivanje moda pinovima
  pinMode(DallasPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT);
  }
void loop(){

sensorReadings();
buttonSwitch();
}

void sensorReadings(){
 /////mq135 sekcija
int co2now[10];                               //int broj uzoraka za prosek
int co2raw = 0;                               //int za grubo ocitavanje co2
int co2comp = 0;                              //int za kompenzovanu vrednost co2 
int co2ppm = 0;                               //int za srednju vrednost ppm
int prosek = 0;                                  //int za prosek
////
for (int x = 0;x<10;x++){                     //samplpe co2 10x over 2 seconds
    co2now[x]=analogRead(mq135Pin);
    delay(200);
  }
for (int x = 0;x<10;x++){                     //sastavljamo uzorke
    prosek=prosek + co2now[x];    
  }
  co2raw = prosek/10;                         //delimo uzorke sa 10
  co2comp = co2raw - co2Zero;                 //dobijamo kompenzovanu vrednost
  co2ppm = map(co2comp,0,1023,400,5000);      //mapa za srednju vrednost 
  //Serial.print("Koncentracija CO2 = ");
  //Serial.print(co2ppm);
  //Serial.println(" ppm");

  //citanje mq2
  mq2Const=analogRead(mq2Pin);                //slicno kao i za mq135
  mq2Const=map(mq2Const,0,1023,0,255);        //samo bez kalkulacija
  //Serial.print("Prirodni gasovi: "); 
  //Serial.println(mq2Const);
  //citanje rain
  rainConst=analogRead(rainPin);              //ocitavanje kisnog senzora
  if(rainConst<300){rainSensor="Lije kao iz kabla";}
  else if(rainConst>300&&rainConst<=500){rainSensor="Ovo je \nosrednja kisa";}
  else if(rainConst>500&&rainConst<=700){rainSensor="Tek po neka kap";}
  else if(rainConst>700){rainSensor="Suvo ko barut";}
   //Serial.print("Rain sensor ");
   //Serial.print(rainConst);
   //Serial.println(rainSensor);
  //citanje ntc
  Vout=analogRead(ntcPin);                    //o ntc je bilo reci
  R2 = R1 * (1023.0 / (float)Vout - 1.0);     // konvertovanje iz analogne u digitalnu
  Tk = (1.0 / (Ac + Bc*log(R2) + Cc*log(R2)*log(R2)*log(R2))); // temperatura u Kelvinima (K)
  Tc = Tk - 273.15;                           //temperatura u stepenima Celzijusa
   //Serial.print("Temp NTC ");
   //Serial.println(Tc);
 //citanje DHT22
 float temp=dht.readTemperature();            //dht temperatura
 float vlaga=dht.readHumidity();              //dht relativna vlaznost vazduha
 //Serial.print("DHT22 temperatura ");
 //Serial.print(temp);
 //Serial.print(" a vlaznost je ");
 //Serial.println(vlaga);
 //citanje dalasa
 sensors.requestTemperatures();              //zahtev sensor objekta za Dallas
 //Serial.print("Dalas temperatura je ");
 //Serial.println(sensors.getTempCByIndex(0));
 //citanje UV //////////////////////////
 int uvLevel = averageAnalogRead(UVOutPin);    //ocitavanje UV pina
 int refLevel = averageAnalogRead(Reff3V3Pin); //ocitavanje 3v3 radi reference
 float outputVoltage = 3.3/refLevel*uvLevel;   //UV senzor radi na 3.3V
 float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);//dobijanje UV intenziteta
 if(uvLevel<0){UVindex=0;}
 else if(uvLevel>50&&uvLevel<=227){UVindex=0;}
 else if(uvLevel>227&&uvLevel<=318){UVindex=1;}
 else if(uvLevel>318&&uvLevel<=408){UVindex=2;}
 else if(uvLevel>408&&uvLevel<=503){UVindex=3;}
 else if(uvLevel>503&&uvLevel<=606){UVindex=4;}
 else if(uvLevel>606&&uvLevel<=696){UVindex=5;}
 else if(uvLevel>696&&uvLevel<=795){UVindex=6;}
 else if(uvLevel>795&&uvLevel<=881){UVindex=7;}
 else if(uvLevel>881&&uvLevel<=976){UVindex=8;}
 else if(uvLevel>976&&uvLevel<=1079){UVindex=9;}
 else if(uvLevel>1079&&uvLevel<=1170){UVindex=10;}
 else if(uvLevel>1170){UVindex=11;}
 //Serial.print("ML8511 UV index= ");
 //Serial.print(UVindex);
 //Serial.print(" UV intenzitet (mW/cm^2)= ");
 //Serial.println(uvIntensity);
 //citanje bmp280/////////
 int pritisak=bmp.readPressure()/100;         //citamo pritisak sa BMP280
 int tempBmp=bmp.readTemperature();           //i temperaturu
 int alt=bmp.readAltitude(1013.25);           //kao i nadmorsku visinu
 //Serial.print("BMP280 ");
 //Serial.print(pritisak);
 //Serial.print(" mbar, pri temperaturi od ");
 //Serial.print(tempBmp);
 //Serial.print(" i na nadmorskoj visini od ");
 //Serial.println(alt);
 /////////////////////////
 //Serial.println();
 ///////
 /////////////// Prikazivanje na displeju ///////////////
  if(millis()>vreme1+period){
    vreme1=millis();
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("DIY #1");
    display.setTextSize(1);
    display.println("Meteo\nStation");
    display.println("Svet\nKompjutera");
    display.display();
    }
//ocitavanje vrednosti MQ135 senzora
  if(millis()>vreme2+period){
    vreme2=millis();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Prema MQ135\nkoncentracija\nCO2 je ");
    display.print(co2ppm);
    display.println(" ppm");
    display.display();
    }
  if(millis()>vreme3+period){
    vreme3=millis();
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.println("Prema MQ2\nzasicenost\nprirodnim\ngasovima je ");
display.print(mq2Const);
display.println(" ppm");
display.display();
  }
//ocitavanje vrednosti kisnog senzora
  if(millis()>vreme4+period){
    vreme4=millis();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Kisni senzor\nkaze:");
    display.println(rainSensor);
    display.display();
  }
//ocitavanje vrednosti NTC termistora
  if(millis()>vreme5+period){
    vreme5=millis();
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.println("Prema NTC-u\ntemperatura je");
display.print(Tc);
display.println(" *C");
display.display();
   }
//ocitavanje vrednosti DHT22 senzora
  if(millis()>vreme6+period){
    vreme6=millis();
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.println("Prema DHT22\ntemperatura je");
display.print(temp);
display.println(" *C");
display.println("a rel. vlaga");
display.print(vlaga);
display.println(" %");
display.display();
}
//ocitavanje vrednosti Dallas DS18B20
  if(millis()>vreme7+period){
    vreme7=millis();
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.println("Prema Dallas-u\ntemperatura je");
display.print(sensors.getTempCByIndex(0));
display.println(" *C");
display.display();
  }
//ocitavanje vrednosti UV zracenja preko ML8511
  if(millis()>vreme8+period){
    vreme8=millis();
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.println("Prema ML8511\nUV index je");
display.println(UVindex);
display.println("dok je UV\nintezitet");
display.print(uvIntensity);
display.println("mW/cm^2");
display.display();
  }
//ocitavanje vrednosti BMP280 senzora
  if(millis()>vreme9+period){
    vreme9=millis();
display.clearDisplay();
display.setTextSize(1);
display.setCursor(0,0);
display.println("BMP280 prit.");
display.print(pritisak);
display.println(" mbar");
display.println("a temperatura");
display.print(tempBmp);
display.println(" *C");
display.println("na nadm.vis.");
display.print(alt);
display.println(" m");
display.display();
  }

/////////
 }
///////// funkcija preko koje kontrolisemo prekidac
void buttonSwitch(){
    reading = digitalRead(buttonPin);
  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
    if (state == HIGH)
      state = LOW;
    else
      state = HIGH;

    time = millis();    
  }
  digitalWrite(relayPin, state);
  previous = reading;
 }
// funkcija preko koje dobijamo prosecnu vrednost UV nivoa
int averageAnalogRead(int pinToRead){
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;
  for(int x=0;x<numberOfReadings;x++)
  runningValue +=analogRead(pinToRead);
  runningValue /=numberOfReadings;
  return(runningValue);
  }
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  return(x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
  }

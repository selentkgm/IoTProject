#include <Wire.h>
#include <SPI.h>
#include <TinyGPS++.h> // GPS modülü için kullanılan kütüphane
#include <SoftwareSerial.h>
#include <Adafruit_BMP280.h> // BMP280 ısı ve sıcaklık sensörü için kullanılan kütüphane
#include <ESP8266WiFi.h>
#include <Time.h> // Zaman ölçmesi için kullanılan kütüphane
#include <TimeLib.h>

TinyGPSPlus gps;
SoftwareSerial ss(3, 1);

//Kullanılan değişkenler
float latitude , longitude;
String DateString , TimeString, LatitudeString , LongitudeString;

int gaz_pin = A0;
int Buzzer = D5;
int led_pin_red = D7; //örnek olarak verdim.

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BMP280 bmp;

const char* ssid = "MASTERS";
const char* password = "english123";
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(Buzzer, OUTPUT);
  pinMode(gaz_pin, INPUT);
  pinMode(led_pin_red, OUTPUT);
  ss.begin(9600);
  setTime(12, 0, 0, 1, 1, 20);

  unsigned status;
  status = bmp.begin(0x76);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  int gazsensorAnalog = analogRead(gaz_pin);
  digitalWrite(led_pin_red, LOW);

  while (ss.available() > 0) {
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        LatitudeString = String(latitude , 6);
        longitude = gps.location.lng();
        LongitudeString = String(longitude , 6);
      }
    }
  }

  TimeString = "";
  int hourr = hour();
  int minutee = minute();
  int secondd = second();

  if (hourr < 10)
    TimeString = '0';
  TimeString += String(hourr);
  TimeString += " : ";

  if (minutee < 10)
    TimeString += '0';
  TimeString += String(minutee);
  TimeString += " : ";

  if (secondd < 10)
    TimeString += '0';
  TimeString += String(secondd);

  Serial.print("Gaz Sensor: ");
  Serial.print(gazsensorAnalog);

  Serial.print("Sicaklik = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("Basinc = ");
  Serial.print(bmp.readPressure());
  Serial.println(" hPa");

  Serial.print("Yukseklik = ");
  Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  if (gazsensorAnalog > 3) { //Eşik değerler
    Serial.println("Gaz Yüksek Seviyede!");
    digitalWrite (Buzzer, HIGH) ;
    digitalWrite(led_pin_red, HIGH);
    delay(1000);
    digitalWrite (Buzzer, LOW) ;
    digitalWrite(led_pin_red, LOW);
  }

  else if (bmp.readTemperature() > 10.00) { //Eşik değerler
    Serial.println("Sıcaklık Yüksek Seviyede!");
    digitalWrite (Buzzer, HIGH) ;
    digitalWrite(led_pin_red, HIGH);
    delay(1000);
    digitalWrite (Buzzer, LOW) ;
    digitalWrite(led_pin_red, LOW);
  }

  else if ( bmp.readPressure() > 1000.00) { //Eşik değerler
    Serial.println("Basınç Yüksek Seviyede!");
    digitalWrite (Buzzer, HIGH) ;
    digitalWrite(led_pin_red, HIGH);
    delay(1000);
    digitalWrite (Buzzer, LOW) ;
    digitalWrite(led_pin_red, LOW);
  }

  else if ( gazsensorAnalog > 3 && bmp.readTemperature() > 10.00) { //Eşik değerler
    Serial.println("!!!!ACİL DURUM!!!!");
    digitalWrite (Buzzer, HIGH) ;
    digitalWrite(led_pin_red, HIGH);
    delay(1000);
  }

  else {
    digitalWrite(led_pin_red, LOW);
    digitalWrite (Buzzer, LOW) ;
  }

  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  //web sayfasının tasarımının yapıldığı kod parçası
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>Zonguldak Maden Ocagi</title> <style>";
  s += "table, th, td {border: 1px solid blue;} </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER>Maden Emekcisi Hayati Bilgiler</h1>";
  s += "<p ALIGN=CENTER style=""font-size:200%;""";
  s += "> <b>Sensor Bilgileri</b></p> <table ALIGN=CENTER style=";
  s += "width:60%";
  s += "> <tr> <th>Gaz Degeri</th>";
  s += "<td ALIGN=CENTER >";
  s += gazsensorAnalog;
  s += "</td> </tr> <tr> <th>Sicaklik</th> <td ALIGN=CENTER >";
  s += bmp.readTemperature();
  s += "</td> </tr> <tr> <th>Basinc</th> <td ALIGN=CENTER >";
  s += bmp.readPressure();
  s += "</td> </tr> <tr>  <th>Yukseklik</th> <td ALIGN=CENTER >";
  s += bmp.readAltitude(SEALEVELPRESSURE_HPA);
  s += "</td></tr> <tr> <th>Zaman</th> <td ALIGN=CENTER >";
  s += TimeString;
  s += "</td>  </tr> </table> ";
  s += "<p align=center><a style=""color:RED;font-size:125%;"" href=""http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
  s += LatitudeString;
  s += "+";
  s += LongitudeString;
  s += """ target=""_top"">Buraya Tiklayin</a>-Google Haritalar uzerinden konuma ulasabilmek icin tiklayin.</p>";
  s += "</body> </html> \n";

  client.print(s);
  delay(100);
}

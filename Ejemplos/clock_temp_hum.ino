//
//    FILE: clock_temp_hum.ino
//  AUTHOR: Ariel Armijo Q.
// VERSION: 1.0
// PURPOSE: Mostrar en la pantalla LCD hora, fecha, temperatura y humedad
//     URL: https://github.com/cochayuyo/GreenHouseIno.git
//
// Released to the public domain
//

#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Time.h>
#include <DS1302RTC.h>

#define DHT11_PIN 5
#define RES 6
#define DAT 7
#define CLK 8

DHT dht(DHT11_PIN);   // Crea el objeto dht que representa al sensor DHT11
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Set the LCD address to 0x27 for a 16 chars and 2 line display
DS1302RTC rtc(RES, DAT, CLK);


const unsigned long DHTInterval = 5;
unsigned long previousDHTMeasure = 0;
const unsigned long timeUpdateInterval = 1;
unsigned long previousTimeUpdate = 0;
uint8_t degree[8] = {0x00, 0x0E, 0x0A, 0x0E, 0x00, 0x00, 0x00, 0x00};
char buffer[20];

void setup() {
  dht.setTempOffset(-2);
  dht.setHumOffset(0);
  lcd.begin();
  lcd.createChar(0, degree);
  lcd.backlight(); 

  // Setup Time library
  lcd.clear();
  lcd.print("  RTC Sync");
  setSyncProvider(rtc.get);
  if (timeStatus() == timeSet)
    lcd.print(" Ok!");
  else
    lcd.print(" FAIL!");
  delay (2000);
  lcd.clear();
}

void loop() {
  
  unsigned long currentSecond = now();

  if (currentSecond - previousTimeUpdate >= timeUpdateInterval) {
    previousTimeUpdate = currentSecond;    
    lcd.setCursor(0,0);
    print2digits(hour());
    lcd.print(':');
    print2digits(minute());
    lcd.print(':');
    print2digits(second());
    lcd.setCursor(9,0);
    lcd.print(day());
    lcd.print('/');
    lcd.print(month());
    lcd.print('/');
    lcd.print(year()-2000);
  }
  
  if (currentSecond - previousDHTMeasure >= DHTInterval) {
    previousDHTMeasure = currentSecond;
    dht.read();
    lcd.setCursor(1,1);
    lcd.print(dtostrf(dht.temperature, 4, 1, buffer));
    lcd.write(0);
    lcd.print("C    ");
    lcd.setCursor(10,1);
    lcd.print(dtostrf(dht.humidity, 3, 0, buffer));
    lcd.print("%  ");

  }
}

void print2digits(int number) {
  if (number >= 0 && number < 10)
    lcd.print('0');
  lcd.print(number);
}

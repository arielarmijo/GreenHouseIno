//
//    FILE: lcd_test.ino
//  AUTHOR: Ariel Armijo Q.
// VERSION: 1.0
// PURPOSE: Mostrar en la pantalla LCD la temperatura y humedad
//     URL: https://github.com/cochayuyo/GreenHouseIno.git
//
// Released to the public domain
//

#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define DHT11_PIN 5

DHT dht(DHT11_PIN);   // Crea el objeto dht que representa al sensor DHT11
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Set the LCD address to 0x27 for a 16 chars and 2 line display


uint8_t degree[8] = {0x00, 0x0E, 0x0A, 0x0E, 0x00, 0x00, 0x00, 0x00};   // Crea el carácter "°" para mostrar en la pantalla LCD
char buffer[20];    // Variable temporal para formatear los decimales de la temperatura

void setup() {
  dht.setTempOffset(-2); // Calibra la temperatura medida por el sensor
  dht.setHumOffset(0);   // Calibra la humedad medida por el sensor
  lcd.begin();           // Inicializa la pantalla lcd 
  lcd.createChar(0, degree);  // Crea el signo "°" para mostrar en pantalla
  lcd.backlight();            // Enciende la pantalla lcd
  lcd.setCursor(0,0);   // Ubica el cursor al inicio de la primera fila
  lcd.print("Temp:");   // Imprime en pantalla
  lcd.setCursor(0,1);   // Ubica el cursor al inicio de la segunda fila
  lcd.print("Hum:");    // Imprime en pantalla
}

void loop() {
  // READ DATA
  dht.read();
  // DISPLAY DATA
  lcd.setCursor(6,0);
  lcd.print(dtostrf(dht.temperature, 3, 1, buffer));  // Muestra la temperatura con 3 enteros y 1 decimal
  lcd.write(0);
  lcd.print("C    ");
  lcd.setCursor(6,1);
  lcd.print(dtostrf(dht.humidity, 3, 1, buffer));   // Muestra la humedad con 3 enteros y 1 decimal
  lcd.print(" %    ");
  delay(2000);    // El sensor DHT necesita un retraso de 5 segundos entre cada medida para evitar errores. En la práctica lo mínimo es 1.5 segundos.
}

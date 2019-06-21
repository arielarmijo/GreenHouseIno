//
//    FILE: dht_test.ino
//  AUTHOR: Ariel Armijo Q.
// VERSION: 1.0
// PURPOSE: Mostrar por el puerto seria la temperatura y humedad
//     URL: https://github.com/cochayuyo/GreenHouseIno.git
//
// Released to the public domain
//

#include <DHT.h>

#define DHT11_PIN 5

DHT dht(DHT11_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.print("IRQ: ");
  Serial.println(dht.getDisableIRQ());
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tHumOffset,\tTemperature (C),\tTempOffset");
  dht.setTempOffset(-2);
  dht.setHumOffset(2.5);
}

void loop() {
  // READ DATA
  Serial.print(dht.getType());
  Serial.print(", \t");
  int chk = dht.read();
  switch (chk) {
    case DHTLIB_OK:  
		Serial.print("OK,\t"); 
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		Serial.print("Checksum error,\t"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		Serial.print("Time out error,\t"); 
		break;
    default: 
		Serial.print("Unknown error,\t"); 
		break;
  }
  // DISPLAY DATA
  Serial.print(dht.humidity, 1);
  Serial.print(",\t");
  Serial.print(dht.getHumOffset());
  Serial.print(",\t");
  Serial.print(dht.temperature, 1);
  Serial.print(",\t");
  Serial.println(dht.getTempOffset());
  delay(2000);
}

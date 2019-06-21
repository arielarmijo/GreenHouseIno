// DS1302_Serial_Easy (C)2010 Henning Karlsen
// web: http://www.henningkarlsen.com/electronics
//
// Adopted for DS1302RTC library by Timur Maksimov 2014
//
// A quick demo of how to use my DS1302-library to 
// quickly send time and date information over a serial link

#include <Time.h>
#include <DS1302RTC.h>

// Set pins:  RES, DAT, CLK
DS1302RTC rtc(6, 7, 8);


void setup() {
  // Setup Serial connection
  Serial.begin(9600);
  
  Serial.println("DS1302RTC Read Test");
  Serial.println("-------------------");
  
  if (rtc.haltRTC()) {
    Serial.println("The DS1302 is stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running.");
    Serial.println();
  }
  if (!rtc.writeEN()) {
    Serial.println("The DS1302 is write protected. This normal.");
    Serial.println();
  }
  
  delay(5000);
}

void loop()
{
  tmElements_t tm;
  
  Serial.print("UNIX Time: ");
  Serial.print(rtc.get());

  if (! rtc.read(tm)) {
    Serial.print("  Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print(", DoW = ");
    Serial.print(tm.Wday);
    Serial.println();
  } else {
    Serial.println("DS1302 read error!  Please check the circuitry.");
    Serial.println();
    delay(10000);
  }
  
  // Wait one second before repeating :)
  delay (1000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10)
    Serial.write('0');
  Serial.print(number);
}

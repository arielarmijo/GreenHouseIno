/*  ARDUINO GREENHOUSE RTC SD
 *  
 *  Controla el estado de un invernadero según los requerimientos de temperatura 
 *  y humedad específicos para cada etapa del ciclo vital de un hongo.
 *  
 *  El programa lleva un registro de la fecha y la hora mediante un reloj en 
 *  tiempo real (RTC) y mide la temperatura y humedad con un sensor DHT11 a 
 *  intervalos de tiempo definidos por el usuario. Esos datos son utilizados 
 *  para controlar el estado del invernadero.
 *  
 *  El estado del invernadero consiste en la etapa del ciclo vital del hongo,
 *  el estado del ventilador y el estado de la válvula. El ciclo vital de un
 *  hongo se divide en una etapa de crecimiento y otra de reproducción, cada
 *  una con una duración en días determinada y unos umbrales de temperatura y
 *  humedad definidos que pueden ser modificados por el usuario. Este estado 
 *  se simboliza en el programa con tres letras: G de crecimiento, R de 
 *  reproducción y E de ciclo finalizado. El estado del ventilador se simboliza 
 *  con un 1 (encendido) o un 0 (apagado), lo mismo que el estado de la válvula. 
 *  Por ejemplo, el estado G10 significa que se está en la etapa de crecimiento, 
 *  el ventilador está encendido y la válvula está apagada.
 *  
 *  El ventilador está pensado para mantener la temperatura BAJO un valor umbral, 
 *  mientras que la válvula está diseñada para mantener la humedad SOBRE un 
 *  valor umbral. 
 *  
 *  En la pantalla LCD se muestra información relevante como la hora, el número
 *  de días transcurridos, el estado del invernadero y los valores de temperatura
 *  y humedad relativa. Al presionar el pulsador se muestran los valores umbrales
 *  de temperatura y humedad específicos de la etapa del ciclo. También se utiliza 
 *  la pantalla LCD para mostros mensajes de error en la operación del RTC o de 
 *  la tarjeta micro SD.
 *  
 *  El programa también registra las variables y el estado del invernadero en 
 *  una tarjeta microSD en el archivo "DATALOG.TXT". Por otro lado, almacena la
 *  fecha y hora de inicio del ciclo en el archivo "DATE.TXT".
 *  
 *  CONEXIONES:
 *  
 *  Interruptor pull-down (muestra los umbrales de temperatura y humedad):
 *    5V - pushbutton 
 *    pushbutton - pin 4 
 *    pin 4 - 10 kilohm resistor  
 *    10 kilohm resistor - GND
 *  
 *  Módulo relé 2 canales (controla los actuadores):
 *    VCC - 5V
 *    IN1 - pin 6 (controla el ventilador)
 *    IN2 - pin 5 (controla la válvula)
 *    GND - GND
 *  
 *  Módulo sensor DHT11 (mide la temperatura y humedad):
 *    VCC - 5V
 *    OUT - pin 7
 *    GND - GND
 *    
 *  Pantalla LCD + I2C (muestra estado del invernadero):
 *    SCL - A5
 *    SDA - A4
 *    VCC - 5V
 *    GND - GND
 *    
 *  Módulo RTC (registra la fecha y la hora):
 *    SCL - A5
 *    SDA - A4
 *    VCC - 5V
 *    GND - GND
 *  
 *  Módulo lector de tajeras microSD (registra el estado del invernadero):
 *    CS   - pin 10
 *    MOSI - pin 11
 *    MISO - pin 12
 *    CLK  - pin 13
 *    VCC - 5V
 *    GND - GND
 *    
 * creado  20 Oct 2018
 * modificado 21 Oct 2018
 * por Ariel Armijo -  arielarmijo@yahoo.es 
 */
 
#include <SPI.h> 
#include <SD.h>
#include <DHT.h> 
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

#define DHTTYPE DHT11     // Define el tipo de sensor utilizado.
#define ON  0             // Define la constante On como 0.
#define OFF 1             // Define la constante Off como 1.

const bool debug = true;  // Si es true, imprime en el puerto serie los datos mostrados en la pantalla LCD.

const unsigned long DHTInterval = 5;  // Intervalo en segundos de mediciones con el sensor DHT 11.
unsigned long previousDHTMeasure = 0; // Segundo en el cual ocurrió la última medición.

const unsigned long timeUpdateInterval = 1; // Intervalo en segundos de actualización del tiempo y los días del ciclo.
unsigned long previousTimeUpdate = 0;       // Segundo en el cual se actualizó por última vez el tiempo y los días del ciclo.

const unsigned long fileWriteInterval = 300;  // Intervalo en segundos en que se escribirá la tarjeta SD.
unsigned long previousFileWrite = 0;          // Segundo en el cual se escribió por última vez en la tarjeta SD.


const int growthDays = 10;       // Duración en días del ciclo de crecimiento.
const int reproductiveDays = 5;  // Duración en días del ciclo de reproducción.
const int GDTemperature = 20;    // Temperatura necesaria en el ciclo de crecimiento.
const int GDHumidity = 50;       // Humedad relativa necesaria en el ciclo de crecimiento.
const int RDTemperature = 15;    // Temperatura necesaria en el ciclo reproductivo.
const int RDHumidity = 25;       // Humedad relativa necesaria en el ciclo reproductivo.

int temperatureThreshold; // Umbral de temperatura, depende de la etapa del ciclo.
int humidityThreshold;    // Umbral de humedad, depende de la etapa del ciclo.

int temperature;  // Temperatura medida por el sensor.
int humidity;     // Humedad medida por el sensor.
int days;         // Día del ciclo.

const int chipSelect = 10;  // Pin digital conectado a CS del módulo lector tarjetas microSD.
const int dhtPin = 7;       // Pin digital conectado al sensor.
const int fanPin = 6;       // Pin digital conectado al relé que controla el ventilador.
const int valvePin = 5;     // Pin digital conectado al rele que controla la válvula.
const int inputPin = 4;     // Pin digital conectado al pulsador (pull-down).

char ciclePhase = 'U';  // Etapa del ciclo. Puede ser G (crecimiento, R (reproducción), E (Ciclo finalizado) o U (desconocido).
int fanState = OFF;     // Estado del ventilador. Puede ser ON u OFF.
int valveState = OFF;   // Estado de la válvula. Puede ser ON u OFF.

uint8_t degree[8] = {0x00, 0x0E, 0x0A, 0x0E, 0x00, 0x00, 0x00, 0x00};   // Crea el carácter "°" para mostrar en la pantalla LCD.

DateTime start; // Fecha y hora de inicio del ciclo.

DHT dht(dhtPin, DHTTYPE);             // Sensor de temperatura y humedad.
RTC_DS1307 rtc;                       // Reloj en tiempo real.
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Pantalla LCD.

void setup() {

  pinMode(fanPin, OUTPUT);
  pinMode(valvePin, OUTPUT);
  pinMode(inputPin, INPUT);
  
  digitalWrite(fanPin, OFF);
  digitalWrite(valvePin, OFF);

  dht.begin();

  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("ARDUINO");
  lcd.setCursor(3,1);
  lcd.print("GREENHOUSE");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("WAITING DATA...");

  if (debug)
    Serial.begin(9600);

  if (! rtc.begin()) { 
    // Couldn't find RTC
    errorMsg(11);
    while (1);
  }

  if (! rtc.isrunning()) {
    // RTC is NOT running!
    errorMsg(12);
    while (1);
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    
  }
  
  if (!SD.begin(chipSelect)) {   
    //Card failed, or not present
    errorMsg(21);
   while (1);
  }

  if (SD.exists("date.txt")) {
     
    File dataFile = SD.open("date.txt");

    if (dataFile) {
      char letter;
      String data;
      while (dataFile.available()) {
        letter = dataFile.read();
        data += letter;      
      }
      
      dataFile.close();

      int year = data.substring(6,10).toInt();
      int month = data.substring(3,5).toInt();
      int day = data.substring(0,2).toInt();
      int hour = data.substring(11,13).toInt();
      int minute = data.substring(14,16).toInt();
      int second = data.substring(17,19).toInt();

      start = DateTime(year, month, day, hour, minute, second);
    
    } else {
      // Error opening file
      errorMsg(22);
    }
     
  } else {
    
    start = rtc.now();
    //start = DateTime(2018, 10, 20);
    
    File dataFile = SD.open("date.txt", FILE_WRITE);

    if (dataFile) {
      dataFile.println(printDate(start) + " " + printTime(start));
      dataFile.close(); 
       
    } else {
      // Error opening file
      errorMsg(22);
    }
    
  }

  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  if (dataFile) {   
    dataFile.print("Fecha inicio: ");
    dataFile.println(printDate(start) + " " + printTime(start));
    dataFile.println("Fecha Hora, Dia, T (C), HR (%), Estado");
    dataFile.close();
    
  } else { 
    // Error opening file
    errorMsg(22);
  }
  
  if (debug) {
    Serial.print("Fecha inicio: ");
    Serial.println(printDate(start) + " " + printTime(start));
    Serial.println("Fecha Hora, Dia, T (C), HR (%), Estado");
  }

}

void loop() {

  DateTime now = rtc.now();
  unsigned long currentSecond = now.unixtime() - start.unixtime();

  if (currentSecond - previousTimeUpdate >= timeUpdateInterval) {
    
    previousTimeUpdate = currentSecond;
    days = currentSecond / 86400;
    
    // Muestra el tiempo en la pantalla LCD.
    lcd.setCursor(0,0);
    lcd.print(printTime(now));
    lcd.print(" ");
    
    // Muestra los días del ciclo en la pantalla LCD.
    lcd.setCursor(9,0);
    lcd.print(formatNumber(days));
    lcd.print(" ");

    // Muestra el estado del invernadero en la pantalla LCD.
    printStatutToLCD();
    
  }

  if (currentSecond - previousDHTMeasure >= DHTInterval) { 
       
    previousDHTMeasure = currentSecond;
    
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    
    greenHouseControl();
    
    // Muestra la temperatura y humedad en la pantalla LCD.
    printVariablesToLCD(temperature, humidity);

    // Muestra la información en el monitor serie.
    if (debug) {
      Serial.print(printDate(now));
      Serial.print(" ");
      Serial.print(printTime(now));
      Serial.print(", ");
      Serial.print(days);
      Serial.print(", ");
      Serial.print(temperature);
      Serial.print(", ");
      Serial.print(humidity);
      Serial.print(", ");
      Serial.print(ciclePhase);
      Serial.print(toggleState(fanState));
      Serial.println(toggleState(valveState));
    }
  }

  if (currentSecond - previousFileWrite >= fileWriteInterval) {
    
    previousFileWrite = currentSecond;
    
    String dataString = printDate(now) + " " + printTime(now) + ", " + days +
                        ", " + temperature + ", " + humidity + ", " +
                        ciclePhase + toggleState(fanState) + toggleState(valveState);
    
    if (debug)
      Serial.print("Writing data... ");
      
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
    
    if (dataFile) {     
      dataFile.println(dataString);
      dataFile.close();
      if (debug)
        Serial.println("Ok");
        
    } else {
      // Error opening file
      errorMsg(22);
        
      if (!SD.begin(chipSelect)) {
        //Card failed, or not present
        errorMsg(21);
 
      } else {
        previousFileWrite -= fileWriteInterval;
      }
    } 
  }
  
  if (digitalRead(inputPin) == HIGH ){
    
    // Muestra en la pantalla LCD los umbrales de temperatura y humedad.
    lcd.setCursor(0,0);
    lcd.print("    UMBRALES    ");   
    printVariablesToLCD(temperatureThreshold, humidityThreshold);
    delay(1000);
    printStatutToLCD();
    printVariablesToLCD(temperature, humidity);
    
  }
  
}

// Muestra mensajes de error.
void errorMsg(int e) {
  
  lcd.setCursor(0,1);
  lcd.print("   ERROR ");
  
  switch (e) {
    case 11:
            if (debug) 
              Serial.println("ERROR 11: Couldn't find RTC");
            lcd.print("011    ");
            break;
    case 12:
            if (debug) 
              Serial.println("ERROR 12: RTC is NOT running");
            lcd.print("012    ");
            break;
    case 21:
            if (debug) 
              Serial.println("ERROR 21: Card failed, or not present");
            lcd.print("021    ");
            break;        
    case 22:
            if (debug) 
              Serial.println("ERROR 22: Error opening file");
            lcd.print("022    ");         
            break;           
  }
  
}

// Muestra el estado del invernadero en la pantalla LCD.
void printStatutToLCD() {
  lcd.setCursor(12,0);
  lcd.print(ciclePhase);
  lcd.print(toggleState(fanState));
  lcd.print(toggleState(valveState));
  lcd.print(" ");
}

// Muestra la temperatura y humedad en la pantalla LCD.
void printVariablesToLCD(int temperature, int humidity) {
  lcd.setCursor(0,1);
  lcd.print("TEMP ");
  if (temperature < 10) lcd.print(" ");
  lcd.print(temperature);
  lcd.write(0);
  lcd.print("C ");
  lcd.print("HR ");
  if (humidity < 10) lcd.print(" ");
  lcd.print(humidity);
  lcd.print("%");
}

// Configura la etapa del ciclo y los umbrales de temperatura y humedad
// según el día del ciclo y luego cambia el estado del invernadero.
void greenHouseControl() {
  
  // Fase de crecimiento
  if (days < growthDays) {   
    ciclePhase = 'G';
    temperatureThreshold = GDTemperature;
    humidityThreshold = GDHumidity;

  // Fase reproductiva
  } else if ( (days >= growthDays) && (days < (growthDays + reproductiveDays)) ) {    
    ciclePhase = 'R';
    temperatureThreshold = RDTemperature;
    humidityThreshold = RDHumidity;
      
  // Ciclo finalizado
  } else {   
    ciclePhase = 'E';
    temperatureThreshold = 99;
    humidityThreshold = 0;        
  }
  
  changeState();
  digitalWrite(fanPin, fanState);
  digitalWrite(valvePin, valveState);
    
}

// Cambia el estado del ventilador y de la válvula según la temperatura 
// y humedad actuales y los umbrales definidos previamente.
void changeState(){
  
  if (temperature > temperatureThreshold)
    fanState = ON;
  else
    fanState = OFF;
  
  if (humidity < humidityThreshold)
    valveState = ON;
  else
    valveState = OFF;
        
}

// Devuelve un string formado por la hora actual.
String printTime(DateTime dt){
  return formatNumber(dt.hour()) + ":" + formatNumber(dt.minute()) + ":" + formatNumber(dt.second());
}

// Devuelve un string formado por la fecha actual.
String printDate(DateTime dt){
  return formatNumber(dt.day()) + "/" + formatNumber(dt.month()) + "/" + formatNumber(dt.year());
}

// Agrega un 0 antes de un número si este es menor a 10.
String formatNumber(int n) {
  if (n < 10) {
    return "0" + String(n);
  } else {
    return String(n);
  }
}

// Cambia los estados del ventilador y de la válvula para ser mostrados de una
// forma más intuitiva en la pantalla LCD o en el puerto serial. Esto es necesario
// porque el relé funciona con una lógica invertida: 0 activa el relé y 1 lo desactiva.
int toggleState(int pinState) {
  if (pinState == ON)
    return 1;
  else 
    return 0;
}

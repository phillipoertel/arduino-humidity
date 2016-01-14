#include "DHT.h" // für DHT22 Temperatur/Feuchtigkeitssensor
#include <Wire.h> // Wire Bibliothek hochladen für LCD display
#include <LiquidCrystal_I2C.h> // Vorher hinzugefügte LiquidCrystal_I2C Bibliothek hochladen

#define SENSORPIN  2
#define RELAYPIN   3
#define LEDPIN     4
#define POTIPIN    A0
#define DHTTYPE    DHT22

#define LCD_ROW_0  0
#define LCD_ROW_1  1

// desired minimum adjustable humidity (poti turned all the way to the left)
#define ADJUSTABLE_HUMIDITY_MIN 30
// desired maximum adjustable humidity
#define ADJUSTABLE_HUMIDITY_MAX 70
// poti readout when it's turned all the way to the left
#define POTI_RANGE_MIN 4
// poti readout when turned all the way to the right
#define POTI_RANGE_MAX 1016

// keep the actual humidity within this range
#define ALLOWED_HUMIDITY_DEVIATION 2

// DHT22 gives new readings every 2 seconds
#define LOOP_DELAY 2000

// track how often the relay gets turned on
int numCycles = 0;
bool relayState = 0; 

// Initialize DHT sensor.
DHT dht(SENSORPIN, DHTTYPE); 
// Initialize display
LiquidCrystal_I2C lcd(0x27, 16, 2); // Hier wird festgelegt um was für einen Display es sich handelt. In diesem Fall einer mit 16 Zeichen in 2 Zeilen. 

void setup() {
  Serial.begin(9600);
  lcd.begin();
  pinMode(RELAYPIN, OUTPUT); // only this way the relay is supplied with 5V!
  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  //
  // get readings
  //
  int targetHumidity = getTargetHumidity();
  Serial.print("Target humidity is ");
  Serial.println(targetHumidity);
  int measuredHumidity = getMeasuredHumidity();
  Serial.print("Measured humidity is ");
  Serial.println(measuredHumidity);

  //
  // control relay with "hysteresis"
  //
  int minHumidity = (targetHumidity - ALLOWED_HUMIDITY_DEVIATION);
  int maxHumidity = (targetHumidity + ALLOWED_HUMIDITY_DEVIATION);
  if (measuredHumidity > maxHumidity) {
    relayOff();
  } else if (measuredHumidity <= minHumidity) {
    if (relayState == 0) {
      numCycles++;
    }
    relayOn();
  }

  //
  // show readings
  //
  lcd.setCursor(0, LCD_ROW_0);
  String row0 = "Ziel " + String(targetHumidity) + "% Akt " + String(measuredHumidity) + "%";
  lcd.print(row0);
  lcd.setCursor(0, LCD_ROW_1);
  String row1 = "Zyk " + String(numCycles) + " T" + getUptime();
  lcd.print(row1);

  delay(LOOP_DELAY);
}


// calculate the desired humidity from the poti position
int getTargetHumidity() {
  int potiReading   = analogRead(POTIPIN);
  int humidityRange = ADJUSTABLE_HUMIDITY_MAX - ADJUSTABLE_HUMIDITY_MIN;
  float potiRange   = POTI_RANGE_MAX - POTI_RANGE_MIN;
  int target        = (potiReading * (humidityRange / potiRange)) + ADJUSTABLE_HUMIDITY_MIN;
  return target;
}

// Reading temperature or humidity takes about 250 milliseconds.
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
int getMeasuredHumidity() {
  float reading = dht.readHumidity();
  // turn on red LED if measurement went wrong
  //bool turnLedOn = isnan(reading);
  //digitalWrite(LEDPIN, turnLedOn);
  return reading;
}

void relayOn() {
  relayState = 1;
  digitalWrite(RELAYPIN, HIGH);
  //digitalWrite(LEDPIN, HIGH);
}

void relayOff() {
  relayState = 0;
  digitalWrite(RELAYPIN, LOW);
  //digitalWrite(LEDPIN, LOW);
}

String getUptime() {
  long uptimeInSeconds = millis() / 1000;
  int s = uptimeInSeconds % 60;
  int m = (uptimeInSeconds / 60) % 60;
  int h = (uptimeInSeconds / (60 * 60) ) % 24;
  int d = (uptimeInSeconds / (60 * 60 * 24));
  return String(String(d) + "d" + String(h) + "h" + String(m) + "m");
}

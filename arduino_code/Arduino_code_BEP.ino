#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "RTClib.h"
#include <Adafruit_MAX31865.h>
#include <SD.h>

// Pin definitions
//// SD Card, RTC, LCD, button
#define CS_SD 4         // SD card module chip select
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27,  16, 2);
const int buttonPin = 2;
bool writingToFile = false;

//// Temp sensors
#define CS_PIN_1 6      // MAX31865 sensor 1 chip select
#define CS_PIN_2 7      // MAX31865 sensor 2 chip select
#define CS_PIN_3 8      // MAX31865 sensor 3 chip select
#define RREF 430.0      // Reference resistor for MAX31865
#define RNOMINAL 100.0  // Nominal resistance for PT100 sensor
Adafruit_MAX31865 sensor1 = Adafruit_MAX31865(CS_PIN_1);  // First sensor
Adafruit_MAX31865 sensor2 = Adafruit_MAX31865(CS_PIN_2);  // Second sensor
Adafruit_MAX31865 sensor3 = Adafruit_MAX31865(CS_PIN_3);  // Third sensor

// Files to write data onto
char filename1[30];
char filename2[30];
char filename3[30];

unsigned long previousMillis = 0;
const long interval = 1000; // time interval

void setup() {
  Serial.begin(115200);
  
  pinMode(buttonPin, INPUT);
  //initialize lcd screen
  lcd.init();
  delay(500);
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("T1:");
  
  lcd.setCursor(0,1);
  lcd.print("T2:");

  // Initialize SD card
  if (!SD.begin(CS_SD)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized.");

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  Serial.println("RTC initialized.");
  // Uncomment this line ONCE to set the RTC to your computer's current date and time
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

//  // Check if the RTC is running
//  if (rtc.lostPower()) {
//    Serial.println("RTC lost power, setting the time!");
//    // Uncomment to set RTC to the compile time
//     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  }

  // Initialize sensors
  sensor1.begin(MAX31865_4WIRE);
  sensor2.begin(MAX31865_4WIRE);
  sensor3.begin(MAX31865_4WIRE);

  Serial.println("Reading temperatures from three MAX31865 sensors...");

  // Get current date and time from RTC
  DateTime now = rtc.now();
  char dateBuffer[9];  // Buffer for date (YYMMDD)
  char timeBuffer[7];  // Buffer for time (HHMM)
  snprintf(dateBuffer, sizeof(dateBuffer), "%02d%02d%02d", now.year() % 100, now.month(), now.day());
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d%02d", now.hour(), now.minute());

  sprintf(filename1, "1_%02d%02d.txt", now.month(), now.day());
  sprintf(filename2, "2_%02d%02d.txt", now.month(), now.day());
  sprintf(filename3, "3_%02d%02d.txt", now.month(), now.day());
  
  // Initial measurement on power-up
  takeMeasurements();
}

void loop() {
  unsigned long currentMillis = millis();

  // Take measurement at intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    takeMeasurements();
  }
  
  if (digitalRead(buttonPin) == HIGH) {
  writingToFile = !writingToFile;
  Serial.print("button is: ");
  Serial.println(writingToFile);
  delay(300);
  }

  // Get current date and time from RTC
  DateTime now = rtc.now();
  char timeprint[7];  // Buffer for time (HHMMSS)
  snprintf(timeprint, sizeof(timeprint), "%02d:%02d", now.hour(), now.minute());
}

void takeMeasurements() {
  // Read temperatures
  float temp1 = sensor1.temperature(RNOMINAL, RREF); // Read sensor 1
  float temp2 = sensor2.temperature(RNOMINAL, RREF); // Read sensor 2
  float temp3 = sensor3.temperature(RNOMINAL, RREF); // Read sensor 3
  // Print the temperatures to the Serial Monitor
  Serial.print("Sensor 1 Temperature: ");
  Serial.print(temp1);
  Serial.println(" C");

  Serial.print("Sensor 2 Temperature: ");
  Serial.print(temp2);
  Serial.println(" C");

  Serial.print("Sensor 3 Temperature: ");
  Serial.print(temp3);
  Serial.println(" C");

  lcd.setCursor(0,0);
  lcd.print("1:");
  lcd.print(temp1,1);

  lcd.setCursor(8,0);
  lcd.print("2:");
  lcd.print(temp2,1);

  lcd.setCursor(0,1);
  lcd.print("3:");
  lcd.print(temp3,1);

  // Write to files
  if (writingToFile){
    writeToFile(filename1, temp1);
    writeToFile(filename2, temp2);
    writeToFile(filename3, temp3);
    lcd.setCursor(14,1);
    lcd.print("SD");
  }else{
    lcd.setCursor(14,1);
    lcd.print("  ");
  }
}

void writeToFile(const char *filename, float temp) {
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.println(temp);
    file.close();
    Serial.print("Data written to ");
    Serial.println(filename);
  } else {
    Serial.print("Failed to open file: ");
    Serial.println(filename);
  }
}

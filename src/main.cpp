#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ShiftRegister74HC595.h>

#define ADC_VREF_mV 5000.0
#define ADC_RESOLUTION 4095.0
#define LM35_SENSOR 36
#define FLAME_SENSOR 4
#define GAS_SENSOR 34

#define RED_LED 12
#define GREEN_LED 13
#define BUZZER 14

#define DATA_PIN 25
#define LATCH_PIN 33
#define CLOCK_PIN 32

const char* ssid = "iot";
const char* password = "1234567890";

ShiftRegister74HC595<1> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(LM35_SENSOR, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(GAS_SENSOR, INPUT);

  lcd.init();
  lcd.backlight();

  bool ledOn = false;
  digitalWrite(GREEN_LED, HIGH);
  delay(300);

  Serial.printf("\n\nConnecting to %s \n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    digitalWrite(GREEN_LED, ledOn);
    ledOn = !ledOn;
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(GREEN_LED, HIGH);
}

void loop()
{
  int tempRaw = analogRead(LM35_SENSOR);
  float tempMilliVolt = tempRaw * (ADC_VREF_mV / ADC_RESOLUTION);
  float temp = tempMilliVolt / 10;

  int gasRaw = analogRead(GAS_SENSOR);

  int flameRaw = digitalRead(FLAME_SENSOR);

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C");
  Serial.print(" ~ ");
  Serial.print("Gas: ");
  Serial.print(gasRaw);
  Serial.print("ppm");
  Serial.print(" ~ ");

  if (flameRaw != HIGH) {
    Serial.println("ada api");
  } else {
    Serial.println("tidak ada api");
  }

  lcd.setCursor(0, 0);
  lcd.printf("Temp: %.2f", temp);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.printf("Gas : %d ppm", gasRaw);

  delay(1000);
}

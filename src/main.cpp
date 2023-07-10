#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ShiftRegister74HC595.h>
#include <BlynkSimpleEsp32.h>
#include <millisDelay.h>

#define BLYNK_PRINT Serial

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

char auth[] = "LoKztmBi--ARtaRQsubVvsb0twnDB9VQ";
char ssid[] = "hub space";
char pass[] = "Password123@";
char server[] = "103.186.0.26";
int port = 8080;

WiFiClient espClient;
ShiftRegister74HC595<1> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

const unsigned long SENSOR_DELAY = 1000;
millisDelay sensorDelay;

const unsigned long NOTIFICATION_DELAY = 5000;
millisDelay notificationDelay;

WidgetLCD blynkLcd(V2);

float temp, gas;
bool flameStatus = false;
DynamicJsonDocument data(1024);

void triggerAction();
void notificationAction();
void readSensors();

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
  WiFi.begin(ssid, pass);
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

  blynkLcd.clear();
  blynkLcd.print(0, 0, "IP address:");
  blynkLcd.print(0, 1, WiFi.localIP());

  Blynk.begin(auth, ssid, pass, server, port);
  sensorDelay.start(SENSOR_DELAY);
  notificationDelay.start(NOTIFICATION_DELAY);
}

void loop()
{
  Blynk.run();
  notificationAction();
  readSensors();
}

void triggerAction() {
  char outputData[1024];
  bool highTemp = temp >= 80;
  bool highGas = gas > 4000;
  if (highGas) {
    data["status"] = "gas menaik";
    Serial.println("Gas Menaik");
  }

  if (highTemp) {
    data["status"] = "gas menaik";
    Serial.println("Suhu menaik");
  }

  if (highGas && highTemp) {
    data["status"] = "gas menaik";
    Serial.println("Gas dan suhu menaik");
  }

  if (highGas && highTemp && flameStatus) {
    data["status"] = "gas menaik";
    Serial.println("Gas dan suhu menaik, ada api");
  }

  serializeJson(data, outputData);
  Blynk.virtualWrite(V11, outputData);
}

void notificationAction() {
  if (notificationDelay.justFinished()) {
    notificationDelay.repeat();

    blynkLcd.clear();
    blynkLcd.print(0, 0, "Status:");
    blynkLcd.print(0, 1, "Online");
  }
}

void readSensors()
{
  if (sensorDelay.justFinished()) {
    sensorDelay.repeat();

    int tempRaw = analogRead(LM35_SENSOR);
    float tempMilliVolt = tempRaw * (ADC_VREF_mV / ADC_RESOLUTION);
    temp = tempMilliVolt / 10;

    gas = analogRead(GAS_SENSOR);
    int flameRaw = digitalRead(FLAME_SENSOR);

    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("°C");
    Serial.print(" ~ ");
    Serial.print("Gas: ");
    Serial.print(gas);
    Serial.print("PPM");
    Serial.print(" ~ ");

    flameStatus = flameRaw != HIGH;
    if (flameStatus) {
      Serial.println("ada api");
    } else {
      Serial.println("tidak ada api");
    }

    lcd.setCursor(0, 0);
    lcd.printf("Temp: %.2f", temp);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.printf("Gas : %d ppm", gas);

    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, gas);
    Blynk.virtualWrite(V2, flameRaw != HIGH);

    data["flame"] = flameRaw != HIGH;
    data["temperature"] = temp;
    data["gas"] = gas;
  }
}

BLYNK_WRITE(V3)
{
  if (param.asInt() == 1){
    sr.set(0, HIGH);
  } else {
    sr.set(0, LOW);
  }
}

BLYNK_WRITE(V4)
{
  if (param.asInt() == 1){
    sr.set(1, HIGH);
  } else {
    sr.set(1, LOW);
  }
}

BLYNK_WRITE(V5)
{
  if (param.asInt() == 1){
    sr.set(2, HIGH);
  } else {
    sr.set(2, LOW);
  }
}

BLYNK_WRITE(V6)
{
  if (param.asInt() == 1){
    sr.set(3, HIGH);
  } else {
    sr.set(3, LOW);
  }
}

BLYNK_WRITE(V7)
{
  if (param.asInt() == 1){
    sr.set(4, HIGH);
  } else {
    sr.set(4, LOW);
  }
}

BLYNK_WRITE(V8)
{
  if (param.asInt() == 1){
    sr.set(5, HIGH);
  } else {
    sr.set(5, LOW);
  }
}

BLYNK_WRITE(V9)
{
  if (param.asInt() == 1){
    sr.set(6, HIGH);
  } else {
    sr.set(6, LOW);
  }
}

BLYNK_WRITE(V10)
{
  if (param.asInt() == 1){
    sr.set(7, HIGH);
  } else {
    sr.set(7, LOW);
  }
}

BLYNK_WRITE(V11)
{
  Serial.println("WebHook data:");
  Serial.println(param.asStr());
}

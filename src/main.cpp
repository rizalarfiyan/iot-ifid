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

const unsigned long NOTIFICATION_DELAY = 200;
millisDelay notificationDelay;

WidgetLCD blynkLcd(V2);

float temp = 0.0;
int gas = 0;
bool flameStatus = false;
DynamicJsonDocument data(1024);

bool isAlarm = false;
bool stateNotification = false;

// 0 => aman
// 1 => gas naik
// 2 => suhu naik
// 3 => ada api
// 4 => gas dan suhu naik
// 5 => gas dan suhu naik, ada api
int status, prevStatus = 0;

void triggerAction();
void triggerWebhook();
void notificationAction();
void readSensors();
void setStatus(int state);
bool isSameStatus();
void handleAction();
void setBlynkLCD(const char statusChar[]);
void handleTriggerAction(const uint8_t valueBlower, const uint8_t valueSprinkler);

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

  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  Serial.printf("\n\nConnecting to %s \n", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    digitalWrite(GREEN_LED, ledOn);
    ledOn = !ledOn;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(GREEN_LED, HIGH);

  Blynk.begin(auth, ssid, pass, server, port);

  setBlynkLCD("Online");
  delay(500);

  sensorDelay.start(SENSOR_DELAY);
  notificationDelay.start(NOTIFICATION_DELAY);
}

void loop()
{
  Blynk.run();
  notificationAction();
  triggerAction();
  readSensors();
}

void setStatus(int state) {
  prevStatus = status;
  status = state;
}

void triggerAction() {
  bool highTemp = temp >= 80;
  bool highGas = gas > 4000;
  if (highGas && highTemp && flameStatus) {
    setStatus(5);
  } else if (highGas && highTemp) {
    setStatus(4);
  } else if (flameStatus) {
    setStatus(3);
  } else if (highTemp) {
    setStatus(2);
  } else if (highGas) {
    setStatus(1);
  } else {
    setStatus(0);
  }
  handleAction();
}

void handleAction() {
  if (status == prevStatus) return;

  switch (status)
  {
  case 5:
    isAlarm = true;
    setBlynkLCD("Bahaya");
    Serial.println("Gas dan suhu naik, ada api");
    data["message"] = "Gas dan suhu naik, ada api";
    data["state"] = 5;
    triggerWebhook();
    handleTriggerAction(1, 1);
    break;
  case 4:
    isAlarm = true;
    setBlynkLCD("Bahaya");
    Serial.println("Gas dan suhu naik");
    data["message"] = "Gas dan suhu naik";
    data["state"] = 4;
    triggerWebhook();
    handleTriggerAction(1, 0);
    break;
  case 3:
    isAlarm = true;
    setBlynkLCD("Bahaya");
    Serial.println("Ada api");
    data["message"] = "Ada api";
    data["state"] = 3;
    triggerWebhook();
    handleTriggerAction(1, 1);
    break;
  case 2:
    isAlarm = true;
    setBlynkLCD("Waspada");
    Serial.println("Suhu naik");
    data["message"] = "Suhu naik";
    data["state"] = 2;
    triggerWebhook();
    handleTriggerAction(1, 0);
    break;
  case 1:
    isAlarm = true;
    setBlynkLCD("Waspada");
    Serial.println("Gas naik");
    data["message"] = "Gas naik";
    data["state"] = 1;
    triggerWebhook();
    handleTriggerAction(1, 0);
    break;
  default:
    isAlarm = false;
    setBlynkLCD("Online");
    handleTriggerAction(0, 0);
    break;
  }
}

void setBlynkLCD(const char statusChar[]) {
  blynkLcd.clear();
  blynkLcd.print(0, 0, "Status:");
  blynkLcd.print(0, 1, statusChar);
}

void handleTriggerAction(const uint8_t valueBlower, const uint8_t valueSprinkler) {
  sr.set(0, valueBlower);
  Blynk.virtualWrite(V3, valueBlower);
  sr.set(1, valueSprinkler);
  Blynk.virtualWrite(V4, valueSprinkler);
}

void notificationAction() {
  if (notificationDelay.justFinished()) {
    notificationDelay.repeat();

    if (!isAlarm) {
      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER, LOW);
      return;
    }

    if (stateNotification) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(BUZZER, HIGH);
      stateNotification = false;
      return;
    }

    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);
    stateNotification = true;
  }
}

void triggerWebhook() {
  char outputData[1024];
  serializeJson(data, outputData);
  Blynk.virtualWrite(V11, outputData);
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
    flameStatus = flameRaw == HIGH;

    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.print("°C");
    Serial.print(" ~ ");
    Serial.print("Gas: ");
    Serial.print(gas);
    Serial.print("PPM");
    Serial.print(" ~ ");
    if (flameStatus) {
      Serial.print("ada api");
    } else {
      Serial.print("tidak ada api");
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Temp: %.2f", temp);
    lcd.print((char)223);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.printf("Gas : %d ppm", gas);

    Blynk.virtualWrite(V0, temp);
    Blynk.virtualWrite(V1, gas);
    Blynk.virtualWrite(V2, flameStatus);

    data["flame"] = flameStatus;
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
  Serial.println("Webhook:");
  Serial.println(param.asStr());
}

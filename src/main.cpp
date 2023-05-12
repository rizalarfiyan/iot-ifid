#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHT_SENSOR_PIN 15
#define DHT_SENSOR_TYPE DHT11

#define RED_LED 12
#define GREEN_LED 13
#define BUZZER 14

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  dht_sensor.begin();
}

void loop() {
  float humi  = dht_sensor.readHumidity();
  float temp = dht_sensor.readTemperature();

  if ( isnan(temp) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");
    Serial.print("  |  ");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println("°C");
  }

  delay(2000);
}

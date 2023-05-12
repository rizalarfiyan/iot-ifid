#include <Arduino.h>

#define ADC_VREF_mV 5000.0
#define ADC_RESOLUTION 4095.0
#define LM35_SENSOR 36
#define FLAME_SENSOR 4
#define GAS_SENSOR 15

#define RED_LED 12
#define GREEN_LED 13
#define BUZZER 14

void setup()
{
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(LM35_SENSOR, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(GAS_SENSOR, INPUT);
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

  if (flameRaw == HIGH) {
    Serial.println("ada api");
  } else {
    Serial.println("tidak ada api");
  }

  delay(1000);
}
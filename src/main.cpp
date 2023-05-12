#include <Arduino.h>

#define ADC_VREF_mV 5000.0
#define ADC_RESOLUTION 4095.0
#define LM35_SENSOR 36

#define RED_LED 12
#define GREEN_LED 13
#define BUZZER 14

void setup()
{
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop()
{
  int adcVal = analogRead(LM35_SENSOR);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float temp = milliVolt / 10;

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("°C");

  delay(1000);
}
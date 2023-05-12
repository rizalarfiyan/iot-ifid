#include <Arduino.h>

#define RED_LED 12
#define GREEN_LED 13
#define BUZZER 14

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(300);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
  delay(300);
}

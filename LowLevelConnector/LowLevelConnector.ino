#define PWM_PIN 3
#define MIN_PWM 0
#define MAX_PWM 255
#define DELAY_MS 50
#define BAUD_RATE 115200

#include "BlindConnector.h"

volatile int i = 0;
volatile int sign = 1;

BlindConnector blindConnector(PWM_PIN, A0, A1);

void setup() {
  Serial.begin(BAUD_RATE);
}

void loop() {
  blindConnector.writePWM(i);
  if (i >= MAX_PWM) sign = -1;
  else if (i <= MIN_PWM) sign = 1;
  i += sign;
  delay(DELAY_MS);
  Serial.println(i);
  Serial.println(blindConnector.readLuxAnalog());
  Serial.println(blindConnector.readBlindAnalog());
}

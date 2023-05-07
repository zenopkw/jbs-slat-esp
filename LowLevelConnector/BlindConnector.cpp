#include <Arduino.h>
#include "BlindConnector.h"

BlindConnector::BlindConnector(uint8_t writePin, uint8_t blindPin, uint8_t luxPin) {
  pinMode(writePin, OUTPUT);
  pinMode(blindPin, INPUT);
  pinMode(luxPin, INPUT);
  _writePin = writePin;
  _blindPin = blindPin;
  _luxPin = luxPin;
}
uint8_t BlindConnector::_clipPWM(uint8_t val) {
  return val < 0 ? 0 : val > 255 ? 255
                                 : val;
}
void BlindConnector::writePWM(uint8_t val) {
  int safeVal = _clipPWM(val);
  analogWrite(_writePin, safeVal);
}
uint16_t BlindConnector::readBlindAnalog() {
  return analogRead(_blindPin);
}
uint16_t BlindConnector::readLuxAnalog() {
  return analogRead(_luxPin);
}
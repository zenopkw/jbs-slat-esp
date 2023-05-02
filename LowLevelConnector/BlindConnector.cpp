#include <Arduino.h>
#include "BlindConnector.h"

BlindConnector::BlindConnector(int writePin, int blindPin, int luxPin) {
  pinMode(writePin, OUTPUT);
  pinMode(blindPin, INPUT);
  pinMode(luxPin, INPUT);
  _writePin = writePin;
  _blindPin = blindPin;
  _luxPin = luxPin;
}
int BlindConnector::_clipPWM(int val) {
  return val < 0 ? 0 : val > 255 ? 255
                                 : val;
}
void BlindConnector::writePWM(int val) {
  int safeVal = _clipPWM(val);
  analogWrite(_writePin, safeVal);
}
int BlindConnector::readBlindAnalog() {
  return analogRead(_blindPin);
}
int BlindConnector::readLuxAnalog() {
  return analogRead(_luxPin);
}
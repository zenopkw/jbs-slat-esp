#ifndef BlindConnector_h
#define BlindConnector_h
#include <Arduino.h>
class BlindConnector {
public:
  BlindConnector(int writePin, int blindPin, int luxPin);
  void writePWM(int val);
  int readBlindAnalog();
  int readLuxAnalog();
private:
  int _writePin;
  int _luxPin;
  int _blindPin;
  int _clipPWM(int val);
};
#endif
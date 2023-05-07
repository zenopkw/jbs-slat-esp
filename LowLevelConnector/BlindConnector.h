#ifndef BlindConnector_h
#define BlindConnector_h
#include <Arduino.h>
class BlindConnector {
public:
  BlindConnector(uint8_t writePin, uint8_t blindPin, uint8_t luxPin);
  void writePWM(uint8_t val);
  uint16_t readBlindAnalog();
  uint16_t readLuxAnalog();
private:
  uint8_t _writePin;
  uint8_t _luxPin;
  uint8_t _blindPin;
  uint8_t _clipPWM(uint8_t val);
};
#endif
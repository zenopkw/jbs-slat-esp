#define PWM_PIN 3
#define DELAY_MS 50
#define BAUD_RATE 115200
#define CMD_WRITE_BLIND 0x01
#define CMD_READ_BLIND 0x02
#define CMD_READ_LUX 0x03

#include "BlindConnector.h"
#include "CRC16.h"

CRC16 crc;
BlindConnector blindConnector(PWM_PIN, A0, A1);

uint8_t msg[5] = {};

void constructMessage(uint8_t *msg, uint8_t command, uint16_t value) {
  msg[0] = command;
  msg[1] = value & 0xFF;
  msg[2] = (value >> 8) & 0xFF;
  crc.reset();
  crc.add((uint8_t *)msg, 3);
  uint16_t crc_val = crc.getCRC();
  msg[3] = crc_val & 0xFF;
  msg[4] = (crc_val >> 8) & 0xFF;
}

void setup() {
  Serial.begin(BAUD_RATE);
}

void loop() {
  crc.reset();
  if (Serial.available() >= 3) {
    uint8_t command = Serial.read();
    crc.add(command);
    uint16_t crc_val;
    uint8_t buffer[2];
    if (command == CMD_WRITE_BLIND) {
      if (Serial.readBytes(buffer, 2) == 2) {
        uint8_t pwmValue = buffer[0];
        crc.add(pwmValue);
        crc.add(0x00);
        if (Serial.readBytes(buffer, 2) == 2) {
          crc_val = ((uint16_t)buffer[1] << 8) | buffer[0];
          uint16_t calculatedCrc = crc.getCRC();
          if (calculatedCrc == crc_val) {
            blindConnector.writePWM((uint8_t)pwmValue);
          }
        }
      }
    } else if (command == CMD_READ_BLIND) {
      if (Serial.readBytes(buffer, 2) == 2) {
        crc_val = ((uint16_t)buffer[1] << 8) | buffer[0];
        uint16_t calculatedCrc = crc.getCRC();
        if (calculatedCrc == crc_val) {
          uint16_t blindAnalog = blindConnector.readBlindAnalog();
          constructMessage(msg, command, blindAnalog);
          Serial.write(msg, sizeof(msg));
        }
      }
    } else if (command == CMD_READ_LUX) {
      if (Serial.readBytes(buffer, 2) == 2) {
        crc_val = ((uint16_t)buffer[1] << 8) | buffer[0];
        uint16_t calculatedCrc = crc.getCRC();
        if (calculatedCrc == crc_val) {
          uint8_t *msg;
          uint16_t luxAnalog = blindConnector.readLuxAnalog();
          constructMessage(msg, command, luxAnalog);
          Serial.write(msg, sizeof(msg));
        }
      }
    }
    Serial.flush();
  }
  delay(DELAY_MS);
}
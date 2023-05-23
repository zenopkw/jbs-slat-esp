#define PWM_PIN 3
#define DELAY_MS 50
#define BAUD_RATE 115200
#define CMD_WRITE_BLIND 0x01
#define CMD_READ_BLIND 0x02
#define CMD_READ_LUX 0x03
#define CRC16_POLICY 0xF0

#include "BlindConnector.h"
#include "CRC16.h"

CRC16 crc;
BlindConnector blindConnector(PWM_PIN, A0, A1);

void setup()
{
  Serial.begin(BAUD_RATE);
}

void loop()
{
  crc.reset();
  if (Serial.available() >= 6)
  {
    crc.reset();
    uint8_t command = 0x00;
    uint8_t numBytes = 0x00;
    uint8_t i = 0x00;
    uint8_t _msg[20] = {};
    while (Serial.available() > 0)
    {
      if (command == 0x00)
      {
        command = Serial.read();
        crc.add(command);
        if (command != CRC16_POLICY && command != CMD_WRITE_BLIND)
        {
          _msg[i++] = command;
        }
      }
      else if (numBytes == 0x00)
      {
        numBytes = Serial.read();
        crc.add(numBytes);
        if (command != CRC16_POLICY && command != CMD_WRITE_BLIND)
        {
          _msg[i++] = numBytes;
        }
      }
      else
      {
        uint8_t buffer[numBytes];
        Serial.readBytes(buffer, numBytes);
        if (command != CRC16_POLICY)
        {
          crc.add(buffer, numBytes);
          if (command == CMD_READ_BLIND)
          {
            uint16_t value = blindConnector.readBlindAnalog();
            _msg[i++] = value & 0xFF;
            _msg[i++] = (value >> 8) & 0xFF;
          }
          else if (command == CMD_READ_LUX)
          {
            uint16_t value = blindConnector.readLuxAnalog();
            _msg[i++] = value & 0xFF;
            _msg[i++] = (value >> 8) & 0xFF;
          }
          command = 0x00;
          numBytes = 0x00;
        }
        else
        {
          uint16_t crc_val = ((uint16_t)buffer[1] << 8) | buffer[0];
          uint16_t calculatedCrc = crc.getCRC();
          if (calculatedCrc == crc_val)
          {
            _msg[i++] = CRC16_POLICY;
            _msg[i++] = 0x02;
            crc.reset();
            crc.add((uint8_t *)_msg, i);
            uint16_t crc_val = crc.getCRC();
            _msg[i++] = crc_val & 0xFF;
            _msg[i++] = (crc_val >> 8) & 0xFF;
            Serial.write(_msg, sizeof(_msg));
          }
        }
      }
    }
  }
  while (Serial.available() > 0)
  {
    Serial.flush();
  }
  delay(DELAY_MS);
}
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "CRC16.h"

#include "ota_credentials.h"
#include "wifi_credentials.h"

#define CMD_WRITE_BLIND 0x01
#define CMD_READ_BLIND 0x02
#define CMD_READ_LUX 0x03
#define CRC16_POLICY 0xF0
#define MQTT_SERVER "192.168.1.52"
#define MQTT_PORT 1883
// #define MQTT_USERNAME "[USERNAME]"
// #define MQTT_PASSWORD "[PASSWORD]"
#define MQTT_NAME "esp8266"
#define MSG_BUFFER_SIZE 100
#define MQTT_TOPIC_SLAT "jbs/slat"
#define MQTT_TOPIC_READ "jbs/slat/read"
#define MQTT_TOPIC_FEEDBACK "jbs/slat/feedback"
#define MQTT_TOPIC_BLIND_SET "jbs/slat/blind/set"

const char *ssid = STASSID;
const char *password = STAPSK;
uint8_t msg[10] = {};

CRC16 crc;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length)
{
  if (strcmp(topic, MQTT_TOPIC_BLIND_SET) == 0)
  {
    payload[length] = '\0';
    byte pwmVal = atoi((char *)payload);
    constructMessage(msg, CMD_WRITE_BLIND, pwmVal);
    Serial.write(msg, sizeof(msg));
    char mqttMsg[MSG_BUFFER_SIZE];
    snprintf(mqttMsg, MSG_BUFFER_SIZE, "{\"blind\": \"%ld\", \"illuminance\": \"%ld\"}", 1, 1);
    client.publish(MQTT_TOPIC_FEEDBACK, mqttMsg);
  }
  else if (strcmp(topic, MQTT_TOPIC_READ) == 0)
  {
    payload[length] = '\0';
    byte command = atoi((char *)payload);
    if (command == 2)
    {
      constructMessage(msg, CMD_READ_BLIND, 0);
      Serial.write(msg, sizeof(msg));
    }
    else if (command == 3)
    {
      constructMessage(msg, CMD_READ_LUX, 0);
      Serial.write(msg, sizeof(msg));
    }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(MQTT_TOPIC_SLAT, "{\"status\":\"active\"}");
      // ... and resubscribe
      client.subscribe(MQTT_TOPIC_BLIND_SET);
      client.subscribe(MQTT_TOPIC_READ);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void wifiSetup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setPort(OTAPORT);
  ArduinoOTA.setHostname(OTAHOST);
  ArduinoOTA.setPassword(OTAPASS);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void constructMessage(uint8_t *msg, uint8_t command, uint16_t value)
{
  msg[0] = command;
  msg[1] = 0x02;
  msg[2] = value & 0xFF;
  msg[3] = (value >> 8) & 0xFF;
  msg[4] = CRC16_POLICY;
  msg[5] = 0x02;
  crc.reset();
  crc.add((uint8_t *)msg, 6);
  uint16_t crc_val = crc.getCRC();
  msg[6] = crc_val & 0xFF;
  msg[7] = (crc_val >> 8) & 0xFF;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");
  wifiSetup();

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

void loop()
{
  ArduinoOTA.handle();
  if (!client.connected())
  {
    reconnect();
  }
  if (Serial.available() > 3)
  {
    delay(10);
    crc.reset();
    uint8_t command = 0x00;
    uint8_t numBytes = 0x00;
    uint16_t blind_val = 0x00;
    uint16_t lux_val = 0x00;
    while (Serial.available() > 0)
    {
      if (command == 0x00)
      {
        command = Serial.read();
        crc.add(command);
      }
      else if (numBytes == 0x00)
      {
        numBytes = Serial.read();
        crc.add(numBytes);
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
            blind_val = ((uint16_t)buffer[1] << 8) | buffer[0];
          }
          else if (command == CMD_READ_LUX)
          {
            lux_val = ((uint16_t)buffer[1] << 8) | buffer[0];
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
            char mqttMsg[MSG_BUFFER_SIZE];
            if (blind_val && lux_val)
            {
              snprintf(mqttMsg, MSG_BUFFER_SIZE, "{\"blind\": \"%ld\", \"illuminance\": \"%ld\"}", blind_val, lux_val);
            }
            else if (blind_val)
            {
              snprintf(mqttMsg, MSG_BUFFER_SIZE, "{\"blind\": \"%ld\"}", blind_val);
            }
            else if (lux_val)
            {
              snprintf(mqttMsg, MSG_BUFFER_SIZE, "{\"illuminance\": \"%ld\"}", lux_val);
            }
            client.publish(MQTT_TOPIC_FEEDBACK, mqttMsg);
          }
        }
      }
    }
    Serial.flush();
  }
  client.loop();
}
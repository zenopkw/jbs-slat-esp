arduino-cli compile -b esp8266:esp8266:generic -e HighLevelConnector
python espota.py -d -i 192.168.1.73 -a jgsee -f HighLevelConnector/build/esp8266.esp8266.generic/HighLevelConnector.ino.bin
arduino-cli compile -b esp8266:esp8266:generic -e BasicOTA
python espota.py -d -i 192.168.1.70 -a jgsee -f BasicOTA/build/esp8266.esp8266.generic/BasicOTA.ino.bin
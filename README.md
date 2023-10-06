## ESP8266 Gosund Switch
This is a simple firmware for the ESP8266 based Gosund SP111. The firmware includes automatically opening a hotspot to allow configuration of WiFi Credentials and MQTT Broker.  
After configuring the plug can be controlled with a MQTT message with the payload `{"state": false}` to turn off the plug and `{"state": true}` to turn it on.

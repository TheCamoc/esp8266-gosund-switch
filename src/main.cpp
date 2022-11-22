#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESPTools.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void reconnect()
{
    Serial.print("Attempting MQTT connection... ");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    client.setServer(ESPTools.config["mqtt_server"].c_str(), 1883);
    if (client.connect(clientId.c_str()))
    {
        Serial.println("connected");
        if (!ESPTools.config["mqtt_topic"].equals("")) {
            client.subscribe(ESPTools.config["mqtt_topic"].c_str());
        }
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" trying again");
        delay(100);
    }
}

void onMessage(char *topic, byte *payload, unsigned int length)
{
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (char *)payload);

    if (doc["state"])
    {
        digitalWrite(15, HIGH);
    }
    else
    {
        digitalWrite(15, LOW);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("");

    ESPTools.begin(&server);
    ESPTools.wifiAutoConnect();
    ESPTools.setupHTTPUpdates();
    ESPTools.addConfigString("mqtt_server");
    ESPTools.addConfigString("mqtt_topic");

    server.on("/restart", [&]() {
        server.send(200, "text/plain", "Ok");
        delay(500);
        ESP.restart();
    });

    server.begin();

    // Setup MQTT
    reconnect();
    client.setCallback(onMessage);

    // Relay
    pinMode(15, OUTPUT);
}

void loop()
{
    server.handleClient();
    if (!client.connected() && ESPTools.config["mqtt_server"] != "")
    {
        reconnect();
    }
    client.loop();
}
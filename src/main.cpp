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
        client.subscribe(ESPTools.config["mqtt_topic"].c_str());
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
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
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

void resetWifi()
{
    server.send(200, "text/plain", "done");
    delay(100);
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(200);
    ESP.restart();
}

void setup()
{
    // WiFi, WebServer and OTA setup
    Serial.begin(115200);
    Serial.println();
    delay(1000);

    ESPTools.begin(&server);

    ESPTools.addConfigString("hostname");
    ESPTools.addConfigString("mqtt_server");
    ESPTools.addConfigString("mqtt_topic");

    WiFi.mode(WIFI_STA);
    WiFi.hostname(ESPTools.config["hostname"].c_str());

    WiFiManager wifiManager;
    Serial.println("");

    if (!wifiManager.autoConnect())
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/reset_wifi", resetWifi);
    server.on("/restart", [&]() {
        server.send(200, "text/plain", "Ok");
        delay(500);
        ESP.restart();
    });
    // OTA Stuff
    httpUpdater.setup(&server);
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
#include <Arduino.h>
#include <Ticker.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "secrets.h"

Ticker t1;
Ticker t2;
WiFiClient client;

void t1sr()
{
    digitalWrite(D1, !digitalRead(D1));
}
void t2sr()
{
    digitalWrite(D2, !digitalRead(D2));
    digitalWrite(D8, !digitalRead(D8));
}

void setupWiFi()
{
    WiFi.begin(ssid, password);
    WiFi.hostname(hostname);

    Serial.print("Connecting ");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    String s;

    s = "Connected\n IP address: ";
    s += WiFi.localIP().toString();
    s += "\n Hostname: ";
    s += WiFi.hostname();

    Serial.println(s);
}

void connect()
{
    const char* host = "rwmmvdid1hvmqy6p.myfritz.net";
    int port(1130);
    Serial.printf("\n[Connecting to %s ... ", host);
    if (client.connect(host, port)) {
        Serial.println("connected]");
        Serial.println("[Sending a request]");
        client.print(String(R"(GET /?o={"command":"ticket"})") + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n" + "\r\n");
        Serial.println("[Response:]");
        while (client.connected() || client.available()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
        }
        client.stop();
        Serial.println("\n[Disconnected]");
    } else {
        Serial.println("connection failed!]");
        client.stop();
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D8, OUTPUT);

    t1.attach(0.7, t1sr);
    t2.attach(0.3, t2sr);

    Serial.printf("Hallo...");

    setupWiFi();
}

void loop()
{
    connect();
    delay(5000);
}

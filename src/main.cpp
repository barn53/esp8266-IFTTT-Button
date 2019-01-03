#include <Arduino.h>
#include <Ticker.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#include "secrets.h"

Ticker t1;

void t1sr()
{
    digitalWrite(D1, !digitalRead(D1));
    digitalWrite(D2, !digitalRead(D1));
}

ADC_MODE(ADC_VCC);

HTTPClient http;

void startRoutine()
{
    t1.attach(0.05, t1sr);
}

void offRoutine(bool ok)
{
    t1.detach();
    if (ok) {
        digitalWrite(D1, HIGH);
        digitalWrite(D2, LOW);
    } else {
        digitalWrite(D1, LOW);
        digitalWrite(D2, HIGH);
    }

    delay(2000);

    digitalWrite(D7, HIGH); // off
    digitalWrite(D8, HIGH); // off

    // not reached, when hardware switches off, but otherwise...
    Serial.println("ESP.restart()");
    ESP.restart();
}

void setupWiFi()
{
    WiFi.begin(ssid, password);
    WiFi.hostname(hostname);

    Serial.print("Connecting ");
    int counter(0);
    bool ok(true);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        if (counter > 50) {
            ok = false;
            break;
        }
        ++counter;
        Serial.print(".");
    }

    String s;
    if (ok) {
        s = "Connected\n IP address: ";
        s += WiFi.localIP().toString();
        s += "\n Hostname: ";
        s += WiFi.hostname();
    } else {
        s = "Connection failed!";
        Serial.println(s);

        offRoutine(ok);
    }

    Serial.println(s);
}

bool send(const String& json, String& response)
{
    response = "";
    bool ret(false);
    String url;
    url = "/?o=";
    url += json;
    Serial.println(url);

    http.begin(host, port, url);
    int httpCode = http.GET();
    if (httpCode) {
        if (httpCode == 200) {
            response = http.getString();
            ret = true;
        }
    }
    http.end();
    return ret;
}

bool getTicket(String& ticket)
{
    String chipID(ESP.getChipId());
    String vcc(ESP.getVcc() / 1024.f);
    String json(R"({"command":"ticket"})");

    return send(json, ticket);
}

bool command(const String& ticket, const String& cmd, String& response)
{
    String chipID(ESP.getChipId());
    String vcc(ESP.getVcc() / 1024.f);

    String json(R"({"command":")");
    json += cmd;
    json += R"(","ticket":")";
    json += ticket;
    json += R"(","user":")";
    json += user;
    json += R"(","vcc":)";
    json += vcc;
    json += R"(,"chip-id":")";
    json += chipID;
    json += R"(","latitude":)";
    json += latitude;
    json += R"(,"longitude":)";
    json += longitude;
    json += "}";

    return send(json, response);
}

void setup()
{
    pinMode(D8, OUTPUT); // GPIO15, off
    pinMode(D7, OUTPUT); // GPIO13, off
    pinMode(D1, OUTPUT); // GPIO5, ok
    pinMode(D2, OUTPUT); // GPIO4, fail

    digitalWrite(D8, LOW);
    digitalWrite(D7, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);

    Serial.begin(115200);
    Serial.println("Hallo...");

    startRoutine();

    setupWiFi();
}

void loop()
{
    String ticket;
    String response;
    bool ok(false);

    if (getTicket(ticket)) {
        Serial.println(ticket);
        if (command(ticket, "door", response)) {
            Serial.println(response);
            ok = true;
        }
    }

    offRoutine(ok);
}

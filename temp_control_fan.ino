#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <TridentTD_LineNotify.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
// Set IFTTT Webhooks event name and key

#define relayPin 5
#define Line_Token "-"
#define port 80
#define host "api.thingspeak.com"

String GET = "GET /update?key=-";
const char *server = "api.thingspeak.com";

DHT dht(4, DHT22);
int count = 0;
bool check = true;
bool temp_check;
String formattedDate;
String dayStamp;
String timeStamp;

WiFiUDP wifiudp;
NTPClient timeClient(wifiudp);
WiFiClient client;

void return_temp(void *parameter)
{
    float t = dht.readTemperature();
    LINE.notify("目前溫度:" + String(t));

    delay(10000);
}

void setup()
{
    Serial.begin(9600);
    dht.begin();
    WiFi.begin("-", "-");
    Serial.println(WiFi.localIP());
    pinMode(relayPin, OUTPUT);
    pinMode(0, OUTPUT);
    timeClient.begin();
    timeClient.setTimeOffset(28800);
}

void loop()
{
    while (!timeClient.update())
    {
        timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    LINE.setToken(Line_Token);

    float t = dht.readTemperature();
    Serial.println(t);
    temp_check = false;
    if (t > 26 || temp_check == true)
    {
        digitalWrite(relayPin, LOW);
        temp_check = true;
        if (check)
        {
            LINE.notify("電源開啟");
            check = false;
        }
    }
    else
    {
        if (t <= 25.2)
        {
            temp_check = false;
            digitalWrite(relayPin, HIGH);
            if (check == false)
            {
                LINE.notify("電源關閉");
                check = true;
            }
        }
    }
    count++;
    delay(1000);

    if (count == 60)
    {
        char c[10];
        char day[100];
        char time[100];
        sprintf(day, "%s", dayStamp);
        sprintf(time, "%s", timeStamp);
        sprintf(c, "%.1f", t);

        if (!client.connect(host, port))
        {
            Serial.println("connection failed");
            return;
        }
        else
        {
            String getStr = GET + "&field1=" + c +
                            " HTTP/1.1\r\n";
            client.print(getStr);
            client.print("Host: api.thingspeak.com\n");
            client.print("Connection: close\r\n\r\n");

            LINE.notify("目前溫度:" + String(t));
            count = 0;

            client.stop();
        }
    }
}
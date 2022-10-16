// WIFI
#include <WiFi.h>
// MQTT
#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 1844
// user Setting
#include "esp32LED.h"

// MQTT Setting
WiFiClient espClient;
PubSubClient client(espClient);

// LED define
const byte LED_A = 21;
const byte LED_B = 16;
const byte LED_C = 17;

void connectWiFi()
{
    WiFi.mode(WIFI_STA); // setting wifi -> STA model

    WiFi.begin(ssid, pwd);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
}

void connectMQTT()
{
    Serial.println("Connecting to MQTT...");
    if (client.connect(EspName))
    {
        client.subscribe(mqtt_subscribe);
        Serial.println("connected");
    }
    else
    {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
}
void setup()
{
    Serial.begin(115200);
    pinMode(LED_A, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_C, OUTPUT);

    client.setServer(mqttServer, mqttPort);
    client.setCallback(receiveCallback);
}

// 收到信息后的回调函数
void receiveCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("[");
    Serial.print(topic);
    Serial.print("]: ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println("");
    Serial.print("Message Length(Bytes) ");
    Serial.println(length);
    switch ((char)payload[0])
    {
    case 'A':
        digitalWrite(LED_A, HIGH);
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_C, LOW);
        Serial.println("LED_A ON");
        break;

    case 'B':
        digitalWrite(LED_A, LOW);
        digitalWrite(LED_B, HIGH);
        digitalWrite(LED_C, LOW);
        Serial.println("LED_B ON");
        break;

    case 'C':
        digitalWrite(LED_A, LOW);
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_C, HIGH);
        Serial.println("LED_C ON");
        break;
    case 'R':
        digitalWrite(LED_A, LOW);
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_C, LOW);
        Serial.println("LED_C ON");
        break;
    default:
        digitalWrite(LED_A, LOW);
        digitalWrite(LED_B, LOW);
        digitalWrite(LED_C, LOW);
        break;
    }
}

void loop()
{
    // WIFI
    while (WiFi.status() != WL_CONNECTED)
    {
        connectWiFi();
    }

    // MQTT
    while (!client.connected())
    {
        connectMQTT();
    }
    if (client.connected())
    {
        client.loop();
    }
}

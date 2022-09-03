// Bluetooth BLE
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>
// WIFI
#include <WiFi.h>
// MQTT
#include <PubSubClient.h>

/*
########################################
Frist Edit BLEBeacon.cpp and BLEBeacon.h
########################################
*/

BLEScan *pBLEScan;

// Wifi Setting
const char ssid[] = "yoyoyoyo";
const char pwd[] = "00000001";

// MQTT Setting
WiFiClient espClient;
PubSubClient client(espClient);
const char mqttServer[] = "192.168.0.178";
const int mqttPort = 1883; // PubSubClient doesn't support websockets. #1884
const char topic[] = "indoor/";
String msgStr = "";
char json[30];

// Phone original UUID 2eadb97e-1dd2-11b2-8000-080027b246c5
const String uuid = "b2270008-0080-b211-d21d-7eb9ad2e1502";

// Scan Beacon Time
const int BeaconScanTime = 5000;
const int ScanTime = 1000;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // new BLEBeacon
        BLEBeacon id;
        uint8_t *payload = advertisedDevice.getPayload();
        id.setData(payload);

        String m_uuid = id.getProximityUUID().to128().toString().c_str();

        int m_rssi = advertisedDevice.haveRSSI() ? advertisedDevice.getRSSI() : 0;
        String m_macaddress = advertisedDevice.getAddress().toString().c_str();

        if (m_uuid == uuid)
        {
            Serial.print("MacAddress:");
            Serial.println(m_macaddress);
            Serial.print("RSSI:");
            Serial.println(m_rssi);
        }
    }
};

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

void ScanBeacon()
{
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);

    Serial.println("Scanning...");
    BLEScanResults foundDevices = pBLEScan->start(BeaconScanTime);
    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

    Serial.print("Wait a ");
    Serial.print(ScanTime / 1000);
    Serial.println(" second.");
    delay(ScanTime);
}

void connectMQTT()
{
    client.setServer(mqttServer, mqttPort);
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Clientmqtt"))
    {
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
    BLEDevice::init("ESP32_School");
}

void loop()
{
    // BLE
    // ScanBeacon();

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
    client.loop();

    // 建立MQTT訊息（JSON格式的字串）
    msgStr = msgStr + "{\"temp\":" + (19 + random(10)) + ",\"humid\":" + 20 + "}";
    // 把String字串轉換成字元陣列格式
    msgStr.toCharArray(json, 30);
    // 發布MQTT主題與訊息
    client.publish(topic, json);
    // 清空MQTT訊息內容
    msgStr = "";
}

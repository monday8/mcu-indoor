// Bluetooth BLE
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>
// WIFI
#include <WiFi.h>
// MQTT
#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 1844
#define DataNum 5

#include "user.h"
//#include"user_example.h"

//#include "esp1.h"
//#include "esp2.h"
#include "esp3.h"

// uint16 轉int
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))
// https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/BLE_Beacon_Scanner/BLE_Beacon_Scanner.ino

/*
########################################
Frist Edit BLEBeacon.cpp and BLEBeacon.h
########################################
*/
//宣告任務Task1
TaskHandle_t Task1;

BLEScan *pBLEScan;

// MQTT Setting
WiFiClient espClient;
PubSubClient client(espClient);

String msgStr = "";
char JSON[MQTT_MAX_PACKET_SIZE];

typedef struct
{
    int major;
    int minor;
    int rssi;
} BeaconData;

BeaconData m_beacondata[DataNum];
uint8_t beacondataIndex = 0;

// Phone original UUID 2eadb97e-1dd2-11b2-8000-080027b246c5
const String uuid = "2eadb97e-1dd2-11b2-8000-080027b246c5";
// const String uuid = "b2270008-0080-b211-d21d-7eb9ad2e1502"; //old

// Scan Beacon Time
const int BeaconScanTime = 5; // s
const int ScanTime = 500;     // ms
boolean mqttgo = false;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // new BLEBeacon
        BLEBeacon id;
        uint8_t *payload = advertisedDevice.getPayload();
        id.setData(payload);

        //取得UUID，Major，Minor
        String m_uuid = id.getProximityUUID().toString().c_str();

        if (m_uuid == uuid)
        {
            int m_major = ENDIAN_CHANGE_U16(id.getMajor());
            int m_minor = ENDIAN_CHANGE_U16(id.getMinor());

            int m_rssi = advertisedDevice.haveRSSI() ? advertisedDevice.getRSSI() : 0;
            Serial.println(m_rssi);
            if (beacondataIndex > DataNum) // DataNum define
            {
                beacondataIndex = 0;
            }
            else
            {
                beacondataIndex++;
            }
            m_beacondata[beacondataIndex].major = m_major;
            m_beacondata[beacondataIndex].minor = m_minor;
            m_beacondata[beacondataIndex].rssi = m_rssi;
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

void connectMQTT()
{
    client.setServer(mqttServer, mqttPort);
    Serial.println("Connecting to MQTT...");
    if (client.connect(EspName))
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

void ScanBeacon()
{
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);

    Serial.println("Scanning...");
    BLEScanResults foundDevices = pBLEScan->start(BeaconScanTime);
    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

    Serial.print("Wait a ");
    Serial.print(ScanTime);
    Serial.println(" ms.");
    delay(ScanTime);
}

void setup()
{
    Serial.begin(115200);
    BLEDevice::init("ESP32_School");

    //在核心1啟動任務1
    xTaskCreatePinnedToCore(
        Task_scanbeacon, /*任務實際對應的Function*/
        "Task1",         /*任務名稱*/
        10000,           /*堆疊空間*/
        NULL,            /*無輸入值*/
        0,               /*優先序0*/
        &Task1,          /*對應的任務變數位址*/
        1);              /*指定在核心1執行 */
}

void Task_scanbeacon(void *pvParameters)
{

    //無窮迴圈
    while (1)
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
        //發布MQTT主題與訊息
        msgStr = msgStr + "{\"station\":\"";
        msgStr = msgStr + device + "\",\"info\":[";
        for (int i = 0; i < DataNum; i++)
        {
            // {"mac":"test1","rssi":10}, //m:mac,r:rssi
            msgStr = msgStr + "{\"major\":\"" + m_beacondata[i].major + "\",\"minor\":\"" + m_beacondata[i].minor + "\",\"rssi\":" + m_beacondata[i].rssi + "}";
            if (i < DataNum - 1)
            {
                msgStr += ',';
            }
        }
        msgStr = msgStr + "]}";

        // Serial.println(msgStr);

        // 把String字串轉換成字元陣列格式
        // 建立MQTT訊息（JSON格式的字串）
        msgStr.toCharArray(JSON, msgStr.length() + 1);
        client.loop();
        client.publish(topic, JSON);

        // 清空MQTT訊息內容
        msgStr = "";
        // for (int i = 0; i < DataNum; i++)
        // {
        //    m_beacondata[i].major=NULL;
        //    m_beacondata[i].minor=NULL;
        //    m_beacondata[i].rssi=NULL;
        // }
        delay(2000);
    }
}

void loop()
{
    // BLE
    ScanBeacon();
}

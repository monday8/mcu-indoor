//include
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include<BLEBeacon.h>
#include <WiFi.h>


BLEScan *pBLEScan;



int scanTime = 5; //In seconds
const char ssid[]="yoyoyoyo";
const char pwd[]="00000001";



class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        String MacUUID = advertisedDevice.getServiceUUID().toString().c_str();

        //取得附近設備UUID
        int MabRSSI = advertisedDevice.getRSSI();

        //偵測UUID是否正確
        if (MacUUID == "2eadb97e-1dd2-11b2-8000-080027b246c3")
        {
            String MacAddress = advertisedDevice.getAddress().toString().c_str();
            String MacName = advertisedDevice.getName().c_str();

            Serial.print("Name:");
            Serial.println(MacName);
            Serial.print("RSSI:");
            Serial.println(MabRSSI);
            Serial.print("Address:");
            Serial.println(MacAddress);
        }
    }
};

void connectWiFi()
{
    WiFi.mode(WIFI_STA); //setting wifi -> STA model

    WiFi.begin(ssid,pwd);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
}

void ScanBle()
{
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);

    Serial.println("Scanning...");
    BLEScanResults foundDevices = pBLEScan->start(scanTime);
    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
    Serial.println("Wait a two second.");
    delay(2000);
}


void setup()
{
    Serial.begin(115200);
    BLEDevice::init("ESP32_School");
    String hostname= "ESP32_school";
    WiFi.setHostname(hostname.c_str()); //define hostname
    connectWiFi();
}

void loop()
{
    //btStart();
    //ScanBle();
    //btStop();
}

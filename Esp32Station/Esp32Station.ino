// include
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEBeacon.h>
#include <WiFi.h>

BLEScan *pBLEScan;

int BeaconScanTime = 2000; // In seconds
const char ssid[] = "yoyoyoyo";
const char pwd[] = "00000001";

//original UUID 2eadb97e-1dd2-11b2-8000-080027b246c5
const String uuid = "b2270008-0080-b211-d21d-7eb9ad2e1502";

const int ScanTime = 2000;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //new BLEBeacon
        BLEBeacon id;
        uint8_t *payload = advertisedDevice.getPayload();
        id.setData(payload);

        String m_uuid = id.getProximityUUID().to128().toString().c_str();
        
        int m_rssi = advertisedDevice.getRSSI();
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
    Serial.print(ScanTime/1000);
    Serial.println(" second.");
    delay(ScanTime);
}

void setup()
{
    Serial.begin(115200);
    BLEDevice::init("ESP32_School");
    // String hostname = "ESP32_school";
    // WiFi.setHostname(hostname.c_str()); // define hostname
    // connectWiFi();
}

void loop()
{
    ScanBeacon();
    // while (WiFi.status() != WL_CONNECTED){
    //     connectWiFi();
    // }
}

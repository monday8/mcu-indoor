# edit
### References
https://github.com/nkolban/esp32-snippets/issues/935

#### edit BLEBeacon.cpp
#### BLEBeacon::setData(std::string data) -> BLEBeacon::setData(uint8_t* data)

```
void BLEBeacon::setData(std::string data) {
	if (data.length() != sizeof(m_beaconData)) {
		log_e("Unable to set the data ... length passed in was %d and expected %d", data.length(), sizeof(m_beaconData));
		return;
	}
	memcpy(&m_beaconData, data.data(), sizeof(m_beaconData));
} 

void BLEBeacon::setData(uint8_t* data) {
	memcpy(&m_beaconData, data.data(), sizeof(m_beaconData));
}
________________________________________________________________________

BLEUUID BLEBeacon::getProximityUUID() {
	return BLEUUID(m_beaconData.proximityUUID, 16, false);
}

BLEUUID BLEBeacon::getProximityUUID() {
	return BLEUUID(m_beaconData.proximityUUID, 16, true);
}

```
#### edit BLEBeacon.h
#### setData(std::string data) -> setData(uint8_t* data)

```
void        setData(std::string data);

void        setData(uint8_t* data);
```

#### example
```
BLEBeacon id;
uint8_t *payload = advertisedDevice.getPayload();
id.setData(payload);

String m_uuid = id.getProximityUUID().toString().c_str();

```

## esp32 MQTT Setting
### serve ip from computer "cmd->ipconfig ipv4 address"

## user_example.h Required
## Version
v1.1.0 (28 Oktober 2025)

## Board
ESP32 Devkit Board v3.3.1

## Library
- SynapsisLibrary
- Adafruit_ADS1X15
- ArduinoJson
- WebServer
- SPIFFS
- WiFi

## Feature
- Read 1 Sensor AC Voltage (ZMPT101B) via ADS1115
- Read 6 Sensor AC Current (SCT-013 1V/10A) via ADS1115
- Dual Mode Wireless Network : Access Point and Station (DHCP Only)
- HTTP API Configuration Interface
  1. Get Network Information -> GET /api/get/network/info
  2. Get Sensor Data -> GET /api/get/sensor/data
  3. Get Sensor Calibration -> GET /api/get/sensor/calibration
  4. Set Network Wireless Station -> POST /api/set/network/station
  5. Set Sensor AC Current Calibration -> POST /api/set/calibration/voltage
  6. Set Sensor AC Voltage Calibration -> POST /api/set/calibration/current

## Default 
- Network
  -  Station : SSID "Synapsis.id Head Office", Password "synaps1sjkt23@", DHCP
  -  Access Point : SSID "PanelSensor Hotspot", Password "12345678"
- Port Sensor
  1. AC Voltage : ADS1115 0x49 Channel 2 -> ZMPT101B
  2. AC Current Port 1 : ADS1115 0x48 Channel 0 -> SCT-013 1V/10A
  3. AC Current Port 2 : ADS1115 0x48 Channel 1 -> SCT-013 1V/10A
  4. AC Current Port 3 : ADS1115 0x48 Channel 2 -> SCT-013 1V/10A
  5. AC Current Port 4 : ADS1115 0x48 Channel 3 -> SCT-013 1V/10A
  6. AC Current Port 5 : ADS1115 0x49 Channel 0 -> SCT-013 1V/10A
  7. AC Current Port 6 : ADS1115 0x49 Channel 1 -> SCT-013 1V/10A

# API Documentation - Dual Server Sensor Panel Extension

## Overview
Sistem ini mendukung dual server:
- **WiFi WebServer** pada port 8000
- **EthernetServer (W5500)** pada port 80

## API Endpoints

### 1. Network Information
**GET** `/api/get/network/info`
- **WiFi**: `http://[IP_WIFI]:8000/api/get/network/info`
- **Ethernet**: `http://[IP_ETHERNET]:80/api/get/network/info`

**Response:**
```json
{
  "status": "Success",
  "message": "Request Network Information Successfully",
  "data": {
    "wireless": {
      "mode": "Dual Mode",
      "ap": {
        "mac": "MAC_ADDR_AP",
        "ip": "IP_ADDR_AP",
        "ssid": "PanelSensor Hotspot",
        "password": "12345678"
      },
      "station": {
        "mac": "MAC_ADDR_STA",
        "ip": "IP_ADDR",
        "ssid": "SSID_STA",
        "password": "PASS_STA",
        "rssi": "WIFI_RSSI",
        "state": "WIFI_STATUS",
        "configuration": {
          "mode": "DHCP",
          "ip": "IP_ADDR",
          "subnet": "255.255.254.0",
          "gateway": "10.19.51.1",
          "dns": "10.19.51.1"
        }
      }
    },
    "ethernet": {
      "mode": "DHCP",
      "mac": "DE:AD:BE:EF:FE:ED",
      "ip": "192.168.1.100",
      "subnet": "255.255.255.0",
      "gateway": "192.168.1.1",
      "dns": "8.8.8.8"
    }
  }
}
```

### 2. Sensor Data
**GET** `/api/get/sensor/data`
- **WiFi**: `http://[IP_WIFI]:8000/api/get/sensor/data`
- **Ethernet**: `http://[IP_ETHERNET]:80/api/get/sensor/data`

**Response:**
```json
{
  "status": "Success",
  "message": "Request Sensor Information Successfully",
  "data": {
    "tegangan": 220.5,
    "current": {
      "port1": 1.2,
      "port2": 0.8,
      "port3": 2.1,
      "port4": 0.5,
      "port5": 1.8,
      "port6": 0.3
    },
    "power": {
      "port1": 264.6,
      "port2": 176.4,
      "port3": 463.05,
      "port4": 110.25,
      "port5": 396.9,
      "port6": 66.15
    }
  }
}
```

### 3. Sensor Calibration
**GET** `/api/get/sensor/calibration`
- **WiFi**: `http://[IP_WIFI]:8000/api/get/sensor/calibration`
- **Ethernet**: `http://[IP_ETHERNET]:80/api/get/sensor/calibration`

**Response:**
```json
{
  "status": "Success",
  "message": "Request Calibrate Information Successfully",
  "data": {
    "tegangan": 1.0,
    "arus": {
      "port1": 0.1,
      "port2": 0.2,
      "port3": 0.3,
      "port4": 0.4,
      "port5": 0.5,
      "port6": 0.6
    }
  }
}
```

### 4. Set Network Station
**POST** `/api/set/network/station`
- **WiFi**: `http://[IP_WIFI]:8000/api/set/network/station`
- **Ethernet**: `http://[IP_ETHERNET]:80/api/set/network/station`

**Request Body:**
```json
{
  "ssid": "YourWiFiSSID",
  "password": "YourWiFiPassword"
}
```

**Response:**
```json
{
  "status": "Success",
  "message": "Setting Station Network Successfully",
  "data": {
    "ssid": "YourWiFiSSID",
    "password": "YourWiFiPassword"
  }
}
```

### 5. Set Voltage Calibration
**POST** `/api/set/calibration/voltage`
- **WiFi**: `http://[IP_WIFI]:8000/api/set/calibration/voltage`
- **Ethernet**: `http://[IP_ETHERNET]:80/api/set/calibration/voltage`

**Request Body:**
```json
{
  "value": "1.5"
}
```

**Response:**
```json
{
  "status": "Success",
  "message": "Setting Sensitivity Voltage Successfully!",
  "data": {
    "tegangan": "1.5"
  }
}
```

### 6. Set Current Calibration
**POST** `/api/set/calibration/current`
- **WiFi**: `http://[IP_WIFI]:8000/api/set/calibration/current`
- **Ethernet**: `http://[IP_ETHERNET]:80/api/set/calibration/current`

**Request Body:**
```json
{
  "port1": "0.1",
  "port2": "0.2",
  "port3": "0.3",
  "port4": "0.4",
  "port5": "0.5",
  "port6": "0.6"
}
```

**Response:**
```json
{
  "status": "Success",
  "message": "Setting Calibration Offset Successfully",
  "data": {
    "port1": "0.1",
    "port2": "0.2",
    "port3": "0.3",
    "port4": "0.4",
    "port5": "0.5",
    "port6": "0.6"
  }
}
```

## Hardware Configuration

### Ethernet (W5500)
- **CS Pin**: 2
- **MAC Address**: DE:AD:BE:EF:FE:ED
- **Default IP**: 192.168.1.100 (DHCP)
- **Fallback IP**: 192.168.1.28 (Static)

### WiFi
- **AP Mode**: PanelSensor Hotspot (12345678)
- **Station Mode**: Configurable via API
- **Port**: 8000

## Error Handling
Semua endpoint mengembalikan response dengan format:
```json
{
  "status": "Failed",
  "message": "Error description",
  "data": {
    "field": "value"
  }
}
```

## Testing
1. **WiFi Server**: Akses melalui `http://[IP_WIFI]:8000/api/get/network/info`
2. **Ethernet Server**: Akses melalui `http://[IP_ETHERNET]:80/api/get/network/info`
3. Kedua server akan mengembalikan response yang sama untuk endpoint yang sama

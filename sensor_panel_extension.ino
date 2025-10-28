/*
 * Sensor Panel Extension - Main File
 * Clean version with only setup() and loop()
 */

#include <ArduinoJson.h>
#include <SynapsisLibrary.h>
#include <WebServer.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>
#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>

// Pin untuk Ethernet (W5500)
#define ETH_CS_PIN 2
#define WIFI_WEB_SERVER_PORT 8000
#define ETHERNET_WEB_SERVER_PORT 80
#define ETH_MAC_ADDRESS "DE:AD:BE:EF:FE:ED"

Connection Connections;
String HOSTNAME = "PanelSensor Device";
String SSID_AP     = "PanelSensor Hotspot";
String PASS_AP = "12345678";
String SSID_STA = "Synapsis.id Head Office";
String PASS_STA = "synaps1sjkt23@";
String IP_ADDR, IP_ADDR_AP, MAC_ADDR_STA, MAC_ADDR_AP, WIFI_RSSI;
String WIFI_STATUS;
int dual_wifi = 0; int trigger = 0; int readStatus;

// Ethernet configuration
EthernetServer ethernetServer(ETHERNET_WEB_SERVER_PORT);
String lastUnifiedResponse = "";

Adafruit_ADS1115 SENSOR1_ADS;
Adafruit_ADS1115 SENSOR2_ADS;

// External sensor status variables
extern bool sensor1_connected;
extern bool sensor2_connected;

// Forward declarations
void fromStringMacAddress(const String& macStr, uint8_t* mac);
String getWiFiStatusDescription(int status);
bool testWiFiHardware();
void printAccessInstructions();

// Connection state tracking
bool lastEthernetState = false;
bool lastWiFiState = false;
unsigned long lastConnectionCheck = 0;

#define ADC_BITS    16
#define ADC_COUNTS  (1<<ADC_BITS)
#define ADC_SCALE   32767.0
#define V_REF       5.0

const float ADC_SCALE_FACTOR = 0.125;
const int SAMPLE_COUNT_VOLT  = 100;
const int SAMPLE_COUNT_CURR  = 100;
const uint16_t FREQUENCY     = 50;
float SENSITIVITY;
float VOLT_READ;

uint32_t periode;
double currentSamples;
double sqCurrent;
double currentFiltered;
double totalCurrent;
double currentRMS;
double offsetCurrent;
float voltRMS;

float power1, power2, power3, power4, power5, power6;
float currentPort1, currentPort2, currentPort3, currentPort4, currentPort5, currentPort6;
float port1, port2, port3, port4, port5, port6;
float calibrate_tegangan;
int trigger_calibrate = 0;

TaskHandle_t SensorTask;
WebServer wifiServer(WIFI_WEB_SERVER_PORT);
Memory flash;
OTA ota;

void setup() {
  Serial.begin(115200);
  Serial.println("--- Panel Sensor Started ---");
  Serial.println("");

  flash.begin();
  readFlash();
  
  // Test WiFi hardware first
  if (!testWiFiHardware()) {
    Serial.println("✗ FATAL: WiFi hardware tidak ditemukan!");
    while (true) delay(1000);
  }
  
  WiFiBegin();
  
  // Initialize Ethernet
  ethBegin();

  setupSensor();

  // Setup WiFi server routes
  wifiServer.on("/", HTTP_GET, []() {
    wifiServer.send(200, F("text/html"), F("Welcome to the Panel Sensor"));
  });
  wifiServer.on("/test", HTTP_GET, []() {
    JsonDocument doc;
    doc["status"] = "Success";
    doc["message"] = "WiFi server is working!";
    doc["data"]["server"] = "WiFi";
    doc["data"]["port"] = WIFI_WEB_SERVER_PORT;
    doc["data"]["ip"] = WiFi.localIP().toString();
    doc["data"]["uptime"] = millis() / 1000;
    doc["data"]["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
    doc["data"]["ethernet_connected"] = (Ethernet.linkStatus() == LinkON);
    String response;
    serializeJson(doc, response);
    wifiServer.send(200, "application/json", response);
  });
  wifiServer.on(F("/api/get/sensor/data"), HTTP_GET, getSensorData);
  wifiServer.on(F("/api/get/sensor/calibration"), HTTP_GET, getSensorCalibration);
  wifiServer.on(F("/api/get/network/info"), HTTP_GET, getNetworkInfo);
  wifiServer.on(F("/api/get/samples-volt/info"), HTTP_GET, getSamplesVoltInfo);
  wifiServer.on(F("/api/get/samples-current/info"), HTTP_GET, getSamplesCurrInfo);

  wifiServer.on(F("/api/set/network/station"), HTTP_POST, setNetworkStation);
  wifiServer.on(F("/api/set/calibration/voltage"), HTTP_POST, setSensitivityVolt);
  wifiServer.on(F("/api/set/calibration/current"), HTTP_POST, setCalibrateCurrent);
  
  wifiServer.onNotFound(handleNotFound);
  wifiServer.begin();
  
  // Start Ethernet server
  ethernetServer.begin();
  Serial.println("Ethernet server started on port " + String(ETHERNET_WEB_SERVER_PORT));

  // Initialize connection states
  lastEthernetState = (Ethernet.linkStatus() == LinkON);
  lastWiFiState = (WiFi.status() == WL_CONNECTED);
  lastConnectionCheck = millis();
  
  // Print access instructions
  printAccessInstructions();

  ota.setMDNS(HOSTNAME.c_str());
  ota.setDebug(HOSTNAME.c_str());

  //  Create task for sensor reading
  xTaskCreatePinnedToCore(
    readPanelSensorTask,    // Function to be called
    "SensorTask",           // Name of the task
    10000,                  // Stack size (bytes)
    NULL,                   // Parameter to pass
    1,                      // Task priority
    &SensorTask,            // Task handle
    0                       // Core where the task should run
  );
}

void loop() {
  // Handle WiFi server
  wifiServer.handleClient();
  
  // Handle Ethernet server
  handleEthernetServer();
  
  ota.DebugHandle();
  if (trigger == 1) {
    WiFiBegin();
  }
//  if (trigger_calibrate == 1) {
//    setupSensor();
//    trigger_calibrate = 0;
//  }
  WiFiHandle();
  
  // Monitor connections every 30 seconds
  if (millis() - lastConnectionCheck > 30000) {
    monitorConnections();
    lastConnectionCheck = millis();
  }
  
  delay(100);
}

// Efficient connection monitoring function
void monitorConnections() {
  // Check Ethernet connection
  bool currentEthernetState = (Ethernet.linkStatus() == LinkON);
  if (currentEthernetState != lastEthernetState) {
    lastEthernetState = currentEthernetState;
    if (currentEthernetState) {
      Serial.println("=== Ethernet CONNECTED ===");
      Serial.println("IP: " + Ethernet.localIP().toString());
      Serial.println("Gateway: " + Ethernet.gatewayIP().toString());
      Serial.println("=========================");
    } else {
      Serial.println("=== Ethernet DISCONNECTED ===");
      Serial.println("Attempting to reconnect...");
      Serial.println("=============================");
      // Attempt Ethernet reconnection
      attemptEthernetReconnect();
    }
  }
  
  // Check WiFi connection
  bool currentWiFiState = (WiFi.status() == WL_CONNECTED);
  if (currentWiFiState != lastWiFiState) {
    lastWiFiState = currentWiFiState;
    if (currentWiFiState) {
      Serial.println("=== WiFi CONNECTED ===");
      Serial.println("SSID: " + WiFi.SSID());
      Serial.println("IP: " + WiFi.localIP().toString());
      Serial.println("MAC: " + WiFi.macAddress());
      Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
      Serial.println("=====================");
    } else {
      Serial.println("=== WiFi DISCONNECTED ===");
      Serial.println("Status: " + getWiFiStatusDescription(WiFi.status()));
      Serial.println("Target SSID: " + SSID_STA);
      Serial.println("Attempting to reconnect...");
      Serial.println("=========================");
      // Trigger WiFi reconnection
      trigger = 1;
    }
  }
}

// Function to attempt Ethernet reconnection
void attemptEthernetReconnect() {
  Serial.println("Attempting Ethernet reconnection...");
  
  // Convert MAC address string to byte array
  uint8_t mac[6];
  fromStringMacAddress(ETH_MAC_ADDRESS, mac);
  
  // Try DHCP first
  if (Ethernet.begin(mac)) {
    Serial.println("Ethernet reconnected via DHCP");
    Serial.println("New IP: " + Ethernet.localIP().toString());
  } else {
    // Fallback to static IP
    Serial.println("DHCP failed, trying static IP...");
    IPAddress ip(192, 168, 1, 100);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(8, 8, 8, 8);
    Ethernet.begin(mac, ip, dns, gateway, subnet);
    Serial.println("Ethernet reconnected via static IP");
    Serial.println("New IP: " + Ethernet.localIP().toString());
  }
}

// Function to test WiFi hardware
bool testWiFiHardware() {
  Serial.println("Testing WiFi hardware...");
  WiFi.mode(WIFI_STA);
  delay(100);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("✗ WiFi hardware tidak terdeteksi!");
    return false;
  }

  Serial.println("✓ WiFi hardware OK");
  return true;
}

// Function to print access instructions
void printAccessInstructions() {
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║  CARA AKSES WEB INTERFACE:            ║");
  Serial.println("╚═══════════════════════════════════════╝\n");

  if (dual_wifi == 2) {
    Serial.println("📱 Via Station (di jaringan " + SSID_STA + "):");
    Serial.println("   http://" + IP_ADDR + ":" + String(WIFI_WEB_SERVER_PORT) + "/\n");
  }

  Serial.println("📡 Via Access Point:");
  Serial.println("   1. Connect ke WiFi: " + SSID_AP);
  Serial.println("   2. Password: " + PASS_AP);
  Serial.println("   3. Buka: http://" + IP_ADDR_AP + ":" + String(WIFI_WEB_SERVER_PORT) + "/\n");
  
  Serial.println("🌐 Via Ethernet:");
  Serial.println("   http://" + Ethernet.localIP().toString() + ":" + String(ETHERNET_WEB_SERVER_PORT) + "/\n");
}
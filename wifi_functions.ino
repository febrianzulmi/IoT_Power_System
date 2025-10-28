/*
 * WiFi Functions for Sensor Panel Extension
 */

void WiFiBegin() {
  Serial.println("--- Setup Wireless ---");
  
  // Scan for available networks first
  scanWiFiNetworks();
  
  // Debug: Print WiFi credentials
  Serial.println("Debug - SSID_STA: '" + SSID_STA + "'");
  Serial.println("Debug - PASS_STA: '" + PASS_STA + "'");
  Serial.println("Debug - Current WiFi Status: " + String(WiFi.status()));
  Serial.println("Debug - Current SSID: '" + WiFi.SSID() + "'");
  
  // Declare stationConnected at function scope
  bool stationConnected = false;
  
  // Check if already connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✓ Already connected to WiFi!");
    stationConnected = true;
    dual_wifi = 2;
    IP_ADDR = WiFi.localIP().toString();
    MAC_ADDR_STA = WiFi.macAddress();
    Serial.println("IP Address Station : " + IP_ADDR);
    Serial.println("MAC Address Station : " + MAC_ADDR_STA);
    Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
  } else {
    // First, try to connect to station WiFi using standard WiFi library
    
    if (SSID_STA != "" && PASS_STA != "") {
      Serial.println("Attempting to connect to: " + SSID_STA);
      WiFi.begin(SSID_STA.c_str(), PASS_STA.c_str());
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      Serial.println();
      
      if (WiFi.status() == WL_CONNECTED) {
        stationConnected = true;
        dual_wifi = 2;
        IP_ADDR = WiFi.localIP().toString();
        MAC_ADDR_STA = WiFi.macAddress();
        
        Serial.println("✓ Connected to " + SSID_STA);
        Serial.println("IP Address Station : " + IP_ADDR);
        Serial.println("MAC Address Station : " + MAC_ADDR_STA);
        Serial.println("RSSI: " + String(WiFi.RSSI()) + " dBm");
      } else {
        Serial.println("✗ Failed to connect to " + SSID_STA);
        Serial.println("WiFi Status: " + getWiFiStatusDescription(WiFi.status()));
        dual_wifi = 1;
        IP_ADDR = "";
        MAC_ADDR_STA = "";
      }
    } else {
      Serial.println("⚠ WiFi credentials not set");
      dual_wifi = 1;
      IP_ADDR = "";
      MAC_ADDR_STA = "";
    }
  }
  
  // Always create Access Point
  Serial.println("\nSetting up Access Point...");
  WiFi.softAP(SSID_AP.c_str(), PASS_AP.c_str());
  delay(500);
  
  IP_ADDR_AP = WiFi.softAPIP().toString();
  MAC_ADDR_AP = WiFi.softAPmacAddress();
  
  Serial.println("✓ Access Point created");
  Serial.println("SSID: " + SSID_AP);
  Serial.println("Password: " + PASS_AP);
  Serial.println("IP Address AP : " + IP_ADDR_AP);
  Serial.println("MAC Address AP : " + MAC_ADDR_AP);
  
  Serial.println("\n➤ Mode: " + String(dual_wifi == 2 ? "DUAL (AP + Station)" : "AP ONLY"));
  
  WiFiState();
  trigger = 0;
  Serial.println("---------------------\n");
  delay(1000);
}

void WiFiState() {
  // Use standard WiFi library instead of SynapsisLibrary
  WIFI_STATUS = String(WiFi.status());
  WIFI_RSSI = String(WiFi.RSSI());
  
  // Update station info if connected
  if (WiFi.status() == WL_CONNECTED) {
    IP_ADDR = WiFi.localIP().toString();
    MAC_ADDR_STA = WiFi.macAddress();
  } else {
    IP_ADDR = "";
    MAC_ADDR_STA = "";
  }
}

void WiFiHandle() {
  // WiFi handling logic can be added here if needed
}

// Function to scan WiFi networks
void scanWiFiNetworks() {
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    Serial.println("✗ No networks found");
  } else {
    Serial.println("✓ Found " + String(n) + " networks:");
    for (int i = 0; i < n; ++i) {
      Serial.print("  ");
      Serial.print(i + 1);
      Serial.print(". ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(" dBm) ");
      Serial.println(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "[Open]" : "[Encrypted]");
    }
    Serial.println();
    
    // Check if target SSID is found
    bool foundTarget = false;
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == SSID_STA) {
        foundTarget = true;
        Serial.println("✓ Target SSID '" + SSID_STA + "' found!");
        break;
      }
    }
    if (!foundTarget && SSID_STA != "") {
      Serial.println("⚠ Target SSID '" + SSID_STA + "' not found");
    }
  }
}

// Function to get WiFi status description
String getWiFiStatusDescription(int status) {
  switch (status) {
    case WL_CONNECTED: return "Connected";
    case WL_NO_SSID_AVAIL: return "No SSID Available";
    case WL_CONNECT_FAILED: return "Connection Failed";
    case WL_CONNECTION_LOST: return "Connection Lost";
    case WL_DISCONNECTED: return "Disconnected";
    case WL_IDLE_STATUS: return "Idle";
    default: return "Unknown (" + String(status) + ")";
  }
}

void readFlash() {
  // Read configuration from flash memory
  SENSITIVITY = flash.readFile("/sensitivityVolt.txt").toFloat();
  port1 = flash.readFile("/port1.txt").toFloat();
  port2 = flash.readFile("/port2.txt").toFloat();
  port3 = flash.readFile("/port3.txt").toFloat();
  port4 = flash.readFile("/port4.txt").toFloat();
  port5 = flash.readFile("/port5.txt").toFloat();
  port6 = flash.readFile("/port6.txt").toFloat();
  
  // Read WiFi credentials with fallback to default values
  String ssidFromFlash = flash.readFile("/ssid_sta.txt");
  String passFromFlash = flash.readFile("/pass_sta.txt");
  
  if (ssidFromFlash != "" && passFromFlash != "") {
    SSID_STA = ssidFromFlash;
    PASS_STA = passFromFlash;
    Serial.println("✓ WiFi credentials loaded from flash");
  } else {
    // Use default values if flash is empty
    SSID_STA = "Synapsis.id Head Office";
    PASS_STA = "synaps1sjkt23@";
    Serial.println("⚠ Using default WiFi credentials (flash empty)");
    
    // Save default credentials to flash for future use
    flash.writeFile("/ssid_sta.txt", SSID_STA.c_str());
    flash.writeFile("/pass_sta.txt", PASS_STA.c_str());
    Serial.println("✓ Default WiFi credentials saved to flash");
  }
}

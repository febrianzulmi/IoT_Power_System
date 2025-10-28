/*
 * Ethernet Functions for Sensor Panel Extension
 */

// External sensor status variables
extern bool sensor1_connected;
extern bool sensor2_connected;

// Forward declarations
String getWiFiStatusDescription(int status);

// Fungsi untuk konversi MAC address string ke byte array
void fromStringMacAddress(const String& macStr, uint8_t* mac) {
  for (int i = 0; i < 6; i++) {
    mac[i] = strtol(macStr.substring(i * 3, i * 3 + 2).c_str(), NULL, 16);
  }
}


void ethBegin() {
  Serial.println("--- Setup Ethernet ---");
  Serial.println("CS Pin: " + String(ETH_CS_PIN));
  Serial.println("MAC Address: " + String(ETH_MAC_ADDRESS));

  // Initialize ethernet with your proven configuration
  Ethernet.init(ETH_CS_PIN);

  // Convert MAC address string to byte array
  uint8_t mac[6];
  fromStringMacAddress(ETH_MAC_ADDRESS, mac);

  Serial.println("Begin Ethernet Device");

  if (Ethernet.begin(mac)) {  // Dynamic IP setup
    Serial.println("DHCP OK!");
  } else {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found. Sorry, can't run without hardware. :(");
      return;
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // Fallback to static IP
    Serial.println("Trying static IP as fallback...");
    IPAddress ip(192, 168, 1, 100);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(8, 8, 8, 8);
    Ethernet.begin(mac, ip, dns, gateway, subnet);
    Serial.println("STATIC OK!");
  }

  // Set faster Ethernet timeouts
  Ethernet.setRetransmissionCount(2);  // Reduce from default 8 to 2
  Ethernet.setRetransmissionTimeout(100);  // Reduce from default 200ms to 100ms

  delay(500);  // Reduce delay from 1000ms to 500ms

  Serial.println("=== Ethernet Configuration ===");
  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.println("Ethernet Successfully Initialized");
  Serial.println("----------------------\n");
}

String readLine(EthernetClient &client) {
  String line = "";
  char c;
  while (client.available()) {
    c = client.read();
    line += c;
    int len = line.length();
    // check for CRLF termination
    if (len >= 2 && line.charAt(len-2) == '\r' && line.charAt(len-1) == '\n') {
      break;
    }
  }
  return line;
}

void handleEthernetServer() {
  EthernetClient client = ethernetServer.available();
  if (client) {
    Serial.println("Ethernet client connected");
    
    // Wait for data (with small timeout)
    unsigned long start = millis();
    while (!client.available() && (millis() - start) < 2000) {
      delay(1);
    }
    if (!client.available()) {
      client.stop();
      return;
    }

    // Read request line
    String requestLine = readLine(client);
    requestLine.trim();
    if (requestLine.length() == 0) {
      client.stop();
      return;
    }

    Serial.print("Ethernet Request: ");
    Serial.println(requestLine);

    // Parse method and path
    String method, path;
    int sp1 = requestLine.indexOf(' ');
    int sp2 = requestLine.indexOf(' ', sp1 + 1);
    if (sp1 > 0 && sp2 > sp1) {
      method = requestLine.substring(0, sp1);
      path = requestLine.substring(sp1 + 1, sp2);
    } else {
      client.stop();
      return;
    }

    // Read headers
    int contentLength = 0;
    while (true) {
      String header = readLine(client);
      header.trim();
      if (header.length() == 0) break; // end of headers
      
      String lower = header;
      lower.toLowerCase();
      if (lower.startsWith("content-length:")) {
        int colon = header.indexOf(':');
        if (colon >= 0) {
          String val = header.substring(colon + 1);
          val.trim();
          contentLength = val.toInt();
        }
      }
    }

    // Read body if present
    String body = "";
    if (contentLength > 0) {
      unsigned long tstart = millis();
      while ((int)body.length() < contentLength && (millis() - tstart) < 5000) {
        while (client.available() && (int)body.length() < contentLength) {
          char c = client.read();
          body += c;
        }
      }
      Serial.print("Received body (len=");
      Serial.print(body.length());
      Serial.println("):");
      Serial.println(body);
    }

    // Handle API endpoints
    handleEthernetAPI(client, method, path, body);
    
    delay(1);
    client.stop();
    Serial.println("Ethernet client disconnected");
  }
}

void handleEthernetAPI(EthernetClient &client, String method, String path, String body) {
  // Use unified route handler
  handleUnifiedEthernetRoute(method, path, body);
  
  // Send response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(lastUnifiedResponse.length());
  client.println("Connection: close");
  client.println();
  client.print(lastUnifiedResponse);
}

// ===== UNIFIED ROUTE HANDLER =====

void handleUnifiedEthernetRoute(String method, String path, String body) {
  lastUnifiedResponse = "";
  
  if (method == "GET") {
    if (path == "/" || path == "/test") {
      handleUnifiedTest();
    } else if (path == "/api/get/network/info") {
      handleUnifiedNetworkInfo();
    } else if (path == "/api/get/sensor/data") {
      handleUnifiedSensorData();
    } else if (path == "/api/get/sensor/calibration") {
      handleUnifiedSensorCalibration();
    } else {
      handleUnifiedNotFound();
    }
  } else if (method == "POST") {
    if (path == "/api/set/network/station") {
      handleUnifiedNetworkStation(body);
    } else if (path == "/api/set/calibration/voltage") {
      handleUnifiedCalibrationVoltage(body);
    } else if (path == "/api/set/calibration/current") {
      handleUnifiedCalibrationCurrent(body);
    } else {
      handleUnifiedNotFound();
    }
  } else {
    handleUnifiedNotFound();
  }
}

void handleUnifiedNetworkInfo() {
  WiFiState();
  JsonDocument doc;
  String modewifi;
  if (dual_wifi == 1) {
    modewifi = "AP Mode";
  } else {
    modewifi = "Dual Mode";
  }
  doc["status"] = "Success";
  doc["message"] = "Request Network Information Successfully";
  doc["data"]["wireless"]["mode"] = modewifi;
  doc["data"]["wireless"]["ap"]["mac"] = MAC_ADDR_AP;
  doc["data"]["wireless"]["ap"]["ip"] = IP_ADDR_AP;
  doc["data"]["wireless"]["ap"]["ssid"] = SSID_AP;
  doc["data"]["wireless"]["ap"]["password"] = PASS_AP;
  doc["data"]["wireless"]["station"]["mac"] = MAC_ADDR_STA;
  doc["data"]["wireless"]["station"]["ip"] = IP_ADDR;
  doc["data"]["wireless"]["station"]["ssid"] = SSID_STA;
  doc["data"]["wireless"]["station"]["password"] = PASS_STA;
  doc["data"]["wireless"]["station"]["rssi"] = WIFI_RSSI;
  doc["data"]["wireless"]["station"]["state"] = WIFI_STATUS;
  doc["data"]["wireless"]["station"]["status_description"] = getWiFiStatusDescription(WiFi.status());
  doc["data"]["wireless"]["station"]["connected"] = (WiFi.status() == WL_CONNECTED);
  doc["data"]["wireless"]["station"]["configuration"]["mode"] = "DHCP";
  doc["data"]["wireless"]["station"]["configuration"]["ip"] = IP_ADDR;
  doc["data"]["wireless"]["station"]["configuration"]["subnet"] = "255.255.254.0";
  doc["data"]["wireless"]["station"]["configuration"]["gateway"] = "10.19.51.1";
  doc["data"]["wireless"]["station"]["configuration"]["dns"] = "10.19.51.1";
  
  // Ethernet info
  doc["data"]["ethernet"]["mode"] = "DHCP";
  doc["data"]["ethernet"]["mac"] = ETH_MAC_ADDRESS;
  doc["data"]["ethernet"]["ip"] = Ethernet.localIP().toString();
  doc["data"]["ethernet"]["subnet"] = Ethernet.subnetMask().toString();
  doc["data"]["ethernet"]["gateway"] = Ethernet.gatewayIP().toString();
  doc["data"]["ethernet"]["dns"] = Ethernet.dnsServerIP().toString();
  
  // Sensor status
  doc["data"]["sensors"]["sensor1_connected"] = sensor1_connected;
  doc["data"]["sensors"]["sensor2_connected"] = sensor2_connected;
  
  serializeJson(doc, lastUnifiedResponse);
}

void handleUnifiedSensorData() {
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Request Sensor Information Successfully";
  doc["data"]["tegangan"] = voltRMS;
  doc["data"]["current"]["port1"] = currentPort1;
  doc["data"]["current"]["port2"] = currentPort2;
  doc["data"]["current"]["port3"] = currentPort3;
  doc["data"]["current"]["port4"] = currentPort4;
  doc["data"]["current"]["port5"] = currentPort5;
  doc["data"]["current"]["port6"] = currentPort6;
  doc["data"]["power"]["port1"] = power1;
  doc["data"]["power"]["port2"] = power2;
  doc["data"]["power"]["port3"] = power3;
  doc["data"]["power"]["port4"] = power4;
  doc["data"]["power"]["port5"] = power5;
  doc["data"]["power"]["port6"] = power6;

  serializeJsonPretty(doc, lastUnifiedResponse);
}

void handleUnifiedSensorCalibration() {
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Request Calibrate Information Successfully";
  doc["data"]["tegangan"] = SENSITIVITY;
  doc["data"]["arus"]["port1"] = port1;
  doc["data"]["arus"]["port2"] = port2;
  doc["data"]["arus"]["port3"] = port3;
  doc["data"]["arus"]["port4"] = port4;
  doc["data"]["arus"]["port5"] = port5;
  doc["data"]["arus"]["port6"] = port6;
  
  serializeJsonPretty(doc, lastUnifiedResponse);
}

void handleUnifiedNetworkStation(String body) {
  JsonDocument jDoc;
  DeserializationError err = deserializeJson(jDoc, body);
  String ssid_input = jDoc["ssid"];
  String password_input = jDoc["password"];
  flash.writeFile("/ssid_sta.txt", ssid_input.c_str());
  flash.writeFile("/pass_sta.txt", password_input.c_str());
  SSID_STA = ssid_input;
  PASS_STA = password_input;
  
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Setting Station Network Successfully";
  doc["data"]["ssid"] = SSID_STA;
  doc["data"]["password"] = PASS_STA;
  
  serializeJson(doc, lastUnifiedResponse);
  trigger = 1;
}

void handleUnifiedCalibrationVoltage(String body) {
  JsonDocument jDoc;
  DeserializationError err = deserializeJson(jDoc, body);
  String newSensitivityVal = jDoc["value"];
  
  JsonDocument doc;
  if (newSensitivityVal != "") {
    if (!isNumeric(newSensitivityVal)) {
      doc["status"] = "Failed";
      doc["message"] = "Sensitivity voltage must be a number !";
      doc["data"]["tegangan"] = newSensitivityVal;
    } else {
      float rawSensitivity = newSensitivityVal.toFloat();
      flash.writeFile("/sensitivityVolt.txt", newSensitivityVal.c_str());
      SENSITIVITY = rawSensitivity;
      
      doc["status"] = "Success";
      doc["message"] = "Setting Sensitivity Voltage Successfully!";
      doc["data"]["tegangan"] = newSensitivityVal;
    }
  } else {
    doc["status"] = "Failed";
    doc["message"] = "Offset Voltage is Empty";
    doc["data"]["tegangan"] = newSensitivityVal;
  }
  
  serializeJson(doc, lastUnifiedResponse);
}

void handleUnifiedCalibrationCurrent(String body) {
  JsonDocument jDoc;
  DeserializationError err = deserializeJson(jDoc, body);
  
  String ports[] = {"port1", "port2", "port3", "port4", "port5", "port6"};
  float portValues[sizeof(ports) / sizeof(ports[0])];
  
  bool error = false;
  String errorMessage = "";
  
  for (int i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
    String newPort = jDoc[ports[i]];
    
    if (newPort != "") {
      if (isNumeric(newPort)) {
        float portValue = newPort.toFloat();
        flash.writeFile(("/" + ports[i] + ".txt").c_str(), newPort.c_str());
        
        switch (i) {
          case 0: port1 = portValue; break;
          case 1: port2 = portValue; break;
          case 2: port3 = portValue; break;
          case 3: port4 = portValue; break;
          case 4: port5 = portValue; break;
          case 5: port6 = portValue; break;
        }
      } else {
        error = true;
        errorMessage = "Port " + String(i + 1) + " must be a number";
        break;
      }
    } else {
      error = true;
      errorMessage = "Port is Empty";
      break;
    }
  }
  
  JsonDocument doc;
  if (error) {
    doc["status"] = "Failed";
    doc["message"] = errorMessage;
    doc["data"]["port1"] = jDoc[ports[0]];
    doc["data"]["port2"] = jDoc[ports[1]];
    doc["data"]["port3"] = jDoc[ports[2]];
    doc["data"]["port4"] = jDoc[ports[3]];
    doc["data"]["port5"] = jDoc[ports[4]];
    doc["data"]["port6"] = jDoc[ports[5]];
  } else {
    doc["status"] = "Success";
    doc["message"] = "Setting Calibration Offset Successfully";
    doc["data"]["port1"] = jDoc[ports[0]];
    doc["data"]["port2"] = jDoc[ports[1]];
    doc["data"]["port3"] = jDoc[ports[2]];
    doc["data"]["port4"] = jDoc[ports[3]];
    doc["data"]["port5"] = jDoc[ports[4]];
    doc["data"]["port6"] = jDoc[ports[5]];
  }
  
  serializeJson(doc, lastUnifiedResponse);
  trigger_calibrate = 1;
}

void handleUnifiedTest() {
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Ethernet server is working!";
  doc["data"]["server"] = "Ethernet";
  doc["data"]["port"] = ETHERNET_WEB_SERVER_PORT;
  doc["data"]["ip"] = Ethernet.localIP().toString();
  doc["data"]["uptime"] = millis() / 1000;
  doc["data"]["ethernet_connected"] = isEthernetConnected();
  doc["data"]["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  serializeJson(doc, lastUnifiedResponse);
}

void handleUnifiedNotFound() {
  JsonDocument doc;
  doc["status"] = "Failed";
  doc["message"] = "API not found";
  serializeJson(doc, lastUnifiedResponse);
}

// Function to get ethernet status (for API responses)
bool isEthernetConnected() {
  return (Ethernet.linkStatus() == LinkON);
}
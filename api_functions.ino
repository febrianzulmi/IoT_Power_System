/*
 * API Functions for Sensor Panel Extension
 */

// External sensor status variables
extern bool sensor1_connected;
extern bool sensor2_connected;

// Forward declarations
String getWiFiStatusDescription(int status);

bool isNumeric(const String& str) {
  bool hasDecimalPoint = false;
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!isdigit(c)) {
      // Check if character is a decimal point
      if (c == '.') {
        // If decimal point is already found, return false
        if (hasDecimalPoint) {
          return false;
        }
        hasDecimalPoint = true;
      } else {
        // If character is not a digit or decimal point, return false
        return false;
      }
    }
  }
  return true;
}

void getSamplesVoltInfo(){
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Request N-Samples Voltage Reading";
  doc["data"] = SAMPLE_COUNT_VOLT;
  String data_string;
  serializeJson(doc, data_string);
  wifiServer.send(200, "text/json", data_string);
  
  Serial.println("--- Request N-Samples Voltage Reading ---");
  Serial.println("Sampling Reading Count\t: " + SAMPLE_COUNT_VOLT);
  Serial.println("");
}

void getSamplesCurrInfo(){
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Request N-Samples Current Reading";
  doc["data"] = SAMPLE_COUNT_CURR;
  String data_string;
  serializeJson(doc, data_string);
  wifiServer.send(200, "text/json", data_string);
  
  Serial.println("--- Request N-Samples Current Reading ---");
  Serial.println("Sampling Reading Count\t: " + SAMPLE_COUNT_CURR);
  Serial.println("");
}

void getNetworkInfo() {
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
  
  String data_string;
  serializeJson(doc, data_string);
  Serial.println("--- Request GET Network ---");
  Serial.println("Mode WiFi : " + modewifi);
  Serial.println("MAC Address AP : " + MAC_ADDR_AP);
  Serial.println("MAC Address Station : " + MAC_ADDR_STA);
  Serial.println("SSID AP : " + SSID_AP);
  Serial.println("IP Address AP : " + IP_ADDR_AP);
  Serial.println("SSID Station : " + SSID_STA);
  Serial.println("IP Address Station : " + IP_ADDR);
  Serial.println("RSSI Station : " + WIFI_RSSI);
  Serial.println("---------------------------");
  Serial.println("");
  wifiServer.send(200, "text/json", data_string);
}

void getSensorData() {
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

  String data_string;
  serializeJsonPretty(doc, data_string);
  Serial.println("--- Request GET Data Power Panel Sensor ---");
  Serial.println("Tegangan : " + String(voltRMS));
  Serial.println("Arus AC 1 : " + String(currentPort1));
  Serial.println("Arus AC 2 : " + String(currentPort2));
  Serial.println("Arus AC 3 : " + String(currentPort3));
  Serial.println("Arus AC 4 : " + String(currentPort4));
  Serial.println("Arus AC 5 : " + String(currentPort5));
  Serial.println("Arus AC 6 : " + String(currentPort6));
  Serial.println("Power AC 1 : " + String(power1));
  Serial.println("Power AC 2 : " + String(power2));
  Serial.println("Power AC 3 : " + String(power3));
  Serial.println("Power AC 4 : " + String(power4));
  Serial.println("Power AC 5 : " + String(power5));
  Serial.println("Power AC 6 : " + String(power6));
  Serial.println("-------------------------------");
  Serial.println("");
  wifiServer.send(200, "application/json", data_string);
}

void getSensorCalibration() {
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Request Calibrate Information Succesfully";
  doc["data"]["tegangan"] = SENSITIVITY;
  doc["data"]["arus"]["port1"] = port1;
  doc["data"]["arus"]["port2"] = port2;
  doc["data"]["arus"]["port3"] = port3;
  doc["data"]["arus"]["port4"] = port4;
  doc["data"]["arus"]["port5"] = port5;
  doc["data"]["arus"]["port6"] = port6;
  
  String data_string;
  serializeJsonPretty(doc, data_string);
  Serial.println("--- Request GET Calibrate Sensor ---");
  Serial.println("Calibrate Voltage Offset : " + String(SENSITIVITY));
  Serial.println("Calibrate Current Offset Port (1) : " + String(port1));
  Serial.println("Calibrate Current Offset Port (2) : " + String(port2));
  Serial.println("Calibrate Current Offset Port (3) : " + String(port3));
  Serial.println("Calibrate Current Offset Port (4) : " + String(port4));
  Serial.println("Calibrate Current Offset Port (5) : " + String(port5));
  Serial.println("Calibrate Current Offset Port (6) : " + String(port6));
  Serial.println("-------------------------------");
  Serial.println("");
  wifiServer.send(200, "application/json", data_string);
}

void setNetworkStation() {
  String data_string = wifiServer.arg("plain");
  JsonDocument jDoc;
  DeserializationError err = deserializeJson(jDoc, data_string);
  String ssid_input = jDoc["ssid"];
  String password_input = jDoc["password"];
  flash.writeFile("/ssid_sta.txt", ssid_input.c_str());
  flash.writeFile("/pass_sta.txt", password_input.c_str());
  SSID_STA = ssid_input;
  PASS_STA = password_input;
  Serial.println("--- Request POST Station Network ---");
  Serial.println("Input SSID : " + ssid_input);
  Serial.println("Password SSID : " + password_input);
  Serial.println("------------------------------------");
  Serial.println("");
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Setting Station Network Successfully";
  doc["data"]["ssid"] = SSID_STA;
  doc["data"]["password"] = PASS_STA;
  String data_message;
  serializeJson(doc, data_message);
  wifiServer.send(200, "application/json", data_message);
  trigger = 1;
}

void setSensitivityVolt() {
  String data_string = wifiServer.arg("plain");
  JsonDocument jDoc;
  DeserializationError err = deserializeJson(jDoc, data_string);
  String newSensitivityVal = jDoc["value"];
  
  Serial.println("Settings have been updated!");
  Serial.println("New Sensitivity Volt : " + newSensitivityVal);
  Serial.println("------------------------------------");
  Serial.println("");
  
  if (newSensitivityVal != "") {
    if (!isNumeric(newSensitivityVal)) {
      handlePostErorr("Sensitivity voltage must be a number !", "tegangan", newSensitivityVal);
      Serial.println("Failed : Offset voltage must be a number!");
      return;
    }
    
    float rawSensitivity = newSensitivityVal.toFloat();
    flash.writeFile("/sensitivityVolt.txt", newSensitivityVal.c_str());
    SENSITIVITY = rawSensitivity;

  } else {
    handlePostErorr("Offset Voltage is Empty", "tegangan", newSensitivityVal);
    Serial.println("Offset Voltage is Empty : " + String(newSensitivityVal) + " (NULL)");
    return;
  }
  
  JsonDocument doc;
  doc["status"] = "Success";
  doc["message"] = "Setting Sensitivity Voltage Successfully!";
  doc["data"]["tegangan"] = newSensitivityVal;
  
  String dataMessage;
  serializeJson(doc, dataMessage);
  wifiServer.send(200, "application/json", dataMessage);
}

void setCalibrateCurrent() {
  int statusSend;
  String data_string = wifiServer.arg("plain");
  String errorMessage = "";
  JsonDocument jDoc;
  DeserializationError err = deserializeJson(jDoc, data_string);

  // Array of port strings
  String ports[] = {"port1", "port2", "port3", "port4", "port5", "port6"};

  // Array of port values
  float portValues[sizeof(ports) / sizeof(ports[0])];

  Serial.println("--- Request Post Calibrate Current ACS712 ---");
  bool error = false;

  // Loop through each port
  for (int i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
    String newPort = jDoc[ports[i]];

    // Check if port is not empty
    if (newPort != "") {
      // Check if port is numeric
      if (isNumeric(newPort)) {
        // Convert port value to float and store it
        float portValue = newPort.toFloat();

        // Update the corresponding port file
        flash.writeFile(("/" + ports[i] + ".txt").c_str(), newPort.c_str());
        Serial.println("Calibrate Current Offset Port (" + String(i + 1) + ") : " + newPort);

        // Assign value to global variable based on port number
        switch (i) {
          case 0: port1 = portValue; break;
          case 1: port2 = portValue; break;
          case 2: port3 = portValue; break;
          case 3: port4 = portValue; break;
          case 4: port5 = portValue; break;
          case 5: port6 = portValue; break;
        }
      } else {
        // If port is not numeric, return an error
        error = true;
        errorMessage = "Port " + String(i + 1) + " must be a number";
        Serial.println("Failed : " + errorMessage);
        return;
      }
    } else {
      error = true;
      errorMessage = "Port is Empty";
      Serial.println("Failed : " + errorMessage);
    }
  }

  Serial.println("------------------------------------");
  Serial.println("");

  // Create JSON response
  JsonDocument doc;
  if (error) {
    // If there is an error, set response status to "Failed"
    statusSend = 400;
    doc["status"] = "Failed";
    doc["message"] = errorMessage;
    doc["data"]["port1"] = jDoc[ports[0]];
    doc["data"]["port2"] = jDoc[ports[1]];
    doc["data"]["port3"] = jDoc[ports[2]];
    doc["data"]["port4"] = jDoc[ports[3]];
    doc["data"]["port5"] = jDoc[ports[4]];
    doc["data"]["port6"] = jDoc[ports[5]];
    String data_message;
    serializeJson(doc, data_message);
    wifiServer.send(statusSend, "application/json", data_message);
    
  } else {
    statusSend = 200;
    doc["status"] = "Success";
    doc["message"] = "Setting Calibration Offset Successfully";
    doc["data"]["port1"] = jDoc[ports[0]];
    doc["data"]["port2"] = jDoc[ports[1]];
    doc["data"]["port3"] = jDoc[ports[2]];
    doc["data"]["port4"] = jDoc[ports[3]];
    doc["data"]["port5"] = jDoc[ports[4]];
    doc["data"]["port6"] = jDoc[ports[5]];
    String data_message;
    serializeJson(doc, data_message);
    wifiServer.send(statusSend, "application/json", data_message);
    
  }
  trigger_calibrate = 1;
}

void handlePostErorr(const String message, const String dataLabel, const String dataInput) {
  JsonDocument doc;
  doc["status"] = "Failed";
  doc["message"] = message;
  doc["data"][dataLabel] = dataInput;
  String data_message;
  serializeJson(doc, data_message);
  wifiServer.send(400, "application/json", data_message);
}

void handleNotFound() {
  String message = "API ";
  message += wifiServer.uri();
  message += " with method ";
  message += (wifiServer.method() == HTTP_GET) ? "GET" : "POST";
  message += " not found";
  for (uint8_t i = 0; i < wifiServer.args(); i++) {
    message += " " + wifiServer.argName(i) + ": " + wifiServer.arg(i) + "\n";
  }
  handlePost("Failed", message, 404);
}

void handlePost(const String statusMessage, const String message, const int statusCode) {
  JsonDocument doc;
  doc["status"] = statusMessage;
  doc["message"] = message;
  String data_message;
  serializeJson(doc, data_message);
  wifiServer.send(statusCode, "application/json", data_message);
}

void handleErorrCalibrateCurrent(const String message, const String dataLabel, const String dataLabe2, const String dataLabe3, const String dataLabe4, const String dataLabe5, const String dataLabe6, const String dataInput1, const String dataInput2, const String dataInput3, const String dataInput4, const String dataInput5, const String dataInput6) {
  JsonDocument doc;
  doc["status"] = "Failed";
  doc["message"] = message;
  doc["data"][dataLabel] = dataInput1;
  doc["data"][dataLabe2] = dataInput2;
  doc["data"][dataLabe3] = dataInput3;
  doc["data"][dataLabe4] = dataInput4;
  doc["data"][dataLabe5] = dataInput5;
  doc["data"][dataLabe6] = dataInput6;
  String data_message;
  serializeJson(doc, data_message);
  wifiServer.send(400, "application/json", data_message);
}

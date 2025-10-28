/*
 * Sensor Functions for Sensor Panel Extension
 */

// Global variables to track sensor status
bool sensor1_connected = false;
bool sensor2_connected = false;

// Function to scan I2C addresses
void scanI2CAddresses() {
  Serial.println("--- I2C Address Scan ---");
  Wire.begin();
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }    
  }
  
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.print("Found ");
    Serial.print(nDevices);
    Serial.println(" I2C device(s)");
  }
  Serial.println("----------------------");
}

// Function to test sensor connection by reading a value
bool testSensorConnection(Adafruit_ADS1115& sensor, const char* sensorName) {
  // Try to read a value from the sensor
  int16_t testValue = sensor.readADC_SingleEnded(0);
  
  // Check if the value is reasonable (not 0 or extreme values)
  if (testValue != 0 && testValue > -32768 && testValue < 32767) {
    Serial.println(sensorName + String(" connection test: OK (value: ") + String(testValue) + ")");
    return true;
  } else {
    Serial.println(sensorName + String(" connection test: FAILED (value: ") + String(testValue) + ")");
    return false;
  }
}

void setupSensor() {
  Serial.println("--- Sensor Initialization ---");
  
  // Scan for I2C devices first
  scanI2CAddresses();
  
  // Initialize Sensor 1 ADS
  SENSOR1_ADS.setGain(GAIN_ONE);
  if (!SENSOR1_ADS.begin(0x48)) {
    Serial.println("Failed to initialize Sensor 1 ADS at address 0x48");
    Serial.println("Trying alternative address 0x49...");
    if (!SENSOR1_ADS.begin(0x49)) {
      Serial.println("Failed to initialize Sensor 1 ADS at address 0x49");
      Serial.println("Sensor 1 will be disabled. Continuing...");
      sensor1_connected = false;
    } else {
      Serial.println("Sensor 1 ADS initialized at address 0x49");
      sensor1_connected = testSensorConnection(SENSOR1_ADS, "Sensor 1");
    }
  } else {
    Serial.println("Sensor 1 ADS initialized at address 0x48");
    sensor1_connected = testSensorConnection(SENSOR1_ADS, "Sensor 1");
  }
  
  // Initialize Sensor 2 ADS
  SENSOR2_ADS.setGain(GAIN_ONE);
  if (!SENSOR2_ADS.begin(0x49)) {
    Serial.println("Failed to initialize Sensor 2 ADS at address 0x49");
    Serial.println("Trying alternative address 0x48...");
    if (!SENSOR2_ADS.begin(0x48)) {
      Serial.println("Failed to initialize Sensor 2 ADS at address 0x48");
      Serial.println("Sensor 2 will be disabled. Continuing...");
      sensor2_connected = false;
    } else {
      Serial.println("Sensor 2 ADS initialized at address 0x48");
      sensor2_connected = testSensorConnection(SENSOR2_ADS, "Sensor 2");
    }
  } else {
    Serial.println("Sensor 2 ADS initialized at address 0x49");
    sensor2_connected = testSensorConnection(SENSOR2_ADS, "Sensor 2");
  }

  Serial.println("Sensor initialization completed!");
  Serial.println("Sensor 1 connected: " + String(sensor1_connected ? "YES" : "NO"));
  Serial.println("Sensor 2 connected: " + String(sensor2_connected ? "YES" : "NO"));
  delay(1000);
  periode = 1000000 / FREQUENCY;
  Serial.println("--------------------");
  Serial.println("");
}

void readPanelSensor() {
  float rawVoltRMS, rawCurrentPort1,rawCurrentPort2,rawCurrentPort3,rawCurrentPort4,rawCurrentPort5,rawCurrentPort6;
  
  // Read voltage from Sensor 2
  rawVoltRMS = getRmsVoltage2(SENSITIVITY);
  voltRMS = handleValue(rawVoltRMS, 50.0);  
  Serial.println("VoltageRMS : " + String(voltRMS));
  
  // Read current from Sensor 1 (Ports 1-4)
  rawCurrentPort1 = calcCurrentRMS(0, port1, SAMPLE_COUNT_CURR, 1);
  currentPort1 = handleValue(rawCurrentPort1, 0.05);
  Serial.println("Current Port 1 : " + String(currentPort1));
  
  rawCurrentPort2 = calcCurrentRMS(1, port2, SAMPLE_COUNT_CURR, 1);
  currentPort2 = handleValue(rawCurrentPort2, 0.05);
  Serial.println("Current Port 2 : " + String(currentPort2));
  
  rawCurrentPort3 = calcCurrentRMS(2, port3, SAMPLE_COUNT_CURR, 1);
  currentPort3 = handleValue(rawCurrentPort3, 0.05);
  Serial.println("Current Port 3 : " + String(currentPort3));
  
  rawCurrentPort4 = calcCurrentRMS(3, port4, SAMPLE_COUNT_CURR, 1);
  currentPort4 = handleValue(rawCurrentPort4, 0.05);
  Serial.println("Current Port 4 : " + String(currentPort4));
  
  // Read current from Sensor 2 (Ports 5-6)
  rawCurrentPort5 = calcCurrentRMS(0, port5, SAMPLE_COUNT_CURR, 2);
  currentPort5 = handleValue(rawCurrentPort5, 0.05);
  Serial.println("Current Port 5 : " + String(currentPort5));
  
  rawCurrentPort6 = calcCurrentRMS(1, port6, SAMPLE_COUNT_CURR, 2);
  currentPort6 = handleValue(rawCurrentPort6, 0.05);
  Serial.println("Current Port 6 : " + String(currentPort6));
  
  Serial.println("");

  power1 = currentPort1 * voltRMS;
  power2 = currentPort2 * voltRMS;
  power3 = currentPort3 * voltRMS;
  power4 = currentPort4 * voltRMS;
  power5 = currentPort5 * voltRMS;
  power6 = currentPort6 * voltRMS;
}

float handleValue(float value, float threshold){
  if (value <= threshold){
    value = 0.0;
  }
  return value;
}

uint32_t getZeroPoint() {
  uint32_t totalVolt = 0;
  uint32_t measurementCount = 0;
  uint32_t startTime = micros();

  // Check if Sensor 2 is available
  if (!sensor2_connected) {
    Serial.println("Sensor 2 not connected, returning 0 zero point");
    return 0;
  }

  while (micros() - startTime < periode) {
    totalVolt += SENSOR2_ADS.readADC_SingleEnded(2);
    measurementCount++;
  }

  return totalVolt / measurementCount;
}

float getRmsVoltage2(double sensitivityValue) {
  double readingVoltage = 0.0f;
  
  // Check if Sensor 2 is available
  if (!sensor2_connected) {
    Serial.println("Sensor 2 not connected, returning 0 voltage");
    return 0.0;
  }
  
  int zeroPoint = getZeroPoint();
  for (uint8_t n = 0; n < SAMPLE_COUNT_VOLT; n++) {
    int zeroPoint = getZeroPoint();
    int32_t voltageNow = 0;
    uint32_t totalVolt = 0;
    uint32_t measurementsCount = 0;
    uint32_t timeStart = micros();

    while (micros() - timeStart < periode) {
      int16_t valueADC = SENSOR2_ADS.readADC_SingleEnded(2);
      voltageNow = valueADC - zeroPoint;
      totalVolt += sq(voltageNow);
      measurementsCount++;
    }
    double rawValue = sqrt(totalVolt / measurementsCount) * ADC_SCALE_FACTOR;
    rawValue = rawValue * sensitivityValue;
    readingVoltage += rawValue;
  }

  return readingVoltage / SAMPLE_COUNT_VOLT;
}

double calcCurrentRMS(unsigned int pinCurrent, double calibrateValue, unsigned int numberOfSamples, int pinSelectADC) {
  const float adcReferenceVolt = 4.096;
  const int adcResolution = 65536;
  const float voltADCcount = (2 * adcReferenceVolt) / adcResolution;
  double totalADCVal;
  totalCurrent = 0;
  sqCurrent = 0;

  // Check if the selected sensor is available
  if (pinSelectADC == 1 && !sensor1_connected) {
    Serial.println("Sensor 1 not connected, returning 0 current");
    return 0.0;
  }
  if (pinSelectADC == 2 && !sensor2_connected) {
    Serial.println("Sensor 2 not connected, returning 0 current");
    return 0.0;
  }

  for (unsigned int n = 0; n < 100; n++) {
    int16_t adcValue;
    if (pinSelectADC == 1) {
      adcValue = SENSOR1_ADS.readADC_SingleEnded(pinCurrent);
    } else {
      adcValue = SENSOR2_ADS.readADC_SingleEnded(pinCurrent);
    }
    totalADCVal += adcValue;
  }

  float zeroPoint = totalADCVal / 100;
  zeroPoint = (zeroPoint - (adcResolution / 2)) * voltADCcount;
  for (unsigned int n = 0; n < numberOfSamples; n++) {
    int16_t adcValue;
    if (pinSelectADC == 1) {
      adcValue = SENSOR1_ADS.readADC_SingleEnded(pinCurrent);
    } else {
      adcValue = SENSOR2_ADS.readADC_SingleEnded(pinCurrent);
    }

    float calculateVolt = (adcValue - (adcResolution / 2)) * voltADCcount;
    double Value = calculateVolt - zeroPoint;
    sqCurrent = sq(Value);
    totalCurrent += sqCurrent;
  }
  currentRMS = calibrateValue * sqrt(totalCurrent / numberOfSamples);

  return currentRMS;
}

void readPanelSensorTask(void * pvParameters) {
  while (1) {
    readPanelSensor();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

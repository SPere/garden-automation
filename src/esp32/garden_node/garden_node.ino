#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Ensure "ArduinoJson by Benoit Blanchon" is installed
#include "DHT.h"

// Sensor Pin Definitions
#define DHTPIN 13     // Using GPIO 13 as established earlier
#define DHTTYPE DHT11
#define SOIL1_PIN 32
#define SOIL2_PIN 33
#define SOIL3_PIN 34
#define SOIL4_PIN 25  // Safe Analog pin 25
#define LDR_PIN 35

// Deep Sleep Settings (15 minutes = 900 seconds)
#define TIME_TO_SLEEP  900
#define uS_TO_S_FACTOR 1000000ULL

// Wi-Fi Credentials
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// C# Web API Endpoint URL
const char* serverName = "http://your-k8s-api-service/api/telemetry";

// Static unique device name descriptor
const char* deviceName = "esp32_garden_node_01";

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  
  // Give the DHT11 2.5 seconds to settle its electronics upon boot/sleep wakeup
  delay(2500); 
  dht.begin();

  // 1. Initialize Deep Sleep timer
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // 2. Read sensors and dump immediately to local serial (Always executes)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Basic fault safety catch
  if (isnan(humidity) || isnan(temperature)) {
    humidity = -1.0; temperature = -1.0;
  }

  int soil1 = analogRead(SOIL1_PIN);
  int soil2 = analogRead(SOIL2_PIN);
  int soil3 = analogRead(SOIL3_PIN);
  int soil4 = analogRead(SOIL4_PIN);
  int ldrValue = analogRead(LDR_PIN);

  // Debug Print
  Serial.print("Temp: "); Serial.print(temperature, 1);
  Serial.print("°C | Hum: "); Serial.print(humidity, 1);
  Serial.print("% | Soil1: "); Serial.print(soil1);
  Serial.print(" | Soil2: "); Serial.print(soil2);
  Serial.print(" | Soil3: "); Serial.print(soil3);
  Serial.print(" | Soil4: "); Serial.print(soil4);
  Serial.print(" | Light: "); Serial.println(ldrValue);

  // 3. Try to establish Wi-Fi link
  connectToWiFi();

  // 4. If connected, build nested JSON and transmit
  if (WiFi.status() == WL_CONNECTED) {
    sendStructuredTelemetry(temperature, humidity, soil1, soil2, soil3, soil4, ldrValue);
  } else {
    Serial.println("Skipping post window. Router unreachable.");
  }

  // 5. Tear down radio and execute low-power sleep state
  WiFi.disconnect(true);
  Serial.println("Entering deep sleep mode...");
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop() {
  // Stays empty due to deep sleep architecture
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected successfully!");
  } else {
    Serial.println("\nConnection timed out.");
  }
}

void sendStructuredTelemetry(float temp, float hum, int s1, int s2, int s3, int s4, int ldr) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");

  // Allocate JSON Document space (JsonDocument scales automatically in v7)
  JsonDocument doc;

  // Root elements
  doc["device"] = deviceName;
  // Omitted "timestamp" key entirely so C# falls back to safe server DateTime.UtcNow
  
  // Create nested reading array block
  JsonArray readings = doc["readings"].to<JsonArray>();

  // Element 1: Temperature
  JsonObject r1 = readings.add<JsonObject>();
  r1["sensor"] = "dht11_temp";
  r1["type"] = "temperature";
  r1["value"] = String(temp, 1);

  // Element 2: Humidity
  JsonObject r2 = readings.add<JsonObject>();
  r2["sensor"] = "dht11_hum";
  r2["type"] = "humidity";
  r2["value"] = String(hum, 1);

  // Element 3: Soil 1
  JsonObject r3 = readings.add<JsonObject>();
  r3["sensor"] = "soil1";
  r3["type"] = "soilmoisture";
  r3["value"] = String(s1);

  // Element 4: Soil 2
  JsonObject r4 = readings.add<JsonObject>();
  r4["sensor"] = "soil2";
  r4["type"] = "soilmoisture";
  r4["value"] = String(s2);

  // Element 5: Soil 3
  JsonObject r5 = readings.add<JsonObject>();
  r5["sensor"] = "soil3";
  r5["type"] = "soilmoisture";
  r5["value"] = String(s3);

  // Element 6: Soil 4
  JsonObject r6 = readings.add<JsonObject>();
  r6["sensor"] = "soil4";
  r6["type"] = "soilmoisture";
  r6["value"] = String(s4);

  // Element 7: LDR Light
  JsonObject r7 = readings.add<JsonObject>();
  r7["sensor"] = "meter1";
  r7["type"] = "meter";
  r7["value"] = String(ldr);

  // Serialize to continuous text string
  String jsonOutput;
  serializeJson(doc, jsonOutput);

  Serial.println("Posting payload structure to backend:");
  Serial.println(jsonOutput);

  // Send request
  int responseCode = http.POST(jsonOutput);
  
  if (responseCode > 0) {
    Serial.print("HTTP response code returned from API: ");
    Serial.println(responseCode);
  } else {
    Serial.print("Network routing error: ");
    Serial.println(http.errorToString(responseCode).c_str());
  }

  http.end();
}

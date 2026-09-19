#include "AsyncCam.hpp"
#include <WiFi.h>
#include "lwip/tcpip.h"

static const char* WIFI_SSID = "VM3290784-2";
static const char* WIFI_PASS = "eatark8kxprDcsx2";

esp32cam::Resolution initialResolution;

AsyncWebServer server(80);

unsigned long lastHeartbeat = 0;

// Track and print network configuration modifications
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("[SYSTEM] Wi-Fi layer connected to AP hardware.");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("[SYSTEM] Wi-Fi layer assigned IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("[SYSTEM] ALERT: Wi-Fi connection dropped from AP!");
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  esp32cam::setLogger(Serial);
  delay(1000);

  // Register event tracker before establishing link
  WiFi.onEvent(WiFiEvent);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  
  // CRUCIAL: Disable dynamic radio sleep. This prevents the chip's antenna 
  // from dropping offline or ignoring inbound browser requests.
  WiFi.setSleep(false); 
  WiFi.setTxPower(WIFI_POWER_19_5dBm); // Crank the antenna to maximum power

  Serial.printf("[SYSTEM] Attempting connection to SSID: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("[SYSTEM] Wi-Fi boot failure code: %d\n", WiFi.status());
    delay(5000);
    ESP.restart();
  }
  Serial.println("[SYSTEM] Wi-Fi loop core connected successfully.");
  delay(1000);

  {
    using namespace esp32cam;
    initialResolution = Resolution::find(1024, 768);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setBufferCount(1);
    cfg.setJpeg(15);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }

  Serial.println("camera starting");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  addRequestHandlers();

  Serial.println("[SYSTEM] Starting web server daemon...");
  //LOCK_TCPIP_CORE();   
  server.begin();
  //UNLOCK_TCPIP_CORE(); 
  Serial.println("[SYSTEM] Web server listening active.");
}

void loop() {
  // FreeRTOS structural loop delay
  delay(1);

  // Send a heartbeat metrics package to the terminal loop every 5 seconds
  if (millis() - lastHeartbeat >= 5000) {
    lastHeartbeat = millis();
    
    Serial.print("[HEARTBEAT] Wi-Fi Active: ");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("YES | IP: %s | Signal RSSI: %d dBm | Free Heap: %d bytes\n", 
                    WiFi.localIP().toString().c_str(), 
                    WiFi.RSSI(), 
                    ESP.getFreeHeap());
    } else {
      Serial.printf("NO (Status Code: %d) -> Attempting Auto-Reconnect...\n", WiFi.status());
      
      // Tell the Wi-Fi chip to reconnect to your Virgin Media network
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
  }
}


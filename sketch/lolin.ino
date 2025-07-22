#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

// Ganti sesuai
const char* ssid = "karimroy";
const char* password = "09871234";
const char* deviceName = "Lolin";  // Ganti: "Lolin", "Wemos1", "Wemos2"

// MAC semua device
uint8_t macLolin[]  = { 0xC4, 0xD8, 0xD5, 0x10, 0x70, 0xC5 };
uint8_t macWemos1[] = { 0x34, 0x5F, 0x45, 0x56, 0x29, 0x2A };
uint8_t macWemos2[] = { 0x34, 0x5F, 0x45, 0x55, 0xE2, 0xD0 };

// Web OTA
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 3000; // 3 detik

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Koneksi ke WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println();
  Serial.printf("Terhubung ke %s\n", ssid);
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());

  // Web OTA setup
  httpUpdater.setup(&server, "/update");
  server.on("/", HTTP_GET, []() {
    String html = "<html><head><title>" + String(deviceName) + "</title></head><body>";
    html += "<h1>Device: " + String(deviceName) + "</h1>";
    html += "<p><b>IP:</b> " + WiFi.localIP().toString() + "</p>";
    html += "<p><b>MAC:</b> " + WiFi.macAddress() + "</p>";
    html += "<p><a href='/update'>OTA Update</a></p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  server.begin();

  // ESP-NOW Setup
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW Init Failed!");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onReceive);

  // Tambah peer lain
  if (String(deviceName) != "Lolin")  esp_now_add_peer(macLolin, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  if (String(deviceName) != "Wemos1") esp_now_add_peer(macWemos1, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  if (String(deviceName) != "Wemos2") esp_now_add_peer(macWemos2, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  server.handleClient();

  // Kirim pesan setiap 3 detik
  if (millis() - lastSendTime > sendInterval) {
    sendMessage("Hello from " + String(deviceName));
    lastSendTime = millis();
  }
}

void sendMessage(String msg) {
  uint8_t* targets[] = {
    (String(deviceName) != "Lolin") ? macLolin : nullptr,
    (String(deviceName) != "Wemos1") ? macWemos1 : nullptr,
    (String(deviceName) != "Wemos2") ? macWemos2 : nullptr,
  };

  for (int i = 0; i < 3; i++) {
    if (targets[i] != nullptr) {
      esp_now_send(targets[i], (uint8_t *)msg.c_str(), msg.length());
    }
  }

  Serial.println("Sent: " + msg);
}

void onReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  String msg = String((char*)incomingData);
	Serial.printf("DARI %s ➜ %s\n", macStr, msg.c_str());
}

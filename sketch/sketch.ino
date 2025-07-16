#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FirebaseESP8266.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// WiFi credentials
const char* ssid = "karimroy";
const char* password = "09871234";

// Firebase credentials
#define FIREBASE_HOST "payunghitam-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_EMAIL "esp8266@yourapp.com"
#define FIREBASE_PASSWORD "password123"

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
FirebaseData fbdo;

String deviceStatus = "Unknown";

void lcdPrint(const String& line1, const String& line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcdPrint("WiFi Connect", ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcdPrint("Connecting...", ".");
  }

  lcdPrint("Connected", WiFi.localIP().toString());
  Serial.println();
  Serial.println("IP address: " + WiFi.localIP().toString());

  // Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_EMAIL, FIREBASE_PASSWORD);
  Firebase.reconnectWiFi(true);

  // Set status ke Online
  deviceStatus = "Online";
  Firebase.setString(fbdo, "/device/status", deviceStatus);

  // Simpan log boot time
  String timeStr = String(millis());
  String logPath = "/device/logs/" + timeStr;
  Firebase.setString(fbdo, logPath, "Booted at millis: " + timeStr);

  // Halaman utama: tampilkan status + log dari Firebase
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>ESP8266 Status</title></head><body style='font-family:sans-serif;background:#111;color:#fff;padding:20px;'>";
    html += "<h2>Status Perangkat</h2>";
    html += "<p>Status: <strong>" + deviceStatus + "</strong></p>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<h2>Riwayat Log</h2><ul>";

    if (Firebase.getJSON(fbdo, "/device/logs")) {
      FirebaseJson& json = fbdo.jsonObject();
      size_t count = json.iteratorBegin();
      for (size_t i = 0; i < count; i++) {
        String key, value;
        int type;
        json.iteratorGet(i, type, key, value);
        html += "<li><b>" + key + ":</b> " + value + "</li>";
      }
      json.iteratorEnd();
    } else {
      html += "<li>Error ambil log: " + fbdo.errorReason() + "</li>";
    }

    html += "</ul>";
    html += "<p><a href='/update' style='color:cyan'>Update Firmware</a></p>";
    html += "</body></html>";

    server.send(200, "text/html", html);
    lcdPrint("Page: /", "View logs");
  });

  // OTA Update via /update
  httpUpdater.setup(&server);

  server.begin();
  Serial.println("HTTP server started");
  lcdPrint("OTA Aktif", "/update");
}

void loop() {
  server.handleClient();
}
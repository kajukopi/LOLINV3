#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// WiFi credentials
const char* ssid = "karimroy";
const char* password = "09871234";

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

String deviceStatus = "Online";
unsigned long lastDisplayMillis = 0;
bool showIpNow = false;

// LCD helper
void lcdMessage(String line1, String line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

// Initialize LCD
void initLCD() {
  lcd.init();
  lcd.backlight();
  lcdMessage("WiFi Connect", ssid);
}

// Connect to WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    dotCount++;
    lcdMessage("Connecting" + String(dotCount % 4 == 0 ? "..." : dotCount % 4 == 1 ? "." : dotCount % 4 == 2 ? ".." : "..."), ssid);
  }
  lcdMessage("WiFi OK", WiFi.localIP().toString());
  delay(1500);
}

// Web server endpoints
void setupEndpoints() {
  server.on("/", []() {
    String response = "";
    response += "Status: " + deviceStatus + "\n";
    response += "IP: " + WiFi.localIP().toString() + "\n";
    response += "Uptime (ms): " + String(millis()) + "\n";
    server.send(200, "text/plain", response);
  });

  httpUpdater.setup(&server);
}

void handleTimedDisplay() {
  unsigned long currentMillis = millis();

  // Setiap 10 detik, tampilkan IP selama 3 detik
  if (!showIpNow && currentMillis - lastDisplayMillis >= 10000) {
    showIpNow = true;
    lastDisplayMillis = currentMillis;
    lcdMessage("IP Address:", WiFi.localIP().toString());
  }

  // Setelah 3 detik, kembalikan ke tampilan status
  if (showIpNow && currentMillis - lastDisplayMillis >= 3000) {
    showIpNow = false;
    lastDisplayMillis = currentMillis;
    lcdMessage("Status:", deviceStatus);
  }
}

void setup() {
  initLCD();
  connectWiFi();
  setupEndpoints();
  server.begin();
  lcdMessage("HTTP Server", "Started");
}

void loop() {
  server.handleClient();
  handleTimedDisplay();
}
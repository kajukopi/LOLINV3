#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const char* ssid = "karimroy";
const char* password = "09871234";

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

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

  // OTA Web Update setup
  httpUpdater.setup(&server); // default di path "/update"
  server.begin();
  Serial.println("HTTP server started");
  lcdPrint("OTA Aktif", "/update");
}

void loop() {
  server.handleClient();
}
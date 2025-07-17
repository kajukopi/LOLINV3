#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Sertakan file firebase.cpp secara langsung
#include "firebase.cpp"

const char* ssid = "karimroy";
const char* password = "09871234";

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connect");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("IP address: " + WiFi.localIP().toString());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP Address:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  // Inisialisasi Firebase dan log boot
  initFirebase();
  logBootTime();

  // REST API JSON Endpoint
  server.on("/", []() {
    FirebaseJson responseJson;
    responseJson.add("status", deviceStatus);
    responseJson.add("ip", WiFi.localIP().toString());

    FirebaseJson logJson;
    if (Firebase.getJSON(fbdo, "/device/logs")) {
      FirebaseJson& logs = fbdo.jsonObject();
      size_t count = logs.iteratorBegin();
      for (size_t i = 0; i < count; i++) {
        String key, value;
        int type;
        logs.iteratorGet(i, type, key, value);
        logJson.add(key, value);
      }
      logs.iteratorEnd();
    } else {
      logJson.add("error", fbdo.errorReason());
    }

    responseJson.set("logs", logJson);

    String jsonStr;
    responseJson.toString(jsonStr, true);
    server.send(200, "application/json", jsonStr);
  });

  // OTA
  httpUpdater.setup(&server);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
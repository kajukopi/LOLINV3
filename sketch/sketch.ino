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
#define FIREBASE_API_KEY "AIzaSyBczsujBWZbP2eq5C1YR1JF3xPixWVYnxY"

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// State
String deviceStatus = "Unknown";

// ---------- Functions ----------

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connect");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
}

void connectWiFi() {
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
}

void initFirebase() {
  config.api_key = FIREBASE_API_KEY;
  config.database_url = "https://" FIREBASE_HOST;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void logDeviceStatus() {
  deviceStatus = "Online";
  Firebase.setString(fbdo, "/device/status", deviceStatus);

  String timeStr = String(millis());
  String logPath = "/device/logs/" + timeStr;
  Firebase.setString(fbdo, logPath, "Booted at millis: " + timeStr);
}

void setupEndpoints() {
  server.on("/", []() {
    String response = "Device Status: " + deviceStatus + "\n";
    response += "IP Address: " + WiFi.localIP().toString() + "\n";
    response += "Logs:\n";

    if (Firebase.getJSON(fbdo, "/device/logs")) {
      FirebaseJson& logs = fbdo.jsonObject();
      size_t count = logs.iteratorBegin();
      for (size_t i = 0; i < count; i++) {
        String key, value;
        int type;
        logs.iteratorGet(i, type, key, value);
        response += " - " + key + ": " + value + "\n";
      }
      logs.iteratorEnd();
    } else {
      response += "Error: " + fbdo.errorReason() + "\n";
    }

    server.send(200, "text/plain", response);
  });

  httpUpdater.setup(&server);
}

// ---------- Setup & Loop ----------

void setup() {
  Serial.begin(115200);
  initLCD();
  connectWiFi();
  initFirebase();
  logDeviceStatus();
  setupEndpoints();
  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
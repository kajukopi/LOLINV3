#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FirebaseESP8266.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// WiFi credentials
const char* ssid = "karimroy";
const char* password = "09871234";

// Firebase config
#define FIREBASE_HOST "payunghitam-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyBczsujBWZbP2eq5C1YR1JF3xPixWVYnxY"
#define FIREBASE_EMAIL "esp8266@yourapp.com"
#define FIREBASE_PASSWORD "password123"

// IR sensor pins
#define IR_SENSOR_1_PIN D5
#define IR_SENSOR_2_PIN D6

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String deviceStatus = "Unknown";
unsigned long lastTriggerTime = 0;
bool lastSensor1State = false;
bool lastSensor2State = false;

void lcdPrint(const String& line1, const String& line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void checkIRSensors() {
  bool sensor1 = digitalRead(IR_SENSOR_1_PIN) == LOW;
  bool sensor2 = digitalRead(IR_SENSOR_2_PIN) == LOW;

  String newStatus;

  if (sensor1 && !lastSensor1State) {
    newStatus = "TERDETEKSI - S1";
    lcdPrint("Sensor 1", "TERDETEKSI");
  } else if (sensor2 && !lastSensor2State) {
    newStatus = "TERDETEKSI - S2";
    lcdPrint("Sensor 2", "TERDETEKSI");
  } else if (!sensor1 && !sensor2 && (lastSensor1State || lastSensor2State)) {
    newStatus = "AMAN";
    lcdPrint("Sensor IR", "AMAN");
  }

  // Jika status berubah → update ke Firebase dan simpan log
  if (newStatus.length() > 0 && newStatus != deviceStatus && millis() - lastTriggerTime > 500) {
    deviceStatus = newStatus;
    lastTriggerTime = millis();

    Firebase.setString(fbdo, "/device/status", deviceStatus);
    String logPath = "/device/logs/" + String(millis());
    Firebase.setString(fbdo, logPath, "Status: " + deviceStatus + " at millis: " + String(millis()));
  }

  lastSensor1State = sensor1;
  lastSensor2State = sensor2;
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcdPrint("WiFi Connect", ssid);

  pinMode(IR_SENSOR_1_PIN, INPUT);
  pinMode(IR_SENSOR_2_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcdPrint("Connecting...", ".");
  }

  lcdPrint("Connected", WiFi.localIP().toString());
  Serial.println();
  Serial.println("IP address: " + WiFi.localIP().toString());

  // Firebase config
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  deviceStatus = "Online";
  Firebase.setString(fbdo, "/device/status", deviceStatus);
  String logPath = "/device/logs/" + String(millis());
  Firebase.setString(fbdo, logPath, "Booted at millis: " + String(millis()));

  // Halaman Web
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

  // OTA
  httpUpdater.setup(&server);
  server.begin();

  Serial.println("HTTP server started");
  lcdPrint("OTA Aktif", "/update");
}

void loop() {
  server.handleClient();
  checkIRSensors();
}
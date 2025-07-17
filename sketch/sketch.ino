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

void lcdMessage(String line1, String line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcdMessage("WiFi Connect", ssid);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    dotCount++;
    lcdMessage("Connecting" + String(dotCount % 4 == 0 ? "..." : dotCount % 4 == 1 ? "." : dotCount % 4 == 2 ? ".." : "..."), ssid);
  }

  lcdMessage("WiFi Connected", WiFi.localIP().toString());
  delay(2000);
}

void initFirebase() {
  lcdMessage("Init Firebase");
  config.api_key = FIREBASE_API_KEY;
  config.database_url = "https://" FIREBASE_HOST;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  lcdMessage("Firebase OK");
  delay(1000);
}

void logDeviceStatus() {
  deviceStatus = "Online";
  Firebase.setString(fbdo, "/device/status", deviceStatus);

  String timeStr = String(millis());
  String logPath = "/device/logs/" + timeStr;
  Firebase.setString(fbdo, logPath, "Booted at millis: " + timeStr);

  lcdMessage("Logged at", timeStr);
  delay(1000);
}

void setupEndpoints() {
  server.on("/", []() {
    String logHtml = "";

    if (Firebase.getJSON(fbdo, "/device/logs")) {
      FirebaseJson& logs = fbdo.jsonObject();
      size_t count = logs.iteratorBegin();
      for (size_t i = 0; i < count; i++) {
        String key, value;
        int type;
        logs.iteratorGet(i, type, key, value);
        logHtml += "<li><strong>" + key + ":</strong> " + value + "</li>\n";
      }
      logs.iteratorEnd();
    } else {
      logHtml += "<li>Error: " + fbdo.errorReason() + "</li>\n";
    }

    // Template HTML dengan token yang bisa diganti
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>ESP8266 Status</title>
        <meta charset="UTF-8">
        <style>
          body { font-family: Arial; text-align: center; margin-top: 40px; }
          h1 { color: #333; }
          ul { text-align: left; max-width: 400px; margin: auto; }
          li { margin: 4px 0; }
        </style>
      </head>
      <body>
        <h1>ESP8266 Device</h1>
        <p><strong>Status:</strong> {{status}}</p>
        <p><strong>IP Address:</strong> {{ip}}</p>
        <p><strong>Uptime:</strong> {{millis}} ms</p>
        <h3>Log:</h3>
        <ul>
          {{logs}}
        </ul>
      </body>
      </html>
    )rawliteral";

    html.replace("{{status}}", deviceStatus);
    html.replace("{{ip}}", WiFi.localIP().toString());
    html.replace("{{millis}}", String(millis()));
    html.replace("{{logs}}", logHtml);

    server.send(200, "text/html", html);
  });

  httpUpdater.setup(&server);
}

// ---------- Setup & Loop ----------

void setup() {
  initLCD();
  connectWiFi();
  initFirebase();
  logDeviceStatus();
  setupEndpoints();

  server.begin();
  lcdMessage("HTTP Server", "Started");
}

void loop() {
  server.handleClient();
}
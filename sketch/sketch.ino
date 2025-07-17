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

String deviceStatus = "Unknown";

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

// Firebase setup
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

// Log device status to Firebase
void logDeviceStatus() {
  deviceStatus = "Online";
  Firebase.setString(fbdo, "/device/status", deviceStatus);
  String timeStr = String(millis());
  String logPath = "/device/logs/" + timeStr;
  Firebase.setString(fbdo, logPath, "Booted at millis: " + timeStr);

  lcdMessage("Logged at", timeStr);
  delay(1000);
}

// Web server endpoints
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
        logHtml += "<li class='list-group-item'><strong>" + key + ":</strong> " + value + "</li>\n";
      }
      logs.iteratorEnd();
    } else {
      logHtml += "<li class='list-group-item text-danger'>Error: " + fbdo.errorReason() + "</li>\n";
    }

    String html = R"rawliteral(
      <!DOCTYPE html>
      <html lang="en">
      <head>
        <meta charset="UTF-8">
        <title>ESP8266 Device</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link href="https://cdnjs.cloudflare.com/ajax/libs/mdb-ui-kit/6.4.2/mdb.min.css" rel="stylesheet">
        <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css" rel="stylesheet">
      </head>
      <body class="bg-light">
        <div class="container mt-5">
          <div class="card shadow-3">
            <div class="card-body text-center">
              <h3 class="card-title"><i class="fas fa-microchip"></i> ESP8266 Status</h3>
              <p class="card-text"><strong>Status:</strong> {{status}}</p>
              <p class="card-text"><strong>IP Address:</strong> {{ip}}</p>
              <p class="card-text"><strong>Uptime:</strong> {{millis}} ms</p>
              <h5 class="mt-4"><i class="fas fa-clipboard-list"></i> Logs</h5>
              <ul class="list-group text-start mt-3">
                {{logs}}
              </ul>
            </div>
          </div>
        </div>
        <script src="https://cdnjs.cloudflare.com/ajax/libs/mdb-ui-kit/6.4.2/mdb.min.js"></script>
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

void handleTimedDisplay() {
  unsigned long currentMillis = millis();

  // Setiap 10 detik, aktifkan tampilan IP selama 3 detik
  if (!showIpNow && currentMillis - lastDisplayMillis >= 10000) {
    showIpNow = true;
    lastDisplayMillis = currentMillis;
    lcdMessage("IP Address:", WiFi.localIP().toString());
  }

  // Setelah 3 detik, kembalikan ke tampilan normal
  if (showIpNow && currentMillis - lastDisplayMillis >= 3000) {
    showIpNow = false;
    lastDisplayMillis = currentMillis;
    lcdMessage("Status:", deviceStatus);
  }
}

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
  handleTimedDisplay(); // Tambahkan ini
}
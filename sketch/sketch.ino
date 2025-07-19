#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SDFS.h>         // Ganti SD.h → SDFS.h
#include <FS.h>           // Untuk File system interface

// WiFi credentials
const char* ssid = "karimroy";
const char* password = "09871234";

// Global objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
File fsUploadFile;

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

// Setup SD card
void setupSD() {
  if (!SDFS.begin()) {
    Serial.println("SDFS init failed!");
    lcdMessage("SD Init Failed");
    deviceStatus = "SD Failed";
    return;
  }

  Serial.println("SDFS initialized.");
  lcdMessage("SD Card", "Initialized");
}

// Setup web endpoints
void setupEndpoints() {
  // Status info
  server.on("/", []() {
    String response = "";
    response += "Status: " + deviceStatus + "\n";
    response += "IP: " + WiFi.localIP().toString() + "\n";
    response += "Uptime (ms): " + String(millis()) + "\n";
    server.send(200, "text/plain", response);
  });

  // Upload handler
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "Upload OK");
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = "/" + upload.filename;
      fsUploadFile = SDFS.open(filename, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      if (fsUploadFile) fsUploadFile.close();
    }
  });

  // Serve SD files
  server.onNotFound([]() {
    String path = server.uri();
    if (path.endsWith("/")) path += "index.html";
    File file = SDFS.open(path, "r");
    if (!file) {
      server.send(404, "text/plain", "File Not Found");
      return;
    }

    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";

    server.streamFile(file, contentType);
    file.close();
  });

  httpUpdater.setup(&server);
}

// Tampilan LCD bergantian
void handleTimedDisplay() {
  unsigned long currentMillis = millis();

  if (!showIpNow && currentMillis - lastDisplayMillis >= 10000) {
    showIpNow = true;
    lastDisplayMillis = currentMillis;
    lcdMessage("IP Address:", WiFi.localIP().toString());
  }

  if (showIpNow && currentMillis - lastDisplayMillis >= 3000) {
    showIpNow = false;
    lastDisplayMillis = currentMillis;
    lcdMessage("Status:", deviceStatus);
  }
}

void setup() {
  Serial.begin(115200);
  initLCD();
  connectWiFi();
  setupSD();
  setupEndpoints();
  server.begin();
  lcdMessage("HTTP Server", "Started");
}

void loop() {
  server.handleClient();
  handleTimedDisplay();
}
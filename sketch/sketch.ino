#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Updater.h>

const char* ssid = "karimroy";
const char* password = "09871234";

// LCD I2C 16x2, alamat bisa 0x27 atau 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

// Halaman HTML sederhana tanpa Bootstrap
const char* upload_html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>ESP8266 OTA</title>
  </head>
  <body style="font-family:sans-serif; background:#111; color:#fff; text-align:center;">
    <h2>OTA Update</h2>
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="firmware"><br><br>
      <input type="submit" value="Upload">
    </form>
    <p>Upload firmware .bin ke ESP8266</p>
  </body>
</html>
)rawliteral";

// Fungsi tampil ke LCD
void lcdPrint(const String& line1, const String& line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setup() {
  lcd.begin(16, 2);
  lcd.backlight();
  lcdPrint("WiFi Connect", ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcdPrint("Connecting...", ".");
  }

  IPAddress ip = WiFi.localIP();
  lcdPrint("WiFi Connected", ip.toString());

  // Root page: form upload firmware
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", upload_html);
    lcdPrint("Akses Web", "/");
  });

  // OTA Upload Handler
  server.on("/update", HTTP_POST, []() {
    String msg = Update.hasError() ? "Update Gagal" : "Update OK";
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", msg + ", Restart...");
    lcdPrint(msg, "Restart...");
    delay(1500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
      lcdPrint("Mulai Upload", upload.filename.substring(0, 16));
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        lcdPrint("Update Error!", "Begin Fail");
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      lcdPrint("Menulis...", String(upload.currentSize) + "B");
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        lcdPrint("Write Error!", "");
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        lcdPrint("Update OK", String(upload.totalSize) + " byte");
      } else {
        lcdPrint("End Error!", "");
      }
    }
  });

  server.begin();
  lcdPrint("OTA Aktif", "Buka browser");
}

void loop() {
  server.handleClient();
}
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Updater.h>

// Ganti dengan WiFi kamu
const char* ssid = "karimroy";
const char* password = "09871234";

// Ganti alamat I2C jika perlu (biasanya 0x27 atau 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

// Halaman HTML OTA ringan (tanpa Bootstrap)
const char* upload_html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>ESP OTA</title>
  </head>
  <body style="background:#111; color:#fff; text-align:center; font-family:sans-serif;">
    <h2>ESP8266 OTA Update</h2>
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="firmware"><br><br>
      <input type="submit" value="Upload">
    </form>
    <p>Upload .bin firmware ke ESP</p>
  </body>
</html>
)rawliteral";

// Fungsi cetak ke LCD
void lcdPrint(const String& line1, const String& line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setup() {
  lcd.init();         // ✅ Gunakan init() sesuai permintaan
  lcd.backlight();    // ✅ Nyalakan lampu belakang
  lcdPrint("WiFi Connect", ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcdPrint("Connecting...", ".");
  }

  IPAddress ip = WiFi.localIP();
  lcdPrint("WiFi Terhubung", ip.toString());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", upload_html);
    lcdPrint("Akses Web", "/");
  });

  server.on("/update", HTTP_POST, []() {
    String msg = Update.hasError() ? "Update Gagal" : "Update OK";
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", msg + ", Restart...");
    lcdPrint(msg, "Restarting...");
    delay(1500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
      lcdPrint("Mulai Upload", upload.filename.substring(0, 16));
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        lcdPrint("Update Error!", "Begin Failed");
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
        lcdPrint("Update Gagal", "End Failed");
      }
    }
  });

  server.begin();
  lcdPrint("OTA Aktif", "Buka Browser");
}

void loop() {
  server.handleClient();
}
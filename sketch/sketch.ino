#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Updater.h>

const char* ssid = "karimroy";
const char* password = "09871234";

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

size_t totalSize = 0;

void lcdProgressBar(int percent) {
  int bars = map(percent, 0, 100, 0, 10);
  lcd.setCursor(0, 1);
  lcd.print("[");
  for (int i = 0; i < 10; i++) lcd.print(i < bars ? "#" : " ");
  lcd.print("]");
}

void lcdPrint(const String& l1, const String& l2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1);
  lcd.setCursor(0, 1);
  lcd.print(l2);
}

// 🔄 Navbar dengan "Update"
String navBar = R"rawliteral(
  <nav style="background:#222;padding:10px;color:#fff;text-align:center">
    <a href="/" style="color:#0ff;margin:0 10px;">Home</a>
    <a href="/update" style="color:#0ff;margin:0 10px;">Update</a>
    <a href="/status" style="color:#0ff;margin:0 10px;">Status</a>
    <a href="/log" style="color:#0ff;margin:0 10px;">Log</a>
  </nav>
)rawliteral";

String homePage = R"rawliteral(
<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Home</title></head>
<body style="font-family:sans-serif;background:#111;color:#fff;text-align:center;">
$NAV$
<h2>ESP8266 Web OTA</h2>
<p>Firmware update, monitoring & logs</p>
</body></html>
)rawliteral";

// ⚙️ Halaman Update (dari sebelumnya /ota)
String updatePage = R"rawliteral(
<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Update</title></head>
<body style="font-family:sans-serif;background:#111;color:#fff;text-align:center;">
$NAV$
<h2>Firmware Update</h2>
<form method='POST' action='/upload' enctype='multipart/form-data'>
  <input type='file' name='firmware'><br><br>
  <input type='submit' value='Upload'>
</form>
</body></html>
)rawliteral";

String statusPage() {
  IPAddress ip = WiFi.localIP();
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head><body style='background:#111;color:#fff;text-align:center;font-family:sans-serif;'>";
  html += navBar;
  html += "<h3>Status</h3><p>IP: " + ip.toString() + "</p></body></html>";
  return html;
}

String logPage = R"rawliteral(
<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Log</title></head>
<body style="font-family:sans-serif;background:#111;color:#fff;text-align:center;">
$NAV$
<h2>Logs</h2>
<p>OTA terakhir: Berhasil</p>
<p>Ukuran: 230,000 bytes</p>
</body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcdPrint("WiFi Connect", ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcdPrint("Connecting...", ".");
    Serial.print(".");
  }

  lcdPrint("Connected", WiFi.localIP().toString());
  Serial.println("\nConnected to WiFi");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  server.on("/", []() {
    String page = homePage;
    page.replace("$NAV$", navBar);
    server.send(200, "text/html", page);
    lcdPrint("Page: Home");
  });

  // Halaman update baru (dulu: /ota)
  server.on("/update", []() {
    String page = updatePage;
    page.replace("$NAV$", navBar);
    server.send(200, "text/html", page);
    lcdPrint("Page: Update");
  });

  server.on("/status", []() {
    String html = statusPage();
    server.send(200, "text/html", html);
    lcdPrint("Page: Status");
  });

  server.on("/log", []() {
    String page = logPage;
    page.replace("$NAV$", navBar);
    server.send(200, "text/html", page);
    lcdPrint("Page: Log");
  });

  // Handler POST Upload
  server.on("/upload", HTTP_POST, []() {
    bool ok = !Update.hasError();
    String msg = ok ? "Sukses" : "Gagal";
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", msg + ", restart...");
    lcdPrint("OTA " + msg, "Restart...");
    delay(2000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update Start: %s\n", upload.filename.c_str());
      totalSize = 0;
      lcdPrint("Upload:", upload.filename.substring(0, 16));

      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        Update.printError(Serial);
        lcdPrint("Begin Failed", "");
        return;
      }

    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
        lcdPrint("Write Error", "Restart...");
        delay(2000);
        ESP.restart();
      }

      totalSize += upload.currentSize;
      int percent = (totalSize * 100) / upload.totalSize;

      lcd.setCursor(0, 0);
      lcd.print("Upload: ");
      lcd.print(percent);
      lcd.print("%   ");
      lcdProgressBar(percent);

    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u bytes\n", totalSize);
        lcdPrint("Update Sukses", String(totalSize) + "B");
      } else {
        Update.printError(Serial);
        lcdPrint("Update Gagal", "End failed");
      }
    }
  });

  server.begin();
  lcdPrint("OTA Aktif", WiFi.localIP().toString());
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
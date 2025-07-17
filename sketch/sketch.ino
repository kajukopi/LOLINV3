#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <FirebaseESP8266.h>
#include <Updater.h>

#define IR_SENSOR_PIN D5  // Pin sensor Flying Fish IR

const char* ssid = "karimroy";
const char* password = "09871234";

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

// Firebase config
#define FIREBASE_HOST "payunghitam-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyBczsujBWZbP2eq5C1YR1JF3xPixWVYnxY"
FirebaseData firebaseData;

String sensorStatus = "Tidak Aktif";

void handleRoot() {
  String page = "<html><head><meta charset='UTF-8'><title>ESP8266 Status</title>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/mdb-ui-kit/6.4.0/mdb.min.css'/>";
  page += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css'/>";
  page += "</head><body><div class='container mt-5'>";
  page += "<h3><i class='fas fa-microchip'></i> ESP8266 Status</h3><hr/>";
  page += "<p><strong>Sensor IR:</strong> " + sensorStatus + "</p>";
  page += "<p><strong>Firebase:</strong> " + String(Firebase.ready() ? "Terhubung" : "Tidak Terhubung") + "</p>";
  page += "</div></body></html>";
  server.send(200, "text/html", page);
}

void setup() {
  pinMode(IR_SENSOR_PIN, INPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Tersambung");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();

  bool irDetected = digitalRead(IR_SENSOR_PIN) == LOW; // LOW jika terdeteksi
  String newStatus = irDetected ? "Terdeteksi" : "Tidak Aktif";

  if (newStatus != sensorStatus) {
    sensorStatus = newStatus;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IR: " + sensorStatus);

    if (Firebase.ready()) {
      Firebase.setString(firebaseData, "/esp8266/sensorIR", sensorStatus);
    }
  }

  delay(1000);
}
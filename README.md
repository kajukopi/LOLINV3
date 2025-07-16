# 🔧 LOLINV3 OTA + Firebase + LCD Web Interface

Proyek ini merupakan integrasi antara **ESP8266 (LOLIN V3)** dengan fitur:

- ✅ **Over-the-Air (OTA) Update** melalui halaman `/update`
- ✅ **Firebase Realtime Database**: menyimpan status & log
- ✅ **LCD I2C 16x2**: menampilkan status real-time
- ✅ **Web Server** ESP8266: halaman `/` menampilkan log & status
- ✅ **GitHub Actions CI**: compile otomatis & rilis firmware `.bin`

---

## 🔌 Koneksi WiFi

```cpp
const char* ssid = "karimroy";
const char* password = "09871234";
```

---

## ☁️ Firebase Configuration

```cpp
#define FIREBASE_HOST "payunghitam-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyBczsujBWZbP2eq5C1YR1JF3xPixWVYnxY"
#define FIREBASE_EMAIL "esp8266@yourapp.com"
#define FIREBASE_PASSWORD "password123"
```

- Log disimpan ke path: `/device/logs/{millis()}`
- Status disimpan ke: `/device/status`

---

## 📟 LCD Display

LCD I2C 16x2 (`0x27`) akan menampilkan:
- Status WiFi & IP
- Halaman yang sedang diakses
- Progres OTA Upload (%)

---

## 🌐 Halaman Web ESP8266

| URL        | Fungsi                         |
|------------|--------------------------------|
| `/`        | Menampilkan status & log dari Firebase |
| `/update`  | Halaman upload OTA firmware    |

---

## 🛠️ GitHub Actions CI/CD

File Workflow: `.github/workflows/compile.yml`

- Auto-compile saat push ke `main`
- Rilis firmware `.bin` dengan nama `LOLINV3-YYYY-MM-DD-HHMM.bin`
- Include tanggal & jam WIB di deskripsi rilis
- Include OTA-signed binary (if applicable)

---

## 📦 Library yang Digunakan

- [`FirebaseESP8266`](https://github.com/mobizt/Firebase-ESP8266)
- `LiquidCrystal_I2C`
- `ESP8266WiFi`
- `ESP8266WebServer`
- `ESP8266HTTPUpdateServer`

---

## 👨‍💻 Author

**Karim Roy** — [`payunghitam.web.app`](https://payunghitam.web.app)  
📡 Project IoT monitoring & OTA management

---

## 📁 Struktur File Penting

```
/sketch/sketch.ino             <-- Source utama
/.github/workflows/compile.yml <-- CI untuk compile & release
```

---

## 🚀 Update Terakhir

Final Code dan workflow telah dikunci:
- 📄 sketch.ino ✅
- 🧪 compile.yml ✅
- 🔐 API Key ✅
- 📲 Firebase host & credential ✅
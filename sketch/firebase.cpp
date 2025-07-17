#include "firebase.h"

// Firebase credentials
#define FIREBASE_HOST "payunghitam-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_EMAIL "esp8266@yourapp.com"
#define FIREBASE_PASSWORD "password123"
#define FIREBASE_API_KEY "AIzaSyBczsujBWZbP2eq5C1YR1JF3xPixWVYnxY"

// Global Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String deviceStatus = "Unknown";

void initFirebase() {
  config.api_key = FIREBASE_API_KEY;
  config.database_url = "https://" FIREBASE_HOST;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  deviceStatus = "Online";
  Firebase.setString(fbdo, "/device/status", deviceStatus);
}

void logBootTime() {
  String timeStr = String(millis());
  String logPath = "/device/logs/" + timeStr;
  Firebase.setString(fbdo, logPath, "Booted at millis: " + timeStr);
}
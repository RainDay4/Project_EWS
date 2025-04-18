#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
const char* ssid = "realme narzo";
const char* password = "cobalagi";
const String token = "8098425116:AAEi8gqf97_4AU0qyMa8-vUUrjzVOk0j3GU";
const String chatID = "8073804850";
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 23

// Variabel Level
int previousLevel = -1; // Untuk cek perubahan level
const int tinggiWadah = 30; // cm
const int pinBad = 12; const int pinGood = 13;
const int pinGreat = 14; const int pinExcellent = 15;
// Pin LED
const int redLED = 4; const int greenLED = 2;
// Deklarasi fungsi
float bacaJarak();
int tentukanLevel(int ketinggianAir);
String generatePesan(int level, int ketinggianAir);
// Variables to track button states
int lastStateBad = HIGH; int lastStateGood = HIGH;
int lastStateGreat = HIGH; int lastStateExcellent = HIGH;
// Koneksi Wi-Fi
void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000); Serial.print("."); attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed! Retrying in 10 seconds...");}
}
// Function to send Telegram message
void sendMessage(const char* message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    connectToWiFi();
  }
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String encodedMessage = "";
    for (int i = 0; i < strlen(message); i++) {
      if (message[i] == ' ') {
        encodedMessage += "%20";
      } else {
        encodedMessage += message[i];
      }
    }
    String url = "https://api.telegram.org/bot" + String(token) +
                 "/sendMessage?chat_id=" + String(chatID) +
                 "&text=" + encodedMessage;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Message sent successfully");
    } else {
      Serial.print("Error sending message. HTTP Code: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi is still disconnected, message not sent.");
  }
}
void setup() {
  Serial.begin(115200);
  connectToWiFi();
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(redLED, OUTPUT); pinMode(greenLED, OUTPUT);
  pinMode(pinBad, INPUT_PULLDOWN); pinMode(pinGood, INPUT_PULLDOWN);
  pinMode(pinGreat, INPUT_PULLDOWN);pinMode(pinExcellent, INPUT_PULLDOWN);
  digitalWrite(redLED, LOW); digitalWrite(greenLED, LOW);
}
// Fungsi baca jarak
float bacaJarak() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float jarak = duration * 0.0343 / 2;
  return jarak;
}
// Fungsi tentukan level
int tentukanLevel(int ketinggianAir) {
  if (ketinggianAir <= 10) return 0;      // Aman
  else if (ketinggianAir <= 15) return 1; // Waspada
  else if (ketinggianAir <= 20) return 2; // Siaga
  else return 3;                         // Bahaya
}
// Fungsi generate pesan
String generatePesan(int level, int ketinggianAir) {
  String status;
  switch (level) {
    case 0: status = "🟢 AMAN"; break;
    case 1: status = "🟡 WASPADA"; break;
    case 2: status = "🟠 SIAGA"; break;
    case 3: status = "🔴 BAHAYA"; break;
  }
  return "Status Banjir: " + status + "\nKetinggian Air: " + String(ketinggianAir) + " cm";
}

void loop() {
  float jarak = bacaJarak();
  int ketinggianAir = tinggiWadah - jarak;
    // Validasi ketinggian air tidak negatif
  if (ketinggianAir < 0) ketinggianAir = 0;
    int currentLevel = tentukanLevel(ketinggianAir);
    // Kontrol LED
    digitalWrite(redLED, (currentLevel >= 2) ? HIGH : LOW);   // Siaga/Bahaya
    digitalWrite(greenLED, (currentLevel <= 1) ? HIGH : LOW); // Aman/Waspada
  if (currentLevel != previousLevel) {
    String pesan = generatePesan(currentLevel, ketinggianAir);
    sendMessage(pesan.c_str());  // Konversi String ke const char*
    previousLevel = currentLevel;
  }
    // Read button states
  int stateBad = digitalRead(pinBad);
  int stateGood = digitalRead(pinGood);
  int stateGreat = digitalRead(pinGreat);
  int stateExcellent = digitalRead(pinExcellent);
  // Calculate how many buttons are pressed (LOW state for active button)
  int pressedButtons = 0;
  if (stateBad == LOW) pressedButtons++;
  if (stateGood == LOW) pressedButtons++;
  if (stateGreat == LOW) pressedButtons++;
  if (stateExcellent == LOW) pressedButtons++;
  if (pressedButtons == 1) {
    digitalWrite(greenLED, HIGH);  // Green LED ON for Bad
    digitalWrite(redLED, LOW);     // Red LED OFF for Bad
  } 
  else if (pressedButtons == 2) {
    digitalWrite(greenLED, HIGH);  // Green LED ON for Good
    digitalWrite(redLED, LOW);     // Red LED OFF for Good
  }
  else if (pressedButtons == 3) {
    digitalWrite(redLED, HIGH);    // Red LED ON for Great
    digitalWrite(greenLED, LOW);   // Green LED OFF for Great
  } 
  else if (pressedButtons == 4) {
    digitalWrite(redLED, HIGH);    // Red LED ON for Excellent
    digitalWrite(greenLED, LOW);   // Green LED OFF for Excellent
  }
  // Kontrol buzzer untuk level Bahaya
  digitalWrite(BUZZER_PIN, (currentLevel == 3) ? HIGH : LOW);
  delay(5000);
}

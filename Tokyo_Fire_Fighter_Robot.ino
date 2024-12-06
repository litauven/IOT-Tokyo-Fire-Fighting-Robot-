#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

// Firebase Includes
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi and Firebase Credentials
#define WIFI_SSID "Epen"
#define WIFI_PASSWORD "epenliee"
#define API_KEY "AIzaSyAKCtxjUfdXXHMPmHOvyTfTBpC6THznV3U"
#define DATABASE_URL "https://esp-32-firebase-demo-b9a40-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase Configuration
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// Pin konfigurasi
const int motorPin1 = 25;
const int motorPin2 = 26;
const int motorEnableA = 33;
const int motorPin3 = 18;
const int motorPin4 = 19;
const int motorEnableB = 21;
const int flameSensor1 = 2;
const int flameSensor2 = 34;
const int flameSensor3 = 4;
const int relayPin = 5;
const int servoPin = 13;

// Servo setup
Servo myServo;
int servoPosition = 90;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // Callback untuk token status

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
  } else {
    Serial.printf("Sign up error: %s\n", config.signer.signupError.message.c_str());
  }
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Setup servo
  myServo.attach(servoPin);
  myServo.write(servoPosition);

  // Setup pin mode
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorEnableA, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  pinMode(motorEnableB, OUTPUT);
  pinMode(flameSensor1, INPUT);
  pinMode(flameSensor2, INPUT);
  pinMode(flameSensor3, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Aktifkan motor
  digitalWrite(motorEnableA, HIGH);
  digitalWrite(motorEnableB, HIGH);
}

void loop() {
  if (Firebase.ready() && signupOK) {
    // Membaca nilai sensor flam
   
     Firebase.RTDB.getString(&fbdo, "motorCommand");
        String command = fbdo.stringData();
        
      
        
      
    int flame1 = analogRead(flameSensor1);
    int flame2 = analogRead(flameSensor2);
    int flame3 = analogRead(flameSensor3);

    // Menulis nilai sensor ke Firebase
    if (!Firebase.RTDB.setInt(&fbdo, "/sensors/flame1", flame1)) {
      Serial.println("Failed to write flame1 data:");
      Serial.println(fbdo.errorReason());
    }
    if (!Firebase.RTDB.setInt(&fbdo, "/sensors/flame2", flame2)) {
      Serial.println("Failed to write flame2 data:");
      Serial.println(fbdo.errorReason());
    }
    if (!Firebase.RTDB.setInt(&fbdo, "/sensors/flame3", flame3)) {
      Serial.println("Failed to write flame3 data:");
      Serial.println(fbdo.errorReason());
    }

    // Menampilkan nilai sensor ke Serial Monitor
    Serial.print("Flame1: "); Serial.print(flame1);
    Serial.print(" | Flame2: "); Serial.print(flame2);
    Serial.print(" | Flame3: "); Serial.println(flame3);

    // Deteksi api berdasarkan sensor
    bool detected1 = flame1 > 250;
    bool detected2 = flame2 < 250;
    bool detected3 = flame3 > 250;

    if (detected1 || detected2 || detected3) {
      // Logika jika api terdeteksi
      digitalWrite(relayPin, LOW);
      Serial.println("Relay aktif.");

      for (int angle = 90; angle <= 140; angle += 3) {
        myServo.write(angle);
        delay(30);
      }
      for (int angle = 140; angle >= 90; angle -= 3) {
        myServo.write(angle);
        delay(30);
      }
      Serial.println("Servo bergerak antara 90 dan 140 derajat.");
    }else if(command == "forward"){
       moveForward();
    }else if(command == "left"){
  
       turnLeft();
    }else if(command == "right"){
  
       turnRight();
    }else if(command == "backward"){
  
       moveBackward();
    }else {
      // Logika jika tidak ada api
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, LOW);
      digitalWrite(motorPin3, LOW);
      digitalWrite(motorPin4, LOW);
      Serial.println("Tidak ada deteksi api. Motor berhenti.");

      digitalWrite(relayPin, HIGH);
      Serial.println("Relay nonaktif.");

      myServo.write(90);
      Serial.println("Servo kembali ke 90 derajat.");
    }
  }

  delay(1000); // Delay 1 detik
}
void stopMotor() {
  digitalWrite(motorEnableA, LOW);
  digitalWrite(motorEnableB, LOW);
}

void moveForward() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorEnableA, HIGH);
  digitalWrite(motorEnableB, HIGH);
}

void moveBackward() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);
  digitalWrite(motorEnableA, HIGH);
  digitalWrite(motorEnableB, HIGH);
}

void turnLeft() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);
  digitalWrite(motorEnableA, HIGH);
  digitalWrite(motorEnableB, HIGH);
}

void turnRight() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);
  digitalWrite(motorEnableA, HIGH);
  digitalWrite(motorEnableB, HIGH);
}
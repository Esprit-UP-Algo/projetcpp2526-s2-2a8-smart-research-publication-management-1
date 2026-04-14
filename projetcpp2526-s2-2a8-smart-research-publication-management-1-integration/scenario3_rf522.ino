// =============================================================
// SCÉNARIO 3 — RFID (module RC522) + feedback TROUVE / INCONNU / ALERTE
// SmartPub — module Chercheurs (Qt route vers scénario 3 si onglet actif)
// -------------------------------------------------------------
// Branchement RC522 :
//   RC522 VCC  → Arduino 3.3V  (pas 5V)
//   RC522 GND  → Arduino GND
//   RC522 RST  → Arduino D9
//   RC522 SDA  → Arduino D10
//   RC522 MOSI → Arduino D11
//   RC522 MISO → Arduino D12
//   RC522 SCK  → Arduino D13
//
// LEDs / buzzer :
//   D6 → LED verte  + résistance 220Ω
//   D7 → LED rouge  + résistance 220Ω
//   D8 → Buzzer actif 5V
// =============================================================

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN    9
#define SS_PIN     10
#define LED_VERTE  6
#define LED_ROUGE  7
#define BUZZER     8

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(LED_VERTE, OUTPUT);
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(BUZZER,    OUTPUT);

  digitalWrite(LED_VERTE, LOW);
  digitalWrite(LED_ROUGE, LOW);
  digitalWrite(BUZZER,    LOW);

  Serial.println(F("=== SmartPub Scenario3 - RC522 ==="));
  Serial.println(F("Passez votre carte RFID..."));
}

void loop() {

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "TROUVE") {
      digitalWrite(LED_ROUGE, LOW);
      digitalWrite(BUZZER,    LOW);
      digitalWrite(LED_VERTE, HIGH);
      delay(2000);
      digitalWrite(LED_VERTE, LOW);
    }
    else if (cmd == "INCONNU") {
      digitalWrite(LED_VERTE, LOW);
      digitalWrite(LED_ROUGE, HIGH);
      digitalWrite(BUZZER,    HIGH);
      delay(2000);
      digitalWrite(LED_ROUGE, LOW);
      digitalWrite(BUZZER,    LOW);
    }
    else if (cmd == "ALERTE") {
      digitalWrite(LED_VERTE, LOW);
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_ROUGE, HIGH);
        digitalWrite(BUZZER,    HIGH);
        delay(300);
        digitalWrite(LED_ROUGE, LOW);
        digitalWrite(BUZZER,    LOW);
        delay(150);
      }
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10)
      uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  Serial.println("RFID:" + uid);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(1000);
}

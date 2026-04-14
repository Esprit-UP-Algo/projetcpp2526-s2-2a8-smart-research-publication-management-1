// =============================================================
// SCÉNARIO 3 — CODE ARDUINO RÉEL (avec module RC522)
// SmartPub — Module Chercheurs
// -------------------------------------------------------------
// Branchement RC522 :
//   RC522 VCC  → Arduino 3.3V  (pas 5V !)
//   RC522 GND  → Arduino GND
//   RC522 RST  → Arduino D9
//   RC522 SDA  → Arduino D10
//   RC522 MOSI → Arduino D11
//   RC522 MISO → Arduino D12
//   RC522 SCK  → Arduino D13
//
// Autres composants :
//   D6 → LED verte  + résistance 220Ω
//   D7 → LED rouge  + résistance 220Ω
//   D8 → Buzzer actif 5V
//
// Protocole série avec Qt (SmartPub) :
//   Arduino → PC : "RFID:<UID_HEX>\n"   ex: "RFID:A1B2C3D4"
//   PC → Arduino : "TROUVE\n"   → LED verte 2s
//                  "INCONNU\n"  → LED rouge + buzzer 2s
//                  "ALERTE\n"   → 3 bips courts (3 échecs consécutifs)
// =============================================================

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN    9
#define SS_PIN    10
#define LED_VERTE  6
#define LED_ROUGE  7
#define BUZZER     8

MFRC522 mfrc522(SS_PIN, RST_PIN);

// -------------------------------------------------------------
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

  Serial.println("=== SmartPub Scenario3 - Reel ===");
  Serial.println("Passez votre carte RFID...");
}

// -------------------------------------------------------------
void loop() {

  // ── 1. Recevoir commandes depuis Qt ──
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "TROUVE") {
      // Chercheur trouvé → LED verte 2 secondes
      digitalWrite(LED_ROUGE, LOW);
      digitalWrite(BUZZER,    LOW);
      digitalWrite(LED_VERTE, HIGH);
      delay(2000);
      digitalWrite(LED_VERTE, LOW);
    }
    else if (cmd == "INCONNU") {
      // Chercheur inconnu → LED rouge + buzzer 2 secondes
      digitalWrite(LED_VERTE, LOW);
      digitalWrite(LED_ROUGE, HIGH);
      digitalWrite(BUZZER,    HIGH);
      delay(2000);
      digitalWrite(LED_ROUGE, LOW);
      digitalWrite(BUZZER,    LOW);
    }
    else if (cmd == "ALERTE") {
      // 3 tentatives échouées → 3 bips courts
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

  // ── 2. Lire carte RFID ──
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial())   return;

  // Construire l'UID en hexadécimal majuscule
  // ex : carte avec bytes {0xA1, 0xB2, 0xC3, 0xD4} → "A1B2C3D4"
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  // Envoyer au PC — Qt reçoit "RFID:A1B2C3D4"
  Serial.println("RFID:" + uid);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Anti-rebond : attendre 1s avant le prochain scan
  delay(1000);
}

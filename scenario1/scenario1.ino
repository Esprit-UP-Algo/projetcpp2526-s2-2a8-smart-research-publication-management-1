// =====================================================================
// scenario1.ino — Controle d'acces RFID au laboratoire
// SmartPub — Systeme de Gestion de Recherche
// =====================================================================

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- PINS ----------------
#define SS_PIN   10
#define RST_PIN   9
#define LED_VERTE 5
#define LED_ROUGE 6

// ---------------- OBJETS ----------------
MFRC522           mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- ETAT ----------------
bool          attendReponse = false;
unsigned long heureEnvoi    = 0;
#define TIMEOUT_REPONSE_MS 15000

// =====================================================================
void setup() {
    Serial.begin(9600);
    delay(500);
    pinMode(LED_VERTE, OUTPUT);
    pinMode(LED_ROUGE, OUTPUT);
    digitalWrite(LED_VERTE, LOW);
    digitalWrite(LED_ROUGE, LOW);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Ligne 0 : nom de l'application
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");

    SPI.begin();
    mfrc522.PCD_Init();
    delay(50);

    byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Erreur RFID");
        lcd.setCursor(0, 1);
        lcd.print(" Verif cablage");
        while (true);
    }

    // Ecran d'accueil 2 secondes
    lcd.setCursor(0, 1);
    lcd.print(" Labo connecte");
    delay(2000);

    afficherAttente();
}

// =====================================================================
void loop() {

    // ----- Attente reponse Qt -----
    if (attendReponse) {
        if (Serial.available()) {
            String resp = Serial.readStringUntil('\n');
            resp.trim();
            traiterReponse(resp);
            attendReponse = false;
        }
        else if (millis() - heureEnvoi > TIMEOUT_REPONSE_MS) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("  SmartPub");
            lcd.setCursor(0, 1);
            lcd.print("Serveur timeout");
            delay(2500);
            afficherAttente();
            attendReponse = false;
        }
        return;
    }

    // ----- Lecture RFID -----
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial())   return;

    String uid = lireUID();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Verification");
    lcd.setCursor(0, 1);
    lcd.print("en cours...");

    Serial.println("RFID:" + uid);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    attendReponse = true;
    heureEnvoi    = millis();
}

// =====================================================================
String lireUID() {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    return uid;
}

// =====================================================================
// Affiche l'ecran d'attente standard
// =====================================================================
void afficherAttente() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");
    lcd.setCursor(0, 1);
    lcd.print("Passez la carte");
}

// =====================================================================
// Affiche une ligne centree sur 16 caracteres
// =====================================================================
String centrer(String texte) {
    int len = texte.length();
    if (len >= 16) return texte.substring(0, 16);
    int pad = (16 - len) / 2;
    String s = "";
    for (int i = 0; i < pad; i++) s += " ";
    s += texte;
    return s;
}

// =====================================================================
// Traitement de la reponse recue depuis Qt (SmartPub)
// =====================================================================
void traiterReponse(String resp) {
    lcd.clear();

    if (resp.startsWith("ENTREE:")) {
        // ── Acces autorise — ENTREE ──────────────────────────────────────
        String nom = resp.substring(7);
        if (nom.length() > 16) nom = nom.substring(0, 16);

        digitalWrite(LED_VERTE, HIGH);

        lcd.setCursor(0, 0);
        lcd.print(centrer("Bienvenue !"));
        lcd.setCursor(0, 1);
        lcd.print(centrer(nom));

        delay(6000);
        digitalWrite(LED_VERTE, LOW);

    }
    else if (resp.startsWith("SORTIE:")) {
        // ── Acces autorise — SORTIE ──────────────────────────────────────
        String nom = resp.substring(7);
        if (nom.length() > 16) nom = nom.substring(0, 16);

        digitalWrite(LED_VERTE, HIGH);

        lcd.setCursor(0, 0);
        lcd.print(centrer("A bientot !"));
        lcd.setCursor(0, 1);
        lcd.print(centrer(nom));

        delay(6000);
        digitalWrite(LED_VERTE, LOW);

    }
    else if (resp == "REFUSE") {
        // ── Acces refuse ─────────────────────────────────────────────────
        digitalWrite(LED_ROUGE, HIGH);

        lcd.setCursor(0, 0);
        lcd.print(centrer("Acces refuse"));
        lcd.setCursor(0, 1);
        lcd.print(centrer("Badge inconnu"));

        delay(6000);
        digitalWrite(LED_ROUGE, LOW);
    }

    afficherAttente();
}

// =====================================================================
// scenario1.ino — Controle d'acces RFID au laboratoire
// SmartPub — Systeme de Gestion de Recherche
//
// HISTORIQUE :
//   v1 : version originale
//   v2 : timer non-bloquant millis(), retry RC522
//   v3 : suppression PCD_Reset() parasite, boucle 5 tentatives
//   v4 : resetRC522() apres chaque transaction, lecture serie non-bloquante
//   v5 (cette version) — CORRECTION retour erreur RFID apres v4 :
//
//   CAUSE IDENTIFIEE :
//     flushReadBuffer() dans arduino.cpp (v4) appelle serial->readAll()
//     cote Qt APRES l'ouverture du port. Sous Windows/Linux, readAll()
//     sur un QSerialPort peut toggler les lignes DTR/RTS selon le driver
//     FTDI ou CH340. Ce toggle reset l'Arduino UNO une SECONDE fois,
//     exactement pendant la fenetre de 550ms de la boucle initRC522().
//     Le RC522 n'a pas le temps de se stabiliser → VersionReg = 0xFF.
//
//   CORRECTIONS v5 :
//
//   1. PROTECTION DTR dans setup() :
//      L'Arduino Uno se reset quand Qt ouvre le port (signal DTR).
//      On ajoute un delay(3000) AU DEBUT de setup(), AVANT Wire.begin()
//      et SPI.begin(). Ce delai absorbe :
//        - le reset initial a la mise sous tension
//        - le reset DTR declenche par Qt a l'ouverture du port
//        - le toggle DTR parasite du flushReadBuffer() Qt
//      3000ms est suffisant pour couvrir tous ces cas sans etre trop long.
//      Ce delay n'est execute QU'UNE FOIS au demarrage, il n'affecte
//      pas les performances en fonctionnement normal.
//
//   2. SUPPRESSION de la dependance a flushReadBuffer() :
//      Le buffer serie Arduino est gere entierement cote Arduino :
//      apres chaque transaction, on appelle drainSerialBuffer() qui lit
//      et jette tous les octets residuels presents dans Serial, sans
//      toucher au port Qt cote PC. Cela elimine la race condition DTR.
//
//   3. Conservation de toutes les corrections v4 :
//      - resetRC522() apres chaque transaction et timeout (BUG 1 + 3)
//      - Lecture serie non-bloquante caractere par caractere (BUG 2)
//      - Garde m_enTraitement cote Qt (BUG A)
// =====================================================================

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ──────────────────────────────────────────────────────────────────────
// PINS
// ──────────────────────────────────────────────────────────────────────
#define SS_PIN    10
#define RST_PIN    9
#define LED_VERTE  5
#define LED_ROUGE  6

// ──────────────────────────────────────────────────────────────────────
// OBJETS
// ──────────────────────────────────────────────────────────────────────
MFRC522           mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ──────────────────────────────────────────────────────────────────────
// ÉTAT GLOBAL
// ──────────────────────────────────────────────────────────────────────
bool          attendReponse   = false;
bool          displayActive   = false;
unsigned long heureEnvoi      = 0;
unsigned long displayUntil    = 0;
unsigned long derniereCapture = 0;
String        serialBuffer    = "";

#define TIMEOUT_REPONSE_MS   15000UL
#define DUREE_AFFICHAGE_MS    5000UL
#define DELAI_ANTI_REBOND_MS  1500UL

// =====================================================================
// drainSerialBuffer()
// Lit et jette tous les octets presents dans le buffer Serial Arduino.
// Remplace flushReadBuffer() cote Qt (qui causait le reset DTR parasite).
// Appele apres chaque transaction complete pour eliminer les residus
// (\r\n, rebond de carte, bytes partiels d'une collision RF).
// =====================================================================
void drainSerialBuffer() {
    unsigned long debut = millis();
    // On draine pendant 50ms max pour attraper les octets retardes
    while (millis() - debut < 50) {
        while (Serial.available() > 0) {
            Serial.read();  // lire et jeter
        }
        delay(5);
    }
    serialBuffer = "";
}

// =====================================================================
// initRC522()
// Initialisation robuste : 5 tentatives x 100ms, sans PCD_Reset().
// Doit etre appele APRES le delay(3000) de protection DTR dans setup().
// =====================================================================
byte initRC522() {
    mfrc522.PCD_Init();
    delay(50);

    byte version = 0x00;
    for (int i = 0; i < 5; i++) {
        version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
        if (version != 0x00 && version != 0xFF) return version;
        mfrc522.PCD_Init();
        delay(100);
    }
    return 0x00;
}

// =====================================================================
// resetRC522()
// Recycle l'antenne et verifie que le module repond apres chaque
// transaction ou timeout.
// =====================================================================
void resetRC522() {
    mfrc522.PCD_AntennaOff();
    delay(20);
    mfrc522.PCD_AntennaOn();
    delay(20);

    byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF) {
        mfrc522.PCD_Init();
        delay(100);
    }
}

// =====================================================================
// setup()
// =====================================================================
void setup() {

    // ── PROTECTION DTR — delay AU TOUT DEBUT de setup() ──────────────
    // CORRECTION v5 PRINCIPALE :
    // L'Arduino Uno se reset automatiquement quand un PC ouvre son port
    // serie (signal DTR). Ce reset recommence l'execution depuis setup().
    // Le delai ci-dessous est place AVANT toute initialisation pour
    // garantir que :
    //   - Le premier reset (mise sous tension) est absorbe
    //   - Le reset DTR de Qt (ouverture du port dans connect_arduino())
    //     est absorbe
    //   - Le toggle DTR parasite de flushReadBuffer() Qt est absorbe
    // Sans ce delai, PCD_Init() peut etre interrompu par le 2e reset
    // et VersionReg retourne 0xFF → fausse erreur "Verif cablage".
    // 3000ms couvre tous les cas connus (DTR typiquement < 2s apres boot).
    delay(3000);

    // ── GPIO ─────────────────────────────────────────────────────────
    pinMode(LED_VERTE, OUTPUT);
    pinMode(LED_ROUGE, OUTPUT);
    digitalWrite(LED_VERTE, LOW);
    digitalWrite(LED_ROUGE, LOW);

    // ── Serial ───────────────────────────────────────────────────────
    // Serial.begin apres le delay pour eviter les octets parasites
    // dus au toggle DTR pendant la periode de stabilisation
    Serial.begin(9600);

    // ── LCD ──────────────────────────────────────────────────────────
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");
    lcd.setCursor(0, 1);
    lcd.print(" Initialisation");

    // ── SPI + RC522 ──────────────────────────────────────────────────
    SPI.begin();
    byte version = initRC522();

    if (version == 0x00) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  Erreur RFID");
        lcd.setCursor(0, 1);
        lcd.print(" Verif cablage");
        while (true) {
            digitalWrite(LED_ROUGE, HIGH); delay(500);
            digitalWrite(LED_ROUGE, LOW);  delay(500);
        }
    }

    // ── Ecran OK ─────────────────────────────────────────────────────
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");
    lcd.setCursor(0, 1);
    lcd.print("   RC522 OK");
    delay(1500);

    afficherAttente();
}

// =====================================================================
// loop()
// =====================================================================
void loop() {

    // ── FIN D'AFFICHAGE ──────────────────────────────────────────────
    if (displayActive && millis() >= displayUntil) {
        displayActive = false;
        digitalWrite(LED_VERTE, LOW);
        digitalWrite(LED_ROUGE, LOW);
        afficherAttente();
    }

    if (displayActive) return;

    // ── ATTENTE REPONSE Qt (non-bloquant) ────────────────────────────
    if (attendReponse) {

        // Lecture caractere par caractere, non-bloquante
        while (Serial.available() > 0) {
            char c = (char)Serial.read();
            if (c == '\n') {
                serialBuffer.trim();
                String resp = serialBuffer;
                serialBuffer = "";

                traiterReponse(resp);
                attendReponse = false;

                // Recycler l'antenne et vider les residus serie
                resetRC522();
                drainSerialBuffer();
                return;

            } else if (c != '\r') {
                serialBuffer += c;
            }
        }

        // Timeout
        if (millis() - heureEnvoi > TIMEOUT_REPONSE_MS) {
            serialBuffer = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("   SmartPub");
            lcd.setCursor(0, 1);
            lcd.print("Serveur timeout");
            displayActive = true;
            displayUntil  = millis() + DUREE_AFFICHAGE_MS;
            attendReponse = false;

            resetRC522();
            drainSerialBuffer();
        }
        return;
    }

    // ── Anti-rebond ───────────────────────────────────────────────────
    if (millis() - derniereCapture < DELAI_ANTI_REBOND_MS) return;

    // ── LECTURE RFID ──────────────────────────────────────────────────
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial())   return;

    derniereCapture = millis();

    String uid = lireUID();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Verification");
    lcd.setCursor(0, 1);
    lcd.print("  en cours...");

    Serial.println("RFID:" + uid);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    attendReponse = true;
    heureEnvoi    = millis();
}

// =====================================================================
// lireUID()
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
// afficherAttente()
// =====================================================================
void afficherAttente() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");
    lcd.setCursor(0, 1);
    lcd.print("Passez la carte");
}

// =====================================================================
// centrer()
// =====================================================================
String centrer(String texte) {
    if ((int)texte.length() >= 16) return texte.substring(0, 16);
    int pad = (16 - (int)texte.length()) / 2;
    String s = "";
    for (int i = 0; i < pad; i++) s += " ";
    s += texte;
    return s;
}

// =====================================================================
// traiterReponse()
// =====================================================================
void traiterReponse(String resp) {
    lcd.clear();

    if (resp.startsWith("ENTREE:")) {
        String nom = resp.substring(7);
        nom.trim();
        if ((int)nom.length() > 16) nom = nom.substring(0, 16);
        digitalWrite(LED_VERTE, HIGH);
        lcd.setCursor(0, 0); lcd.print(centrer("Bienvenue !"));
        lcd.setCursor(0, 1); lcd.print(centrer(nom));

    } else if (resp.startsWith("SORTIE:")) {
        String nom = resp.substring(7);
        nom.trim();
        if ((int)nom.length() > 16) nom = nom.substring(0, 16);
        digitalWrite(LED_VERTE, HIGH);
        lcd.setCursor(0, 0); lcd.print(centrer("A bientot !"));
        lcd.setCursor(0, 1); lcd.print(centrer(nom));

    } else if (resp == "REFUSE") {
        digitalWrite(LED_ROUGE, HIGH);
        lcd.setCursor(0, 0); lcd.print(centrer("Acces refuse"));
        lcd.setCursor(0, 1); lcd.print(centrer("Non reconnu"));

    } else {
        digitalWrite(LED_ROUGE, HIGH);
        lcd.setCursor(0, 0); lcd.print("Erreur comm.");
        lcd.setCursor(0, 1); lcd.print("Reessayez...");
    }

    displayActive = true;
    displayUntil  = millis() + DUREE_AFFICHAGE_MS;
}

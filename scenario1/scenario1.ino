// =====================================================================
// scenario1.ino — Controle d'acces RFID au laboratoire
// SmartPub — Systeme de Gestion de Recherche
//
// HISTORIQUE :
//   v1 : version originale
//   v2 : timer non-bloquant millis(), retry RC522
//   v3 : suppression PCD_Reset() parasite, boucle 5 tentatives
//   v4 : resetRC522() apres chaque transaction, lecture serie non-bloquante
//   v5 : suppression flushReadBuffer() Qt, protection DTR delay(3000)
//   v6 (cette version) — CORRECTIONS + AJOUTS servo SG90 + buzzer :
//
//   BUGS CORRIGES v6 :
//
//   BUG 1 — drainSerialBuffer() bloquant :
//     La v5 utilisait delay(5) dans une boucle de 50ms pour drainer
//     le buffer. Ce delay() bloquait l'Arduino pendant que Qt envoyait
//     sa reponse, causant des octets perdus et un etat attendReponse
//     bloque indefiniment. Correction : drain simple sans delay(),
//     une seule passe de lecture.
//
//   BUG 2 — resetRC522() trop tardif :
//     En v5, resetRC522() n'etait appele qu'APRES reception de la
//     reponse Qt (dans la branche attendReponse). Si Qt tardait a
//     repondre, le RC522 restait dans un etat "carte active" et
//     refusait de detecter une nouvelle carte. Correction : appel
//     de resetRC522() immediatement apres PICC_HaltA() + PCD_StopCrypto1(),
//     avant meme d'envoyer l'UID a Qt.
//
//   BUG 3 — derniereCapture non reinitialise apres timeout :
//     En cas de timeout, derniereCapture n'etait pas mis a jour,
//     permettant une re-lecture immediate de la meme carte fantome.
//     Correction : derniereCapture = millis() au moment du timeout.
//
//   AJOUTS v6 :
//
//   1. SERVO SG90 (pin 3) :
//      - Ouverture porte (90 deg) sur ENTREE ou SORTIE autorisee
//      - Fermeture porte (0 deg) apres DUREE_OUVERTURE_MS
//      - Etat porte suivi par variable porteOuverte
//      - Commande "FERMER_PORTE" envoyee a Qt apres fermeture
//        pour mise a jour de la colonne ETAT_PORTE en base
//
//   2. BUZZER (pin 4) :
//      - 2 bips courts (100ms) : acces autorise (entree ou sortie)
//      - 1 bip long (500ms) : acces refuse
//      - Bips non-bloquants via millis() (buzzerTimer)
//
//   3. PROTOCOLE SERIE ETENDU :
//      Qt → Arduino : "ENTREE:<nom>", "SORTIE:<nom>", "REFUSE"
//      Arduino → Qt : "RFID:<uid>", "PORTE_OUVERTE", "PORTE_FERMEE"
// =====================================================================

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ──────────────────────────────────────────────────────────────────────
// PINS
// ──────────────────────────────────────────────────────────────────────
#define SS_PIN              10
#define RST_PIN              9
#define LED_VERTE            5
#define LED_ROUGE            6
#define SERVO_PIN            3   // Servo SG90 — signal PWM
#define BUZZER_PIN           4   // Buzzer actif ou passif

// ──────────────────────────────────────────────────────────────────────
// SERVO — angles
// ──────────────────────────────────────────────────────────────────────
#define SERVO_FERME          180   // Position porte fermee (0 degres)
#define SERVO_OUVERT        90   // Position porte ouverte (90 degres)

// ──────────────────────────────────────────────────────────────────────
// OBJETS
// ──────────────────────────────────────────────────────────────────────
MFRC522           mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo             servPorte;

// ──────────────────────────────────────────────────────────────────────
// ÉTAT GLOBAL — acces RFID
// ──────────────────────────────────────────────────────────────────────
bool          attendReponse   = false;
bool          displayActive   = false;
unsigned long heureEnvoi      = 0;
unsigned long displayUntil    = 0;
unsigned long derniereCapture = 0;
String        serialBuffer    = "";

// ──────────────────────────────────────────────────────────────────────
// ÉTAT GLOBAL — servo porte
// ──────────────────────────────────────────────────────────────────────
bool          porteOuverte    = false;
unsigned long porteFermerA    = 0;   // millis() cible pour fermeture auto

// ──────────────────────────────────────────────────────────────────────
// ÉTAT GLOBAL — buzzer non-bloquant
// ──────────────────────────────────────────────────────────────────────
// Sequence de bips : tableau de durees ON/OFF en ms, termine par 0
// Ex: {100, 100, 100, 0} = bip 100ms, silence 100ms, bip 100ms
#define MAX_BIPS 8
unsigned int  bipSequence[MAX_BIPS];  // durees alternees ON/OFF
int           bipIndex      = -1;     // -1 = pas de bip en cours
unsigned long bipTimer      = 0;      // millis() du dernier changement

// ──────────────────────────────────────────────────────────────────────
// CONSTANTES TIMING
// ──────────────────────────────────────────────────────────────────────
#define TIMEOUT_REPONSE_MS   15000UL  // Attente max reponse Qt
#define DUREE_AFFICHAGE_MS    5000UL  // Duree affichage LCD resultat
#define DELAI_ANTI_REBOND_MS  2000UL  // Anti-rebond entre deux lectures RFID
#define DUREE_OUVERTURE_MS    5000UL  // Porte reste ouverte 5 secondes

// =====================================================================
// drainSerialBuffer() — v6 CORRIGE
// Lit et jette tous les octets IMMEDIATEMENT disponibles dans Serial.
// CORRECTION v6 : suppression du delay(5) et de la boucle 50ms qui
// bloquaient l'Arduino pendant la reception de la reponse Qt.
// Une seule passe non-bloquante suffit pour vider les residus.
// =====================================================================
void drainSerialBuffer() {
    while (Serial.available() > 0) {
        Serial.read();
    }
    serialBuffer = "";
}

// =====================================================================
// initRC522()
// Initialisation robuste : 5 tentatives x 100ms, sans PCD_Reset().
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
// Recycle l'antenne et verifie que le module repond.
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
// ouvrirPorte()
// Commande le servo en position ouverte et notifie Qt.
// =====================================================================
void ouvrirPorte() {
    servPorte.write(SERVO_OUVERT);
    porteOuverte  = true;
    porteFermerA  = millis() + DUREE_OUVERTURE_MS;
    Serial.println("PORTE_OUVERTE");
    serialDebug("[Servo] Porte OUVERTE");
}

// =====================================================================
// fermerPorte()
// Commande le servo en position fermee et notifie Qt.
// =====================================================================
void fermerPorte() {
    servPorte.write(SERVO_FERME);
    porteOuverte = false;
    Serial.println("PORTE_FERMEE");
    serialDebug("[Servo] Porte FERMEE");
}

// =====================================================================
// serialDebug() — helper debug serie optionnel
// Desactive par defaut pour ne pas polluer le protocole Qt/Arduino.
// Decommenter le corps pour activer les messages de debug.
// =====================================================================
void serialDebug(const char* msg) {
    // Decommenter pour debug (attention : pollue le buffer de reponse Qt) :
    // if (!attendReponse) { Serial.print("[DBG] "); Serial.println(msg); }
    (void)msg;
}

// =====================================================================
// demarrerBips() — buzzer non-bloquant
// seq : tableau de durees {ON, OFF, ON, OFF, ..., 0}
// =====================================================================
void demarrerBips(unsigned int* seq) {
    for (int i = 0; i < MAX_BIPS; i++) {
        bipSequence[i] = seq[i];
        if (seq[i] == 0) break;
    }
    bipIndex = 0;
    bipTimer = millis();
    digitalWrite(BUZZER_PIN, HIGH);  // premier bip ON
}

// =====================================================================
// updateBuzzer() — a appeler dans loop()
// Gere la sequence de bips sans bloquer.
// =====================================================================
void updateBuzzer() {
    if (bipIndex < 0) return;
    if (bipSequence[bipIndex] == 0) {
        // Fin de sequence
        digitalWrite(BUZZER_PIN, LOW);
        bipIndex = -1;
        return;
    }
    if (millis() - bipTimer >= bipSequence[bipIndex]) {
        bipIndex++;
        bipTimer = millis();
        if (bipIndex >= MAX_BIPS || bipSequence[bipIndex] == 0) {
            digitalWrite(BUZZER_PIN, LOW);
            bipIndex = -1;
        } else {
            // Alterner ON/OFF : index pair = ON, index impair = OFF
            digitalWrite(BUZZER_PIN, (bipIndex % 2 == 0) ? HIGH : LOW);
        }
    }
}

// =====================================================================
// setup()
// =====================================================================
void setup() {

    // ── PROTECTION DTR — delay AU TOUT DEBUT de setup() ──────────────
    // L'Arduino Uno se reset quand Qt ouvre le port (signal DTR).
    // Ce delay absorbe le reset initial + le reset DTR de Qt.
    // 3000ms couvre tous les cas connus.
    delay(3000);

    // ── GPIO ─────────────────────────────────────────────────────────
    pinMode(LED_VERTE,  OUTPUT);
    pinMode(LED_ROUGE,  OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_VERTE,  LOW);
    digitalWrite(LED_ROUGE,  LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // ── Servo ────────────────────────────────────────────────────────
    servPorte.attach(SERVO_PIN);
    servPorte.write(SERVO_FERME);   // Porte fermee au demarrage
    porteOuverte = false;

    // ── Serial ───────────────────────────────────────────────────────
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

    // ── Buzzer non-bloquant ───────────────────────────────────────────
    updateBuzzer();

    // ── Fermeture automatique de la porte ────────────────────────────
    if (porteOuverte && millis() >= porteFermerA) {
        fermerPorte();
    }

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

                // Vider les residus serie (sans delay)
                drainSerialBuffer();
                return;

            } else if (c != '\r') {
                serialBuffer += c;
            }
        }

        // Timeout
        if (millis() - heureEnvoi > TIMEOUT_REPONSE_MS) {
            serialBuffer    = "";
            derniereCapture = millis();  // BUG 3 CORRIGE : evite re-lecture immediate

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("   SmartPub");
            lcd.setCursor(0, 1);
            lcd.print("Serveur timeout");
            displayActive = true;
            displayUntil  = millis() + DUREE_AFFICHAGE_MS;
            attendReponse = false;

            // Bip long = erreur
            unsigned int bipErreur[] = {500, 0};
            demarrerBips(bipErreur);

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

    // ── Arreter la carte immediatement — BUG 2 CORRIGE ───────────────
    // resetRC522() est appele ICI, avant d'envoyer l'UID a Qt,
    // pour que le lecteur soit pret a lire une nouvelle carte
    // pendant que Qt traite la requete (jusqu'a 15s).
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    resetRC522();   // CORRECTION v6 : deplace ici depuis traiterReponse()

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  Verification");
    lcd.setCursor(0, 1);
    lcd.print("  en cours...");

    Serial.println("RFID:" + uid);

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
// Traite la reponse de Qt et commande servo + buzzer en consequence.
// =====================================================================
void traiterReponse(String resp) {
    lcd.clear();

    if (resp.startsWith("ENTREE:")) {
        // ── Acces autorise — ENTREE ───────────────────────────────────
        String nom = resp.substring(7);
        nom.trim();
        if ((int)nom.length() > 16) nom = nom.substring(0, 16);

        digitalWrite(LED_VERTE, HIGH);
        lcd.setCursor(0, 0); lcd.print(centrer("Bienvenue !"));
        lcd.setCursor(0, 1); lcd.print(centrer(nom));

        // Ouvrir la porte
        ouvrirPorte();

        // 2 bips courts = acces autorise
        unsigned int bipOk[] = {100, 100, 100, 0};
        demarrerBips(bipOk);

    } else if (resp.startsWith("SORTIE:")) {
        // ── Acces autorise — SORTIE ───────────────────────────────────
        String nom = resp.substring(7);
        nom.trim();
        if ((int)nom.length() > 16) nom = nom.substring(0, 16);

        digitalWrite(LED_VERTE, HIGH);
        lcd.setCursor(0, 0); lcd.print(centrer("A bientot !"));
        lcd.setCursor(0, 1); lcd.print(centrer(nom));

        // Ouvrir la porte
        ouvrirPorte();

        // 2 bips courts = acces autorise
        unsigned int bipOk[] = {100, 100, 100, 0};
        demarrerBips(bipOk);

    } else if (resp == "REFUSE") {
        // ── Acces refuse ──────────────────────────────────────────────
        digitalWrite(LED_ROUGE, HIGH);
        lcd.setCursor(0, 0); lcd.print(centrer("Acces refuse"));
        lcd.setCursor(0, 1); lcd.print(centrer("Non autorise"));

        // 1 bip long = acces refuse
        unsigned int bipRefus[] = {500, 0};
        demarrerBips(bipRefus);

        // Porte reste fermee

    } else {
        // ── Reponse inconnue ──────────────────────────────────────────
        digitalWrite(LED_ROUGE, HIGH);
        lcd.setCursor(0, 0); lcd.print("Erreur comm.");
        lcd.setCursor(0, 1); lcd.print("Reessayez...");

        unsigned int bipErreur[] = {200, 100, 200, 0};
        demarrerBips(bipErreur);
    }

    displayActive = true;
    displayUntil  = millis() + DUREE_AFFICHAGE_MS;
}

// =====================================================================
// scenario2.ino — SmartPub
// Partie A : Affichage OLED SH1106 128x64 I2C (ex-demi_scenario2)
// Partie B : Détection chaleur DHT11             (ex-demi_scenario3)
// =====================================================================
//
// Câblage Arduino Uno :
//   OLED SDA → A4  |  OLED SCL → A5
//   OLED VCC → 3.3V  |  OLED GND → GND
//   DHT11 DATA → D7  |  DHT11 VCC → 5V  |  DHT11 GND → GND
//
// Bibliothèques requises :
//   - Adafruit SH110X  (gestionnaire de bibliothèques Arduino IDE)
//   - DHT sensor library by Adafruit
// =====================================================================

// ── Partie A : OLED ──
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ── Partie B : DHT11 ──
#include <DHT.h>

// ── Constantes OLED ──
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT    64
#define OLED_RESET       -1
#define SCREEN_ADDRESS  0x3C
#define MSG_DISPLAY_MS  3000   // durée d'affichage d'un message (ms)

// ── Constantes DHT11 ──
#define DHT_PIN    7
#define DHT_TYPE   DHT11
#define SEUIL_TEMP 28.1        // seuil de détection chaleur (°C)

// ── Objets ──
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHT_PIN, DHT_TYPE);

// ── Variables Partie A (OLED) ──
bool          oledOk       = false;
bool          msgEnAttente = false;
unsigned long msgTimer     = 0;

// ── Variables Partie B (DHT11) ──
bool ds3_enAttente   = false;
int  ds3_idLaboActif = -1;

// ── Buffer série commun (non bloquant) ──
String inputBuffer = "";

// =====================================================================
// Partie A — Affichage OLED
// =====================================================================

void afficherAccueil() {
    if (!oledOk) return;
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(2);
    display.setCursor(4, 4);
    display.print(F("SmartPub"));
    display.drawLine(0, 24, 127, 24, SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(8,  30); display.print(F("Gestion Recherche"));
    display.setCursor(16, 42); display.print(F("& Publications"));
    display.setCursor(4,  56); display.print(F("SmartPub v1.0"));
    display.display();
}

void scenario2_traiterCmd(const String& cmd) {
    if (cmd == F("CLEAR")) {
        display.clearDisplay();
        display.display();
        msgEnAttente = false;
        Serial.println(F("ACK"));
        return;
    }
    if (cmd == F("DONE")) {
        msgEnAttente = false;
        afficherAccueil();
        return;
    }
    if (cmd.startsWith(F("MSG:"))) {
        if (!oledOk) { Serial.println(F("ACK")); return; }
        display.clearDisplay();
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(1);
        display.setTextWrap(true);
        display.setCursor(0, 0);
        // Utilisation du pointeur C pour ne pas allouer une nouvelle String en RAM
        display.print(cmd.c_str() + 4);
        display.display();
        msgEnAttente = true;
        msgTimer     = millis();
        return;
    }
    Serial.println(F("ACK"));   // commande non reconnue par Partie A
}

// =====================================================================
// Partie B — DHT11
// =====================================================================

void ds3_lireEtEnvoyer() {
    delay(500);   // stabilisation capteur
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        Serial.println(F("ERR:DHT_READ"));
        return;
    }
    Serial.print(F("TEMP:"));
    Serial.println(temperature, 1);
    if (temperature >= SEUIL_TEMP) {
        Serial.print(F("FIRE:"));
        Serial.println(ds3_idLaboActif);
    }
}

// =====================================================================
// Dispatcher commandes (commun aux deux parties)
// =====================================================================

void traiterCommande(const String& cmd) {
    // Partie B : START:<id_labo>
    if (cmd.startsWith(F("START:"))) {
        ds3_idLaboActif = cmd.substring(6).toInt();
        ds3_enAttente   = true;
        return;
    }
    // Partie A : CLEAR / DONE / MSG:
    scenario2_traiterCmd(cmd);
}

// =====================================================================
// setup() — initialisation des deux parties
// =====================================================================
void setup() {
    Serial.begin(9600);
    // Réserve de RAM pour le buffer série afin d'éviter la fragmentation
    inputBuffer.reserve(128);

    // ── Partie A : OLED ──
    Wire.begin();
    delay(3000);   // laisser Qt finir l'ouverture du port

    if (!display.begin(SCREEN_ADDRESS, true)) {
        Serial.println(F("SH1106 allocation failed"));
        for (;;);
    }
    oledOk = true;
    display.clearDisplay();
    display.display();
    afficherAccueil();

    // ── Partie B : DHT11 ──
    dht.begin();

    // Signaler que les deux parties sont prêtes
    for (int i = 0; i < 3; i++) {
        delay(1500);
        Serial.println(F("READY"));
        Serial.flush();
    }
}

// =====================================================================
// loop() — boucle principale non bloquante
// =====================================================================
void loop() {
    // ── Lecture série non bloquante ──
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            inputBuffer.trim();
            if (inputBuffer.length() > 0)
                traiterCommande(inputBuffer);
            inputBuffer = "";
        } else if (c != '\r') {
            inputBuffer += c;
        }
    }

    // ── Partie A : ACK différé après affichage OLED ──
    if (msgEnAttente && (millis() - msgTimer >= MSG_DISPLAY_MS)) {
        msgEnAttente = false;
        Serial.flush();
        Serial.println(F("ACK"));
    }

    // ── Partie B : lecture DHT11 déclenchée par START ──
    if (ds3_enAttente) {
        ds3_enAttente = false;
        ds3_lireEtEnvoyer();
    }
}

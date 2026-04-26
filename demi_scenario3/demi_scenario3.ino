// =====================================================================
// demi_scenario3.ino — Detection incendie laboratoire via capteur DHT11
// SmartPub — Systeme de Gestion de Recherche
// =====================================================================
//
// Materiel :
//   - Arduino Uno
//   - Capteur DHT11 (temperature)
//   - Quelques fils de connexion
//
// Cablage :
//   DHT11 DATA → pin 7
//   DHT11 VCC  → 5V
//   DHT11 GND  → GND
//
// Protocole serie Arduino → Qt :
//   "FIRE:<id_labo>\n"  → incendie detecte, Qt met le labo en Inactif
//   "TEMP:<valeur>\n"   → envoi periodique de la temperature (toutes les 5s)
//   "READY\n"           → Arduino initialise et pret
//   "ERR:DHT_READ\n"    → erreur de lecture du capteur
//
// Protocole serie Qt → Arduino :
//   "ACK\n"             → Qt a bien recu et traite l'alerte incendie
//   "RESET\n"           → reinitialiser l'etat (apres intervention)
//
// Seuil de temperature : 50 degres Celsius (simulant un incendie)
// =====================================================================

#include <DHT.h>

// ── Pins ──────────────────────────────────────────────────────────────
#define DHT_PIN  7
#define DHT_TYPE DHT11

// ── ID du laboratoire surveille (configurable) ────────────────────────
#define ID_LABORATOIRE 1

// ── Seuil de temperature declenchant l'alerte incendie ───────────────
#define SEUIL_TEMP_INCENDIE 50.0

// ── Intervalle de lecture ─────────────────────────────────────────────
#define INTERVALLE_LECTURE_MS 5000

DHT dht(DHT_PIN, DHT_TYPE);

bool          incendieDetecte = false;
bool          alerteEnvoyee   = false;
unsigned long derniereLecture = 0;

// =====================================================================
void setup() {
    Serial.begin(9600);
    dht.begin();
    delay(2000);
    Serial.println("READY");
}

// =====================================================================
void loop() {

    // ── Commandes Qt → Arduino ────────────────────────────────────────
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        traiterCommande(cmd);
    }

    // ── Lecture periodique DHT11 ──────────────────────────────────────
    unsigned long maintenant = millis();
    if (maintenant - derniereLecture >= INTERVALLE_LECTURE_MS) {
        derniereLecture = maintenant;
        lireEtTraiterDHT();
    }
}

// =====================================================================
void lireEtTraiterDHT() {
    float temperature = dht.readTemperature();

    if (isnan(temperature)) {
        Serial.println("ERR:DHT_READ");
        return;
    }

    Serial.print("TEMP:");
    Serial.println(temperature, 1);

    if (temperature >= SEUIL_TEMP_INCENDIE && !alerteEnvoyee) {
        incendieDetecte = true;
        alerteEnvoyee   = true;
        Serial.print("FIRE:");
        Serial.println(ID_LABORATOIRE);
    }
}

// =====================================================================
void traiterCommande(String cmd) {

    if (cmd == "ACK") {
        return;
    }

    if (cmd == "RESET") {
        incendieDetecte = false;
        alerteEnvoyee   = false;
        Serial.println("RESET:OK");
    }
}

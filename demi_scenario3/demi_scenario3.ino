// =====================================================================
// demi_scenario3.ino — Détection chaleur laboratoire via capteur DHT11
// SmartPub — Système de Gestion de Recherche
// =====================================================================
//
// Matériel :
//   - Arduino Uno
//   - Capteur DHT11
//   - Branchement : VCC → 5V, DATA → D7, GND → GND
//
// Protocole série Qt → Arduino :
//   "START:<id_labo>\n"  → déclencher une lecture DHT11 pour ce labo
//
// Protocole série Arduino → Qt :
//   "READY\n"            → Arduino initialisé et prêt
//   "TEMP:<valeur>\n"    → température lue (envoyée après START)
//   "FIRE:<id_labo>\n"   → seuil dépassé, Qt doit passer le labo en Inactif
//   "ERR:DHT_READ\n"     → erreur de lecture du capteur
//
// Seuil : 20 °C (chaleur ambiante normale, facilement atteint)
// Le capteur ne lit RIEN tant qu'il n'a pas reçu "START:<id>\n"
// =====================================================================

#include <DHT.h>

#define DHT_PIN   7
#define DHT_TYPE  DHT11

// Seuil pour détection avec briquet (flamme proche du capteur)
#define SEUIL_TEMP 28.1

DHT dht(DHT_PIN, DHT_TYPE);

bool  enAttente    = false;   // true = un START a été reçu, lecture en cours
int   idLaboActif  = -1;      // id du labo concerné par la lecture en cours

// =====================================================================
void setup() {
    Serial.begin(9600);
    dht.begin();
    delay(2000);
    Serial.println("READY");
}

// =====================================================================
void loop() {
    // Lire les commandes envoyées par Qt
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        traiterCommande(cmd);
    }

    // Si un START a été reçu, effectuer la lecture DHT11
    if (enAttente) {
        enAttente = false;   // une seule lecture par déclenchement
        lireEtEnvoyer();
    }
}

// =====================================================================
void traiterCommande(String cmd) {
    if (cmd.startsWith("START:")) {
        idLaboActif = cmd.substring(6).toInt();
        enAttente   = true;
    }
}

// =====================================================================
void lireEtEnvoyer() {
    // Petite pause pour laisser le capteur se stabiliser
    delay(500);
    float temperature = dht.readTemperature();

    if (isnan(temperature)) {
        Serial.println("ERR:DHT_READ");
        return;
    }

    // Envoyer la température lue
    Serial.print("TEMP:");
    Serial.println(temperature, 1);

    // Si le seuil est dépassé, envoyer l'alerte FIRE
    if (temperature >= SEUIL_TEMP) {
        Serial.print("FIRE:");
        Serial.println(idLaboActif);
    }
}

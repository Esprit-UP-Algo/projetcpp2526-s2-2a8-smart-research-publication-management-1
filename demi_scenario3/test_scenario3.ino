// =====================================================================
// test_scenario3.ino — Simulation Wokwi : Detection incendie DHT11
// =====================================================================
//
// DIFFERENCES avec demi_scenario3.ino (production) :
//   - Seuil abaisse a 30°C pour faciliter la simulation
//   - Intervalle de lecture reduit a 2s (plus reactif en simulation)
//   - Buzzer passif (piezo) au lieu du buzzer actif
//   - Affichage Serial Monitor detaille pour verifier le comportement
//
// CABLAGE Wokwi :
//   DHT11  DATA → pin 7  |  VCC → 5V  |  GND → GND
//   LED rouge   → pin 6  (anode) + resistance 220Ω → GND
//   Piezo       → pin 5  (+ patte) |  GND → GND
//
// COMMENT TESTER :
//   1. Lancer la simulation
//   2. Cliquer sur le composant DHT11 dans Wokwi
//   3. Augmenter la temperature au-dessus de 30°C
//   4. Observer dans le Serial Monitor :
//        READY
//        TEMP:25.0
//        TEMP:31.0
//        FIRE:1
//   5. La LED rouge clignote et le buzzer sonne
//   6. Taper "RESET" dans le Serial Monitor → tout s'eteint
// =====================================================================

#include <DHT.h>

// ── Pins ──────────────────────────────────────────────────────────────
#define DHT_PIN    7
#define DHT_TYPE   DHT22
#define LED_ROUGE  6
#define BUZZER_PIN 5

// ── ID du laboratoire surveille ───────────────────────────────────────
#define ID_LABORATOIRE 1

// ── Seuil abaisse pour la simulation (30°C au lieu de 50°C) ──────────
#define SEUIL_TEMP_INCENDIE 30.0

// ── Intervalle de lecture reduit pour la simulation ───────────────────
#define INTERVALLE_LECTURE_MS 2000

DHT dht(DHT_PIN, DHT_TYPE);

bool          incendieDetecte = false;
bool          alerteEnvoyee   = false;
unsigned long derniereLecture = 0;

// =====================================================================
void setup() {
    Serial.begin(9600);

    pinMode(LED_ROUGE,  OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_ROUGE,  LOW);
    digitalWrite(BUZZER_PIN, LOW);

    dht.begin();
    delay(1000);

    Serial.println("READY");
    Serial.println("--- Simulation test_scenario3 ---");
    Serial.print("Seuil incendie : ");
    Serial.print(SEUIL_TEMP_INCENDIE);
    Serial.println(" C");
    Serial.println("Tapez RESET dans le Serial Monitor pour reinitialiser.");
}

// =====================================================================
void loop() {

    // ── Commandes depuis le Serial Monitor (simule Qt) ────────────────
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

    // ── Alarme : LED clignote + buzzer si incendie ────────────────────
    if (incendieDetecte) {
        digitalWrite(LED_ROUGE, HIGH);
        tone(BUZZER_PIN, 1000, 300);
        delay(400);
        digitalWrite(LED_ROUGE, LOW);
        delay(300);
    }
}

// =====================================================================
void lireEtTraiterDHT() {
    float temperature = dht.readTemperature();
    float humidite    = dht.readHumidity();

    if (isnan(temperature) || isnan(humidite)) {
        Serial.println("ERR:DHT_READ");
        return;
    }

    // Envoyer la temperature (meme format que demi_scenario3.ino)
    Serial.print("TEMP:");
    Serial.println(temperature, 1);

    // Affichage detaille pour la simulation
    Serial.print("[SIM] Temp=");
    Serial.print(temperature, 1);
    Serial.print("C  Hum=");
    Serial.print(humidite, 1);
    Serial.print("%  Seuil=");
    Serial.print(SEUIL_TEMP_INCENDIE);
    Serial.println("C");

    // Detection incendie
    if (temperature >= SEUIL_TEMP_INCENDIE && !alerteEnvoyee) {
        incendieDetecte = true;
        alerteEnvoyee   = true;

        Serial.println("!!! INCENDIE DETECTE !!!");
        Serial.print("FIRE:");
        Serial.println(ID_LABORATOIRE);
        Serial.println("[SIM] -> Laboratoire passe en INACTIF en BD");
    }
}

// =====================================================================
void traiterCommande(String cmd) {

    if (cmd == "ACK") {
        Serial.println("[SIM] ACK recu — alerte traitee par Qt");
        return;
    }

    if (cmd == "RESET") {
        incendieDetecte = false;
        alerteEnvoyee   = false;
        digitalWrite(LED_ROUGE,  LOW);
        noTone(BUZZER_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("RESET:OK");
        Serial.println("[SIM] Systeme reinitialise — surveillance reprise");
    }
}

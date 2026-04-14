// ============================================================================
// SCÉNARIO 3 — Clôture de projet via Arduino
// ============================================================================
//
// Matériel requis :
//   - Potentiomètre sur A0  → sélection de l'ID projet (1..20)
//   - Bouton poussoir sur pin 2 (pull-up interne)
//   - LED verte  sur pin 5  → projet clôturé avec succès
//   - LED rouge  sur pin 6  → erreur
//   - Buzzer passif sur pin 7
//
// Protocole série :
//   Arduino → PC : "CLOTURER:<id_projet>\n"
//   PC → Arduino : "OK:<titre_projet>\n"  → LED verte + 2 bips courts
//                  "ERR:<raison>\n"       → LED rouge + 1 bip long
// ============================================================================

const int PIN_POT      = A0;
const int PIN_BTN      = 2;
const int PIN_LED_OK   = 5;
const int PIN_LED_ERR  = 6;
const int PIN_BUZZ     = 7;

const int ID_MIN = 1;
const int ID_MAX = 20;

const unsigned long DEBOUNCE_MS = 50;
const unsigned long TIMEOUT_MS  = 6000;

bool          waitingForResponse = false;
unsigned long sentAt             = 0;

int  readProjetId();
bool buttonPressed();
void sendRequest(int idProjet);
void handleResponse(const String &resp);
void signalOk();
void signalError();
void allLedsOff();
void beep(int freq, int durationMs);

void setup() {
    Serial.begin(9600);
    pinMode(PIN_BTN,     INPUT_PULLUP);
    pinMode(PIN_LED_OK,  OUTPUT);
    pinMode(PIN_LED_ERR, OUTPUT);
    pinMode(PIN_BUZZ,    OUTPUT);
    allLedsOff();
}

void loop() {

    if (waitingForResponse) {
        if (Serial.available()) {
            String resp = Serial.readStringUntil('\n');
            resp.trim();
            handleResponse(resp);
            waitingForResponse = false;
        } else if (millis() - sentAt > TIMEOUT_MS) {
            signalError();
            waitingForResponse = false;
        }
        return;
    }

    if (buttonPressed()) {
        int idProjet = readProjetId();
        sendRequest(idProjet);
        waitingForResponse = true;
        sentAt = millis();
    }
}

int readProjetId() {
    int raw = analogRead(PIN_POT);
    return constrain(map(raw, 0, 1023, ID_MIN, ID_MAX), ID_MIN, ID_MAX);
}

bool buttonPressed() {
    if (digitalRead(PIN_BTN) == LOW) {
        delay(DEBOUNCE_MS);
        if (digitalRead(PIN_BTN) == LOW) {
            while (digitalRead(PIN_BTN) == LOW) { delay(10); }
            return true;
        }
    }
    return false;
}

void sendRequest(int idProjet) {
    allLedsOff();
    Serial.print("CLOTURER:");
    Serial.println(idProjet);
}

void handleResponse(const String &resp) {
    if (resp.startsWith("OK:")) {
        signalOk();
    } else if (resp.startsWith("ERR:")) {
        signalError();
    }
}

void signalOk() {
    digitalWrite(PIN_LED_OK, HIGH);
    beep(1200, 120);
    delay(80);
    beep(1200, 120);
    delay(800);
    digitalWrite(PIN_LED_OK, LOW);
}

void signalError() {
    digitalWrite(PIN_LED_ERR, HIGH);
    beep(300, 700);
    delay(800);
    digitalWrite(PIN_LED_ERR, LOW);
}

void allLedsOff() {
    digitalWrite(PIN_LED_OK,  LOW);
    digitalWrite(PIN_LED_ERR, LOW);
    noTone(PIN_BUZZ);
}

void beep(int freq, int durationMs) {
    tone(PIN_BUZZ, freq, durationMs);
    delay(durationMs);
    noTone(PIN_BUZZ);
}

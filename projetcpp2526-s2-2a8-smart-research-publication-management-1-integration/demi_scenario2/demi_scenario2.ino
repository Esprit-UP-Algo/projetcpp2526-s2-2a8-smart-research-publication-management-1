// =====================================================================
// demi_scenario2.ino — Affichage programme SmartPub sur panneau
//                      RGB LED Matrix 64x32 pixels (HUB75)
// =====================================================================
//
// Materiel :
//   - Arduino Uno ou Mega
//   - Panneau RGB LED Matrix 64x32, pitch 3mm, interface HUB75
//   - Alimentation 5V / 4A EXTERNE pour le panneau (OBLIGATOIRE)
//   - 1 bouton poussoir sur pin 2
//
// Bibliotheques requises (installer via Arduino IDE > Gerer bibliotheques) :
//   1. "Adafruit RGB Matrix Panel"  by Adafruit
//   2. "Adafruit GFX Library"       by Adafruit
//
// Cablage HUB75 → Arduino Uno :
//   R1  → pin 2    G1  → pin 3
//   B1  → pin 4    R2  → pin 5
//   G2  → pin 6    B2  → pin 7
//   A   → pin A0   B   → pin A1
//   C   → pin A2   D   → pin A3
//   CLK → pin 8    LAT → pin 10
//   OE  → pin 9    GND → GND
//   VCC → alimentation 5V externe UNIQUEMENT
//
// Bouton poussoir :
//   Une patte → pin 12
//   Autre patte → GND
//
// Protocole serie Qt → Arduino :
//   "MSG:<texte>|<couleur>\n"  → defiler le texte avec la couleur
//   "CLEAR\n"                  → eteindre le panneau
//   "DONE\n"                   → fin sequence, retour ecran attente
//
// Protocole serie Arduino → Qt :
//   "READY\n"        → panneau initialise, Arduino pret
//   "BTN:AFFICHER\n" → bouton presse, Qt lance afficherProgrammeGlobal()
// =====================================================================

#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>

// ── Pins HUB75 ──────────────────────────────────────────────────────
#define CLK  8
#define LAT 10
#define OE   9
#define A   A0
#define B   A1
#define C   A2
#define D   A3

// Panneau 64x32 — double largeur (2 panneaux 32x32 en cascade)
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false, 64);

// ── Bouton poussoir ─────────────────────────────────────────────────
#define BTN_PIN 12

// ── Anti-rebond bouton ───────────────────────────────────────────────
bool btnPrecedent = HIGH;

// =====================================================================
void setup() {
    Serial.begin(9600);
    delay(500);

    // Initialiser le panneau
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.fillScreen(0);      // eteindre toutes les LEDs

    // Bouton
    pinMode(BTN_PIN, INPUT_PULLUP);

    // Message d'accueil
    afficherTexteDefilant("SmartPub", matrix.Color333(0, 7, 0)); // vert

    // Signaler a Qt que le panneau est pret
    Serial.println("READY");
}

// =====================================================================
void loop() {

    // ── Lecture serie Qt → Arduino ────────────────────────────────────
    if (Serial.available()) {
        String ligne = Serial.readStringUntil('\n');
        ligne.trim();
        traiterCommande(ligne);
    }

    // ── Lecture bouton Arduino → Qt ───────────────────────────────────
    bool btnActuel = digitalRead(BTN_PIN);
    if (btnActuel == LOW && btnPrecedent == HIGH) {
        Serial.println("BTN:AFFICHER");
        delay(50); // anti-rebond
    }
    btnPrecedent = btnActuel;
}

// =====================================================================
// traiterCommande()
// =====================================================================
void traiterCommande(String cmd) {

    if (cmd == "CLEAR") {
        matrix.fillScreen(0);
        return;
    }

    if (cmd == "DONE") {
        delay(500);
        afficherTexteDefilant("En attente...", matrix.Color333(0, 7, 0));
        return;
    }

    if (cmd.startsWith("MSG:")) {
        // Format : "MSG:<texte>|<couleur>"
        String contenu = cmd.substring(4);
        int sep = contenu.lastIndexOf('|');

        String texte   = (sep > 0) ? contenu.substring(0, sep)  : contenu;
        String couleur = (sep > 0) ? contenu.substring(sep + 1) : "WHITE";

        uint16_t col = resolverCouleur(couleur);
        afficherTexteDefilant(texte, col);
        return;
    }
    // Commande inconnue : ignorer
}

// =====================================================================
// resolverCouleur()
// Color333(R, G, B) — valeurs 0 a 7 par canal
// =====================================================================
uint16_t resolverCouleur(String nom) {
    nom.toUpperCase();
    if (nom == "RED")    return matrix.Color333(7, 0, 0);
    if (nom == "GREEN")  return matrix.Color333(0, 7, 0);
    if (nom == "BLUE")   return matrix.Color333(0, 0, 7);
    if (nom == "YELLOW") return matrix.Color333(7, 7, 0);
    if (nom == "CYAN")   return matrix.Color333(0, 7, 7);
    return matrix.Color333(7, 7, 7); // WHITE
}

// =====================================================================
// afficherTexteDefilant()
// Defilement de droite a gauche sur le panneau 64x32
// Police 1 = 6x8 pixels par caractere
// =====================================================================
void afficherTexteDefilant(String texte, uint16_t couleur) {
    int largeur = texte.length() * 6; // largeur totale en pixels

    matrix.setTextSize(1);
    matrix.setTextColor(couleur);

    // Defiler de x=64 jusqu'a x=-largeur
    for (int x = 64; x >= -largeur; x--) {
        matrix.fillScreen(0);          // effacer
        matrix.setCursor(x, 12);       // centrer verticalement
        matrix.print(texte);
        delay(35);                     // vitesse defilement
    }
}

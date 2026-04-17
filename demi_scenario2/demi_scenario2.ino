// =====================================================================
// demi_scenario2.ino — Affichage LED du programme journalier par module
// SmartPub — Systeme de Gestion de Recherche
// =====================================================================
//
// Protocole serie Qt → Arduino :
//   "MSG:<texte>\n"  → afficher le texte (defilement si > 16 car.)
//   "CLEAR\n"        → effacer l'ecran
//   "DONE\n"         → fin de sequence, retour ecran d'attente
//
// Materiel : Arduino Uno + ecran LCD I2C 16x2 (adresse 0x27)
// =====================================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- OBJET LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- CONSTANTES ----------------
#define SCROLL_DELAY_MS   300   // delai entre chaque pas de defilement (ms)
#define MSG_HOLD_MS      2500   // duree d'affichage d'un message court (ms)
#define SCROLL_HOLD_MS    200   // pause apres defilement complet (ms)

// =====================================================================
void setup() {
    Serial.begin(9600);
    delay(500);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Ecran d'accueil
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");
    lcd.setCursor(0, 1);
    lcd.print(" Programme LED");
    delay(2000);

    afficherAttente();
}

// =====================================================================
void loop() {
    if (Serial.available()) {
        String ligne = Serial.readStringUntil('\n');
        ligne.trim();
        traiterCommande(ligne);
    }
}

// =====================================================================
// traiterCommande()
// Dispatche selon le prefixe recu depuis Qt
// =====================================================================
void traiterCommande(String cmd) {

    if (cmd == "CLEAR") {
        lcd.clear();
        return;
    }

    if (cmd == "DONE") {
        delay(1000);
        afficherAttente();
        return;
    }

    if (cmd.startsWith("MSG:")) {
        String texte = cmd.substring(4);
        afficherTexte(texte);
        return;
    }

    // Commande inconnue : ignorer silencieusement
}

// =====================================================================
// afficherTexte()
// Affiche un texte sur le LCD.
// - Ligne 0 : label du module (avant le ':')
// - Ligne 1 : contenu (apres le ':'), avec defilement si > 16 car.
// =====================================================================
void afficherTexte(String texte) {
    lcd.clear();

    // Detecter le separateur ':'
    int sep = texte.indexOf(':');

    String ligne0 = "";
    String ligne1 = "";

    if (sep > 0 && sep < 14) {
        // Ex: "Projet: Smart Research - En cours"
        //     ligne0 = "Projet:"
        //     ligne1 = " Smart Research - En cours"
        ligne0 = texte.substring(0, sep + 1);
        ligne1 = texte.substring(sep + 1);
        ligne1.trim();
    } else {
        // Pas de separateur clair : tout sur ligne 1
        ligne0 = "";
        ligne1 = texte;
    }

    // Afficher ligne 0 (tronquee a 16 car.)
    lcd.setCursor(0, 0);
    if (ligne0.length() > 16) ligne0 = ligne0.substring(0, 16);
    lcd.print(ligne0);

    // Afficher ligne 1 avec defilement si necessaire
    if (ligne1.length() <= 16) {
        lcd.setCursor(0, 1);
        lcd.print(ligne1);
        delay(MSG_HOLD_MS);
    } else {
        // Defilement horizontal sur la ligne 1
        defilerLigne(ligne1, 1);
    }
}

// =====================================================================
// defilerLigne()
// Fait defiler un texte long sur la ligne indiquee du LCD
// =====================================================================
void defilerLigne(String texte, int ligne) {
    // Ajouter des espaces pour un defilement propre
    String padded = "                " + texte + "                ";
    int len = padded.length();

    for (int i = 0; i <= len - 16; i++) {
        lcd.setCursor(0, ligne);
        lcd.print(padded.substring(i, i + 16));
        delay(SCROLL_DELAY_MS);
    }
    delay(SCROLL_HOLD_MS);
}

// =====================================================================
// afficherAttente()
// Ecran de veille standard entre deux sequences
// =====================================================================
void afficherAttente() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SmartPub");
    lcd.setCursor(0, 1);
    lcd.print("En attente...");
}

// =====================================================================
// centrer()
// Centre un texte sur 16 caracteres
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

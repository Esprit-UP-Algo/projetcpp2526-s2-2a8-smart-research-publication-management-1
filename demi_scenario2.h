#ifndef DEMI_SCENARIO2_H
#define DEMI_SCENARIO2_H

// ============================================================================
// DEMI_SCENARIO2 — Affichage sur panneau RGB LED Matrix 64x32 (HUB75)
//                  du programme journalier par module métier SmartPub
// ============================================================================
//
// Matériel :
//   - Arduino Uno / Mega
//   - Panneau RGB LED Matrix 64x32 pixels, pitch 3mm, interface HUB75
//     (192mm × 96mm, 2048 LEDs RGB)
//   - Alimentation 5V/4A externe pour le panneau (OBLIGATOIRE)
//   - 1 bouton poussoir (pin 2) pour déclencher l'affichage depuis l'Arduino
//
// Protocole série :
//
//   Qt → Arduino :
//     "MSG:<texte>|<couleur>\n"
//         → faire défiler le texte sur le panneau avec la couleur indiquée
//         → couleurs : RED / GREEN / BLUE / YELLOW / CYAN / WHITE
//     "CLEAR\n"   → éteindre le panneau
//     "DONE\n"    → fin de séquence, retour animation d'attente SmartPub
//
//   Arduino → Qt :
//     "READY\n"        → panneau initialisé, Arduino prêt
//     "BTN:AFFICHER\n" → bouton pressé, Qt doit lancer afficherProgrammeGlobal()
//
// Dépendances Qt : Arduino (connexion série), QSqlDatabase (via Connection)
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QStringList>
#include <QSqlDatabase>

class DemiScenario2 {
public:
    // Constructeur : reçoit le pointeur Arduino partagé avec scenario1
    explicit DemiScenario2(Arduino* arduino);

    // ── Input : traitement des messages reçus depuis l'Arduino ───────────
    // À appeler depuis le slot readyRead() dans SmartPub
    void processInput();

    // ── Output : fonctions par module (Qt → Arduino) ──────────────────────
    // Chacune interroge la BD et retourne une liste de messages formatés
    // Format : "<texte>|<couleur>"  ex: "Projet: SmartResearch|GREEN"
    QStringList afficherEvenementsSemaine();
    QStringList afficherProjetsSemaine();
    QStringList afficherFinanceSemaine();
    QStringList afficherLaboratoireSemaine();
    QStringList afficherPublicationsSemaine();
    QStringList afficherChercheursDisponibles();

    // ── Fonction globale ──────────────────────────────────────────────────
    // Agrège tous les modules et envoie la séquence complète au panneau LED
    void afficherProgrammeGlobal();

private:
    Arduino* m_arduino;

    // Envoie "MSG:<texte>|<couleur>\n" au panneau via Arduino
    void envoyerMessage(const QString& texte, const QString& couleur = "WHITE");

    // Envoie toute une liste de messages formatés "<texte>|<couleur>"
    void envoyerListe(const QStringList& messages);

    // Nettoie et tronque un texte pour l'affichage LED (max 64 car.)
    static QString formaterPourLed(const QString& texte, int maxLen = 64);
};

#endif // DEMI_SCENARIO2_H

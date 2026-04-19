#ifndef DEMI_SCENARIO2_H
#define DEMI_SCENARIO2_H

// ============================================================================
// DEMI_SCENARIO2 — Affichage LED du programme journalier par module métier
// ============================================================================
//
// Protocole série Qt → Arduino :
//   "MSG:<texte>\n"   → afficher le texte sur l'écran LCD (défilement si > 16 car.)
//   "CLEAR\n"         → effacer l'écran
//   "DONE\n"          → fin de séquence, retour à l'écran d'attente
//
// Chaque module envoie ses données sous forme de messages courts :
//   "Evenement: <nom>"
//   "Projet: <titre> - <etat>"
//   "Finance: <type> <montant>"
//   "Labo: <nom> - <statut>"
//   "Pub: <titre>"
//   "Chercheur: <nom> dispo"
//
// Dépendances : Arduino (connexion série), QSqlDatabase (via Connection)
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QStringList>
#include <QSqlDatabase>

class DemiScenario2 {
public:
    // Constructeur : reçoit le pointeur Arduino partagé avec scenario1
    explicit DemiScenario2(Arduino* arduino);

    // ── Fonctions par module ──────────────────────────────────────────────
    // Chacune interroge la BD et retourne une liste de messages courts
    QStringList afficherEvenementsSemaine();
    QStringList afficherProjetsSemaine();
    QStringList afficherFinanceSemaine();
    QStringList afficherLaboratoireSemaine();
    QStringList afficherPublicationsSemaine();
    QStringList afficherChercheursDisponibles();

    // ── Fonction globale ──────────────────────────────────────────────────
    // Agrège tous les modules et envoie la séquence complète à l'Arduino
    void afficherProgrammeGlobal();

private:
    Arduino* m_arduino;

    // Envoie un message court à l'Arduino (tronqué à 32 car. max)
    void envoyerMessage(const QString& msg);

    // Envoie toute une liste de messages avec pause entre chaque
    void envoyerListe(const QStringList& messages);

    // Tronque et nettoie un texte pour l'affichage LCD
    static QString formaterPourLed(const QString& texte, int maxLen = 32);
};

#endif // DEMI_SCENARIO2_H

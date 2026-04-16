#ifndef SCENARIO1_H
#define SCENARIO1_H

// ============================================================================
// SCENARIO 1 — Contrôle d'accès RFID au laboratoire (Entrée / Sortie)
// ============================================================================
//
// Protocole série :
//   Arduino → Qt : "RFID:<UID_HEX>\n"
//   Qt → Arduino : "ENTREE:<Nom Prenom>\n"  → accès autorisé, entrée
//                  "SORTIE:<Nom Prenom>\n"  → accès autorisé, sortie
//                  "REFUSE\n"              → accès refusé
//
// Colonnes BD utilisées (table CHERCHEUR) :
//   CLR_RFID            VARCHAR2(50)  — UID de la carte RFID
//   DATE_ENTREE_LAB     TIMESTAMP     — horodatage d'entrée (NULL = dehors)
//   TEMPS_TRAVAIL_PROJET NUMBER       — durée cumulée en secondes
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QSqlDatabase>

class Scenario1 {
public:
    // Constructeur : reçoit le pointeur Arduino et l'id du labo ciblé
    Scenario1(Arduino* arduino, int id_laboratoire);

    // Fonction principale à appeler depuis MainWindow quand des données arrivent
    void processAccess();

    // Résultat du dernier appel (utile pour mettre à jour l'UI dans SmartPub)
    bool    getLastAccessGranted() const { return m_lastGranted; }
    QString getLastChercheurNomPrenom() const { return m_lastNomPrenom; }
    bool    getLastIsEntree() const { return m_lastIsEntree; }

private:
    Arduino* arduino;   // connexion série vers l'Arduino
    int      id_lab;    // ID du laboratoire à contrôler

    // ── Résultat mémorisé ──────────────────────────────────────────────────
    bool    m_lastGranted    = false;
    QString m_lastNomPrenom;
    bool    m_lastIsEntree   = true;

    // ── Étape 1 : retrouver l'ID chercheur depuis le CLR_RFID ─────────────
    // Retourne -1 si non trouvé
    int getChercheurIdByRfid(const QString& rfid);

    // ── Étape 2 : récupérer nom et prénom du chercheur ────────────────────
    bool getChercheurInfo(int id_chercheur, QString& nom, QString& prenom);

    // ── Étape 3 : vérifier qu'il a un projet en cours ─────────────────────
    bool checkProjetEnCours(int id_chercheur);

    // ── Étape 4 : vérifier que le projet est dans CE labo ─────────────────
    bool checkProjetDansLabo(int id_chercheur, int id_lab);

    // ── Détection entrée / sortie ──────────────────────────────────────────
    // Retourne true si DATE_ENTREE_LAB IS NOT NULL (= chercheur déjà dans le labo)
    bool estDansLabo(int id_chercheur);

    // ── Persistance entrée ─────────────────────────────────────────────────
    // UPDATE CHERCHEUR SET DATE_ENTREE_LAB = SYSTIMESTAMP WHERE ID_CHERCHEUR = :id
    bool enregistrerEntree(int id_chercheur);

    // ── Persistance sortie ─────────────────────────────────────────────────
    // Calcule durée = SYSTIMESTAMP - DATE_ENTREE_LAB (en secondes)
    // Incrémente TEMPS_TRAVAIL_PROJET, remet DATE_ENTREE_LAB à NULL
    bool enregistrerSortie(int id_chercheur);

    // ── Envoi de commandes à l'Arduino ────────────────────────────────────
    void grantEntry(const QString& nom, const QString& prenom);
    void grantExit (const QString& nom, const QString& prenom);
    void denyAccess();
};

#endif // SCENARIO1_H

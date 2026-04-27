#ifndef SCENARIO1_H
#define SCENARIO1_H

// ============================================================================
// SCENARIO 1 — Controle d'acces RFID au laboratoire (Entree / Sortie)
// v6 : corrections RFID + ajout servo SG90 + buzzer + colonne ETAT_PORTE
//
// ARCHITECTURE v6 :
//   Un seul point d'entrée : processMessage()
//   Appelé depuis readyRead() dans smartpub.cpp, il lit UNE ligne du
//   buffer série et la dispatche selon son préfixe :
//     "RFID:<uid>"    → flux acces chercheur (inchangé v5)
//     "PORTE_OUVERTE" → mettreAJourEtatPorte(true)
//     "PORTE_FERMEE"  → mettreAJourEtatPorte(false)
//
//   processAccess() est conservé comme alias de processMessage()
//   pour compatibilité avec le code existant dans smartpub.cpp.
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QSqlDatabase>

class Scenario1 {
public:
    explicit Scenario1(Arduino* arduino, int id_laboratoire);

    // Point d'entrée principal — à appeler depuis readyRead()
    // Lit une ligne du buffer et dispatche selon le type de message.
    void processMessage();

    // Alias conservé pour compatibilité avec smartpub.cpp existant
    void processAccess() { processMessage(); }

    // Accesseurs pour l'interface Qt
    bool    getLastAccessGranted()      const { return m_lastGranted;   }
    QString getLastChercheurNomPrenom() const { return m_lastNomPrenom; }
    bool    getLastIsEntree()           const { return m_lastIsEntree;  }

private:
    Arduino* arduino;
    int      id_lab;

    bool    m_lastGranted   = false;
    QString m_lastNomPrenom;
    bool    m_lastIsEntree  = true;
    bool    m_enTraitement  = false;   // garde anti-reentrance readyRead

    // ── Flux RFID — accès chercheur ───────────────────────────────────
    void handleRfid(const QString& uid);

    int  getChercheurIdByRfid (const QString& rfid);
    bool getChercheurInfo     (int id_chercheur, QString& nom, QString& prenom);
    bool checkProjetEnCours   (int id_chercheur);
    bool checkProjetDansLabo  (int id_chercheur, int id_lab);
    bool estDansLabo          (int id_chercheur);
    bool enregistrerEntree    (int id_chercheur);
    bool enregistrerSortie    (int id_chercheur);

    // ── Flux porte — servo SG90 ───────────────────────────────────────
    bool mettreAJourEtatPorte(bool etatOuvert);

    // ── Commandes Arduino ─────────────────────────────────────────────
    void grantEntry (const QString& nom, const QString& prenom);
    void grantExit  (const QString& nom, const QString& prenom);
    void denyAccess ();
};

#endif // SCENARIO1_H

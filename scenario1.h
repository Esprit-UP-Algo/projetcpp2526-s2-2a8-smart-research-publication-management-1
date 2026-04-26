#ifndef SCENARIO1_H
#define SCENARIO1_H

// ============================================================================
// SCENARIO 1 — Controle d'acces RFID au laboratoire (Entree / Sortie)
// v5 : suppression de flushReadBuffer() (causait reset DTR parasite)
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QSqlDatabase>

class Scenario1 {
public:
    explicit Scenario1(Arduino* arduino, int id_laboratoire);

    void processAccess();

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

    int  getChercheurIdByRfid (const QString& rfid);
    bool getChercheurInfo     (int id_chercheur, QString& nom, QString& prenom);
    bool checkProjetEnCours   (int id_chercheur);
    bool checkProjetDansLabo  (int id_chercheur, int id_lab);
    bool estDansLabo          (int id_chercheur);
    bool enregistrerEntree    (int id_chercheur);
    bool enregistrerSortie    (int id_chercheur);

    void grantEntry (const QString& nom, const QString& prenom);
    void grantExit  (const QString& nom, const QString& prenom);
    void denyAccess ();
};

#endif // SCENARIO1_H

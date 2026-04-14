#ifndef SCENARIO3_H
#define SCENARIO3_H

#include "arduino.h"
#include <QString>

// ============================================================================
// Scénario 3 — Vérification d'existence du chercheur par carte RFID
//
//   - Journal horodaté (RfidJournal)
//   - Compteur d'échecs consécutifs → commande ALERTE après 3 scans refusés
//   - Réponses série : TROUVE / INCONNU / ALERTE
// ============================================================================

class Scenario3 {
public:
    explicit Scenario3(Arduino *arduino);

    void processIdentification();
    void processIdentification(const QString &message);

    bool dernierResultat() const { return m_dernierResultat; }
    QString dernierCin() const { return m_dernierCin; }
    QString dernierNom() const { return m_dernierNom; }
    int compteurEchecs() const { return m_compteurEchecs; }

private:
    Arduino *m_arduino;

    bool m_dernierResultat;
    QString m_dernierCin;
    QString m_dernierNom;
    int m_compteurEchecs;

    bool chercheurExiste(const QString &cin, QString &nomComplet);

    void sendTrouve();
    void sendInconnu();
    void sendAlerte();
};

#endif // SCENARIO3_H

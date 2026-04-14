#ifndef SCENARIO3_H
#define SCENARIO3_H

#include "arduino.h"
#include <QString>

// ============================================================================
// Scénario 3 — Vérification existence chercheur par carte RFID
//
// Fonctionnalités :
//   - Reçoit "RFID:<CIN>" depuis l'Arduino
//   - Vérifie l'existence du chercheur dans la table CHERCHEUR (champ CIN)
//   - Répond TROUVE / INCONNU / ALERTE à l'Arduino
//   - Journal horodaté de chaque passage (rfid_journal)
//   - Compteur de tentatives échouées → ALERTE après 3 échecs consécutifs
//   - Envoi commande "ALERTE" à l'Arduino (buzzer long 3 bips)
// ============================================================================

class Scenario3 {
public:
    explicit Scenario3(Arduino* arduino);

    // Appelée depuis SmartPub sur readyRead() quand le module Chercheurs est actif
    void processIdentification();

    // Accesseurs pour l'UI
    bool    dernierResultat()  const { return m_dernierResultat; }
    QString dernierCin()       const { return m_dernierCin; }
    QString dernierNom()       const { return m_dernierNom; }
    int     compteurEchecs()   const { return m_compteurEchecs; }

private:
    Arduino* m_arduino;

    bool    m_dernierResultat;
    QString m_dernierCin;
    QString m_dernierNom;
    int     m_compteurEchecs;   // tentatives échouées consécutives

    // Requête SQL : cherche le chercheur par CIN, remplit nomComplet si trouvé
    bool chercheurExiste(const QString& cin, QString& nomComplet);

    void sendTrouve();   // "TROUVE\n"  → LED verte
    void sendInconnu();  // "INCONNU\n" → LED rouge + buzzer
    void sendAlerte();   // "ALERTE\n"  → buzzer long 3 bips
};

#endif // SCENARIO3_H

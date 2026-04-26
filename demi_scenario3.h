#ifndef DEMI_SCENARIO3_H
#define DEMI_SCENARIO3_H

// ============================================================================
// DEMI_SCENARIO3 — Détection incendie laboratoire via capteur DHT11
//                  Passage automatique du statut laboratoire : Actif → Inactif
// ============================================================================
//
// Matériel :
//   - Arduino Uno
//   - Capteur DHT11 (température et humidité)
//   - LED rouge (alarme visuelle)
//   - Buzzer actif (alerte sonore)
//
// Protocole série :
//
//   Arduino → Qt :
//     "FIRE:<id_labo>\n"  → incendie détecté, Qt met le labo en Inactif en BD
//     "TEMP:<valeur>\n"   → température courante (envoyée toutes les 5s)
//     "READY\n"           → Arduino initialisé et prêt
//     "ERR:DHT_READ\n"    → erreur de lecture du capteur DHT11
//     "RESET:OK\n"        → réinitialisation confirmée
//
//   Qt → Arduino :
//     "ACK\n"             → accusé de réception de l'alerte incendie
//     "RESET\n"           → réinitialiser l'état après intervention
//
// Colonnes BD utilisées (table LABORATOIRE) :
//   ID_LABORATOIRE   NUMBER        — identifiant du laboratoire
//   DISPONIBILITE    VARCHAR2(20)  — 'disponible' (Actif) ou 'indisponible' (Inactif)
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QSqlDatabase>

class DemiScenario3 {
public:
    // Constructeur : reçoit le pointeur Arduino partagé
    explicit DemiScenario3(Arduino* arduino);

    // Fonction principale à appeler depuis le slot readyRead() dans SmartPub
    // Lit les messages Arduino et réagit aux alertes FIRE et TEMP
    void processInput();

    // Accesseurs pour l'UI SmartPub
    double  getDerniereTemperature() const { return m_derniereTemp; }
    bool    isIncendieDetecte()      const { return m_incendieDetecte; }
    int     getIdLaboEnAlerte()      const { return m_idLaboEnAlerte; }

private:
    Arduino* m_arduino;

    double m_derniereTemp     = 0.0;
    bool   m_incendieDetecte  = false;
    int    m_idLaboEnAlerte   = -1;

    // Met le laboratoire en statut 'indisponible' (Inactif) en base de données
    bool desactiverLaboratoire(int id_labo);

    // Envoie "ACK\n" à l'Arduino pour accuser réception de l'alerte
    void envoyerAck();
};

#endif // DEMI_SCENARIO3_H

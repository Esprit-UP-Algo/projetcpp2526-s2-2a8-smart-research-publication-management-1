#ifndef DEMI_SCENARIO3_H
#define DEMI_SCENARIO3_H

// ============================================================================
// DEMI_SCENARIO3 — Détection chaleur laboratoire via capteur DHT11
//                  Passage automatique du statut laboratoire : Actif → Inactif

//

//-------------------------------------------------------------------------------

// Flux :
//   1. L'utilisateur clique sur un laboratoire dans l'interface Qt.
//   2. activerPourLabo(id) envoie "START:<id>\n" à l'Arduino via le port série.
//   3. L'Arduino lit le DHT11 et renvoie "TEMP:<val>\n" puis "FIRE:<id>\n"
//      si la température dépasse le seuil (20 °C).
//   4. DemiScenario3 est connecté directement sur readyRead() du QSerialPort
//      avec son propre buffer interne — indépendant de Scenario1.
//
// Protocole série :
//   Qt → Arduino : "START:<id_labo>\n"
//   Arduino → Qt : "TEMP:<valeur>\n" | "FIRE:<id_labo>\n" | "ERR:DHT_READ\n"
//
// Table BD : LABORATOIRE
//   DISPONIBILITE = 'indisponible'  ↔  statut "Inactif" dans l'UI
// ============================================================================

#include "arduino.h"
#include <QObject>
#include <QString>
#include <QByteArray>

class DemiScenario3 : public QObject {
    Q_OBJECT

public:
    explicit DemiScenario3(Arduino* arduino, QObject* parent = nullptr);

    // Déclenche une lecture DHT11 pour le laboratoire donné.
    // À appeler au clic sur un laboratoire dans l'interface Qt.
    void activerPourLabo(int id_labo);

    // Accesseurs
    double getDerniereTemperature() const { return m_derniereTemp; }
    bool   isIncendieDetecte()      const { return m_incendieDetecte; }
    int    getIdLaboEnAlerte()      const { return m_idLaboEnAlerte; }

signals:
    // Émis quand un laboratoire vient d'être passé en Inactif en BD.
    // SmartPub connecte ce signal pour rafraîchir la table en temps réel.
    void laboDesactive(int id_labo);

private slots:
    // Connecté directement sur QSerialPort::readyRead() — lit son propre buffer.
    void onSerialDataReady();

private:
    Arduino*   m_arduino;
    QByteArray m_buffer;   // buffer interne indépendant de Scenario1

    double m_derniereTemp    = 0.0;
    bool   m_incendieDetecte = false;
    int    m_idLaboEnAlerte  = -1;

    void processLine(const QString& line);
    bool desactiverLaboratoire(int id_labo);
};

#endif // DEMI_SCENARIO3_H

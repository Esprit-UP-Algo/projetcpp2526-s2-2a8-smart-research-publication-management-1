#ifndef SCENARIO3_H
#define SCENARIO3_H

// ============================================================================
// SCÉNARIO 3 — Clôture de projet via Arduino + génération automatique
//              d'un événement de clôture
// ============================================================================
//
// Contexte métier :
//   Le chef de projet appuie sur un bouton Arduino pour signaler
//   physiquement la fin d'un projet. L'application :
//     1. Vérifie que le projet existe et est en état 'en_cours'
//     2. UPDATE PROJET SET ETAT='termine', PROGRESSION=100
//     3. INSERT INTO EVENEMENT → crée un événement "Clôture : <titre>"
//     4. Répond à l'Arduino OK ou ERR
//
// Protocole série :
//   Arduino → PC : "CLOTURER:<id_projet>\n"
//   PC → Arduino : "OK:<titre_projet>\n"
//                  "ERR:<raison>\n"
//
// À utiliser avec le même port série que le scénario 1 : le routeur (SmartPub)
// envoie ici uniquement les lignes commençant par CLOTURER:
// ============================================================================

#include "arduino.h"
#include <QString>

class Scenario3
{
public:
    explicit Scenario3(Arduino *arduino);

    // Lecture série + traitement (si appel isolé sur readyRead)
    void processCloture();

    // Traitement d'une ligne déjà lue (routage commun avec scénario 1)
    void processClotureFromMessage(const QString &message);

private:
    Arduino *m_arduino;

    QString getProjetEnCours(int idProjet);
    bool cloturerProjet(int idProjet);
    int creerEvenementCloture(int idProjet, const QString &titreProjet);
    void sendOk(const QString &titreProjet);
    void sendError(const QString &raison);
};

#endif // SCENARIO3_H

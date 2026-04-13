#ifndef SCENARIO2_H
#define SCENARIO2_H

// ============================================================================
// SCENARIO 2 — Clôture de projet via Arduino + génération automatique
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
// Complètement indépendant de Scenario1. Aucun fichier existant modifié.
// ============================================================================

#include "arduino.h"
#include <QString>

class Scenario2
{
public:
    explicit Scenario2(Arduino *arduino);

    // À connecter sur readyRead() du QSerialPort associé à cet Arduino
    void processCloture();

private:
    Arduino *m_arduino;

    // Étape 1 : vérifier que le projet existe et est 'en_cours'
    // Retourne le titre du projet, ou QString() si invalide
    QString getProjetEnCours(int idProjet);

    // Étape 2 : UPDATE PROJET SET ETAT='termine', PROGRESSION=100
    bool cloturerProjet(int idProjet);

    // Étape 3 : INSERT INTO EVENEMENT (événement de clôture)
    // Retourne le code généré, ou -1 en cas d'échec
    int creerEvenementCloture(int idProjet, const QString &titreProjet);

    // Réponses vers Arduino
    void sendOk(const QString &titreProjet);
    void sendError(const QString &raison);
};

#endif // SCENARIO2_H

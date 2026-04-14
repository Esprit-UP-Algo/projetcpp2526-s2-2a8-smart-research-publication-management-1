#ifndef SCENARIO1_H
#define SCENARIO1_H

#include "arduino.h"
#include <QString>
#include <QSqlDatabase>


class Scenario1 {
public:
    // Constructeur : reçoit le pointeur Arduino et l'id du labo ciblé
    Scenario1(Arduino* arduino, int id_laboratoire);

    // Fonction principale à appeler depuis MainWindow quand des données arrivent
    void processAccess();

    // Ligne déjà extraite du port série (routage commun avec le scénario 3)
    void processAccessFromMessage(const QString &message);

private:
    Arduino* arduino;       // pointeur vers l'objet Arduino (connexion série)
    int id_lab;             // ID du laboratoire à contrôler

    // Étape 1 : retrouver l'ID chercheur depuis l'UID RFID (stocké dans CIN)
    // Retourne -1 si non trouvé
    int getChercheurId(const QString& uid);

    // Étape 2 : vérifier qu'il contribue à un projet en cours
    bool checkProjetEnCours(int id_chercheur);

    // Étape 3 : vérifier que ce projet est bien affecté à CE labo
    bool checkProjetDansLabo(int id_chercheur, int id_lab);

    // Envoyer la réponse à l'Arduino
    void grantAccess();   // OPEN + GREEN
    void denyAccess();    // RED + BUZZ
};

#endif // SCENARIO1_H

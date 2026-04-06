#ifndef EVENEMENT_H
#define EVENEMENT_H

#include <QString>

struct EventData {
    QString id;   // ID_EVENEMENT (clé primaire technique)
    QString code; // CODE_EVENEMENT (unique, saisi par l'utilisateur)
    QString nom;
    QString lieu;
    QString date;
};

#endif // EVENEMENT_H

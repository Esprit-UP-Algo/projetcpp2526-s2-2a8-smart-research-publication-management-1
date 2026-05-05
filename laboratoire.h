#ifndef LABORATOIRE_H
#define LABORATOIRE_H

#include <QString>

struct LaboratoryData {
    int id;
    QString nom;
    QString thematique;
    double budget;
    int capacite;
    QString equipements;
    QString statut;
    QString directeur;
    QString adresse;
    bool etatPorte = false;  // true = porte ouverte (ETAT_PORTE = 1)

    LaboratoryData() : id(0), budget(0.0), capacite(0), etatPorte(false) {}
};

#endif // LABORATOIRE_H
//

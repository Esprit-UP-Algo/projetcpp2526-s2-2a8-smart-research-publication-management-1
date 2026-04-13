#ifndef COLLABORATIONENGINE_H
#define COLLABORATIONENGINE_H

#include "laboratoire.h"
#include <QList>
#include <QString>

struct CollaborationSynergy {
    QString nomLab1;
    QString nomLab2;
    QString typesynergie;
    int     scoreEtoiles = 0;   // 1–5
    double  economies    = 0.0; // économies potentielles en €
    QString recommandation;
};

class CollaborationEngine
{
public:
    // Détecte toutes les synergies entre laboratoires
    static QList<CollaborationSynergy> detect(const QList<LaboratoryData> &labs);

    // Calcule les économies potentielles d'une paire
    static double computeSavings(const LaboratoryData &l1, const LaboratoryData &l2);
};

#endif // COLLABORATIONENGINE_H

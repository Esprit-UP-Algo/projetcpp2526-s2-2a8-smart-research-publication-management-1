#ifndef SCORINGENGINE_H
#define SCORINGENGINE_H

#include "laboratoire.h"
#include <QList>
#include <QString>

struct ScoredLaboratory {
    LaboratoryData lab;
    int    score      = 0;   // 0–100
    QString badge;           // "Excellent", "Bon", "Moyen", "À améliorer"
    QString badgeColor;      // hex color
    QString recommandation;
};

class ScoringEngine
{
public:
    // Calcule et retourne la liste triée par score décroissant
    static QList<ScoredLaboratory> score(const QList<LaboratoryData> &labs);

    // Score individuel d'un laboratoire (0–100)
    static int computeScore(const LaboratoryData &lab);

    // Badge et couleur selon le score
    static QString badge(int score);
    static QString badgeColor(int score);
    static QString recommandation(int score);
};

#endif // SCORINGENGINE_H

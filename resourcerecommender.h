#ifndef RESOURCERECOMMENDER_H
#define RESOURCERECOMMENDER_H

#include "laboratoire.h"
#include <QList>
#include <QString>

// Profil de besoin saisi par l'utilisateur
struct ResourceNeed {
    QString thematique;       // thématique recherchée
    double  budgetNecessaire; // budget requis (€)
    int     placesRequises;   // nombre de chercheurs à accueillir
    QString equipements;      // équipements indispensables (séparés par virgule)
};

// Résultat de recommandation pour un laboratoire
struct RecommendedLab {
    LaboratoryData lab;
    int    score;             // 0–100 (compatibilité %)
    QStringList raisons;      // détails ("capacité suffisante ✅", ...)
    QString recommandation;   // justification finale
};

class ResourceRecommender
{
public:
    // Analyse tous les labs et retourne un classement par compatibilité décroissante
    static QList<RecommendedLab> recommend(const QList<LaboratoryData> &labs,
                                           const ResourceNeed &need);

private:
    static int computeCompatibility(const LaboratoryData &lab,
                                    const ResourceNeed &need,
                                    QStringList &raisons);
    static QString buildRecommandation(int score, const LaboratoryData &lab);
};

#endif // RESOURCERECOMMENDER_H

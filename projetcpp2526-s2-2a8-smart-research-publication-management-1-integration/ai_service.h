#ifndef AI_SERVICE_H
#define AI_SERVICE_H

#include <QVector>
#include <QString>
#include <QStringList>

#include "projet.h"

class AIService {
public:
    struct Recommandation {
        QString titre;
        QString description;
        double scoreSimilarite;
        QStringList collaborateursSuggeres;
        QString raison;
        QString domaine;
    };

    static QVector<Recommandation> genererRecommandations(const QVector<Projet> &projets);
};

#endif // AI_SERVICE_H

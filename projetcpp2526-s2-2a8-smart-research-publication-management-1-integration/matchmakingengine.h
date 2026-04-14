#ifndef MATCHMAKINGENGINE_H
#define MATCHMAKINGENGINE_H

#include <QString>
#include <QVector>
#include <QSqlDatabase>

struct MatchCandidate {
    int idChercheur = 0;
    QString nomComplet;
    double scoreJaccard = 0.0;
    int nbMotsCommuns = 0;
};

class MatchmakingEngine
{
public:
    static QVector<MatchCandidate> suggestColleagues(QSqlDatabase &db, int idChercheurA, int maxResults = 12);
};

#endif // MATCHMAKINGENGINE_H

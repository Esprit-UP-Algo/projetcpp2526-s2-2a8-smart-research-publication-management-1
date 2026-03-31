#ifndef PROMOTIONENGINE_H
#define PROMOTIONENGINE_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>

struct PromotionCriteria {
    int minAnneesAnciennete = 5;
    int minPublications = 10;
    int minProjetsGeres = 2;
};

struct ChercheurPromotionProfile {
    int id = 0;
    QString nom;
    QString prenom;
    QString grade;
    int ancienneteAnnees = 0;
    int nbPublications = 0;
    int nbProjetsContrib = 0;
};

class PromotionEngine
{
public:
    static ChercheurPromotionProfile loadProfile(QSqlDatabase &db, int idChercheur);
    static bool isEligible(const ChercheurPromotionProfile &p, const PromotionCriteria &c);
    static double eligibilityScore(const ChercheurPromotionProfile &p, const PromotionCriteria &c);
    static QStringList detailChecks(const ChercheurPromotionProfile &p, const PromotionCriteria &c);
};

#endif // PROMOTIONENGINE_H

#ifndef PROMOTIONENGINE_H
#define PROMOTIONENGINE_H

#include <QString>
#include <QStringList>
#include <QSqlDatabase>

struct PromotionCriteria {
    int minAnneesAnciennete = 5;
    int minPublications     = 10;
    int minProjetsGeres     = 2;
};

struct ChercheurPromotionProfile {
    int     id               = 0;
    QString nom;
    QString prenom;
    QString grade;
    int     ancienneteAnnees = 0;
    int     nbPublications   = 0;
    int     nbProjetsContrib = 0;
};

class PromotionEngine
{
public:
    // Chargement du profil depuis la BD
    static ChercheurPromotionProfile loadProfile(QSqlDatabase &db, int idChercheur);

    // Éligibilité binaire (tous les critères atteints ?)
    static bool isEligible(const ChercheurPromotionProfile &p, const PromotionCriteria &c);

    // Score de complétude sur 10 (fonctionnel même si un seuil est 0)
    static double eligibilityScore(const ChercheurPromotionProfile &p, const PromotionCriteria &c);

    // Détail lisible des critères avec icônes ✅/❌
    static QStringList detailChecks(const ChercheurPromotionProfile &p, const PromotionCriteria &c);

    // Grade suivant dans la hiérarchie (vide si au sommet ou grade inconnu)
    static QString nextGrade(const QString &currentGrade);

    // Effectue la promotion en base (UPDATE CHERCHEUR SET GRADE = :newGrade)
    static bool promoteInDatabase(QSqlDatabase &db, int idChercheur, const QString &newGrade);
};

#endif // PROMOTIONENGINE_H

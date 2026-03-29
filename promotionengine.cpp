#include "promotionengine.h"
#include <QDate>
#include <QSqlQuery>
#include <QtMath>

static int yearsBetween(const QDate &from, const QDate &to)
{
    if (!from.isValid())
        return 0;
    int y = to.year() - from.year();
    if (to.month() < from.month() || (to.month() == from.month() && to.day() < from.day()))
        y--;
    return qMax(0, y);
}

ChercheurPromotionProfile PromotionEngine::loadProfile(QSqlDatabase &db, int idChercheur)
{
    ChercheurPromotionProfile p;
    p.id = idChercheur;

    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT NOM, PRENOM, GRADE FROM CHERCHEUR WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next()) {
        p.nom = q.value(0).toString();
        p.prenom = q.value(1).toString();
        p.grade = q.value(2).toString();
    }

    q.prepare(QStringLiteral("SELECT COUNT(*) FROM PUBLICATION WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next())
        p.nbPublications = q.value(0).toInt();

    q.prepare(QStringLiteral("SELECT COUNT(DISTINCT CODE_PROJET) FROM CONTRIBUER WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next())
        p.nbProjetsContrib = q.value(0).toInt();

    QDate refDebut;
    q.prepare(QStringLiteral("SELECT MIN(DATE_PUBLICATION) FROM PUBLICATION WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next()) {
        QVariant v = q.value(0);
        if (!v.isNull())
            refDebut = v.toDate();
    }
    if (!refDebut.isValid()) {
        q.prepare(QStringLiteral("SELECT MIN(DATE_DEBUT_CONTRIB) FROM CONTRIBUER WHERE ID_CHERCHEUR = :id"));
        q.bindValue(QStringLiteral(":id"), idChercheur);
        if (q.exec() && q.next()) {
            QVariant v = q.value(0);
            if (!v.isNull())
                refDebut = v.toDate();
        }
    }
    p.ancienneteAnnees = yearsBetween(refDebut, QDate::currentDate());
    return p;
}

bool PromotionEngine::isEligible(const ChercheurPromotionProfile &p, const PromotionCriteria &c)
{
    return p.ancienneteAnnees >= c.minAnneesAnciennete && p.nbPublications >= c.minPublications
           && p.nbProjetsContrib >= c.minProjetsGeres;
}

double PromotionEngine::eligibilityScore(const ChercheurPromotionProfile &p, const PromotionCriteria &c)
{
    if (c.minAnneesAnciennete <= 0 || c.minPublications <= 0 || c.minProjetsGeres <= 0)
        return 0.0;
    const double ra = qMin(1.0, double(p.ancienneteAnnees) / double(c.minAnneesAnciennete));
    const double rp = qMin(1.0, double(p.nbPublications) / double(c.minPublications));
    const double rj = qMin(1.0, double(p.nbProjetsContrib) / double(c.minProjetsGeres));
    return qRound((ra * 34.0 + rp * 33.0 + rj * 33.0) * 10.0) / 10.0;
}

QStringList PromotionEngine::detailChecks(const ChercheurPromotionProfile &p, const PromotionCriteria &c)
{
    QStringList lines;
    lines << QStringLiteral("• Ancienneté estimée : %1 ans (seuil %2)")
                 .arg(p.ancienneteAnnees)
                 .arg(c.minAnneesAnciennete);
    lines << QStringLiteral("• Publications : %1 (seuil %2)").arg(p.nbPublications).arg(c.minPublications);
    lines << QStringLiteral("• Projets (contributions) : %1 (seuil %2)")
                 .arg(p.nbProjetsContrib)
                 .arg(c.minProjetsGeres);
    lines << QStringLiteral("• Grade actuel : %1").arg(p.grade.isEmpty() ? QStringLiteral("—") : p.grade);
    return lines;
}

#include "promotionengine.h"
#include <QDate>
#include <QSqlQuery>
#include <QtMath>

static int yearsBetween(const QDate &from, const QDate &to)
{
    if (!from.isValid())
        return 0;
    int y = to.year() - from.year();
    if (to.month() < from.month() ||
        (to.month() == from.month() && to.day() < from.day()))
        y--;
    return qMax(0, y);
}

// Hiérarchie académique officielle de l'application
static const QStringList s_gradeHierarchy = {
    QStringLiteral("Doctorant"),
    QStringLiteral("Post-doctorant"),
    QStringLiteral("Ingenieur de Recherche"),
    QStringLiteral("Docteur"),
    QStringLiteral("Maitre de Conferences"),
    QStringLiteral("Professeur")
};

QString PromotionEngine::nextGrade(const QString &currentGrade)
{
    int idx = s_gradeHierarchy.indexOf(currentGrade);
    if (idx < 0 || idx >= s_gradeHierarchy.size() - 1)
        return QString(); // Grade inconnu ou déjà au sommet
    return s_gradeHierarchy.at(idx + 1);
}

ChercheurPromotionProfile PromotionEngine::loadProfile(QSqlDatabase &db, int idChercheur)
{
    ChercheurPromotionProfile p;
    p.id = idChercheur;

    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT NOM, PRENOM, GRADE FROM CHERCHEUR WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next()) {
        p.nom    = q.value(0).toString();
        p.prenom = q.value(1).toString();
        p.grade  = q.value(2).toString();
    }

    q.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM PUBLICATION "
        "WHERE ID_CHERCHEUR = :id AND ID_CHERCHEUR IS NOT NULL"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next())
        p.nbPublications = q.value(0).toInt();

    q.prepare(QStringLiteral(
        "SELECT COUNT(DISTINCT ID_PROJET) FROM CONTRIBUER WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next())
        p.nbProjetsContrib = q.value(0).toInt();

    // Ancienneté : 1ère publication, sinon 1er projet contribué
    QDate refDebut;
    q.prepare(QStringLiteral(
        "SELECT MIN(DATE_PUBLICATION) FROM PUBLICATION "
        "WHERE ID_CHERCHEUR = :id AND DATE_PUBLICATION IS NOT NULL"));
    q.bindValue(QStringLiteral(":id"), idChercheur);
    if (q.exec() && q.next()) {
        QVariant v = q.value(0);
        if (!v.isNull()) refDebut = v.toDate();
    }
    if (!refDebut.isValid()) {
        q.prepare(QStringLiteral(
            "SELECT MIN(p.DATE_DEBUT) "
            "FROM CONTRIBUER c "
            "INNER JOIN PROJET p ON p.ID_PROJET = c.ID_PROJET "
            "WHERE c.ID_CHERCHEUR = :id AND p.DATE_DEBUT IS NOT NULL"));
        q.bindValue(QStringLiteral(":id"), idChercheur);
        if (q.exec() && q.next()) {
            QVariant v = q.value(0);
            if (!v.isNull()) refDebut = v.toDate();
        }
    }
    p.ancienneteAnnees = yearsBetween(refDebut, QDate::currentDate());
    return p;
}

bool PromotionEngine::isEligible(const ChercheurPromotionProfile &p,
                                 const PromotionCriteria &c)
{
    return p.ancienneteAnnees  >= c.minAnneesAnciennete
           && p.nbPublications    >= c.minPublications
           && p.nbProjetsContrib  >= c.minProjetsGeres;
}

double PromotionEngine::eligibilityScore(const ChercheurPromotionProfile &p,
                                         const PromotionCriteria &c)
{
    // Chaque critère contribue indépendamment (poids égaux : 10/3 ≈ 3.33 pts)
    // Si le seuil est 0, le critère est considéré comme atteint à 100 %
    double ra, rp, rj;

    ra = (c.minAnneesAnciennete <= 0)
             ? 1.0
             : qMin(1.0, double(p.ancienneteAnnees) / double(c.minAnneesAnciennete));

    rp = (c.minPublications <= 0)
             ? 1.0
             : qMin(1.0, double(p.nbPublications) / double(c.minPublications));

    rj = (c.minProjetsGeres <= 0)
             ? 1.0
             : qMin(1.0, double(p.nbProjetsContrib) / double(c.minProjetsGeres));

    // Score pondéré sur 10, arrondi à 1 décimale
    double raw = (ra + rp + rj) / 3.0 * 10.0;
    return qRound(raw * 10.0) / 10.0;
}

QStringList PromotionEngine::detailChecks(const ChercheurPromotionProfile &p,
                                          const PromotionCriteria &c)
{
    auto icon = [](bool ok) -> QString {
        return ok ? QStringLiteral("✅") : QStringLiteral("❌");
    };

    QStringList lines;

    bool okAnc = (p.ancienneteAnnees >= c.minAnneesAnciennete);
    bool okPub = (p.nbPublications   >= c.minPublications);
    bool okPrj = (p.nbProjetsContrib >= c.minProjetsGeres);

    lines << QString("%1  Ancienneté : %2 ans  (seuil : %3 ans)")
                 .arg(icon(okAnc))
                 .arg(p.ancienneteAnnees)
                 .arg(c.minAnneesAnciennete);

    lines << QString("%1  Publications : %2  (seuil : %3)")
                 .arg(icon(okPub))
                 .arg(p.nbPublications)
                 .arg(c.minPublications);

    lines << QString("%1  Projets (contributions) : %2  (seuil : %3)")
                 .arg(icon(okPrj))
                 .arg(p.nbProjetsContrib)
                 .arg(c.minProjetsGeres);

    lines << QString("🎓  Grade actuel : %1")
                 .arg(p.grade.isEmpty() ? QStringLiteral("—") : p.grade);

    QString next = PromotionEngine::nextGrade(p.grade);
    if (!next.isEmpty())
        lines << QString("⬆️  Grade cible : %1").arg(next);
    else if (!p.grade.isEmpty())
        lines << QStringLiteral("🏆  Grade maximum atteint — aucune promotion possible");

    return lines;
}

bool PromotionEngine::promoteInDatabase(QSqlDatabase &db, int idChercheur,
                                        const QString &newGrade)
{
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "UPDATE CHERCHEUR SET GRADE = :grade WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":grade"), newGrade);
    q.bindValue(QStringLiteral(":id"),    idChercheur);
    return q.exec();
}

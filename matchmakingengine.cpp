#include "matchmakingengine.h"
#include <algorithm>
#include <QSet>
#include <QSqlQuery>
static const QStringList s_stopWords = {
    QStringLiteral("le"), QStringLiteral("la"), QStringLiteral("les"), QStringLiteral("un"), QStringLiteral("une"),
    QStringLiteral("des"), QStringLiteral("de"), QStringLiteral("du"), QStringLiteral("et"), QStringLiteral("ou"),
    QStringLiteral("pour"), QStringLiteral("dans"), QStringLiteral("sur"), QStringLiteral("avec"), QStringLiteral("sans"),
    QStringLiteral("the"), QStringLiteral("a"), QStringLiteral("an"), QStringLiteral("and"), QStringLiteral("or"),
    QStringLiteral("of"), QStringLiteral("to"), QStringLiteral("in"), QStringLiteral("for"), QStringLiteral("with")};

static QSet<QString> tokenizeTitles(const QString &text)
{
    QSet<QString> out;
    const QString simplified = text.toLower();
    QString cur;
    for (QChar ch : simplified) {
        if (ch.isLetterOrNumber() || ch == QLatin1Char('-'))
            cur.append(ch);
        else {
            if (cur.length() >= 4)
                out.insert(cur);
            cur.clear();
        }
    }
    if (cur.length() >= 4)
        out.insert(cur);
    QSet<QString> filtered;
    for (const QString &w : out) {
        if (!s_stopWords.contains(w))
            filtered.insert(w);
    }
    return filtered;
}

static QSet<QString> keywordsForResearcher(QSqlDatabase &db, int id)
{
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT TITRE FROM PUBLICATION WHERE ID_CHERCHEUR = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    QString blob;
    if (q.exec()) {
        while (q.next())
            blob += QLatin1Char(' ') + q.value(0).toString();
    }
    return tokenizeTitles(blob);
}

static bool ontDejaCollabore(QSqlDatabase &db, int idA, int idB)
{
    if (idA == idB)
        return true;
    QSqlQuery q(db);
    q.prepare(
        QStringLiteral("SELECT COUNT(*) FROM CONTRIBUER c1 "
                       "INNER JOIN CONTRIBUER c2 ON c1.CODE_PROJET = c2.CODE_PROJET "
                       "WHERE c1.ID_CHERCHEUR = :a AND c2.ID_CHERCHEUR = :b"));
    q.bindValue(QStringLiteral(":a"), idA);
    q.bindValue(QStringLiteral(":b"), idB);
    if (q.exec() && q.next())
        return q.value(0).toInt() > 0;
    return false;
}

static double jaccard(const QSet<QString> &a, const QSet<QString> &b)
{
    if (a.isEmpty() && b.isEmpty())
        return 0.0;
    int inter = 0;
    for (const QString &s : a) {
        if (b.contains(s))
            inter++;
    }
    const int uni = a.size() + b.size() - inter;
    if (uni <= 0)
        return 0.0;
    return double(inter) / double(uni);
}

QVector<MatchCandidate> MatchmakingEngine::suggestColleagues(QSqlDatabase &db, int idChercheurA, int maxResults)
{
    QVector<MatchCandidate> results;
    const QSet<QString> kwA = keywordsForResearcher(db, idChercheurA);
    if (kwA.isEmpty())
        return results;

    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT ID_CHERCHEUR, NOM, PRENOM FROM CHERCHEUR WHERE ID_CHERCHEUR <> :id"));
    q.bindValue(QStringLiteral(":id"), idChercheurA);
    if (!q.exec())
        return results;

    while (q.next()) {
        const int idB = q.value(0).toInt();
        if (ontDejaCollabore(db, idChercheurA, idB))
            continue;
        const QSet<QString> kwB = keywordsForResearcher(db, idB);
        if (kwB.isEmpty())
            continue;
        int inter = 0;
        for (const QString &s : kwA) {
            if (kwB.contains(s))
                inter++;
        }
        const double score = jaccard(kwA, kwB);
        if (score <= 0.0 && inter == 0)
            continue;
        MatchCandidate mc;
        mc.idChercheur = idB;
        mc.nomComplet = QStringLiteral("%1 %2").arg(q.value(2).toString(), q.value(1).toString()).trimmed();
        mc.scoreJaccard = score;
        mc.nbMotsCommuns = inter;
        results.append(mc);
    }

    std::sort(results.begin(), results.end(),
              [](const MatchCandidate &x, const MatchCandidate &y) { return x.scoreJaccard > y.scoreJaccard; });
    if (results.size() > maxResults)
        results.resize(maxResults);
    return results;
}

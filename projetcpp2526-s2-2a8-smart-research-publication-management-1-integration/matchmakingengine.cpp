#include "matchmakingengine.h"
#include <algorithm>
#include <QSet>
#include <QHash>
#include <QSqlQuery>
#include <QVariant>

static const QSet<QString> s_stopWords = {
    QStringLiteral("le"), QStringLiteral("la"), QStringLiteral("les"), QStringLiteral("un"), QStringLiteral("une"),
    QStringLiteral("des"), QStringLiteral("de"), QStringLiteral("du"), QStringLiteral("et"), QStringLiteral("ou"),
    QStringLiteral("pour"), QStringLiteral("dans"), QStringLiteral("sur"), QStringLiteral("avec"), QStringLiteral("sans"),
    QStringLiteral("the"), QStringLiteral("a"), QStringLiteral("an"), QStringLiteral("and"), QStringLiteral("or"),
    QStringLiteral("of"), QStringLiteral("to"), QStringLiteral("in"), QStringLiteral("for"), QStringLiteral("with")
};

static QSet<QString> tokenizeTitles(const QString &text)
{
    QSet<QString> out;
    const QString simplified = text.toLower();
    QString cur;
    for (QChar ch : simplified) {
        if (ch.isLetterOrNumber() || ch == QLatin1Char('-')) {
            cur.append(ch);
        } else {
            // Changement ici : >= 2 pour garder "IA", "AI", "IoT", etc.
            if (cur.length() >= 2 && !s_stopWords.contains(cur))
                out.insert(cur);
            cur.clear();
        }
    }
    if (cur.length() >= 2 && !s_stopWords.contains(cur))
        out.insert(cur);

    return out;
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

    // 1. Charger les IDs des collaborateurs directs de A (même projet via CONTRIBUER)
    //    On utilise deux paramètres nommés distincts pour Oracle qui n'accepte pas
    //    le même placeholder deux fois dans une même requête.
    QSet<int> collaborateursA;
    collaborateursA.insert(idChercheurA); // S'exclure soi-même
    {
        QSqlQuery qCollab(db);
        qCollab.prepare(QStringLiteral(
            "SELECT c2.ID_CHERCHEUR "
            "FROM CONTRIBUER c1 "
            "INNER JOIN CONTRIBUER c2 ON c1.ID_PROJET = c2.ID_PROJET "
            "WHERE c1.ID_CHERCHEUR = :a1 AND c2.ID_CHERCHEUR <> :a2"));
        qCollab.bindValue(QStringLiteral(":a1"), idChercheurA);
        qCollab.bindValue(QStringLiteral(":a2"), idChercheurA);
        if (qCollab.exec()) {
            while (qCollab.next())
                collaborateursA.insert(qCollab.value(0).toInt());
        }
    }

    // 2. Charger les titres de publications par chercheur (table PUBLICATION, FK ID_CHERCHEUR)
    //    Seules les publications dont ID_CHERCHEUR IS NOT NULL sont prises en compte.
    QHash<int, QString> titresParChercheur;
    {
        QSqlQuery qPubs(db);
        if (qPubs.exec(QStringLiteral(
                "SELECT ID_CHERCHEUR, TITRE FROM PUBLICATION "
                "WHERE ID_CHERCHEUR IS NOT NULL AND TITRE IS NOT NULL"))) {
            while (qPubs.next()) {
                const int id = qPubs.value(0).toInt();
                titresParChercheur[id] += QLatin1Char(' ') + qPubs.value(1).toString();
            }
        }
    }

    const QSet<QString> kwA = tokenizeTitles(titresParChercheur.value(idChercheurA));
    // Si le chercheur A n'a aucune publication avec titre, retourner vide
    if (kwA.isEmpty())
        return results;

    // 3. Parcourir tous les autres chercheurs et calculer le score de Jaccard
    {
        QSqlQuery qChercheurs(db);
        qChercheurs.prepare(QStringLiteral(
            "SELECT ID_CHERCHEUR, NOM, PRENOM FROM CHERCHEUR WHERE ID_CHERCHEUR <> :id"));
        qChercheurs.bindValue(QStringLiteral(":id"), idChercheurA);
        if (!qChercheurs.exec())
            return results;

        while (qChercheurs.next()) {
            const int idB = qChercheurs.value(0).toInt();

            // Exclure les collaborateurs déjà connus
            if (collaborateursA.contains(idB))
                continue;

            // Exclure les chercheurs sans publications
            if (!titresParChercheur.contains(idB))
                continue;

            const QSet<QString> kwB = tokenizeTitles(titresParChercheur.value(idB));
            if (kwB.isEmpty())
                continue;

            // Calculer l'intersection
            int inter = 0;
            for (const QString &s : kwA) {
                if (kwB.contains(s))
                    inter++;
            }
            if (inter == 0)
                continue;

            const double score = jaccard(kwA, kwB);
            if (score <= 0.0)
                continue;

            MatchCandidate mc;
            mc.idChercheur    = idB;
            mc.nomComplet     = QStringLiteral("%1 %2")
                                .arg(qChercheurs.value(2).toString(),
                                     qChercheurs.value(1).toString())
                                .trimmed();
            mc.scoreJaccard   = score;
            mc.nbMotsCommuns  = inter;
            results.append(mc);
        }
    }

    std::sort(results.begin(), results.end(),
              [](const MatchCandidate &x, const MatchCandidate &y) {
                  return x.scoreJaccard > y.scoreJaccard;
              });

    if (results.size() > maxResults)
        results.resize(maxResults);

    return results;
}

#include "resourcerecommender.h"
#include <algorithm>

// Calcule le score de compatibilité (0–100) et remplit les raisons détaillées
int ResourceRecommender::computeCompatibility(const LaboratoryData &lab,
                                        const ResourceNeed &need,
                                        QStringList &raisons)
{
    int score = 0;

    // --- Statut actif (20 pts) ---
    if (lab.statut == "Actif") {
        score += 20;
        raisons << "Laboratoire actif ✅";
    } else if (lab.statut == "En Construction" || lab.statut == "En Rénovation") {
        score += 5;
        raisons << QString("Statut : %1 ⚠️").arg(lab.statut);
    } else {
        raisons << "Laboratoire inactif ❌";
    }

    // --- Thématique (30 pts) ---
    if (!need.thematique.isEmpty()) {
        if (lab.thematique.compare(need.thematique, Qt::CaseInsensitive) == 0) {
            score += 30;
            raisons << "Thématique correspondante ✅";
        } else {
            raisons << QString("Thématique différente (%1) ❌").arg(lab.thematique);
        }
    }

    // --- Capacité (25 pts) ---
    if (need.placesRequises > 0) {
        if (lab.capacite >= need.placesRequises) {
            score += 25;
            raisons << QString("Capacité suffisante (%1 / %2 requis) ✅")
                           .arg(lab.capacite).arg(need.placesRequises);
        } else if (lab.capacite > 0) {
            int partial = (int)(25.0 * lab.capacite / need.placesRequises);
            score += partial;
            raisons << QString("Capacité insuffisante (%1 / %2 requis) ⚠️")
                           .arg(lab.capacite).arg(need.placesRequises);
        } else {
            raisons << "Capacité non renseignée ❌";
        }
    }

    // --- Budget (15 pts) ---
    if (need.budgetNecessaire > 0) {
        if (lab.budget >= need.budgetNecessaire) {
            score += 15;
            raisons << QString("Budget disponible (%1 € ≥ %2 € requis) ✅")
                           .arg(lab.budget, 0, 'f', 0)
                           .arg(need.budgetNecessaire, 0, 'f', 0);
        } else if (lab.budget > 0) {
            int partial = (int)(15.0 * lab.budget / need.budgetNecessaire);
            score += partial;
            raisons << QString("Budget limité (%1 € / %2 € requis) ⚠️")
                           .arg(lab.budget, 0, 'f', 0)
                           .arg(need.budgetNecessaire, 0, 'f', 0);
        } else {
            raisons << "Budget non renseigné ⚠️";
        }
    }

    // --- Équipements (10 pts) ---
    if (!need.equipements.trimmed().isEmpty()) {
        QStringList reqList = need.equipements.split(',', Qt::SkipEmptyParts);
        QStringList labEquip = lab.equipements.split(',', Qt::SkipEmptyParts);

        int matched = 0;
        for (const QString &req : reqList) {
            for (const QString &eq : labEquip) {
                if (eq.trimmed().contains(req.trimmed(), Qt::CaseInsensitive)) {
                    matched++;
                    break;
                }
            }
        }

        if (!reqList.isEmpty()) {
            int pts = (int)(10.0 * matched / reqList.size());
            score += pts;
            if (matched == reqList.size())
                raisons << QString("Tous les équipements disponibles (%1/%1) ✅")
                               .arg(reqList.size());
            else if (matched > 0)
                raisons << QString("Équipements partiellement disponibles (%1/%2) ⚠️")
                               .arg(matched).arg(reqList.size());
            else
                raisons << "Aucun équipement requis disponible ❌";
        }
    }

    return qMin(100, score);
}

QString ResourceRecommender::buildRecommandation(int score, const LaboratoryData &lab)
{
    if (score >= 80)
        return QString("« %1 » est fortement recommandé — excellente compatibilité.").arg(lab.nom);
    else if (score >= 60)
        return QString("« %1 » est une bonne option — quelques ajustements mineurs.").arg(lab.nom);
    else if (score >= 40)
        return QString("« %1 » est envisageable — vérifier les points manquants.").arg(lab.nom);
    else
        return QString("« %1 » est peu adapté — incompatibilités importantes.").arg(lab.nom);
}

QList<RecommendedLab> ResourceRecommender::recommend(const QList<LaboratoryData> &labs,
                                                const ResourceNeed &need)
{
    QList<RecommendedLab> result;
    for (const LaboratoryData &lab : labs) {
        RecommendedLab rl;
        rl.lab   = lab;
        rl.score = computeCompatibility(lab, need, rl.raisons);
        rl.recommandation = buildRecommandation(rl.score, lab);
        result.append(rl);
    }
    std::sort(result.begin(), result.end(), [](const RecommendedLab &a, const RecommendedLab &b) {
        return a.score > b.score;
    });
    return result;
}

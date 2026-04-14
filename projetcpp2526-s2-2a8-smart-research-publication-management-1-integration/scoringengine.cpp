#include "scoringengine.h"
#include <algorithm>
#include <QtMath>

int ScoringEngine::computeScore(const LaboratoryData &lab)
{
    int s = 0;

    // Statut (30 pts)
    if      (lab.statut == "Actif")            s += 30;
    else if (lab.statut == "En Construction")  s += 15;
    else if (lab.statut == "En Rénovation")    s += 10;

    // Budget (25 pts)
    if      (lab.budget >= 500000) s += 25;
    else if (lab.budget >= 200000) s += 18;
    else if (lab.budget >= 50000)  s += 10;
    else if (lab.budget >  0)      s += 5;

    // Capacité (20 pts)
    if      (lab.capacite >= 30) s += 20;
    else if (lab.capacite >= 15) s += 14;
    else if (lab.capacite >= 5)  s += 8;
    else if (lab.capacite >  0)  s += 3;

    // Équipements (15 pts) — 5 pts par équipement, max 15
    if (!lab.equipements.trimmed().isEmpty()) {
        int nb = lab.equipements.split(',', Qt::SkipEmptyParts).size();
        s += qMin(15, nb * 5);
    }

    // Directeur assigné (10 pts)
    if (!lab.directeur.trimmed().isEmpty() &&
        lab.directeur != "-- Sélectionner un directeur --")
        s += 10;

    return qMin(100, s);
}

QString ScoringEngine::badge(int score)
{
    if      (score >= 80) return "🥇 Excellent";
    else if (score >= 60) return "🥈 Bon";
    else if (score >= 40) return "🥉 Moyen";
    else                  return "⚠️ À améliorer";
}

QString ScoringEngine::badgeColor(int score)
{
    if      (score >= 80) return "#059669";
    else if (score >= 60) return "#3b82f6";
    else if (score >= 40) return "#f59e0b";
    else                  return "#ef4444";
}

QString ScoringEngine::recommandation(int score)
{
    if      (score >= 80) return "Labo de référence — maintenir les ressources.";
    else if (score >= 60) return "Bon niveau — renforcer les équipements.";
    else if (score >= 40) return "Potentiel à développer — augmenter la capacité.";
    else                  return "Intervention requise — revoir budget et statut.";
}

QList<ScoredLaboratory> ScoringEngine::score(const QList<LaboratoryData> &labs)
{
    QList<ScoredLaboratory> result;
    for (const LaboratoryData &lab : labs) {
        ScoredLaboratory sl;
        sl.lab           = lab;
        sl.score         = computeScore(lab);
        sl.badge         = badge(sl.score);
        sl.badgeColor    = badgeColor(sl.score);
        sl.recommandation = recommandation(sl.score);
        result.append(sl);
    }
    std::sort(result.begin(), result.end(), [](const ScoredLaboratory &a, const ScoredLaboratory &b) {
        return a.score > b.score;
    });
    return result;
}

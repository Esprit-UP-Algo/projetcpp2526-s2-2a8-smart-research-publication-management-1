#include "collaborationengine.h"

double CollaborationEngine::computeSavings(const LaboratoryData &l1, const LaboratoryData &l2)
{
    return (l1.budget + l2.budget) * 0.15;
}

QList<CollaborationSynergy> CollaborationEngine::detect(const QList<LaboratoryData> &labs)
{
    QList<CollaborationSynergy> result;

    for (int i = 0; i < labs.size(); i++) {
        for (int j = i + 1; j < labs.size(); j++) {
            const LaboratoryData &l1 = labs[i];
            const LaboratoryData &l2 = labs[j];

            // Synergie thématique : même domaine de recherche
            if (l1.thematique == l2.thematique &&
                (l1.statut == "Actif" || l2.statut == "Actif"))
            {
                CollaborationSynergy syn;
                syn.nomLab1        = l1.nom;
                syn.nomLab2        = l2.nom;
                syn.typesynergie   = "🎯 Thématique commune";
                syn.scoreEtoiles   = 5;
                syn.economies      = computeSavings(l1, l2);
                syn.recommandation = "Partage de ressources, projets conjoints";
                result.append(syn);
            }
            // Synergie capacité : un labo saturé + un labo sous-utilisé
            else if (l1.statut == "Actif" && l2.statut == "Actif" &&
                     ((l1.capacite >= 25 && l2.capacite <= 8) ||
                      (l2.capacite >= 25 && l1.capacite <= 8)))
            {
                CollaborationSynergy syn;
                syn.nomLab1        = l1.nom;
                syn.nomLab2        = l2.nom;
                syn.typesynergie   = "⚖️ Rééquilibrage capacité";
                syn.scoreEtoiles   = 3;
                syn.economies      = computeSavings(l1, l2) * 0.5;
                syn.recommandation = "Transfert temporaire de chercheurs";
                result.append(syn);
            }
        }
    }

    return result;
}

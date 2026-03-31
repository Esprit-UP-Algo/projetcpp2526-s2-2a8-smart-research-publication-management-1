#include "ai_service.h"

QVector<AIService::Recommandation> AIService::genererRecommandations(const QVector<Projet> &projets)
{
    QVector<AIService::Recommandation> recommandations;

    bool hasAI = false, hasBio = false, hasQuantum = false, hasEnergy = false;

    for (const auto &p : projets) {
        QString desc = p.description.toLower();
        if (desc.contains("ia") || desc.contains("intelligent") || desc.contains("machine learning")) hasAI = true;
        if (desc.contains("bio") || desc.contains("genome") || desc.contains("medical")) hasBio = true;
        if (desc.contains("quantique")) hasQuantum = true;
        if (desc.contains("energie") || desc.contains("solaire")) hasEnergy = true;
    }

    if (hasAI) {
        Recommandation rec;
        rec.titre = "Deep Learning pour la Santé Prédictive";
        rec.domaine = "IA × Biotechnologie";
        rec.scoreSimilarite = 92.0;
        rec.description = "Extension naturelle de votre expertise en IA vers le domaine médical. "
                          "Ce projet vise à développer des modèles de deep learning pour la prédiction "
                          "précoce des maladies chroniques basés sur l'analyse génomique.";
        rec.raison = "Synergie forte avec Smart-Traffic (IA) et Analyse Génome (Biologie). "
                     "Fort potentiel d'innovation et de publications scientifiques.";
        rec.collaborateursSuggeres = {
            "Dr. Ahmed Ben Ali - Expertise IA (Score: 95%)",
            "Pr. Fatima Zohra - Génomique (Score: 88%)",
            "Dr. Sarah Johnson - Analyse de données (Score: 82%)"
        };
        recommandations.append(rec);
    }

    if (hasQuantum || hasAI) {
        Recommandation rec;
        rec.titre = "Calculateur Quantique pour la Bioinformatique";
        rec.domaine = "Quantique × Biologie";
        rec.scoreSimilarite = 87.0;
        rec.description = "Fusion de trois domaines d'excellence : informatique quantique, "
                          "intelligence artificielle et biologie. Utilisation d'algorithmes quantiques "
                          "pour accélérer l'analyse des séquences génomiques.";
        rec.raison = "Combinaison unique de vos forces en quantique et biologie. "
                     "Projet hautement innovant avec fort potentiel de financement européen.";
        rec.collaborateursSuggeres = {
            "Dr. Mohamed Salah - Informatique Quantique (Score: 96%)",
            "Dr. Ahmed Ben Ali - IA & Algorithmes (Score: 91%)",
            "Pr. Fatima Zohra - Bioinformatique (Score: 89%)"
        };
        recommandations.append(rec);
    }

    if (hasEnergy || hasAI) {
        Recommandation rec;
        rec.titre = "Smart Grid IA pour Villes Durables";
        rec.domaine = "Énergie × IA";
        rec.scoreSimilarite = 84.0;
        rec.description = "Extension de Smart-Traffic vers la gestion énergétique urbaine. "
                          "Développement d'un réseau électrique intelligent optimisé par l'IA "
                          "pour réduire la consommation énergétique des villes.";
        rec.raison = "Continuité logique de Smart-Traffic vers la smart city. "
                     "Répond aux enjeux actuels de transition énergétique.";
        rec.collaborateursSuggeres = {
            "Dr. Sarah Johnson - Énergies Renouvelables (Score: 94%)",
            "Dr. Ahmed Ben Ali - IA/Smart City (Score: 90%)",
            "Pr. Robert Chen - Optimisation systèmes (Score: 85%)"
        };
        recommandations.append(rec);
    }

    return recommandations;
}

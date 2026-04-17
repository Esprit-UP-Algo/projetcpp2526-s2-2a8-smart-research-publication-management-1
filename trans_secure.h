#ifndef TRANS_SECURE_H
#define TRANS_SECURE_H

#include <QString>
#include <QWidget>
#include <QSqlDatabase>

// ============================================================================
// TransSecure — Journal de sécurité des transactions financières
// Toutes les opérations (ajout, modification, suppression) sont tracées
// automatiquement dans un fichier texte horodaté.
// Le fichier est créé automatiquement dès la première ouverture du journal.
// ============================================================================

class TransSecure {
public:
    // Enregistre une opération dans le journal
    // action : "AJOUT" | "MODIFICATION" | "SUPPRESSION"
    static void logTransaction(const QString &action,
                               int            id,
                               const QString &type,
                               double         montant,
                               const QString &date,
                               const QString &categorie,
                               const QString &statut,
                               const QString &projet,
                               const QString &description = QString());

    // Ouvre une fenêtre affichant le contenu du journal
    static void afficherJournal(QWidget *parent = nullptr);

    // Retourne le chemin absolu du fichier journal
    static QString cheminFichier();

    // Crée le fichier journal s'il n'existe pas encore (avec en-tête)
    static void initialiserFichier();
};

#endif // TRANS_SECURE_H

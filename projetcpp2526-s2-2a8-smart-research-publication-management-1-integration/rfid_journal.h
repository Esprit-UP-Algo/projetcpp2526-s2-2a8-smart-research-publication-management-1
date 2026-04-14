#ifndef RFID_JOURNAL_H
#define RFID_JOURNAL_H

#include <QString>
#include <QWidget>

// ============================================================================
// RfidJournal — Journal de sécurité des passages RFID (scénario 3)
// Chaque scan de carte est enregistré dans un fichier texte horodaté
// ============================================================================

class RfidJournal {
public:
    static void logPassage(const QString &cin,
                           const QString &resultat,
                           const QString &nom = QString());

    static void afficherJournal(QWidget *parent = nullptr);

    static QString cheminFichier();
};

#endif // RFID_JOURNAL_H

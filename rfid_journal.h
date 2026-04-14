#ifndef RFID_JOURNAL_H
#define RFID_JOURNAL_H

#include <QString>
#include <QWidget>

// ============================================================================
// RfidJournal — Journal de sécurité des passages RFID (Scénario 3)
// Chaque scan de carte est enregistré dans un fichier texte horodaté.
// ============================================================================

class RfidJournal {
public:
    // Enregistre un passage dans le journal
    // cin     : UID de la carte scannée
    // resultat: "TROUVE" ou "INCONNU"
    // nom     : nom du chercheur si trouvé, vide sinon
    static void logPassage(const QString &cin,
                           const QString &resultat,
                           const QString &nom = QString());

    // Ouvre une fenêtre Qt affichant le journal complet
    static void afficherJournal(QWidget *parent = nullptr);

    // Chemin du fichier journal (Documents/rfid_passages.txt)
    static QString cheminFichier();
};

#endif // RFID_JOURNAL_H

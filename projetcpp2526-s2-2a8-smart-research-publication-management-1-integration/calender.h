#ifndef CALENDER_H
#define CALENDER_H

#include <QDialog>
#include <QDate>
#include <QList>
#include <QString>
#include <QWidget>
#include "evenement.h"

// ============================================================================
// CalendarDialog — Calendrier visuel des événements (style Google Calendar)
// Charge les événements depuis la BD à chaque ouverture.
// ============================================================================

class CalendarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalendarDialog(QWidget *parent = nullptr);

private slots:
    void moisPrecedent();
    void moisSuivant();
    void anneeChangee(int annee);

private:
    void chargerEvenements();
    void construireCalendrier();
    void mettreAJourTitre();

    QDate          m_dateActuelle;   // mois/année affiché
    QList<EventData> m_evenements;   // liste chargée depuis la BD

    // Widgets persistants mis à jour à chaque navigation
    class QLabel    *m_labelMois;
    class QGridLayout *m_gridJours;
    class QWidget   *m_gridWidget;
    class QSpinBox  *m_spinAnnee;
};

#endif // CALENDER_H

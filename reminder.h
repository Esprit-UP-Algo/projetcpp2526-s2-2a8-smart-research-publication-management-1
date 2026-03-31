#ifndef REMINDER_H
#define REMINDER_H

#include <QVector>
#include <QDate>

#include "projet.h"

class Reminder {
public:
    static QVector<Projet> projectsEndingSoon(const QVector<Projet> &projets,
                                              int thresholdDays = 7,
                                              const QDate &today = QDate::currentDate());

    static void notifyEndingSoon(const QVector<Projet> &projets,
                                 int thresholdDays = 7,
                                 const QDate &today = QDate::currentDate());
};

#endif // REMINDER_H

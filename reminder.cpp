#include "reminder.h"

#include <QDebug>

QVector<Projet> Reminder::projectsEndingSoon(const QVector<Projet> &projets,
                                            int thresholdDays,
                                            const QDate &today)
{
    QVector<Projet> endingSoon;
    endingSoon.reserve(projets.size());

    for (const auto &p : projets) {
        const int daysRemaining = today.daysTo(p.dateFin);
        if (daysRemaining >= 0 && daysRemaining <= thresholdDays) {
            endingSoon.append(p);
        }
    }

    return endingSoon;
}

void Reminder::notifyEndingSoon(const QVector<Projet> &projets,
                               int thresholdDays,
                               const QDate &today)
{
    const QVector<Projet> endingSoon = projectsEndingSoon(projets, thresholdDays, today);
    for (const auto &p : endingSoon) {
        qDebug().noquote()
            << QStringLiteral("[REMINDER] Projet '%1' (code=%2) se termine le %3 (dans %4 jours)")
                   .arg(p.titre, p.code, p.dateFin.toString("dd/MM/yyyy"))
                   .arg(today.daysTo(p.dateFin));
    }
}

#ifndef REMINDER_H
#define REMINDER_H

#include <QVector>
#include <QDate>
#include <QSet>
#include <QString>

#include "projet.h"

// ============================================================================
// Reminder — Système de rappel d'échéances de projets
//
// Comportement :
//   - Détecte les projets dont la date de fin est dans [0, thresholdDays] jours
//   - Envoie un email au responsable (via QNetworkAccessManager / Gmail API)
//   - Anti-spam : un seul envoi par projet par jour (clé = ID:date)
// ============================================================================

class Reminder {
public:
    // Retourne les projets dont la date de fin est dans [0, thresholdDays] jours
    static QVector<Projet> projectsEndingSoon(const QVector<Projet> &projets,
                                              int thresholdDays = 10,
                                              const QDate &today = QDate::currentDate());

    // Vérifie les échéances et envoie les emails de rappel si nécessaire.
    // Appeler cette méthode au démarrage et/ou via un QTimer périodique.
    static void notifyEndingSoon(const QVector<Projet> &projets,
                                 int thresholdDays = 10,
                                 const QDate &today = QDate::currentDate());

    // Réinitialise le cache anti-spam (utile pour les tests)
    static void resetNotifiedCache();

private:
    // Cache anti-spam : clé = "ID_PROJET:YYYY-MM-DD"
    static QSet<QString> s_notifiedKeys;

    // Envoie l'email de rappel via QNetworkAccessManager (Gmail SMTP REST)
    static void sendReminderEmail(const QString &toEmail,
                                  const QString &projetTitre,
                                  const QString &projetCode,
                                  int daysRemaining,
                                  const QDate &dateFin);
};

#endif // REMINDER_H

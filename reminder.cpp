// ============================================================================
// Reminder — Rappels sur les échéances de projets
//
// Déclenchement : appelé depuis projChargerProjets()
// Notification  : envoi email silencieux via curl + Gmail SMTP
// Anti-spam     : 1 email par projet par jour
// ============================================================================

#include "reminder.h"
#include "connection.h"
#include "osnotification.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>

// ── Membre statique anti-spam ─────────────────────────────────────────────
QSet<QString> Reminder::s_notifiedKeys;

static QString makeNotifKey(int projetId, const QDate &date)
{
    return QString::number(projetId) + QLatin1Char(':') + date.toString(Qt::ISODate);
}

// ─────────────────────────────────────────────────────────────────────────────
// projectsEndingSoon
// ─────────────────────────────────────────────────────────────────────────────
QVector<Projet> Reminder::projectsEndingSoon(const QVector<Projet> &projets,
                                             int thresholdDays,
                                             const QDate &today)
{
    QVector<Projet> endingSoon;
    for (const auto &p : projets) {
        const int days = today.daysTo(p.dateFin);
        if (days >= 0 && days <= thresholdDays)
            endingSoon.append(p);
    }
    return endingSoon;
}

// ─────────────────────────────────────────────────────────────────────────────
// sendReminderEmail
// Envoi silencieux via curl (natif Windows 10+) + Gmail SMTP SSL port 465
// ─────────────────────────────────────────────────────────────────────────────
void Reminder::sendReminderEmail(const QString &toEmail,
                                 const QString &projetTitre,
                                 const QString &projetCode,
                                 int daysRemaining,
                                 const QDate &dateFin)
{
    if (toEmail.isEmpty()) {
        qWarning() << "[REMINDER] Email vide — envoi annulé pour" << projetCode;
        return;
    }

    // ── Credentials Gmail ─────────────────────────────────────────────────────
    // Remplacez par votre adresse Gmail et mot de passe d'application
    // (Compte Google → Sécurité → Mots de passe des applications)
    const QString SENDER_EMAIL = QStringLiteral("smartpub.projet@gmail.com");
    const QString APP_PASSWORD = QStringLiteral("egwiardkigqgyoqj");

    const QString subject = QString("Rappel : Projet '%1' expire dans %2 jour(s)")
                                .arg(projetTitre).arg(daysRemaining);

    const QString mimeMsg = QString(
        "From: SmartPub <%1>\r\n"
        "To: %2\r\n"
        "Subject: %3\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n"
        "Bonjour,\r\n\r\n"
        "Rappel automatique SmartPub :\r\n\r\n"
        "  Projet   : %4\r\n"
        "  Code     : %5\r\n"
        "  Echeance : %6\r\n"
        "  Restant  : %7 jour(s)\r\n\r\n"
        "Veuillez prendre les mesures necessaires avant l'expiration.\r\n\r\n"
        "SmartPub - Gestion de Recherche Scientifique\r\n"
    ).arg(SENDER_EMAIL, toEmail, subject,
          projetTitre, projetCode,
          dateFin.toString("dd/MM/yyyy"),
          QString::number(daysRemaining));

    QStringList args;
    args << QStringLiteral("--ssl-reqd")
         << QStringLiteral("--url")       << QStringLiteral("smtps://smtp.gmail.com:465")
         << QStringLiteral("--user")      << QString("%1:%2").arg(SENDER_EMAIL, APP_PASSWORD)
         << QStringLiteral("--mail-from") << SENDER_EMAIL
         << QStringLiteral("--mail-rcpt") << toEmail
         << QStringLiteral("--upload-file") << QStringLiteral("-")
         << QStringLiteral("--verbose");  // verbose pour voir les erreurs dans Qt Output

    QProcess *proc = new QProcess();
    proc->start(QStringLiteral("curl"), args);

    if (!proc->waitForStarted(3000)) {
        qWarning() << "[REMINDER] curl introuvable — email non envoyé pour" << projetCode;
        proc->deleteLater();
        return;
    }

    proc->write(mimeMsg.toUtf8());
    proc->closeWriteChannel();

    QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [proc, toEmail, projetCode](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            qDebug().noquote()
                << QStringLiteral("[REMINDER] Email envoye a '%1' pour le projet '%2'")
                       .arg(toEmail, projetCode);
        } else {
            qWarning().noquote()
                << QStringLiteral("[REMINDER] Echec envoi email a '%1' (exit=%2): %3")
                       .arg(toEmail, QString::number(exitCode),
                            QString::fromUtf8(proc->readAllStandardError()));
        }
        proc->deleteLater();
    });
}

// ─────────────────────────────────────────────────────────────────────────────
// notifyEndingSoon
// ─────────────────────────────────────────────────────────────────────────────
void Reminder::notifyEndingSoon(const QVector<Projet> &projets,
                                int thresholdDays,
                                const QDate &today)
{
    const QVector<Projet> endingSoon = projectsEndingSoon(projets, thresholdDays, today);

    if (endingSoon.isEmpty()) {
        qDebug() << "[REMINDER] Aucun projet proche de l'echeance.";
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();

    for (const auto &p : endingSoon) {
        const int daysRemaining = today.daysTo(p.dateFin);
        const QString notifKey  = makeNotifKey(p.id, today);

        // Anti-spam : un seul envoi par projet par jour
        // (désactivé temporairement pour les tests — réactiver en production)
        // if (s_notifiedKeys.contains(notifKey)) {
        //     qDebug().noquote()
        //         << QStringLiteral("[REMINDER] Deja notifie aujourd'hui — projet '%1'").arg(p.titre);
        //     continue;
        // }

        // Récupérer l'email du responsable depuis CHERCHEUR
        QString responsableEmail;
        if (db.isOpen() && p.responsableId > 0) {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("SELECT EMAIL FROM CHERCHEUR WHERE ID_CHERCHEUR = :id"));
            q.bindValue(QStringLiteral(":id"), p.responsableId);
            if (q.exec() && q.next())
                responsableEmail = q.value(0).toString().trimmed().toLower();
            else
                qWarning() << "[REMINDER] Email introuvable pour ID_CHERCHEUR=" << p.responsableId;
        }

        qDebug().noquote()
            << QStringLiteral("[REMINDER] Projet '%1' (%2) — %3j restants — email: %4")
                   .arg(p.titre, p.code, QString::number(daysRemaining),
                        responsableEmail.isEmpty() ? QStringLiteral("inconnu") : responsableEmail);

        // Notification OS (System Tray)
        OsNotification::instance()->alertEcheance(p.titre, daysRemaining);

        // Email au responsable
        sendReminderEmail(responsableEmail, p.titre, p.code, daysRemaining, p.dateFin);

        s_notifiedKeys.insert(notifKey);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void Reminder::resetNotifiedCache()
{
    s_notifiedKeys.clear();
}

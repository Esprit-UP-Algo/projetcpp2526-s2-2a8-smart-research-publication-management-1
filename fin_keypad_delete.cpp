// =========================================================================
// fin_keypad_delete.cpp
// =========================================================================
// Suppression sécurisée de transactions via clavier HX-543 + Arduino.
// Le code OTP est envoyé par email au DG via curl + Gmail SMTP —
// exactement le même système que Reminder::sendReminderEmail().
// =========================================================================

#include "fin_keypad_delete.h"
#include "arduino.h"
#include "smartpub.h"
#include "connection.h"
#include "trans_secure.h"

#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// ── Credentials Gmail (mêmes que Reminder::sendReminderEmail) ────────────────
static const QString SENDER_EMAIL = QStringLiteral("smartpub.projet@gmail.com");
static const QString APP_PASSWORD = QStringLiteral("egwiardkigqgyoqj");

// =========================================================================
// Construction
// =========================================================================

FinKepadDelete::FinKepadDelete(Arduino *arduino, SmartPub *parent)
    : QObject(parent)
    , m_arduino(arduino)
    , m_parent(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &FinKepadDelete::onTimeoutCode);
}

// =========================================================================
// Démarrer la procédure de suppression sécurisée
// dgEmail : adresse email du Directeur Général
// =========================================================================

void FinKepadDelete::demanderSuppression(int transactionId, const QString &dgEmail)
{
    if (m_enCours) {
        qWarning() << "[KeypadDelete] Une procédure est déjà en cours.";
        return;
    }

    m_transId    = transactionId;
    m_dgEmail    = dgEmail;
    m_tentatives = 0;
    m_enCours    = true;
    m_affichage  = "";
    m_code       = genererCode();

    qDebug() << "[KeypadDelete] *** CODE OTP GENERE ***" << m_code
             << "pour transaction" << m_transId;

    // Afficher le code dans une boite de dialogue (pour tester)
    // ET envoyer l'email en parallèle
    QMessageBox *info = new QMessageBox(m_parent);
    info->setWindowTitle("Suppression sécurisée");
    info->setText(QString(
        "📧 Un code de confirmation a été envoyé par email à %1\n\n"
        "Le Directeur Général doit entrer ce code sur le clavier Arduino.\n"
        "Validité : 2 minutes.")
        .arg(dgEmail));
    info->setIcon(QMessageBox::Information);
    info->setStandardButtons(QMessageBox::Ok);
    info->exec();

    // 1. Envoyer le code par email
    envoyerEmailDG(m_dgEmail, m_code, m_transId);

    // 2. Demander à l'Arduino d'activer la saisie clavier
    QByteArray cmd = QByteArray("WAIT_CODE:") + QByteArray::number(m_transId) + "\n";
    envoyerArduino(cmd);

    // 3. Lancer le minuteur (2 min)
    m_timer->start(TIMEOUT_SAISIE_MS);

    emit statutMessage(
        QString("📧 Code OTP envoyé au DG — transaction #%1 en attente de validation clavier")
        .arg(m_transId));
}

// =========================================================================
// Annulation depuis Qt
// =========================================================================

void FinKepadDelete::annuler()
{
    if (!m_enCours) return;
    envoyerArduino("CODE_CANCEL\n");
    emit suppressionAnnulee(m_transId);
    emit statutMessage(
        QString("Suppression de la transaction #%1 annulée.").arg(m_transId));
    reinitialiser();
}

// =========================================================================
// Traitement ligne série reçue de l'Arduino
// =========================================================================

bool FinKepadDelete::traiterLigneSerie(const QString &ligne)
{
    if (!m_enCours) return false;

    // ── Affichage étoile pour chaque chiffre tapé ────────────────────────
    if (ligne == "KEY:*") {
        m_affichage += "*";
        emit statutMessage(
            QString("🔐 Code en cours de saisie : %1").arg(m_affichage));
        return true;
    }

    // ── Code saisi par le DG ─────────────────────────────────────────────
    if (ligne.startsWith("CODE:")) {
        const QString codeSaisi = ligne.mid(5).trimmed();
        m_tentatives++;

        qDebug() << "[KeypadDelete] Code reçu :" << codeSaisi
                 << "— tentative" << m_tentatives << "/" << MAX_TENTATIVES;

        if (codeSaisi == m_code) {
            // ✅ Correct
            m_timer->stop();
            envoyerArduino("CODE_OK\n");
            supprimerEnBD();
            emit suppressionReussie(m_transId);
            emit statutMessage(
                QString("✅ Transaction #%1 supprimée avec succès.").arg(m_transId));
            reinitialiser();

        } else if (m_tentatives >= MAX_TENTATIVES) {
            // ❌ Trop de tentatives
            m_timer->stop();
            envoyerArduino("CODE_FAIL\n");
            QMessageBox::warning(m_parent, "Code incorrect",
                QString("Code refusé après %1 tentatives.\n"
                        "Suppression de la transaction #%2 annulée.")
                .arg(MAX_TENTATIVES).arg(m_transId));
            emit suppressionRefusee(m_transId);
            emit statutMessage(
                QString("❌ Transaction #%1 — suppression refusée (trop de tentatives).")
                .arg(m_transId));
            reinitialiser();

        } else {
            // ❌ Incorrect, réessayer
            m_affichage = "";   // reset affichage pour nouvelle tentative
            envoyerArduino("CODE_FAIL\n");
            emit statutMessage(
                QString("⚠ Code incorrect — tentative %1/%2 pour transaction #%3")
                .arg(m_tentatives).arg(MAX_TENTATIVES).arg(m_transId));
        }
        return true;
    }

    // ── DG a appuyé sur * (annulation physique) ──────────────────────────
    if (ligne == "CANCEL") {
        m_timer->stop();
        QMessageBox::information(m_parent, "Annulation",
            QString("Le Directeur Général a annulé la suppression\n"
                    "de la transaction #%1 depuis le clavier.").arg(m_transId));
        emit suppressionAnnulee(m_transId);
        emit statutMessage(
            QString("Suppression de la transaction #%1 annulée par le DG (clavier).")
            .arg(m_transId));
        reinitialiser();
        return true;
    }

    return false;
}

// =========================================================================
// Timeout
// =========================================================================

void FinKepadDelete::onTimeoutCode()
{
    if (!m_enCours) return;
    qWarning() << "[KeypadDelete] Timeout — code non saisi dans les 2 minutes.";
    envoyerArduino("CODE_CANCEL\n");
    QMessageBox::warning(m_parent, "Délai expiré",
        QString("Le code n'a pas été saisi dans les 2 minutes.\n"
                "Suppression de la transaction #%1 annulée.").arg(m_transId));
    emit suppressionAnnulee(m_transId);
    emit statutMessage(
        QString("⏱ Transaction #%1 — suppression annulée (délai expiré).")
        .arg(m_transId));
    reinitialiser();
}

// =========================================================================
// Génération OTP 6 chiffres
// =========================================================================

QString FinKepadDelete::genererCode() const
{
    quint32 val = QRandomGenerator::securelySeeded().bounded(100000u, 1000000u);
    return QString::number(val);
}

// =========================================================================
// Envoi email via curl + Gmail SMTP — exactement la même méthode que
// Reminder::sendReminderEmail() qui est fonctionnelle.
// =========================================================================

void FinKepadDelete::envoyerEmailDG(const QString &toEmail,
                                     const QString &code,
                                     int            transId)
{
    if (toEmail.isEmpty()) {
        qWarning() << "[KeypadDelete] Email DG vide — envoi annulé.";
        return;
    }

    const QString subject = QString("SmartPub - Code de suppression transaction #%1")
                                .arg(transId);

    const QString mimeMsg = QString(
        "From: SmartPub <%1>\r\n"
        "To: %2\r\n"
        "Subject: %3\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "\r\n"
        "Bonjour Directeur General,\r\n\r\n"
        "Une demande de suppression de transaction a ete initiee.\r\n\r\n"
        "  Transaction ID : #%4\r\n"
        "  Code secret    : %5\r\n\r\n"
        "Entrez ce code sur le clavier du systeme SmartPub.\r\n"
        "Ce code est valable 2 minutes.\r\n\r\n"
        "Si vous n'etes pas a l'origine de cette demande, ignorez ce message.\r\n\r\n"
        "SmartPub - Systeme de Gestion de Recherche Scientifique\r\n"
    ).arg(SENDER_EMAIL, toEmail, subject,
          QString::number(transId), code);

    QStringList args;
    args << QStringLiteral("--ssl-reqd")
         << QStringLiteral("--ssl-no-revoke")   // fix schannel Windows CRYPT_E_NO_REVOCATION_CHECK
         << QStringLiteral("--url")       << QStringLiteral("smtps://smtp.gmail.com:465")
         << QStringLiteral("--user")      << QString("%1:%2").arg(SENDER_EMAIL, APP_PASSWORD)
         << QStringLiteral("--mail-from") << SENDER_EMAIL
         << QStringLiteral("--mail-rcpt") << toEmail
         << QStringLiteral("--upload-file") << QStringLiteral("-")
         << QStringLiteral("--verbose");  // verbose pour voir les erreurs dans Qt Output

    QProcess *proc = new QProcess();
    proc->start(QStringLiteral("curl"), args);

    if (!proc->waitForStarted(3000)) {
        qWarning() << "[KeypadDelete] curl introuvable — email non envoyé.";
        proc->deleteLater();
        return;
    }

    proc->write(mimeMsg.toUtf8());
    proc->closeWriteChannel();

    QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [proc, toEmail, code](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            qDebug() << "[KeypadDelete] ✅ Email OTP envoyé à" << toEmail;
        } else {
            qWarning() << "[KeypadDelete] ❌ Échec email. Code OTP console :" << code;
            qWarning().noquote() << QString::fromUtf8(proc->readAllStandardError());
        }
        proc->deleteLater();
    });

    qDebug() << "[KeypadDelete] Envoi email OTP →" << toEmail << "| Code :" << code;
}

// =========================================================================
// Envoi commande Arduino
// =========================================================================

void FinKepadDelete::envoyerArduino(const QByteArray &cmd)
{
    if (m_arduino && m_arduino->getserial() && m_arduino->getserial()->isOpen()) {
        m_arduino->write_to_arduino(cmd);
        qDebug() << "[KeypadDelete] → Arduino :" << cmd.trimmed();
    } else {
        qWarning() << "[KeypadDelete] Arduino non connecté :" << cmd.trimmed();
    }
}

// =========================================================================
// Suppression en BD + log sécurité
// =========================================================================

void FinKepadDelete::supprimerEnBD()
{
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        qWarning() << "[KeypadDelete] BD non disponible.";
        return;
    }

    QSqlQuery selectQ(db);
    selectQ.prepare("SELECT MONTANT, TYPE_TRANS, DATE_TRANSACTION, CATEGORIE, "
                    "STATUT, DESCRIPTION FROM FINANCE WHERE ID_TRANSACTION = :id");
    selectQ.bindValue(":id", m_transId);

    double  montant = 0.0;
    QString type, date, cat, statut, desc;
    if (selectQ.exec() && selectQ.next()) {
        montant = selectQ.value(0).toDouble();
        type    = selectQ.value(1).toString();
        date    = selectQ.value(2).toDate().toString("dd/MM/yyyy");
        cat     = selectQ.value(3).toString();
        statut  = selectQ.value(4).toString();
        desc    = selectQ.value(5).toString();
    }

    QSqlQuery delQ(db);
    delQ.prepare("DELETE FROM FINANCE WHERE ID_TRANSACTION = :id");
    delQ.bindValue(":id", m_transId);
    if (!delQ.exec()) {
        qWarning() << "[KeypadDelete] Échec DELETE :" << delQ.lastError().text();
        return;
    }

    TransSecure::logTransaction(
        "SUPPRESSION (clavier DG)",
        m_transId, type, montant, date, cat, statut, "—", desc);

    qDebug() << "[KeypadDelete] Transaction" << m_transId << "supprimée en BD.";
}

// =========================================================================
// Réinitialisation
// =========================================================================

void FinKepadDelete::reinitialiser()
{
    m_enCours    = false;
    m_transId    = 0;
    m_code       = "";
    m_dgEmail    = "";
    m_tentatives = 0;
    m_affichage  = "";   // reset des étoiles
    m_timer->stop();
}
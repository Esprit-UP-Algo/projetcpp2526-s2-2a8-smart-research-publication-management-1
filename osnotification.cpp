// ============================================================================
// OsNotification — Notifications système OS (System Tray / Toast)
// ============================================================================

#include "osnotification.h"
#include <QPainter>
#include <QFont>
#include <QDebug>

// ── Singleton ──────────────────────────────────────────────────────────────
OsNotification *OsNotification::s_instance = nullptr;

OsNotification *OsNotification::instance()
{
    if (!s_instance)
        s_instance = new OsNotification(qApp);
    return s_instance;
}

OsNotification::OsNotification(QObject *parent)
    : QObject(parent)
{}

// ─────────────────────────────────────────────────────────────────────────────
// initialize
// ─────────────────────────────────────────────────────────────────────────────
void OsNotification::initialize(const QIcon &appIcon)
{
    if (m_initialized) return;

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "[NOTIF] System Tray non disponible sur cette plateforme.";
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);

    // ── Icône ───────────────────────────────────────────────────────────────
    QIcon icon = appIcon;
    if (icon.isNull()) {
        // Icône de secours : carré bleu SmartPub
        QPixmap pix(32, 32);
        pix.fill(Qt::transparent);
        QPainter painter(&pix);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor("#3b82f6"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(0, 0, 32, 32, 6, 6);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, "S");
        painter.end();
        icon = QIcon(pix);
    }

    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip("SmartPub — Système de Gestion de Recherche");

    // ── Menu contextuel ─────────────────────────────────────────────────────
    QMenu *menu = new QMenu();
    QAction *titleAction = menu->addAction("SmartPub");
    titleAction->setEnabled(false);
    menu->addSeparator();
    menu->addAction("Quitter", qApp, &QApplication::quit);
    m_trayIcon->setContextMenu(menu);

    m_trayIcon->show();

    // ── Signal clic sur la notification ────────────────────────────────────
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger ||
            reason == QSystemTrayIcon::DoubleClick)
            emit notificationActivated();
    });

    m_initialized = true;
    qDebug() << "[NOTIF] Notifications OS initialisees (System Tray actif).";
}

// ─────────────────────────────────────────────────────────────────────────────
// show — notification générique
// ─────────────────────────────────────────────────────────────────────────────
void OsNotification::show(const QString &title,
                           const QString &message,
                           QSystemTrayIcon::MessageIcon iconType,
                           int durationMs)
{
    qDebug().noquote()
        << QStringLiteral("[NOTIF OS] %1 | %2").arg(title, message);

    if (!m_initialized || !m_trayIcon) {
        // System Tray non disponible : fallback QMessageBox informatif
        qWarning() << "[NOTIF] Tray non initialise — notification perdue.";
        return;
    }

    m_trayIcon->showMessage(title, message, iconType, durationMs);
}

// ─────────────────────────────────────────────────────────────────────────────
// alertBudget — dépassement de budget projet
// ─────────────────────────────────────────────────────────────────────────────
void OsNotification::alertBudget(const QString &projetNom,
                                  double montantDepense,
                                  double seuil)
{
    const QString title = QString::fromUtf8("\u26a0 D\u00e9passement de budget");
    const QString msg   = QString(
        "Projet : %1\n"
        "D\u00e9penses totales : %2 TND\n"
        "Seuil critique    : %3 TND\n"
        "D\u00e9passement       : +%4 TND"
    ).arg(projetNom,
          QString::number(montantDepense, 'f', 2),
          QString::number(seuil,          'f', 2),
          QString::number(montantDepense - seuil, 'f', 2));

    show(title, msg, QSystemTrayIcon::Warning, 9000);
}

// ─────────────────────────────────────────────────────────────────────────────
// alertEcheance — projet proche de sa date de fin
// ─────────────────────────────────────────────────────────────────────────────
void OsNotification::alertEcheance(const QString &projetNom, int joursRestants)
{
    const bool critique = joursRestants <= 3;

    const QString title = critique
        ? QString::fromUtf8("\U0001f534 \u00c9ch\u00e9ance critique")
        : QString::fromUtf8("\U0001f7e1 Rappel d\u2019\u00e9ch\u00e9ance");

    const QString msg = QString(
        "Projet : %1\n"
        "Expire dans %2 jour(s)\n"
        "%3"
    ).arg(projetNom,
          QString::number(joursRestants),
          critique ? QString::fromUtf8("Action imm\u00e9diate requise !")
                   : QString::fromUtf8("Pensez \u00e0 mettre \u00e0 jour l\u2019avancement."));

    const QSystemTrayIcon::MessageIcon icon =
        critique ? QSystemTrayIcon::Critical : QSystemTrayIcon::Warning;

    show(title, msg, icon, 8000);
}

// ─────────────────────────────────────────────────────────────────────────────
// isSupported
// ─────────────────────────────────────────────────────────────────────────────
bool OsNotification::isSupported() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

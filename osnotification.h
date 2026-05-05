#ifndef OSNOTIFICATION_H
#define OSNOTIFICATION_H

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QPixmap>
#include <QApplication>

// ============================================================================
// OsNotification — Notifications système (System Tray / Toast Windows 10/11)
//
// Utilise QSystemTrayIcon::showMessage() pour afficher des notifications
// natives OS (ballons Windows 7/8, Toast Windows 10+).
// Usage :
//   OsNotification::instance()->initialize(appIcon);   // au démarrage
//   OsNotification::instance()->show("Titre", "Corps");
//   OsNotification::instance()->alertBudget("Projet X", 15000, 10000);
//   OsNotification::instance()->alertEcheance("Projet Y", 3);
// ============================================================================

class OsNotification : public QObject {
    Q_OBJECT
public:
    // Singleton — une seule instance partagée dans toute l'application
    static OsNotification *instance();

    // Initialiser (appeler UNE FOIS au démarrage, avant tout show())
    void initialize(const QIcon &appIcon = QIcon());

    // Notification générique
    void show(const QString &title,
              const QString &message,
              QSystemTrayIcon::MessageIcon iconType = QSystemTrayIcon::Information,
              int durationMs = 6000);

    // Alerte dépassement de budget (icône Warning)
    void alertBudget(const QString &projetNom,
                     double montantDepense,
                     double seuil);

    // Alerte approche d'échéance (Warning ou Critical selon jours restants)
    void alertEcheance(const QString &projetNom, int joursRestants);

    // Vrai si le system tray est disponible sur cette plateforme
    bool isSupported() const;

signals:
    void notificationActivated(); // émis quand l'utilisateur clique la notif

private:
    explicit OsNotification(QObject *parent = nullptr);
    ~OsNotification() override = default;

    static OsNotification *s_instance;
    QSystemTrayIcon *m_trayIcon = nullptr;
    bool             m_initialized = false;
};

#endif // OSNOTIFICATION_H

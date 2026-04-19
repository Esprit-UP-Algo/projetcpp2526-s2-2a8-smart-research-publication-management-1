#ifndef PUBLICATIONAUTH_H
#define PUBLICATIONAUTH_H

#include <QString>
#include <QList>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

// ============================================================================
// SYSTÈME D'AUTHENTIFICATION CENTRALISÉ — SmartPub
// ============================================================================
// Ce fichier est l'unique source de vérité pour tous les comptes utilisateurs.
// Il ne dépend d'aucun widget Qt — pur C++/Qt Core.
//
// Index modules (stackedWidgetModules) :
//   0 = Chercheurs | 1 = Publications | 2 = Finances
//   3 = Événements | 4 = Projets      | 5 = Laboratoires
//  -1 = Tous les modules (Directeur Général)
// ============================================================================

struct AppUserAccount {
    QString email;
    QString password;
    QString displayName;
    QString allowedModule; // "ALL" ou nom du module (ex: "Chercheurs")
    int     moduleIndex;   // -1 = tous les modules
};

class AppAuthService {
public:
    AppAuthService();

    // Tente l'authentification.
    // Retourne true et remplit outUser si les identifiants sont corrects.
    // En cas d'échec, errorMessage (si non-null) décrit la raison.
    bool authenticate(const QString &email,
                      const QString &password,
                      AppUserAccount *outUser,
                      QString *errorMessage = nullptr) const;

    // Accès à la liste complète des comptes (lecture seule)
    const QList<AppUserAccount> &accounts() const { return m_accounts; }

    // Vérifie si l'email existe dans les comptes
    bool emailExists(const QString &email) const;

    // Enregistre un nouveau mot de passe (hashé SHA-256) dans le fichier JSON
    bool updatePassword(const QString &email, const QString &newPassword);

    // Hache un mot de passe en SHA-256
    static QString hashPassword(const QString &password);

private:
    void setupAccounts();
    QString passwordFilePath() const;
    QString getOverridePassword(const QString &email) const;

    QList<AppUserAccount> m_accounts;
};

#endif // PUBLICATIONAUTH_H

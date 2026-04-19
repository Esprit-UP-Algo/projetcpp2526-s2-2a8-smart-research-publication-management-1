#include "publicationauth.h"

// ============================================================================
// MOTEUR D'AUTHENTIFICATION CENTRALISÉ — SmartPub
// ============================================================================
// Tous les comptes de l'application sont définis ici et nulle part ailleurs.
// PublicationLoginDialog a été supprimé : la page de login est gérée
// directement par le QStackedWidget de SmartPub (cherchStackedWidgetLogin).
// ============================================================================

AppAuthService::AppAuthService()
{
    setupAccounts();
}

void AppAuthService::setupAccounts()
{
    // -----------------------------------------------------------------------
    // Format : { email, password, displayName, allowedModule, moduleIndex }
    // moduleIndex -1 = accès à tous les modules
    // -----------------------------------------------------------------------
    m_accounts = {
        {
            QStringLiteral("admin@gmail.com"),
            QStringLiteral("admin123"),
            QStringLiteral("Directeur Général"),
            QStringLiteral("ALL"),
            -1
        },
        {
            QStringLiteral("smartpub.chercheur@gmail.com"),
            QStringLiteral("chercheur123"),
            QStringLiteral("Responsable RH"),
            QStringLiteral("Chercheurs"),
            0
        },
        {
            QStringLiteral("smartpub.publications@gmail.com"),
            QStringLiteral("pub123"),
            QStringLiteral("Gestionnaire Publications"),
            QStringLiteral("Publications"),
            1
        },
        {
            QStringLiteral("smartpub.finances@gmail.com"),
            QStringLiteral("fin123"),
            QStringLiteral("Gestionnaire Finances"),
            QStringLiteral("Finances"),
            2
        },
        {
            QStringLiteral("smartpub.evenements@gmail.com"),
            QStringLiteral("event123"),
            QStringLiteral("Gestionnaire Événements"),
            QStringLiteral("Evenements"),
            3
        },
        {
            QStringLiteral("smartpub.projet@gmail.com"),
            QStringLiteral("projet123."),
            QStringLiteral("Chef de Projet"),
            QStringLiteral("Projets"),
            4
        },
        {
            QStringLiteral("smartpub.laboratoires@gmail.com"),
            QStringLiteral("lab123"),
            QStringLiteral("Dr de Recherche"),
            QStringLiteral("Laboratoires"),
            5
        },
    };
}

bool AppAuthService::authenticate(const QString &email,
                                  const QString &password,
                                  AppUserAccount *outUser,
                                  QString *errorMessage) const
{
    if (errorMessage) errorMessage->clear();

    if (email.trimmed().isEmpty() || password.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Veuillez saisir votre email et mot de passe.");
        return false;
    }

    const QString hashedInput = hashPassword(password);

    for (const AppUserAccount &acc : m_accounts) {
        if (acc.email != email.trimmed()) continue;

        // Check JSON override first (stores SHA-256 hashed passwords)
        const QString override = getOverridePassword(acc.email);
        if (!override.isEmpty()) {
            if (override == hashedInput) {
                if (outUser) *outUser = acc;
                return true;
            }
            // Override exists but password wrong
            if (errorMessage)
                *errorMessage = QStringLiteral("Email ou mot de passe incorrect.");
            return false;
        }

        // Fallback: compare plain text (default hardcoded passwords)
        if (acc.password == password) {
            if (outUser) *outUser = acc;
            return true;
        }

        if (errorMessage)
            *errorMessage = QStringLiteral("Email ou mot de passe incorrect.");
        return false;
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("Email ou mot de passe incorrect.");
    return false;
}

bool AppAuthService::emailExists(const QString &email) const
{
    for (const AppUserAccount &acc : m_accounts)
        if (acc.email == email.trimmed())
            return true;
    return false;
}

QString AppAuthService::hashPassword(const QString &password)
{
    return QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString AppAuthService::passwordFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/smartpub_passwords.json";
}

QString AppAuthService::getOverridePassword(const QString &email) const
{
    QFile file(passwordFilePath());
    if (!file.open(QIODevice::ReadOnly)) return QString();
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) return QString();
    return doc.object().value(email).toString();
}

bool AppAuthService::updatePassword(const QString &email, const QString &newPassword)
{
    QJsonObject obj;
    QFile file(passwordFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isObject()) obj = doc.object();
    }
    obj[email.trimmed()] = hashPassword(newPassword);
    file.setFileName(passwordFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    file.write(QJsonDocument(obj).toJson());
    file.close();
    return true;
}

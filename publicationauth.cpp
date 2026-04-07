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
            QStringLiteral("smartpub.projets@gmail.com"),
            QStringLiteral("proj123"),
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

    for (const AppUserAccount &acc : m_accounts) {
        if (acc.email == email.trimmed() && acc.password == password) {
            if (outUser) *outUser = acc;
            return true;
        }
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("Email ou mot de passe incorrect.");
    return false;
}

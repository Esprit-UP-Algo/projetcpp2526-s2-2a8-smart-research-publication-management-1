#include "publicationauth.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

bool PublicationAuthService::authenticate(const QString &email, const QString &password, QString *errorMessage) const {
    if (errorMessage)
        errorMessage->clear();

    if (email.trimmed().isEmpty() || password.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Veuillez saisir votre email et mot de passe.");
        return false;
    }

    const QString fixedEmail = QStringLiteral("Salma.Habaib@espit.tn");
    const QString fixedPassword = QStringLiteral("21326619");

    if (email.trimmed() == fixedEmail && password == fixedPassword)
        return true;

    if (errorMessage)
        *errorMessage = QStringLiteral("Email ou mot de passe invalide.");
    return false;
}

PublicationLoginDialog::PublicationLoginDialog(QWidget *parent)
    : QDialog(parent), m_emailEdit(nullptr), m_passwordEdit(nullptr),
      m_errorLabel(nullptr), m_loginButton(nullptr), m_cancelButton(nullptr) {
    setupUI();
}

void PublicationLoginDialog::setupUI() {
    setWindowTitle(QStringLiteral("Connexion - Publications"));
    setModal(true);
    setMinimumSize(420, 280);
    setStyleSheet(
        "QDialog { background-color: #f8fafc; }"
        "QLabel { color: #1e293b; }"
        "QLineEdit { background: white; color: #1e293b; border: 1px solid #cbd5e1; border-radius: 8px; padding: 10px; }"
        "QLineEdit::placeholder { color: #64748b; }"
        "QLineEdit:focus { border-color: #3b82f6; }"
        "QPushButton { border-radius: 8px; padding: 10px 16px; font-weight: 600; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(14);

    QLabel *title = new QLabel(QStringLiteral("Accès au module Publications"));
    title->setStyleSheet("font-size: 18px; font-weight: 700;");
    mainLayout->addWidget(title);

    QLabel *subtitle = new QLabel(QStringLiteral("Veuillez vous authentifier pour continuer."));
    subtitle->setStyleSheet("color: #64748b;");
    mainLayout->addWidget(subtitle);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText(QStringLiteral("Email"));
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText(QStringLiteral("Mot de passe"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    formLayout->addRow(QStringLiteral("Email"), m_emailEdit);
    formLayout->addRow(QStringLiteral("Mot de passe"), m_passwordEdit);
    mainLayout->addLayout(formLayout);

    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color: #dc2626; font-size: 12px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    mainLayout->addWidget(m_errorLabel);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch();
    m_cancelButton = new QPushButton(QStringLiteral("Annuler"));
    m_cancelButton->setStyleSheet("QPushButton { background: #e2e8f0; color: #334155; }");
    m_loginButton = new QPushButton(QStringLiteral("Se connecter"));
    m_loginButton->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3b82f6, stop:1 #10b981); color: white; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #2563eb, stop:1 #059669); }");
    buttons->addWidget(m_cancelButton);
    buttons->addWidget(m_loginButton);
    mainLayout->addLayout(buttons);

    connect(m_loginButton, &QPushButton::clicked, this, &PublicationLoginDialog::onLoginClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &PublicationLoginDialog::onLoginClicked);
}

void PublicationLoginDialog::onLoginClicked() {
    QString error;
    if (m_authService.authenticate(m_emailEdit->text(), m_passwordEdit->text(), &error)) {
        accept();
        return;
    }

    m_errorLabel->setText(error.isEmpty() ? QStringLiteral("Authentification échouée.") : error);
    m_errorLabel->setVisible(true);
}

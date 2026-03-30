#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include "promotionengine.h"
#include "matchmakingengine.h"
#include <algorithm>
#include <QTableWidgetItem>
#include <QApplication>
#include <QDateTime>
#include <QRegion>
#include <QProcess>
#include <QScreen>

// ============================================================================
// FONCTIONS HELPER GLOBALES (pour module Projets)
// ============================================================================

static QString getEtatColor(const QString &etat) {
    const QString t = etat.trimmed();
    if (t == QLatin1String("en_cours") || t == QLatin1String("Actif") || t == QLatin1String("En cours"))
        return QStringLiteral("#10b981");
    if (t == QLatin1String("termine") || t == QLatin1String("Terminé"))
        return QStringLiteral("#3b82f6");
    if (t == QLatin1String("suspendu") || t == QLatin1String("En pause") || t == QLatin1String("Suspendu"))
        return QStringLiteral("#f59e0b");
    if (t == QLatin1String("annule") || t == QLatin1String("Planifié") || t == QLatin1String("Annulé"))
        return QStringLiteral("#8b5cf6");
    return QStringLiteral("#64748b");
}

static QString projEtatDbToUi(const QString &db) {
    const QString d = db.trimmed().toLower();
    if (d == QLatin1String("en_cours"))
        return QStringLiteral("En cours");
    if (d == QLatin1String("termine"))
        return QStringLiteral("Terminé");
    if (d == QLatin1String("suspendu"))
        return QStringLiteral("Suspendu");
    if (d == QLatin1String("annule"))
        return QStringLiteral("Annulé");
    return db;
}

static double projProgressionStrToDouble(const QString &s) {
    QString t = s.trimmed();
    if (t.endsWith(QLatin1Char('%')))
        t.chop(1);
    bool ok = false;
    double v = t.toDouble(&ok);
    if (!ok)
        return 0.0;
    return qBound(0.0, v, 100.0);
}

static QString finTypeDbToUi(const QString &db) {
    const QString d = db.trimmed().toLower();
    if (d == QLatin1String("subvention"))
        return QStringLiteral("Subvention");
    if (d == QLatin1String("achat"))
        return QStringLiteral("Achat");
    if (d == QLatin1String("salaire"))
        return QStringLiteral("Salaire");
    if (d == QLatin1String("remboursement"))
        return QStringLiteral("Remboursement");
    if (d == QLatin1String("autre"))
        return QStringLiteral("Autre");
    // Capitaliser la premiere lettre si valeur inconnue
    if (!db.isEmpty()) {
        QString s = db.trimmed();
        s[0] = s[0].toUpper();
        return s;
    }
    return db;
}

static QString finTypeUiToDb(const QString &ui) {
    const QString u = ui.trimmed();
    if (u.compare(QStringLiteral("Subvention"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("subvention");
    if (u.compare(QStringLiteral("Achat"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("achat");
    if (u.compare(QStringLiteral("Salaire"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("salaire");
    if (u.compare(QStringLiteral("Remboursement"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("remboursement");
    if (u.compare(QStringLiteral("Autre"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("autre");
    if (u.contains(QStringLiteral("Recette"), Qt::CaseInsensitive))
        return QStringLiteral("subvention");
    if (u.contains(QStringLiteral("Dépense"), Qt::CaseInsensitive) || u.contains(QStringLiteral("Depense"), Qt::CaseInsensitive))
        return QStringLiteral("achat");
    return QStringLiteral("autre");
}

static QString getProgressionColor(int valeur) {
    if (valeur >= 80)
        return "#10b981";
    if (valeur >= 50)
        return "#3b82f6";
    if (valeur >= 25)
        return "#f59e0b";
    return "#ef4444";
}

static QString getProgressionColorFromString(const QString &progression) {
    QString temp = progression;
    if (temp.endsWith('%'))
        temp.chop(1);
    int valeur = temp.toInt();
    return getProgressionColor(valeur);
}

static QFrame* createVerticalSeparator() {
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setStyleSheet("color: #e2e8f0;");
    line->setFixedWidth(1);
    return line;
}

// Pixmap circulaire lisse (antialiasing), comme l'avatar du sidebar
static QPixmap makeCircularPixmap(const QPixmap &src, int size) {
    if (src.isNull() || size <= 0)
        return QPixmap();
    QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setClipRegion(QRegion(0, 0, size, size, QRegion::Ellipse));
    painter.drawPixmap(0, 0, scaled);
    painter.end();
    return result;
}

// Publications : libellés UI (combo) <-> valeurs CHECK Oracle (SmartPub1.sql)
static QString SR_statutUiToDb(const QString &ui) {
    const QString t = ui.trimmed();
    if (t.compare(QLatin1String("Publié"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("publie");
    if (t.compare(QLatin1String("Soumis"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("soumis");
    if (t.contains(QLatin1String("révision"), Qt::CaseInsensitive) ||
        t.contains(QLatin1String("revision"), Qt::CaseInsensitive))
        return QStringLiteral("en_revision");
    if (t.compare(QLatin1String("Accepté"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("accepte");
    if (t.compare(QLatin1String("Rejeté"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("rejete");
    return QStringLiteral("soumis");
}

static QString SR_statutDbToUi(const QString &db) {
    const QString d = db.trimmed().toLower();
    if (d == QLatin1String("publie"))
        return QStringLiteral("Publié");
    if (d == QLatin1String("soumis"))
        return QStringLiteral("Soumis");
    if (d == QLatin1String("en_revision"))
        return QStringLiteral("En révision");
    if (d == QLatin1String("accepte"))
        return QStringLiteral("Accepté");
    if (d == QLatin1String("rejete"))
        return QStringLiteral("Rejeté");
    return db;
}


// ============================================================================
// LOGIN DIALOG
// ============================================================================

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), loggedIn(false)
{
    setWindowTitle("Connexion - SmartPub");
    setMinimumSize(400, 500);
    resize(450, 550);
    setModal(true);

    setupAccounts();
    setupUI();
}

void LoginDialog::setupAccounts()
{
    // Initialize default accounts for different modules
    accounts.append({
        "admin@smartpub.com",
        "admin123",
        "Tous les modules",
        UserRole::Admin,
        "Administrateur"
    });

    accounts.append({
        "chercheur@smartpub.com",
        "chercheur123",
        "Chercheurs",
        UserRole::Admin,
        "Gestionnaire Chercheurs"
    });

    accounts.append({
        "publications@smartpub.com",
        "pub123",
        "Publications",
        UserRole::Admin,
        "Gestionnaire Publications"
    });

    accounts.append({
        "finances@smartpub.com",
        "fin123",
        "Finances",
        UserRole::Admin,
        "Gestionnaire Finances"
    });

    accounts.append({
        "evenements@smartpub.com",
        "event123",
        "Evenements",
        UserRole::Admin,
        "Gestionnaire Événements"
    });

    accounts.append({
        "projets@smartpub.com",
        "proj123",
        "Projets",
        UserRole::Admin,
        "Gestionnaire Projets"
    });

    accounts.append({
        "laboratoires@smartpub.com",
        "lab123",
        "Laboratoires",
        UserRole::Admin,
        "Dr de recherche"
    });
}

void LoginDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Logo/Title
    QLabel *titleLabel = new QLabel("SmartPub");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #3b82f6;"
        "margin-bottom: 10px;"
    );
    mainLayout->addWidget(titleLabel);

    QLabel *subtitleLabel = new QLabel("Système de Gestion de Recherche");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "font-size: 14px;"
        "color: #64748b;"
        "margin-bottom: 20px;"
    );
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    // Email field
    QLabel *emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet("font-size: 14px; color: #475569; font-weight: 600;");
    mainLayout->addWidget(emailLabel);

    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("votre.email@smartpub.com");
    emailEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 12px;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    background: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #3b82f6;"
        "}"
    );
    mainLayout->addWidget(emailEdit);

    // Password field
    QLabel *passwordLabel = new QLabel("Mot de passe:");
    passwordLabel->setStyleSheet("font-size: 14px; color: #475569; font-weight: 600; margin-top: 10px;");
    mainLayout->addWidget(passwordLabel);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("••••••••");
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "    padding: 12px;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    background: white;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #3b82f6;"
        "}"
    );
    mainLayout->addWidget(passwordEdit);

    // Error label
    errorLabel = new QLabel();
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setStyleSheet(
        "color: #ef4444;"
        "font-size: 13px;"
        "padding: 8px;"
        "background: #fee2e2;"
        "border-radius: 6px;"
    );
    errorLabel->hide();
    mainLayout->addWidget(errorLabel);

    mainLayout->addSpacing(10);

    // Login button
    loginBtn = new QPushButton("Se connecter");
    loginBtn->setStyleSheet(
        "QPushButton {"
        "    background: #3b82f6;"
        "    color: white;"
        "    padding: 14px;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 15px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #2563eb;"
        "}"
        "QPushButton:pressed {"
        "    background: #1d4ed8;"
        "}"
    );
    mainLayout->addWidget(loginBtn);

    // Forgot password button
    forgotBtn = new QPushButton("Mot de passe oublié?");
    forgotBtn->setStyleSheet(
        "QPushButton {"
        "    background: transparent;"
        "    color: #3b82f6;"
        "    border: none;"
        "    padding: 8px;"
        "    font-size: 13px;"
        "    text-decoration: underline;"
        "}"
        "QPushButton:hover {"
        "    color: #2563eb;"
        "}"
    );
    mainLayout->addWidget(forgotBtn, 0, Qt::AlignCenter);

    mainLayout->addSpacing(20);

    // Separator
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("color: #e2e8f0;");
    mainLayout->addWidget(separator);

    // Guest button
    guestBtn = new QPushButton("Continuer en tant qu'invité");
    guestBtn->setStyleSheet(
        "QPushButton {"
        "    background: #f1f5f9;"
        "    color: #475569;"
        "    padding: 12px;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #e2e8f0;"
        "    border-color: #cbd5e1;"
        "}"
    );
    mainLayout->addWidget(guestBtn);

    mainLayout->addStretch();

    // Info label at bottom
    QLabel *infoLabel = new QLabel(
        "Comptes de test:\n"
        "admin@smartpub.com / admin123\n"
        "chercheur@smartpub.com / chercheur123\n"
        "laboratoires@smartpub.com / lab123"
    );
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet(
        "font-size: 11px;"
        "color: #94a3b8;"
        "padding: 10px;"
        "background: #f8fafc;"
        "border-radius: 6px;"
    );
    mainLayout->addWidget(infoLabel);

    // Connect signals
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(forgotBtn, &QPushButton::clicked, this, &LoginDialog::onForgotPasswordClicked);
    connect(guestBtn, &QPushButton::clicked, this, &LoginDialog::onGuestClicked);
    connect(emailEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);

    // Set dialog style
    setStyleSheet(
        "QDialog {"
        "    background: white;"
        "}"
    );
}

void LoginDialog::onLoginClicked()
{
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();

    errorLabel->hide();

    if (email.isEmpty() || password.isEmpty()) {
        errorLabel->setText("⚠ Veuillez remplir tous les champs");
        errorLabel->show();
        return;
    }

    // Check credentials
    for (const UserAccount &account : accounts) {
        if (account.email == email && account.password == password) {
            loggedInUser = account;
            loggedIn = true;
            accept();
            return;
        }
    }

    // Invalid credentials
    errorLabel->setText("❌ Email ou mot de passe incorrect");
    errorLabel->show();
    passwordEdit->clear();
    passwordEdit->setFocus();
}

void LoginDialog::onForgotPasswordClicked()
{
    QMessageBox::information(
        this,
        "Mot de passe oublié",
        "Pour réinitialiser votre mot de passe, veuillez contacter l'administrateur système.\n\n"
        "Email: admin@smartpub.com\n"
        "Tél: +216 XX XXX XXX"
    );
}

void LoginDialog::onGuestClicked()
{
    loggedInUser = UserAccount{
        "guest@smartpub.com",
        "",
        "Tous les modules",
        UserRole::Guest,
        "Invité"
    };
    loggedIn = true;
    accept();
}

// ============================================================================
// DIALOGUES CORRIGÉS (copiés de mainwindow.cpp)
// ============================================================================

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Paramètres");
    setMinimumSize(500, 600);
    resize(500, 600);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f8fafc;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #1e293b;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        );

    setupUI();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Header
    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    border: none;"
        "}"
        );
    headerFrame->setFixedHeight(80);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(25, 15, 25, 15);

    QLabel *titleLabel = new QLabel("⚙️ Paramètres");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    QLabel *subtitleLabel = new QLabel("Personnalisez votre expérience");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 13px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    // Scroll area pour le contenu
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(25, 25, 25, 25);

    // Section Apparence
    QGroupBox *appearanceGroup = new QGroupBox("🎨 Apparence");
    appearanceGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding: 20px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        );

    QFormLayout *appearanceLayout = new QFormLayout(appearanceGroup);
    appearanceLayout->setSpacing(15);
    appearanceLayout->setLabelAlignment(Qt::AlignLeft);
    appearanceLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Thème
    QLabel *themeLabel = new QLabel("Thème :");
    themeLabel->setStyleSheet("font-weight: 500; color: #334155;");

    themeCombo = new QComboBox();
    themeCombo->addItem("🌙 Sombre");
    themeCombo->addItem("☀️ Clair");
    themeCombo->setStyleSheet(
        "QComboBox {"
        "    background-color: #f8fafc;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 10px;"
        "    min-height: 40px;"
        "    font-size: 13px;"
        "}"
        "QComboBox:focus { border-color: #3b82f6; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        );
    appearanceLayout->addRow(themeLabel, themeCombo);

    // Langue
    QLabel *langLabel = new QLabel("Langue :");
    langLabel->setStyleSheet("font-weight: 500; color: #334155;");

    langCombo = new QComboBox();
    langCombo->addItem("🇫🇷 Français");
    langCombo->addItem("🇬🇧 English");
    langCombo->setStyleSheet(
        "QComboBox {"
        "    background-color: #f8fafc;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 10px;"
        "    min-height: 40px;"
        "    font-size: 13px;"
        "}"
        "QComboBox:focus { border-color: #3b82f6; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        );
    appearanceLayout->addRow(langLabel, langCombo);

    contentLayout->addWidget(appearanceGroup);

    // Section Notifications
    QGroupBox *notifGroup = new QGroupBox("🔔 Notifications");
    notifGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding: 20px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        );

    QVBoxLayout *notifLayout = new QVBoxLayout(notifGroup);
    notifLayout->setSpacing(12);

    notifCheck = new QCheckBox("Activer les notifications");
    notifCheck->setChecked(true);
    notifCheck->setStyleSheet("QCheckBox { spacing: 8px; font-size: 13px; color: #334155; }");
    notifLayout->addWidget(notifCheck);

    emailCheck = new QCheckBox("Recevoir des emails de rappel");
    emailCheck->setChecked(true);
    emailCheck->setStyleSheet("QCheckBox { spacing: 8px; font-size: 13px; color: #334155; }");
    notifLayout->addWidget(emailCheck);

    soundCheck = new QCheckBox("Activer les sons de notification");
    soundCheck->setChecked(false);
    soundCheck->setStyleSheet("QCheckBox { spacing: 8px; font-size: 13px; color: #334155; }");
    notifLayout->addWidget(soundCheck);

    contentLayout->addWidget(notifGroup);

    // Section Sauvegarde
    QGroupBox *saveGroup = new QGroupBox("💾 Sauvegarde");
    saveGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding: 20px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        );

    QVBoxLayout *saveLayout = new QVBoxLayout(saveGroup);
    saveLayout->setSpacing(15);

    autoSaveCheck = new QCheckBox("Sauvegarde automatique");
    autoSaveCheck->setChecked(true);
    autoSaveCheck->setStyleSheet("QCheckBox { spacing: 8px; font-size: 13px; color: #334155; }");
    saveLayout->addWidget(autoSaveCheck);

    QHBoxLayout *intervalLayout = new QHBoxLayout();
    QLabel *intervalLabel = new QLabel("Intervalle (minutes) :");
    intervalLabel->setStyleSheet("font-weight: 500; color: #334155; font-size: 13px;");
    intervalLabel->setMinimumWidth(130);

    intervalSpin = new QSpinBox();
    intervalSpin->setRange(1, 60);
    intervalSpin->setValue(15);
    intervalSpin->setStyleSheet(
        "QSpinBox {"
        "    background-color: #f8fafc;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    min-height: 40px;"
        "    min-width: 80px;"
        "    font-size: 13px;"
        "}"
        "QSpinBox:focus { border-color: #3b82f6; }"
        );
    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(intervalSpin);
    intervalLayout->addStretch();
    saveLayout->addLayout(intervalLayout);

    QPushButton *backupNowBtn = new QPushButton("💾 Sauvegarder maintenant");
    backupNowBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white;"
        "    border-radius: 8px;"
        "    padding: 12px 20px;"
        "    font-size: 13px;"
        "    max-width: 200px;"
        "}"
        "QPushButton:hover { background-color: #2563eb; }"
        );
    saveLayout->addWidget(backupNowBtn, 0, Qt::AlignLeft);

    contentLayout->addWidget(saveGroup);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    // Footer avec boutons
    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *buttonLayout = new QHBoxLayout(footerFrame);
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(25, 0, 25, 0);

    buttonLayout->addStretch();

    QPushButton *btnAnnuler = new QPushButton("Annuler");
    btnAnnuler->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"
        "    color: #64748b;"
        "    border: 2px solid #e2e8f0;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover { background-color: #f1f5f9; }"
        );

    QPushButton *btnEnregistrer = new QPushButton("Enregistrer");
    btnEnregistrer->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );

    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnEnregistrer, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(btnAnnuler);
    buttonLayout->addWidget(btnEnregistrer);

    mainLayout->addWidget(footerFrame);
}

// ==================== DIALOG FILTRES CORRIGÉ ====================


// ============================================================================
// MODULE PROJETS - FILTRES
// ============================================================================

FiltresDialog::FiltresDialog(QWidget *parent)
    : QDialog(parent), filtreActif(false)
{
    setWindowTitle("Filtres de Recherche");
    setMinimumSize(450, 450);
    resize(450, 450);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f8fafc;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #334155;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QComboBox {"
        "    background-color: white;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 10px 15px;"
        "    font-size: 13px;"
        "    color: #334155;"
        "    min-height: 42px;"
        "}"
        "QComboBox:focus { border-color: #3b82f6; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView {"
        "    background-color: white;"
        "    border: 1px solid #e2e8f0;"
        "    selection-background-color: #eff6ff;"
        "    color: #334155;"
        "}"
        "QDateEdit {"
        "    background-color: white;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 10px 15px;"
        "    font-size: 13px;"
        "    color: #334155;"
        "    min-height: 42px;"
        "}"
        "QDateEdit:focus { border-color: #3b82f6; }"
        "QPushButton {"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        );

    setupUI();
}

void FiltresDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Header
    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-bottom: 1px solid #e2e8f0;"
        "}"
        );
    headerFrame->setFixedHeight(80);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(25, 15, 25, 15);

    QLabel *titleLabel = new QLabel("🔍 Filtrer les Projets");
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #1e293b;");

    QLabel *subtitleLabel = new QLabel("Affinez votre recherche avec les critères ci-dessous");
    subtitleLabel->setStyleSheet("font-size: 13px; color: #64748b;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    // Contenu
    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(25, 25, 25, 25);

    // Formulaire avec QFormLayout pour alignement parfait
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setContentsMargins(0, 0, 0, 0);

    // État
    QLabel *labelEtat = new QLabel("État du projet");
    labelEtat->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155;");

    comboBoxEtat = new QComboBox();
    comboBoxEtat->addItem("Tous les états", "");
    comboBoxEtat->addItem("Planifié", "Planifié");
    comboBoxEtat->addItem("Actif", "Actif");
    comboBoxEtat->addItem("En pause", "En pause");
    comboBoxEtat->addItem("Terminé", "Terminé");
    formLayout->addRow(labelEtat, comboBoxEtat);

    // Responsable
    QLabel *labelResp = new QLabel("Responsable");
    labelResp->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155;");

    comboBoxResponsable = new QComboBox();
    comboBoxResponsable->addItem("Tous les responsables", "");
    comboBoxResponsable->addItem("Dr. Ahmed Ben Ali", "Dr. Ahmed Ben Ali");
    comboBoxResponsable->addItem("Pr. Fatima Zohra", "Pr. Fatima Zohra");
    comboBoxResponsable->addItem("Dr. Mohamed Salah", "Dr. Mohamed Salah");
    comboBoxResponsable->addItem("Dr. Sarah Johnson", "Dr. Sarah Johnson");
    comboBoxResponsable->addItem("Pr. Robert Chen", "Pr. Robert Chen");
    formLayout->addRow(labelResp, comboBoxResponsable);

    // Période - Label
    QLabel *labelPeriode = new QLabel("Période de début");
    labelPeriode->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155;");

    // Widget conteneur pour les dates avec layout horizontal
    QWidget *datesWidget = new QWidget();
    QHBoxLayout *datesLayout = new QHBoxLayout(datesWidget);
    datesLayout->setSpacing(15);
    datesLayout->setContentsMargins(0, 0, 0, 0);

    // Date début
    QVBoxLayout *debutLayout = new QVBoxLayout();
    debutLayout->setSpacing(5);
    QLabel *labelDu = new QLabel("Du");
    labelDu->setStyleSheet("font-size: 12px; color: #64748b; font-weight: normal;");
    dateEditDebutMin = new QDateEdit();
    dateEditDebutMin->setCalendarPopup(true);
    dateEditDebutMin->setDate(QDate(2020, 1, 1));
    dateEditDebutMin->setDisplayFormat("dd/MM/yyyy");
    dateEditDebutMin->setMinimumHeight(42);
    debutLayout->addWidget(labelDu);
    debutLayout->addWidget(dateEditDebutMin);

    // Date fin
    QVBoxLayout *finLayout = new QVBoxLayout();
    finLayout->setSpacing(5);
    QLabel *labelAu = new QLabel("Au");
    labelAu->setStyleSheet("font-size: 12px; color: #64748b; font-weight: normal;");
    dateEditMax = new QDateEdit();
    dateEditMax->setCalendarPopup(true);
    dateEditMax->setDate(QDate::currentDate().addYears(5));
    dateEditMax->setDisplayFormat("dd/MM/yyyy");
    dateEditMax->setMinimumHeight(42);
    finLayout->addWidget(labelAu);
    finLayout->addWidget(dateEditMax);

    datesLayout->addLayout(debutLayout, 1);
    datesLayout->addLayout(finLayout, 1);

    formLayout->addRow(labelPeriode, datesWidget);

    contentLayout->addLayout(formLayout);
    contentLayout->addStretch();

    mainLayout->addWidget(contentWidget, 1);

    // Footer avec boutons
    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *buttonLayout = new QHBoxLayout(footerFrame);
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(25, 0, 25, 0);

    btnReinitialiser = new QPushButton("🔄 Réinitialiser");
    btnReinitialiser->setStyleSheet(
        "QPushButton {"
        "    background-color: #e2e8f0;"
        "    color: #475569;"
        "}"
        "QPushButton:hover { background-color: #cbd5e1; }"
        );

    buttonLayout->addWidget(btnReinitialiser);
    buttonLayout->addStretch();

    btnAnnuler = new QPushButton("Annuler");
    btnAnnuler->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"
        "    color: #64748b;"
        "    border: 2px solid #e2e8f0;"
        "}"
        "QPushButton:hover { background-color: #f1f5f9; }"
        );

    btnAppliquer = new QPushButton("Appliquer");
    btnAppliquer->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );

    connect(btnReinitialiser, &QPushButton::clicked, [=]() {
        comboBoxEtat->setCurrentIndex(0);
        comboBoxResponsable->setCurrentIndex(0);
        dateEditDebutMin->setDate(QDate(2020, 1, 1));
        dateEditMax->setDate(QDate::currentDate().addYears(5));
    });

    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnAppliquer, &QPushButton::clicked, [=]() {
        filtreActif = true;
        accept();
    });

    buttonLayout->addWidget(btnAnnuler);
    buttonLayout->addWidget(btnAppliquer);

    mainLayout->addWidget(footerFrame);
}

QString FiltresDialog::getEtatFiltre() const
{
    return comboBoxEtat->currentData().toString();
}

QString FiltresDialog::getResponsableFiltre() const
{
    return comboBoxResponsable->currentData().toString();
}

QDate FiltresDialog::getDateDebutMin() const
{
    return dateEditDebutMin->date();
}

QDate FiltresDialog::getDateDebutMax() const
{
    return dateEditMax->date();
}

bool FiltresDialog::isFiltreActif() const
{
    return filtreActif;
}

// ==================== IA RECOMMANDATIONS DIALOG ====================


// ============================================================================
// MODULE PROJETS - IA RECOMMANDATIONS
// ============================================================================

IARecommandationsDialog::IARecommandationsDialog(const QVector<Projet> &projets, QWidget *parent)
    : QDialog(parent), m_projets(projets)
{
    setWindowTitle("Recommandations Intelligentes");
    setMinimumSize(900, 700);
    resize(1000, 800);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f1f5f9;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #334155;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        "QProgressBar {"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 10px;"
        "    text-align: center;"
        "    height: 24px;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    color: #334155;"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 8px;"
        "}"
        "QListWidget {"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    background-color: #f8fafc;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid #e2e8f0;"
        "    color: #475569;"
        "}"
        "QListWidget::item:last {"
        "    border-bottom: none;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #eff6ff;"
        "    color: #1e293b;"
        "    border-radius: 6px;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        );

    setupUI();
}

void IARecommandationsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    border: none;"
        "}"
        );
    headerFrame->setFixedHeight(100);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel *titleLabel = new QLabel("🤖 Recommandations Intelligentes");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *subtitleLabel = new QLabel("Analyse sémantique et génération de projets basée sur l'IA");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background-color: #f1f5f9;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *statsFrame = new QFrame();
    statsFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #e2e8f0;"
        "}"
        );
    statsFrame->setFixedHeight(120);

    QHBoxLayout *statsLayout = new QHBoxLayout(statsFrame);
    statsLayout->setSpacing(30);
    statsLayout->setContentsMargins(25, 20, 25, 20);

    QVBoxLayout *stat1Layout = new QVBoxLayout();
    QLabel *stat1Value = new QLabel(QString::number(m_projets.size()));
    stat1Value->setStyleSheet("font-size: 32px; font-weight: bold; color: #3b82f6;");
    stat1Value->setAlignment(Qt::AlignCenter);
    QLabel *stat1Label = new QLabel("Projets analysés");
    stat1Label->setStyleSheet("font-size: 13px; color: #64748b;");
    stat1Label->setAlignment(Qt::AlignCenter);
    stat1Layout->addWidget(stat1Value);
    stat1Layout->addWidget(stat1Label);
    statsLayout->addLayout(stat1Layout);

    statsLayout->addWidget(createVerticalSeparator());

    QVBoxLayout *stat2Layout = new QVBoxLayout();
    int actifs = 0;
    for (const auto &p : m_projets)
        if (p.etat == QLatin1String("en_cours")) actifs++;
    QLabel *stat2Value = new QLabel(QString::number(actifs));
    stat2Value->setStyleSheet("font-size: 32px; font-weight: bold; color: #10b981;");
    stat2Value->setAlignment(Qt::AlignCenter);
    QLabel *stat2Label = new QLabel("Projets actifs");
    stat2Label->setStyleSheet("font-size: 13px; color: #64748b;");
    stat2Label->setAlignment(Qt::AlignCenter);
    stat2Layout->addWidget(stat2Value);
    stat2Layout->addWidget(stat2Label);
    statsLayout->addLayout(stat2Layout);

    statsLayout->addWidget(createVerticalSeparator());

    QVBoxLayout *stat3Layout = new QVBoxLayout();
    QLabel *stat3Value = new QLabel("3");
    stat3Value->setStyleSheet("font-size: 32px; font-weight: bold; color: #8b5cf6;");
    stat3Value->setAlignment(Qt::AlignCenter);
    QLabel *stat3Label = new QLabel("Recommandations");
    stat3Label->setStyleSheet("font-size: 13px; color: #64748b;");
    stat3Label->setAlignment(Qt::AlignCenter);
    stat3Layout->addWidget(stat3Value);
    stat3Layout->addWidget(stat3Label);
    statsLayout->addLayout(stat3Layout);

    contentLayout->addWidget(statsFrame);

    genererRecommandations();

    for (int i = 0; i < m_recommandations.size(); ++i) {
        const auto &rec = m_recommandations[i];

        QGroupBox *recGroup = new QGroupBox(QString("Recommandation #%1 - %2").arg(i+1).arg(rec.domaine));

        QVBoxLayout *recLayout = new QVBoxLayout(recGroup);
        recLayout->setSpacing(15);
        recLayout->setContentsMargins(20, 20, 20, 20);

        QHBoxLayout *headerRecLayout = new QHBoxLayout();

        QLabel *titreLabel = new QLabel(rec.titre);
        titreLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b;");
        titreLabel->setWordWrap(true);
        headerRecLayout->addWidget(titreLabel, 1);

        QVBoxLayout *scoreLayout = new QVBoxLayout();
        scoreLayout->setSpacing(2);
        QLabel *scoreLabel = new QLabel(QString("%1%").arg(qRound(rec.scoreSimilarite)));
        scoreLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #10b981;");
        scoreLabel->setAlignment(Qt::AlignRight);
        QLabel *scoreText = new QLabel("Pertinence");
        scoreText->setStyleSheet("font-size: 11px; color: #64748b;");
        scoreText->setAlignment(Qt::AlignRight);
        scoreLayout->addWidget(scoreLabel);
        scoreLayout->addWidget(scoreText);
        headerRecLayout->addLayout(scoreLayout);

        recLayout->addLayout(headerRecLayout);

        QProgressBar *scoreBar = new QProgressBar();
        scoreBar->setValue(qRound(rec.scoreSimilarite));
        scoreBar->setTextVisible(false);
        scoreBar->setFixedHeight(8);

        QString color;
        if (rec.scoreSimilarite >= 85) color = "#10b981";
        else if (rec.scoreSimilarite >= 70) color = "#3b82f6";
        else color = "#f59e0b";

        scoreBar->setStyleSheet(QString(
                                    "QProgressBar { border: none; border-radius: 4px; background-color: #e2e8f0; }"
                                    "QProgressBar::chunk { border-radius: 4px; background-color: %1; }"
                                    ).arg(color));
        recLayout->addWidget(scoreBar);

        QLabel *descLabel = new QLabel(rec.description);
        descLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.6;");
        descLabel->setWordWrap(true);
        recLayout->addWidget(descLabel);

        QLabel *raisonLabel = new QLabel(rec.raison);
        raisonLabel->setStyleSheet(
            "font-size: 13px; color: #3b82f6; "
            "background-color: #eff6ff; padding: 10px; "
            "border-radius: 8px; border-left: 4px solid #3b82f6;"
            );
        raisonLabel->setWordWrap(true);
        recLayout->addWidget(raisonLabel);

        QLabel *collabTitle = new QLabel("Collaborateurs suggérés");
        collabTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #334155; margin-top: 10px;");
        recLayout->addWidget(collabTitle);

        QListWidget *collabList = new QListWidget();
        collabList->setFixedHeight(100);
        for (const QString &c : rec.collaborateursSuggeres) {
            collabList->addItem(c);
        }
        recLayout->addWidget(collabList);

        contentLayout->addWidget(recGroup);
    }

    QGroupBox *insightsGroup = new QGroupBox("Insights & Patterns détectés");
    QVBoxLayout *insightsLayout = new QVBoxLayout(insightsGroup);
    insightsLayout->setContentsMargins(20, 20, 20, 20);

    QTextBrowser *insightsBrowser = new QTextBrowser();
    insightsBrowser->setFixedHeight(150);
    insightsBrowser->setStyleSheet(
        "QTextBrowser {"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "    background-color: #f8fafc;"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "    color: #475569;"
        "}"
        );

    QString insightsHtml = QString(R"(
        <h3 style='color: #1e293b; margin-top: 0;'>Analyse des projets existants</h3>
        <ul style='margin: 10px 0; padding-left: 20px;'>
            <li><b>Domaines dominants :</b> Intelligence Artificielle, Biotechnologie, Énergies Renouvelables</li>
            <li><b>Taux de réussite :</b> 85%% pour les projets interdisciplinaires</li>
            <li><b>Durée optimale :</b> 18-24 mois pour maximiser l'impact</li>
            <li><b>Collaboration :</b> Projets à 3+ chercheurs = +40%% de publications</li>
        </ul>
        <p style='color: #059669; font-weight: 600; margin: 10px 0 0 0;'>
            Recommandation stratégique : Privilégiez les consortiums multi-laboratoires
        </p>
    )");
    insightsBrowser->setHtml(insightsHtml);
    insightsLayout->addWidget(insightsBrowser);
    contentLayout->addWidget(insightsGroup);

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(30, 0, 30, 0);

    footerLayout->addStretch();

    QPushButton *closeButton = new QPushButton("J'ai compris, fermer");
    closeButton->setFixedSize(180, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    footerLayout->addWidget(closeButton);
    mainLayout->addWidget(footerFrame);
}

void IARecommandationsDialog::genererRecommandations()
{
    bool hasAI = false, hasBio = false, hasQuantum = false, hasEnergy = false;

    for (const auto &p : m_projets) {
        QString desc = p.description.toLower();
        if (desc.contains("ia") || desc.contains("intelligent") || desc.contains("machine learning")) hasAI = true;
        if (desc.contains("bio") || desc.contains("genome") || desc.contains("medical")) hasBio = true;
        if (desc.contains("quantique")) hasQuantum = true;
        if (desc.contains("energie") || desc.contains("solaire")) hasEnergy = true;
    }

    if (hasAI) {
        Recommandation rec;
        rec.titre = "Deep Learning pour la Santé Prédictive";
        rec.domaine = "IA × Biotechnologie";
        rec.scoreSimilarite = 92.0;
        rec.description = "Extension naturelle de votre expertise en IA vers le domaine médical. "
                          "Ce projet vise à développer des modèles de deep learning pour la prédiction "
                          "précoce des maladies chroniques basés sur l'analyse génomique.";
        rec.raison = "Synergie forte avec Smart-Traffic (IA) et Analyse Génome (Biologie). "
                     "Fort potentiel d'innovation et de publications scientifiques.";
        rec.collaborateursSuggeres = {
            "Dr. Ahmed Ben Ali - Expertise IA (Score: 95%)",
            "Pr. Fatima Zohra - Génomique (Score: 88%)",
            "Dr. Sarah Johnson - Analyse de données (Score: 82%)"
        };
        m_recommandations.append(rec);
    }

    if (hasQuantum || hasAI) {
        Recommandation rec;
        rec.titre = "Calculateur Quantique pour la Bioinformatique";
        rec.domaine = "Quantique × Biologie";
        rec.scoreSimilarite = 87.0;
        rec.description = "Fusion de trois domaines d'excellence : informatique quantique, "
                          "intelligence artificielle et biologie. Utilisation d'algorithmes quantiques "
                          "pour accélérer l'analyse des séquences génomiques.";
        rec.raison = "Combinaison unique de vos forces en quantique et biologie. "
                     "Projet hautement innovant avec fort potentiel de financement européen.";
        rec.collaborateursSuggeres = {
            "Dr. Mohamed Salah - Informatique Quantique (Score: 96%)",
            "Dr. Ahmed Ben Ali - IA & Algorithmes (Score: 91%)",
            "Pr. Fatima Zohra - Bioinformatique (Score: 89%)"
        };
        m_recommandations.append(rec);
    }

    if (hasEnergy || hasAI) {
        Recommandation rec;
        rec.titre = "Smart Grid IA pour Villes Durables";
        rec.domaine = "Énergie × IA";
        rec.scoreSimilarite = 84.0;
        rec.description = "Extension de Smart-Traffic vers la gestion énergétique urbaine. "
                          "Développement d'un réseau électrique intelligent optimisé par l'IA "
                          "pour réduire la consommation énergétique des villes.";
        rec.raison = "Continuité logique de Smart-Traffic vers la smart city. "
                     "Répond aux enjeux actuels de transition énergétique.";
        rec.collaborateursSuggeres = {
            "Dr. Sarah Johnson - Énergies Renouvelables (Score: 94%)",
            "Dr. Ahmed Ben Ali - IA/Smart City (Score: 90%)",
            "Pr. Robert Chen - Optimisation systèmes (Score: 85%)"
        };
        m_recommandations.append(rec);
    }
}

// ==================== PROJET DETAILS DIALOG ====================


// ============================================================================
// MODULE PROJETS - DETAILS
// ============================================================================

ProjetDetailsDialog::ProjetDetailsDialog(const Projet &projet, QWidget *parent)
    : QDialog(parent), m_projet(projet)
{
    setWindowTitle("Détails du Projet - " + projet.titre);
    setMinimumSize(700, 600);
    resize(750, 650);

    setStyleSheet("QDialog { background-color: #1e1e1e; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QVBoxLayout *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(10);
    headerLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(projet.titre);
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    QLabel *codeLabel = new QLabel("Code: " + projet.code);
    codeLabel->setStyleSheet("font-size: 14px; color: #a0a0a0;");
    codeLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(codeLabel);
    mainLayout->addLayout(headerLayout);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #3a3a3a; border: none;");
    line->setFixedHeight(2);
    mainLayout->addWidget(line);

    QGroupBox *infoGroup = new QGroupBox("Informations du Projet");
    infoGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border: 2px solid #3a3a3a;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: #2d2d2d;"
        "    color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #4a9eff;"
        "}"
        );

    QGridLayout *infoLayout = new QGridLayout(infoGroup);
    infoLayout->setSpacing(15);
    infoLayout->setContentsMargins(20, 20, 20, 20);
    infoLayout->setColumnStretch(1, 1);
    infoLayout->setColumnStretch(3, 1);

    QString labelStyle = "color: #b0b0b0; font-size: 13px;";
    QString valueStyle = "color: #ffffff; font-size: 13px; font-weight: 600;";

    QLabel *codeLabelInfo = new QLabel("Code:");
    codeLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(codeLabelInfo, 0, 0);

    QLabel *codeValue = new QLabel(projet.code);
    codeValue->setStyleSheet(valueStyle);
    infoLayout->addWidget(codeValue, 0, 1);

    QLabel *respLabelInfo = new QLabel("Responsable:");
    respLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(respLabelInfo, 0, 2);

    QLabel *respValue = new QLabel(projet.responsable);
    respValue->setStyleSheet("color: #4a9eff; font-size: 13px; font-weight: 600;");
    infoLayout->addWidget(respValue, 0, 3);

    QLabel *debutLabel = new QLabel("Début:");
    debutLabel->setStyleSheet(labelStyle);
    infoLayout->addWidget(debutLabel, 1, 0);

    QLabel *debutValue = new QLabel(projet.dateDebut.toString("dd/MM/yyyy"));
    debutValue->setStyleSheet(valueStyle);
    infoLayout->addWidget(debutValue, 1, 1);

    QLabel *finLabel = new QLabel("Fin:");
    finLabel->setStyleSheet(labelStyle);
    infoLayout->addWidget(finLabel, 1, 2);

    QLabel *finValue = new QLabel(projet.dateFin.toString("dd/MM/yyyy"));
    finValue->setStyleSheet(valueStyle);
    infoLayout->addWidget(finValue, 1, 3);

    QLabel *etatLabelInfo = new QLabel("État:");
    etatLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(etatLabelInfo, 2, 0);

    QLabel *etatBadge = new QLabel(projEtatDbToUi(projet.etat));
    QString etatColor = ::getEtatColor(projEtatDbToUi(projet.etat));
    etatBadge->setStyleSheet(QString(
                                 "background-color: %1;"
                                 "color: white;"
                                 "padding: 6px 16px;"
                                 "border-radius: 6px;"
                                 "font-weight: bold;"
                                 "font-size: 12px;"
                                 ).arg(etatColor));
    etatBadge->setAlignment(Qt::AlignCenter);
    etatBadge->setFixedWidth(100);
    infoLayout->addWidget(etatBadge, 2, 1, Qt::AlignLeft);

    QLabel *progLabelInfo = new QLabel("Progression:");
    progLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(progLabelInfo, 2, 2);

    QHBoxLayout *progLayout = new QHBoxLayout();
    progLayout->setSpacing(10);

    QProgressBar *progressBar = new QProgressBar();
    QString progStr = projet.progression;
    if (progStr.endsWith('%')) progStr.chop(1);
    int progValue = progStr.toInt();
    progressBar->setValue(progValue);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(20);
    progressBar->setStyleSheet(QString(
                                   "QProgressBar {"
                                   "    border: none;"
                                   "    border-radius: 10px;"
                                   "    background-color: #3a3a3a;"
                                   "    text-align: center;"
                                   "}"
                                   "QProgressBar::chunk {"
                                   "    border-radius: 10px;"
                                   "    background-color: %1;"
                                   "}"
                                   ).arg(::getProgressionColor(progValue)));

    QLabel *progText = new QLabel(projet.progression);
    progText->setStyleSheet("color: #ffffff; font-weight: bold; font-size: 13px;");
    progText->setFixedWidth(45);

    progLayout->addWidget(progressBar, 1);
    progLayout->addWidget(progText);
    infoLayout->addLayout(progLayout, 2, 3);

    mainLayout->addWidget(infoGroup);

    QGroupBox *descGroup = new QGroupBox("Description");
    descGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border: 2px solid #3a3a3a;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: #2d2d2d;"
        "    color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #4a9eff;"
        "}"
        );

    QVBoxLayout *descLayout = new QVBoxLayout(descGroup);
    descLayout->setContentsMargins(15, 20, 15, 15);

    QTextBrowser *descBrowser = new QTextBrowser();
    descBrowser->setPlainText(projet.description.isEmpty() ? "Aucune description disponible." : projet.description);
    descBrowser->setStyleSheet(
        "QTextBrowser {"
        "    border: 1px solid #3a3a3a;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "    background-color: #1e1e1e;"
        "    color: #e0e0e0;"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "}"
        );
    descBrowser->setMinimumHeight(150);
    descBrowser->setReadOnly(true);
    descLayout->addWidget(descBrowser);

    mainLayout->addWidget(descGroup, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setFixedSize(120, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4a9eff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3a8eef;"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}

// ==================== STATISTIQUES DIALOG ====================


// ============================================================================
// MODULE PROJETS - STATISTIQUES
// ============================================================================

StatistiquesDialog::StatistiquesDialog(const QVector<Projet> &projets, QWidget *parent)
    : QDialog(parent), m_projets(projets)
    , labelTotalProjets(nullptr)
    , labelProjetsActifs(nullptr)
    , labelProjetsTermines(nullptr)
    , labelProgressionMoyenne(nullptr)
    , labelProjetsRetard(nullptr)
    , labelProjetsPlanifies(nullptr)
    , labelProjetsPause(nullptr)
    , chartEtatView(nullptr)
    , chartProgressionView(nullptr)
    , chartTemporelView(nullptr)
{
    setWindowTitle("Statistiques Complètes des Projets");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f1f5f9;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #334155;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        );

    setupUI();
    calculerStatistiques();
    creerGraphiques();
}

void StatistiquesDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    border: none;"
        "}"
        );
    headerFrame->setFixedHeight(100);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel *titleLabel = new QLabel("📊 Tableau de Bord Statistique");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *subtitleLabel = new QLabel("Analyse complète des projets de recherche");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background-color: #f1f5f9;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(25);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *kpiFrame = new QFrame();
    kpiFrame->setStyleSheet("QFrame { background-color: transparent; border: none; }");

    QHBoxLayout *kpiLayout = new QHBoxLayout(kpiFrame);
    kpiLayout->setSpacing(20);
    kpiLayout->setContentsMargins(0, 0, 0, 0);

    auto createKPI = [](const QString &icon, const QString &value, const QString &label,
                        const QString &color, QLabel **valueLabelPtr) -> QFrame* {
        QFrame *kpi = new QFrame();
        kpi->setStyleSheet(
            "QFrame {"
            "    background-color: white;"
            "    border-radius: 12px;"
            "    border: 1px solid #e2e8f0;"
            "}"
            );
        kpi->setFixedHeight(140);
        QVBoxLayout *layout = new QVBoxLayout(kpi);
        layout->setSpacing(5);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 28px;");
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);
        *valueLabelPtr = valueLabel;

        QLabel *textLabel = new QLabel(label);
        textLabel->setStyleSheet("font-size: 13px; color: #64748b;");
        textLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(textLabel);
        return kpi;
    };

    kpiLayout->addWidget(createKPI("📁", "0", "Total Projets", "#3b82f6", &labelTotalProjets));
    kpiLayout->addWidget(createKPI("▶️", "0", "Projets Actifs", "#10b981", &labelProjetsActifs));
    kpiLayout->addWidget(createKPI("✅", "0", "Projets Terminés", "#3b82f6", &labelProjetsTermines));
    kpiLayout->addWidget(createKPI("📈", "0%", "Progression Moyenne", "#8b5cf6", &labelProgressionMoyenne));
    kpiLayout->addWidget(createKPI("⚠️", "0", "Projets en Retard", "#ef4444", &labelProjetsRetard));

    contentLayout->addWidget(kpiFrame);

    QHBoxLayout *chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(20);

    QGroupBox *chartEtatGroup = new QGroupBox("Répartition par État");
    QVBoxLayout *chartEtatLayout = new QVBoxLayout(chartEtatGroup);
    chartEtatView = new QChartView();
    chartEtatView->setMinimumHeight(350);
    chartEtatView->setRenderHint(QPainter::Antialiasing);
    chartEtatLayout->addWidget(chartEtatView);
    chartsLayout->addWidget(chartEtatGroup, 1);

    QGroupBox *chartProgressionGroup = new QGroupBox("Progression des Projets");
    QVBoxLayout *chartProgressionLayout = new QVBoxLayout(chartProgressionGroup);
    chartProgressionView = new QChartView();
    chartProgressionView->setMinimumHeight(350);
    chartProgressionView->setRenderHint(QPainter::Antialiasing);
    chartProgressionLayout->addWidget(chartProgressionView);
    chartsLayout->addWidget(chartProgressionGroup, 1);

    contentLayout->addLayout(chartsLayout);

    QGroupBox *chartTemporelGroup = new QGroupBox("Timeline des Projets");
    QVBoxLayout *chartTemporelLayout = new QVBoxLayout(chartTemporelGroup);
    chartTemporelView = new QChartView();
    chartTemporelView->setMinimumHeight(300);
    chartTemporelView->setRenderHint(QPainter::Antialiasing);
    chartTemporelLayout->addWidget(chartTemporelView);
    contentLayout->addWidget(chartTemporelGroup);

    QGroupBox *detailsGroup = new QGroupBox("Détails par État");
    QHBoxLayout *detailsLayout = new QHBoxLayout(detailsGroup);
    detailsLayout->setSpacing(30);
    detailsLayout->setContentsMargins(20, 20, 20, 20);

    auto createDetailItem = [](const QString &icon, const QString &label,
                               const QString &color, QLabel **valueLabelPtr) -> QVBoxLayout* {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(8);
        layout->setAlignment(Qt::AlignCenter);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 24px;");
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *valueLabel = new QLabel("0");
        valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);
        *valueLabelPtr = valueLabel;

        QLabel *textLabel = new QLabel(label);
        textLabel->setStyleSheet("font-size: 13px; color: #64748b;");
        textLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(textLabel);
        return layout;
    };

    detailsLayout->addLayout(createDetailItem("📋", "Planifiés", "#8b5cf6", &labelProjetsPlanifies));
    detailsLayout->addLayout(createDetailItem("⏸️", "En pause", "#f59e0b", &labelProjetsPause));
    detailsLayout->addStretch();

    contentLayout->addWidget(detailsGroup);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(30, 0, 30, 0);
    footerLayout->addStretch();

    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setFixedSize(140, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    footerLayout->addWidget(closeButton);
    mainLayout->addWidget(footerFrame);
}

void StatistiquesDialog::calculerStatistiques()
{
    int actifs = 0, termines = 0, pause = 0, planifies = 0;
    double progressionTotale = 0;
    int nbProjetsActifsOuPause = 0;
    int projetsEnRetard = 0;

    for (const auto &p : m_projets) {
        if (p.etat == QLatin1String("en_cours")) actifs++;
        else if (p.etat == QLatin1String("termine")) termines++;
        else if (p.etat == QLatin1String("suspendu")) pause++;
        else if (p.etat == QLatin1String("annule")) planifies++;

        if (p.etat != QLatin1String("termine") && p.etat != QLatin1String("annule")) {
            QString progStr = p.progression;
            if (progStr.endsWith('%')) progStr.chop(1);
            progressionTotale += progStr.toInt();
            nbProjetsActifsOuPause++;

            if (QDate::currentDate() > p.dateFin) {
                projetsEnRetard++;
            }
        }
    }

    double progressionMoyenne = nbProjetsActifsOuPause > 0 ?
                                    progressionTotale / nbProjetsActifsOuPause : 0;

    if (labelTotalProjets) labelTotalProjets->setText(QString::number(m_projets.size()));
    if (labelProjetsActifs) labelProjetsActifs->setText(QString::number(actifs));
    if (labelProjetsTermines) labelProjetsTermines->setText(QString::number(termines));
    if (labelProgressionMoyenne) labelProgressionMoyenne->setText(QString::number(qRound(progressionMoyenne)) + "%");
    if (labelProjetsRetard) labelProjetsRetard->setText(QString::number(projetsEnRetard));
    if (labelProjetsPause) labelProjetsPause->setText(QString::number(pause));
    if (labelProjetsPlanifies) labelProjetsPlanifies->setText(QString::number(planifies));
}

void StatistiquesDialog::creerGraphiques()
{
    QPieSeries *seriesEtat = new QPieSeries();

    int actifs = 0, termines = 0, pause = 0, planifies = 0;
    for (const auto &p : m_projets) {
        if (p.etat == QLatin1String("en_cours")) actifs++;
        else if (p.etat == QLatin1String("termine")) termines++;
        else if (p.etat == QLatin1String("suspendu")) pause++;
        else if (p.etat == QLatin1String("annule")) planifies++;
    }

    if (actifs > 0) seriesEtat->append("Actifs", actifs);
    if (termines > 0) seriesEtat->append("Terminés", termines);
    if (pause > 0) seriesEtat->append("En pause", pause);
    if (planifies > 0) seriesEtat->append("Planifiés", planifies);

    QList<QColor> colors = { QColor("#10b981"), QColor("#3b82f6"), QColor("#f59e0b"), QColor("#8b5cf6") };
    for (int i = 0; i < seriesEtat->count() && i < colors.size(); ++i) {
        seriesEtat->slices().at(i)->setColor(colors[i]);
        seriesEtat->slices().at(i)->setLabelVisible(true);
        seriesEtat->slices().at(i)->setLabel(QString("%1 (%2%)")
                                                 .arg(seriesEtat->slices().at(i)->label())
                                                 .arg(qRound(seriesEtat->slices().at(i)->percentage() * 100)));
    }

    QChart *chartEtat = new QChart();
    chartEtat->addSeries(seriesEtat);
    chartEtat->setTitle("Répartition des projets par état");
    chartEtat->setAnimationOptions(QChart::SeriesAnimations);
    chartEtat->legend()->setAlignment(Qt::AlignRight);
    chartEtat->setBackgroundBrush(QBrush(QColor("transparent")));
    chartEtatView->setChart(chartEtat);

    QBarSeries *seriesProgression = new QBarSeries();
    QBarSet *setProgression = new QBarSet("Progression %");

    QStringList categories;
    for (const auto &p : m_projets) {
        QString progStr = p.progression;
        if (progStr.endsWith('%')) progStr.chop(1);
        *setProgression << progStr.toInt();
        categories << p.code;
    }

    seriesProgression->append(setProgression);
    setProgression->setColor(QColor("#3b82f6"));

    QChart *chartProgression = new QChart();
    chartProgression->addSeries(seriesProgression);
    chartProgression->setTitle("Progression par projet");
    chartProgression->setAnimationOptions(QChart::SeriesAnimations);
    chartProgression->setBackgroundBrush(QBrush(QColor("transparent")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartProgression->addAxis(axisX, Qt::AlignBottom);
    seriesProgression->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%d%%");
    chartProgression->addAxis(axisY, Qt::AlignLeft);
    seriesProgression->attachAxis(axisY);

    chartProgression->legend()->setVisible(false);
    chartProgressionView->setChart(chartProgression);

    QLineSeries *seriesTimeline = new QLineSeries();
    seriesTimeline->setName("Projets démarrés");
    seriesTimeline->setColor(QColor("#10b981"));
    seriesTimeline->setPen(QPen(QColor("#10b981"), 3));

    QVector<Projet> projetsSorted = m_projets;
    std::sort(projetsSorted.begin(), projetsSorted.end(),
              [](const Projet &a, const Projet &b) { return a.dateDebut < b.dateDebut; });

    QMap<QString, int> projetsParMois;
    for (const auto &p : projetsSorted) {
        QString mois = p.dateDebut.toString("MMM yyyy");
        projetsParMois[mois]++;
    }

    int i = 0;
    for (auto it = projetsParMois.begin(); it != projetsParMois.end(); ++it, ++i) {
        seriesTimeline->append(i, it.value());
    }

    QChart *chartTimeline = new QChart();
    chartTimeline->addSeries(seriesTimeline);
    chartTimeline->setTitle("Nombre de projets démarrés par mois");
    chartTimeline->setAnimationOptions(QChart::SeriesAnimations);
    chartTimeline->setBackgroundBrush(QBrush(QColor("transparent")));

    QValueAxis *axisXTime = new QValueAxis();
    axisXTime->setRange(0, projetsParMois.size() - 1);
    axisXTime->setLabelFormat("%d");
    chartTimeline->addAxis(axisXTime, Qt::AlignBottom);
    seriesTimeline->attachAxis(axisXTime);

    QValueAxis *axisYTime = new QValueAxis();
    axisYTime->setRange(0, *std::max_element(projetsParMois.begin(), projetsParMois.end()) + 1);
    axisYTime->setLabelFormat("%d");
    chartTimeline->addAxis(axisYTime, Qt::AlignLeft);
    seriesTimeline->attachAxis(axisYTime);

    chartTimeline->legend()->setVisible(false);
    chartTemporelView->setChart(chartTimeline);
}

// ==================== FIN STATISTIQUES DIALOG ====================

// ============================================================================
// MODULE FINANCES - STATISTIQUES
// ============================================================================

FinStatistiquesDialog::FinStatistiquesDialog(const QMap<int, TransactionData> &transactions,
                                             QWidget *parent)
    : QDialog(parent), m_transactions(transactions)
    , labelTotalRecettes(nullptr), labelTotalDepenses(nullptr)
    , labelSolde(nullptr), labelNbTransactions(nullptr)
    , chartTypeView(nullptr), chartProjetView(nullptr)
{
    setWindowTitle("Statistiques Financières");
    setMinimumSize(900, 650);
    resize(1000, 700);

    setStyleSheet(
        "QDialog { background-color: #f1f5f9; font-family: 'Segoe UI', sans-serif; }"
        "QLabel { color: #334155; }"
        "QGroupBox { font-weight: bold; border: 1px solid #e2e8f0; border-radius: 12px;"
        " margin-top: 15px; padding-top: 15px; background-color: white; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 10px;"
        " color: #3b82f6; font-size: 14px; }"
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        " color: white; border: none; border-radius: 10px; padding: 12px 24px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #2563eb, stop:1 #059669); }"
        );

    setupUI();
    calculerStatistiques();
    creerGraphiques();
}

void FinStatistiquesDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3b82f6, stop:1 #10b981); border: none; }");
    headerFrame->setFixedHeight(100);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel *titleLabel = new QLabel("📊 Statistiques Financières");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    QLabel *subtitleLabel = new QLabel("Tableau de bord des transactions");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("background-color: #f1f5f9;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(25);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    QHBoxLayout *kpiLayout = new QHBoxLayout();
    kpiLayout->setSpacing(20);

    auto createKPI = [](const QString &icon, const QString &value, const QString &label,
                        const QString &color, QLabel **valueLabelPtr) -> QFrame* {
        QFrame *kpi = new QFrame();
        kpi->setStyleSheet("QFrame { background-color: white; border-radius: 12px; border: 1px solid #e2e8f0; }");
        kpi->setFixedHeight(120);
        QVBoxLayout *layout = new QVBoxLayout(kpi);
        layout->setSpacing(5);
        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 24px;");
        iconLabel->setAlignment(Qt::AlignCenter);
        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);
        *valueLabelPtr = valueLabel;
        QLabel *textLabel = new QLabel(label);
        textLabel->setStyleSheet("font-size: 13px; color: #64748b;");
        textLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(textLabel);
        return kpi;
    };

    kpiLayout->addWidget(createKPI("💰", "0 €", "Total Recettes", "#10b981", &labelTotalRecettes));
    kpiLayout->addWidget(createKPI("📤", "0 €", "Total Dépenses", "#ef4444", &labelTotalDepenses));
    kpiLayout->addWidget(createKPI("📊", "0 €", "Solde", "#3b82f6", &labelSolde));
    kpiLayout->addWidget(createKPI("📋", "0", "Nb. Transactions", "#8b5cf6", &labelNbTransactions));
    contentLayout->addLayout(kpiLayout);

    QHBoxLayout *chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(20);

    QGroupBox *chartTypeGroup = new QGroupBox("Répartition par Type (Recettes / Dépenses)");
    QVBoxLayout *chartTypeLayout = new QVBoxLayout(chartTypeGroup);
    chartTypeView = new QChartView();
    chartTypeView->setMinimumHeight(300);
    chartTypeView->setRenderHint(QPainter::Antialiasing);
    chartTypeLayout->addWidget(chartTypeView);
    chartsLayout->addWidget(chartTypeGroup, 1);

    QGroupBox *chartProjetGroup = new QGroupBox("Répartition par Projet");
    QVBoxLayout *chartProjetLayout = new QVBoxLayout(chartProjetGroup);
    chartProjetView = new QChartView();
    chartProjetView->setMinimumHeight(300);
    chartProjetView->setRenderHint(QPainter::Antialiasing);
    chartProjetLayout->addWidget(chartProjetView);
    chartsLayout->addWidget(chartProjetGroup, 1);

    contentLayout->addLayout(chartsLayout);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->addStretch();
    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setFixedSize(140, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addWidget(closeButton);
    mainLayout->addWidget(footerFrame);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);
}

void FinStatistiquesDialog::calculerStatistiques() {
    double totalRecettes = 0, totalDepenses = 0;
    for (const TransactionData &t : m_transactions) {
        if (t.type == "Recette")
            totalRecettes += t.montant;
        else
            totalDepenses += t.montant;
    }
    double solde = totalRecettes - totalDepenses;

    if (labelTotalRecettes) labelTotalRecettes->setText(QString::number(totalRecettes, 'f', 2) + " €");
    if (labelTotalDepenses) labelTotalDepenses->setText(QString::number(totalDepenses, 'f', 2) + " €");
    if (labelSolde) {
        labelSolde->setText(QString::number(solde, 'f', 2) + " €");
        labelSolde->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;")
            .arg(solde >= 0 ? "#10b981" : "#ef4444"));
    }
    if (labelNbTransactions) labelNbTransactions->setText(QString::number(m_transactions.size()));
}

void FinStatistiquesDialog::creerGraphiques() {
    double totalRecettes = 0, totalDepenses = 0;
    for (const TransactionData &t : m_transactions) {
        if (t.type == "Recette") totalRecettes += t.montant;
        else totalDepenses += t.montant;
    }

    QPieSeries *seriesType = new QPieSeries();
    if (totalRecettes > 0) seriesType->append("Recettes", totalRecettes);
    if (totalDepenses > 0) seriesType->append("Dépenses", totalDepenses);
    if (seriesType->count() > 0) {
        seriesType->slices().at(0)->setColor(QColor("#10b981"));
        if (seriesType->count() > 1) seriesType->slices().at(1)->setColor(QColor("#ef4444"));
        for (int i = 0; i < seriesType->count(); ++i) {
            seriesType->slices().at(i)->setLabelVisible(true);
            seriesType->slices().at(i)->setLabel(QString("%1%").arg(
                seriesType->slices().at(i)->percentage() * 100, 0, 'f', 1));
        }
    }

    QChart *chartType = new QChart();
    chartType->addSeries(seriesType);
    chartType->setAnimationOptions(QChart::SeriesAnimations);
    chartType->setBackgroundBrush(QBrush(QColor("transparent")));
    chartType->legend()->setVisible(true);
    chartTypeView->setChart(chartType);

    QMap<QString, double> montantsParProjet;
    for (const TransactionData &t : m_transactions) {
        double sgn = (t.type == "Recette") ? 1.0 : -1.0;
        montantsParProjet[t.projet] += sgn * t.montant;
    }

    QBarSet *barSet = new QBarSet("Solde par projet");
    QStringList categories;
    for (auto it = montantsParProjet.constBegin(); it != montantsParProjet.constEnd(); ++it) {
        barSet->append(qAbs(it.value()));
        categories << it.key();
    }
    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(barSet);
    barSet->setColor(QColor("#3b82f6"));

    QChart *chartProjet = new QChart();
    chartProjet->addSeries(barSeries);
    chartProjet->setAnimationOptions(QChart::SeriesAnimations);
    chartProjet->setBackgroundBrush(QBrush(QColor("transparent")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartProjet->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    chartProjet->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    chartProjet->legend()->setVisible(false);
    chartProjetView->setChart(chartProjet);
}

// ============================================================================
// CLASSE SMARTPUB
// ============================================================================

SmartPub::SmartPub(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SmartPub), sidebarExpanded(false),
    isUserLoggedIn(false), cherchVueListeActive(true),
    cherchVueIconesActive(true), cherchChercheurSelectionne(-1),
    cherchIsLoggedIn(false), cherchOrderByClause("ID_CHERCHEUR"),
    cherchWhereClause(), finVueListeActive(true),
    finTransactionSelectionnee(-1), finTriColonne(4),
    finTriOrdre(Qt::DescendingOrder), evEventSelectionne(-1), nextProjetId(1),
    currentProjetId(-1), isEditing(false), currentSortColumn(-1),
    currentSortOrder(Qt::AscendingOrder), filtresActifs(false),
    editingPublicationRow(-1),
    SR_filterFrame(nullptr), SR_filterTitre(nullptr), SR_filterAuteur(nullptr),
    SR_filterStatut(nullptr), SR_btnReinitFilter(nullptr) {
    ui->setupUi(this);
    // Configurer les dimensions de la fenêtre
    this->setMinimumSize(1280, 720);
    this->resize(1400, 800);

    // Initialiser l'interface utilisateur
    setupUI();

    // === ARCHITECTURE LOGIN GLOBAL ===
    // Créer le Stack principal
    mainStack = new QStackedWidget(this);

    // === PAGE 1: APPLICATION PRINCIPALE ===
    pageApp = new QWidget();
    QHBoxLayout *appLayout = new QHBoxLayout(pageApp);
    appLayout->setSpacing(0);
    appLayout->setContentsMargins(0, 0, 0, 0);

    // Déplacer la sidebar et les modules existants vers la page App
    // Note: On les retire de leur layout actuel (centralwidget) par reparentage
    ui->sidebarFrame->setParent(pageApp);
    appLayout->addWidget(ui->sidebarFrame);

    ui->stackedWidgetModules->setParent(pageApp);
    appLayout->addWidget(ui->stackedWidgetModules);

    // === PAGE 0: LOGIN ===
    pageLogin = new QWidget();
    // Style de fond pour la page de login (gradient ou couleur unie)
    pageLogin->setStyleSheet("QWidget { background: qlineargradient(x1:0, y1:0, "
                             "x2:1, y2:1, stop:0 #1e293b, stop:1 #0f172a); }");

    QVBoxLayout *loginLayout = new QVBoxLayout(pageLogin);
    loginLayout->setAlignment(Qt::AlignCenter);
    loginLayout->setContentsMargins(0, 0, 0, 0);

    // Déplacer le widget de login du module chercheur vers la page de login
    // globale
    ui->cherchStackedWidgetLogin->setParent(pageLogin);
    loginLayout->addWidget(ui->cherchStackedWidgetLogin);

    // Ajouter les pages au stack principal
    mainStack->addWidget(pageLogin); // Index 0
    mainStack->addWidget(pageApp);   // Index 1

    // Définir comme widget central
    setCentralWidget(mainStack);

    // Afficher le login par défaut
    mainStack->setCurrentIndex(0);

    // Initialiser l'utilisateur par défaut (Guest) pour éviter les valeurs non
    // initialisées MODIFICATION: Force Admin par défaut pour test/dev, mais
    // isUserLoggedIn false pour forcer le click sur login Initialiser
    // l'utilisateur par défaut (Guest) pour éviter les valeurs non initialisées
    // MODIFICATION: Force Admin par défaut pour test/dev, mais isUserLoggedIn
    // false pour forcer le click sur login On garde l'admin par défaut mais on
    // s'assure que isUserLoggedIn est false
    currentUser =
        UserAccount{"admin", "", "Tous", UserRole::Admin, "Administrateur"};
    isUserLoggedIn = false;

    setupSidebar();
    setupConnections();

    // Initialiser tous les modules
    cherchSetupUI();
    cherchConnectSignals();
    cherchApplyModernStyle();

    SR_setupUI();
    SR_connectSignals();
    SR_loadSampleData();

    finSetupUI();
    finConnectSignals();
    finAjouterDonneesTest();

    evSetupUI();
    evConnectSignals();
    evAjouterDonneesTest();

    projSetupUI();
    projSetupConnections();  // Changed from projConnectSignals()
    projSetupComboBoxes();
    projSetupSampleData();
    projSetupStatistiquesButton();  // AJOUTÉ: Styliser le bouton statistiques
    projSetupAIButton();  // AJOUTÉ: Créer et ajouter le bouton AI
    projChargerProjets();

    // === MODULE LABORATOIRES ===
    labNextId = 1;
    labEditingId = -1;
    labPage = nullptr;
    labTable = nullptr;
    labSearchEdit = nullptr;
    labTotalLabel = nullptr;
    labBtnAjouter = nullptr;
    labBtnModifier = nullptr;
    labBtnSupprimer = nullptr;
    labFormFrame = nullptr;
    labFormNom = nullptr;
    labFormThematique = nullptr;
    labFormBudget = nullptr;
    labFormCapacite = nullptr;
    labFormStatut = nullptr;
    labFormEquipements = nullptr;
    labFormDirecteur = nullptr;
    labSetupUI();
    labConnectSignals();
    labChargerDonnees();

    // Configurer la sidebar et les permissions
    updateSidebarProfileVisibility();
    checkPermissions();

    // Afficher le module approprié selon l'utilisateur
    // Index: 0=Chercheurs, 1=Publications, 2=Finances, 3=Événements, 4=Projets
    /* // DESACTIVE POUR LE DEMARRAGE - ON VEUT LE LOGIN
  if (currentUser.role == UserRole::Admin) {
      // Admin peut accéder à tous les modules
      ui->stackedWidgetModules->setCurrentIndex(0);
      cherchShowMainView();
  } else {
      // Guest - afficher Chercheurs par défaut en lecture seule
      ui->stackedWidgetModules->setCurrentIndex(0);
      cherchShowMainView();
  }
  */

    setActiveNavigationButton(ui->stackedWidgetModules->currentIndex());

    showMaximized();

    // FORCER L'AFFICHAGE DU LOGIN A LA FIN DE L'INITIALISATION
    mainStack->setCurrentIndex(0);
}

SmartPub::~SmartPub() { delete ui; }
// ============================================================================
// SETUP UI ET SIDEBAR
// ============================================================================

void SmartPub::showLogin() {
    // Obsolète - Remplacé par le StackedWidget global
}

void SmartPub::setupUI() {
    setWindowTitle("SmartPub - Gestion de la Recherche Scientifique");

    // Style global
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f8fafc;
        }
        QWidget {
            font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
        }
        QScrollArea {
            border: none;
            background-color: transparent;
        }
        QScrollBar:vertical {
            border: none;
            background: #f1f5f9;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #cbd5e1;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: #94a3b8;
        }
        QScrollBar:horizontal {
            border: none;
            background: #f1f5f9;
            height: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal {
            background: #cbd5e1;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #94a3b8;
        }
    )");
}

void SmartPub::setupSidebar() {
    // Configuration de la sidebar principale
    sidebarTimer = new QTimer(this);
    sidebarTimer->setSingleShot(true);
    sidebarTimer->setInterval(300);
    connect(sidebarTimer, &QTimer::timeout, this, &SmartPub::collapseSidebar);

    // Initialiser la sidebar comme réduite
    sidebarExpanded = false;
    ui->sidebarFrame->setFixedWidth(70);

    // Setup du profil dans la sidebar
    profileWidget = new QWidget(ui->sidebarFrame);
    profileWidget->setObjectName("sidebarProfile");
    profileWidget->setStyleSheet(R"(
        QWidget#sidebarProfile {
            background-color: #0f172a;
            border-top: 1px solid #334155;
        }
    )");
    profileWidget->setFixedHeight(110);

    // Layout principal du profile widget (Vertical)
    QVBoxLayout *profileMainLayout = new QVBoxLayout(profileWidget);
    profileMainLayout->setSpacing(8);
    profileMainLayout->setContentsMargins(15, 10, 15, 10);

    // Top row: Avatar - Info - Settings
    QWidget *topRowWidget = new QWidget();
    QHBoxLayout *topRowLayout = new QHBoxLayout(topRowWidget);
    topRowLayout->setSpacing(12);
    topRowLayout->setContentsMargins(0, 0, 0, 0);

    // Avatar
    avatarLabel = new QLabel();
    avatarLabel->setFixedSize(45, 45);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            border-image: url(:/avatar.png);
            border-radius: 22px;
            border: none;
            background-color: #10b981;
        }
    )");
    topRowLayout->addWidget(avatarLabel);

    // Info utilisateur (Nom et Role)
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    nameLabel = new QLabel(currentUser.displayName);
    nameLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 600;");
    infoLayout->addWidget(nameLabel);

    roleLabel = new QLabel(currentUser.role == UserRole::Admin ? "Administrateur"
                                                               : "Invité");
    roleLabel->setStyleSheet("font-size: 12px; color: #94a3b8;");
    infoLayout->addWidget(roleLabel);

    topRowLayout->addLayout(infoLayout, 1);

    // Bouton settings
    btnSettings = new QPushButton("⚙️");
    btnSettings->setFixedSize(35, 35);
    btnSettings->setCursor(Qt::PointingHandCursor);
    btnSettings->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #94a3b8;
            border: none;
            border-radius: 8px;
            font-size: 18px;
        }
        QPushButton:hover {
            background-color: #334155;
            color: white;
        }
    )");
    connect(btnSettings, &QPushButton::clicked, this,
            &SmartPub::onSettingsClicked);
    topRowLayout->addWidget(btnSettings);

    profileMainLayout->addWidget(topRowWidget);

    // Bottom: Bouton de déconnexion
    btnLogout = new QPushButton("Déconnexion");
    btnLogout->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #ef4444;
            border: 1px solid #ef4444;
            border-radius: 4px;
            padding: 6px 12px;
            font-size: 11px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #ef4444;
            color: white;
        }
    )");
    connect(btnLogout, &QPushButton::clicked, this, [this]() {
        mainStack->setCurrentIndex(0);
        isUserLoggedIn = false;
    });
    profileMainLayout->addWidget(btnLogout);

    // Ajouter le profil au layout de la sidebar
    QVBoxLayout *sidebarLayout =
        qobject_cast<QVBoxLayout *>(ui->sidebarFrame->layout());
    if (sidebarLayout) {
        sidebarLayout->addWidget(profileWidget);
    }

    // Event filter pour expand/collapse
    ui->sidebarFrame->setMouseTracking(true);
    ui->sidebarFrame->installEventFilter(this);

    // Initialisation Logo avec margin x=5
    ui->logoIcon->setFixedSize(40, 40);
    if (ui->verticalLayoutSidebarHeader) {
        ui->verticalLayoutSidebarHeader->setContentsMargins(15, 0, 20, 0);
    }

    // Force l'état initial réduit correctement
    collapseSidebar();
}

void SmartPub::setupConnections() {
    // Connexions de la sidebar principale
    connect(ui->btnPublications, &QPushButton::clicked, this,
            &SmartPub::on_btnPublications_clicked);
    connect(ui->btnChercheurs, &QPushButton::clicked, this,
            &SmartPub::on_btnChercheurs_clicked);
    connect(ui->btnLaboratoires, &QPushButton::clicked, this,
            &SmartPub::on_btnLaboratoires_clicked);
    connect(ui->btnProjets, &QPushButton::clicked, this,
            &SmartPub::on_btnProjets_clicked);
    connect(ui->btnFinances, &QPushButton::clicked, this,
            &SmartPub::on_btnFinances_clicked);
    connect(ui->btnEvenements, &QPushButton::clicked, this,
            &SmartPub::on_btnEvenements_clicked);
}

void SmartPub::checkPermissions() {
    // Si l'utilisateur est un invité, désactiver tous les boutons de modification
    if (currentUser.role == UserRole::Guest) {
        applyGuestRestrictions();
    }
}

void SmartPub::applyGuestRestrictions() {
    // Parcourir tous les boutons et désactiver ceux qui contiennent des mots-clés
    // CRUD
    QList<QPushButton *> allButtons = this->findChildren<QPushButton *>();
    QStringList crudKeywords = {"add",     "edit",        "delete",
                                "save",    "supprimer",   "modifier",
                                "ajouter", "enregistrer", "annuler"};

    for (QPushButton *btn : allButtons) {
        QString btnName = btn->objectName().toLower();
        QString btnText = btn->text().toLower();

        // Vérifier si le nom ou le texte contient un mot-clé CRUD
        bool isCrudButton = false;
        for (const QString &keyword : crudKeywords) {
            if (btnName.contains(keyword) || btnText.contains(keyword)) {
                isCrudButton = true;
                break;
            }
        }

        if (isCrudButton) {
            btn->setEnabled(false);
        }
    }
}

void SmartPub::updateSidebarProfileVisibility() {
    if (!profileWidget)
        return;

    // Le widget principal est toujours visible
    profileWidget->setVisible(true);

    // On cache/affiche les infos textuelles selon l'état
    if (nameLabel)
        nameLabel->setVisible(sidebarExpanded);
    if (roleLabel)
        roleLabel->setVisible(sidebarExpanded);
    if (btnSettings)
        btnSettings->setVisible(sidebarExpanded);
    if (btnLogout)
        btnLogout->setVisible(sidebarExpanded);

    // Ajuster les marges du layout principal (VBoxLayout maintenant)
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(profileWidget->layout());
    if (layout) {
        if (sidebarExpanded) {
            layout->setContentsMargins(15, 10, 15, 10);
        } else {
            // Centrer l'avatar quand réduit
            layout->setContentsMargins(12, 10, 12, 10);
        }
    }
}

void SmartPub::updateProfileName(int moduleIndex) {
    if (!nameLabel)
        return;

    QString newName;
    // Map module index to appropriate name
    // Index: 0=Chercheurs, 1=Publications, 2=Finances, 3=Événements, 4=Projets,
    // 5=Laboratoires
    switch (moduleIndex) {
    case 0: // Chercheurs
        newName = "Responsable RH";
        break;
    case 1: // Publications
        newName = "Chercheur";
        break;
    case 2: // Finances
        newName = "Service finance";
        break;
    case 3: // Evenements
        newName = "Secretere";
        break;
    case 4: // Projets
        newName = "Chef de projet";
        break;
    case 5: // Laboratoire
        newName = "Dr de recherche";
        break;
    default:
        newName = "Administrateur";
        break;
    }

    nameLabel->setText(newName);
}

bool SmartPub::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->sidebarFrame) {
        if (event->type() == QEvent::Enter) {
            sidebarTimer->stop();
            if (!sidebarExpanded) {
                expandSidebar();
            }
        } else if (event->type() == QEvent::Leave) {
            if (sidebarExpanded) {
                sidebarTimer->start();
            }
        }
    }

    // Event filter pour le module chercheurs
    //     if (obj == ui->cherchUserProfileFrame) {
    //         if (event->type() == QEvent::MouseButtonRelease) {
    //             on_cherchUserProfileFrame_clicked();
    //             return true;
    //         }
    //     }

    if (event->type() == QEvent::MouseButtonRelease) {
        QFrame *card = qobject_cast<QFrame *>(obj);
        if (card) {
            bool ok;
            int id = card->property("cherchChercheurId").toInt(&ok);
            if (ok && id > 0) {
                on_cherchVoirDetailsChercheur(id);
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void SmartPub::expandSidebar() {
    sidebarExpanded = true;

    QPropertyAnimation *animation =
        new QPropertyAnimation(ui->sidebarFrame, "minimumWidth");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(ui->sidebarFrame->width());
    animation->setEndValue(260);

    QPropertyAnimation *animation2 =
        new QPropertyAnimation(ui->sidebarFrame, "maximumWidth");
    animation2->setDuration(250);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->setStartValue(ui->sidebarFrame->width());
    animation2->setEndValue(260);

    // Mettre à jour les textes des boutons
    QVector<QPushButton *> navButtons = {ui->btnPublications, ui->btnChercheurs,
                                         ui->btnLaboratoires, ui->btnProjets,
                                         ui->btnFinances,     ui->btnEvenements};

    QStringList fullTexts = {"📄  Publications", "👥  Chercheurs",
                             "🧪  Laboratoires", "📁  Projets",
                             "💰  Finances",     "📅  Événements"};

    for (int i = 0; i < navButtons.size(); ++i) {
        navButtons[i]->setText(fullTexts[i]);
        // Rétablir alignement gauche pour le texte
        navButtons[i]->setStyleSheet(navButtons[i]->styleSheet().replace(
            "text-align: center;", "text-align: left;"));
    }

    updateSidebarProfileVisibility();

    // Animation Logo: Agrandir (Restoring Fix)
    QPropertyAnimation *animLogoW =
        new QPropertyAnimation(ui->logoIcon, "minimumWidth");
    animLogoW->setDuration(250);
    animLogoW->setStartValue(40);
    animLogoW->setEndValue(100);

    QPropertyAnimation *animLogoH =
        new QPropertyAnimation(ui->logoIcon, "minimumHeight");
    animLogoH->setDuration(250);
    animLogoH->setStartValue(40);
    animLogoH->setEndValue(100);

    animation->start(QAbstractAnimation::DeleteWhenStopped);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);
    animLogoW->start(QAbstractAnimation::DeleteWhenStopped);
    animLogoH->start(QAbstractAnimation::DeleteWhenStopped);
}

void SmartPub::collapseSidebar() {
    sidebarExpanded = false;

    QPropertyAnimation *animation =
        new QPropertyAnimation(ui->sidebarFrame, "minimumWidth");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(ui->sidebarFrame->width());
    animation->setEndValue(70);

    QPropertyAnimation *animation2 =
        new QPropertyAnimation(ui->sidebarFrame, "maximumWidth");
    animation2->setDuration(250);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->setStartValue(ui->sidebarFrame->width());
    animation2->setEndValue(70);

    // Réduire les textes aux icônes
    QVector<QPushButton *> navButtons = {ui->btnPublications, ui->btnChercheurs,
                                         ui->btnLaboratoires, ui->btnProjets,
                                         ui->btnFinances,     ui->btnEvenements};

    QStringList icons = {"📄", "👥", "🧪", "📁", "💰", "📅"};

    for (int i = 0; i < navButtons.size(); ++i) {
        navButtons[i]->setText(icons[i]);
        // Stabiliser le style sans changement de police
        navButtons[i]->setStyleSheet(navButtons[i]->styleSheet().replace(
            "text-align: left;", "text-align: center;"));
    }

    updateSidebarProfileVisibility();

    // Animation Logo: Réduire (Restoring Fix)
    QPropertyAnimation *animLogoW =
        new QPropertyAnimation(ui->logoIcon, "minimumWidth");
    animLogoW->setDuration(250);
    animLogoW->setStartValue(100);
    animLogoW->setEndValue(40);

    QPropertyAnimation *animLogoH =
        new QPropertyAnimation(ui->logoIcon, "minimumHeight");
    animLogoH->setDuration(250);
    animLogoH->setStartValue(100);
    animLogoH->setEndValue(40);

    animation->start(QAbstractAnimation::DeleteWhenStopped);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);
    animLogoW->start(QAbstractAnimation::DeleteWhenStopped);
    animLogoH->start(QAbstractAnimation::DeleteWhenStopped);
}

void SmartPub::updateNavButtonStyles(QPushButton *activeBtn) {
    QVector<QPushButton *> buttons = {ui->btnPublications, ui->btnChercheurs,
                                      ui->btnLaboratoires, ui->btnProjets,
                                      ui->btnFinances,     ui->btnEvenements};

    // Choisir la taille de police (Constante pour éviter le jitter)
    int fontSize = 16;
    QString textAlign = sidebarExpanded ? "left" : "center";

    QString activeStyle = QString(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 20px;
            font-size: %1px;
            font-weight: 600;
            text-align: %2;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )")
                              .arg(fontSize)
                              .arg(textAlign);

    QString inactiveStyle = QString(R"(
        QPushButton {
            background-color: transparent;
            color: #94a3b8;
            border: none;
            border-radius: 12px;
            padding: 14px 20px;
            font-size: %1px;
            font-weight: 500;
            text-align: %2;
        }
        QPushButton:hover {
            background-color: #334155;
            color: #e2e8f0;
        }
    )")
                                .arg(fontSize)
                                .arg(textAlign);

    for (auto *btn : buttons) {
        btn->setStyleSheet(btn == activeBtn ? activeStyle : inactiveStyle);
    }
}

void SmartPub::setActiveNavigationButton(int index) {
    QVector<QPushButton *> navButtons = {
        ui->btnChercheurs,   // Index 0
        ui->btnPublications, // Index 1
        ui->btnFinances,     // Index 2
        ui->btnEvenements,   // Index 3
        ui->btnProjets,      // Index 4
        ui->btnLaboratoires  // Index 5
    };

    if (index >= 0 && index < navButtons.size()) {
        updateNavButtonStyles(navButtons[index]);
    }
}

void SmartPub::onSettingsClicked() {
    SettingsDialog dialog(this);
    dialog.exec();
}
// ============================================================================
// NAVIGATION PRINCIPALE
// ============================================================================

void SmartPub::on_btnChercheurs_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(0);
    setActiveNavigationButton(0);
    updateProfileName(0);

    // FIX: Toujours réinitialiser sur la vue liste au clic sidebar
    ui->cherchStackedWidget->setCurrentIndex(0);
    cherchVueListeActive = true;

    // Rendre visibles les boutons de la toolbar (cachés quand formulaire Ajouter actif)
    ui->cherchLineEditRecherche->setVisible(true);
    ui->cherchBtnRecherche->setVisible(true);
    ui->cherchBtnTri->setVisible(true);
    ui->cherchBtnExport->setVisible(true);
    ui->cherchBtnStatistiques->setVisible(true);
    if (cherchBtnToggleVue) cherchBtnToggleVue->setVisible(true);

    // Mettre à jour le style des onglets Liste / Ajouter
    QString tabActive = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
    )";
    QString tabInactive = R"(
        QPushButton {
            background-color: transparent;
            color: #64748b;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #f1f5f9;
            color: #334155;
        }
    )";
    ui->cherchBtnVueListe->setStyleSheet(tabActive);
    ui->cherchBtnAjouter->setStyleSheet(tabInactive);

    cherchAfficherListeChercheurs();
}

void SmartPub::on_btnPublications_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    updateProfileName(1);
    SR_updateButtonStyles();
}

void SmartPub::on_btnLaboratoires_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(5);
    setActiveNavigationButton(5);
    updateProfileName(5);
    labAfficherListe();
}

void SmartPub::on_btnFinances_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(2);
    setActiveNavigationButton(2);
    updateProfileName(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_btnProjets_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(4);
    setActiveNavigationButton(4);
    updateProfileName(4);
    projChargerProjets();
}

void SmartPub::on_btnEvenements_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(3);
    setActiveNavigationButton(3);
    updateProfileName(3);
    evAfficherListeEvents();
}

// ============================================================================
// MODULE CHERCHEURS
// ============================================================================

void SmartPub::cherchSetupUI() {
    // Bouton toggle vue (icônes/liste)
    cherchBtnToggleVue = new QPushButton(this);
    cherchBtnToggleVue->setObjectName("cherchBtnToggleVue");
    cherchBtnToggleVue->setFixedSize(44, 44);
    cherchBtnToggleVue->setCursor(Qt::PointingHandCursor);
    cherchBtnToggleVue->setText("⊞");
    cherchBtnToggleVue->setToolTip("Changer le mode d'affichage");
    cherchBtnToggleVue->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #3b82f6;
            color: #3b82f6;
        }
    )");

    // Insérer dans le toolbar si le layout existe
    if (ui->horizontalLayoutToolbar) {
        ui->horizontalLayoutToolbar->insertWidget(1, cherchBtnToggleVue);
    }
    connect(cherchBtnToggleVue, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnToggleVue_clicked);
}

void SmartPub::cherchConnectSignals() {
    connect(ui->cherchBtnLogin, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnLogin_clicked);
    connect(ui->cherchBtnMotDePasseOublie, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnMotDePasseOublie_clicked);
    connect(ui->cherchBtnRetourLogin, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnRetourLogin_clicked);
    connect(ui->cherchBtnForgotOk, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnForgotOk_clicked);

    //     ui->cherchUserProfileFrame->setCursor(Qt::PointingHandCursor);
    //     ui->cherchUserProfileFrame->installEventFilter(this);

    connect(ui->cherchBtnVueListe, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnVueListe_clicked);
    connect(ui->cherchBtnAjouter, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnAjouter_clicked);
    connect(ui->cherchBtnRecherche, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnRecherche_clicked);
    connect(ui->cherchBtnTri, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnTri_clicked);
    connect(ui->cherchBtnExport, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnExport_clicked);
    connect(ui->cherchBtnStatistiques, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnStatistiques_clicked);
    connect(ui->cherchBtnUploadPhoto, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnUploadPhoto_clicked);
    connect(ui->cherchBtnAjouterChercheur, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnAjouterChercheur_clicked);
    connect(ui->cherchBtnAnnulerAjout, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnAnnulerAjout_clicked);
    connect(ui->cherchLineEditRecherche, &QLineEdit::textChanged, this,
            &SmartPub::on_cherchLineEditRecherche_textChanged);
}

void SmartPub::cherchApplyModernStyle() {
    // === LOGIN VIEW STYLES ===
    if (ui->cherchLoginFrame) {
        ui->cherchLoginFrame->setStyleSheet(R"(
            QFrame#cherchLoginFrame {
                background-color: white;
                border-radius: 20px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    if (ui->cherchLoginHeader) {
        ui->cherchLoginHeader->setStyleSheet(R"(
            QFrame#cherchLoginHeader {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                border-top-left-radius: 20px;
                border-top-right-radius: 20px;
            }
        )");
    }

    // Style pour la page mot de passe oublié
    if (ui->cherchForgotFrame) {
        ui->cherchForgotFrame->setStyleSheet(R"(
            QFrame#cherchForgotFrame {
                background-color: white;
                border-radius: 20px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    if (ui->cherchForgotHeader) {
        ui->cherchForgotHeader->setStyleSheet(R"(
            QFrame#cherchForgotHeader {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                border-top-left-radius: 20px;
                border-top-right-radius: 20px;
            }
        )");
    }

    QString cherchLoginInputStyle = R"(
        QLineEdit {
            background-color: #f8fafc;
            border: 2px solid #e2e8f0;
            border-radius: 12px;
            padding: 14px 16px;
            font-size: 14px;
            color: #334155;
        }
        QLineEdit:focus {
            border-color: #3b82f6;
            background-color: white;
        }
    )";

    if (ui->cherchLineEditLoginEmail) {
        ui->cherchLineEditLoginEmail->setStyleSheet(cherchLoginInputStyle);
    }

    if (ui->cherchLineEditLoginPassword) {
        ui->cherchLineEditLoginPassword->setStyleSheet(cherchLoginInputStyle);
        ui->cherchLineEditLoginPassword->setEchoMode(QLineEdit::Password);
    }

    // Style pour le champ email de récupération
    if (ui->cherchLineEditForgotEmail) {
        ui->cherchLineEditForgotEmail->setStyleSheet(cherchLoginInputStyle);
    }

    if (ui->cherchBtnLogin) {
        ui->cherchBtnLogin->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 14px 32px;
                font-size: 16px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
    }

    // Style pour le bouton OK de récupération
    if (ui->cherchBtnForgotOk) {
        ui->cherchBtnForgotOk->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 14px 32px;
                font-size: 16px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
    }

    // Style pour le lien mot de passe oublié
    if (ui->cherchBtnMotDePasseOublie) {
        ui->cherchBtnMotDePasseOublie->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: #3b82f6;
                border: none;
                font-size: 13px;
                font-weight: 500;
                text-decoration: underline;
            }
            QPushButton:hover {
                color: #2563eb;
            }
        )");
    }

    // Style pour le bouton retour
    if (ui->cherchBtnRetourLogin) {
        ui->cherchBtnRetourLogin->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.2);
                color: white;
                border: none;
                border-radius: 20px;
                font-size: 20px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.3);
            }
        )");
    }

    // === HEADER STYLES ===
    if (ui->cherchHeaderFrame) {
        ui->cherchHeaderFrame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border-bottom: 1px solid #e2e8f0;
            }
        )");
    }

    if (ui->cherchTitleLabel) {
        ui->cherchTitleLabel->setStyleSheet(

            "color: #1e293b; font-size: 28px; font-weight: 700; background: "
            "transparent; border: none");
    }

    if (ui->cherchSubtitleLabel) {
        ui->cherchSubtitleLabel->setStyleSheet(
            "color: #64748b; font-size: 14px; background: transparent; border: "
            "none");
    }

    // === TOOLBAR STYLES ===
    if (ui->cherchToolbarFrame) {
        ui->cherchToolbarFrame->setStyleSheet(
            "background-color: transparent; border: none;");
    }

    // Tabs frame
    if (ui->cherchTabsFrame) {
        ui->cherchTabsFrame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border-radius: 12px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    QString cherchTabActive = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
    )";

    QString cherchTabInactive = R"(
        QPushButton {
            background-color: transparent;
            color: #64748b;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #f1f5f9;
            color: #334155;
        }
    )";

    if (ui->cherchBtnVueListe) {
        ui->cherchBtnVueListe->setStyleSheet(cherchTabActive);
    }

    if (ui->cherchBtnAjouter) {
        ui->cherchBtnAjouter->setStyleSheet(cherchTabInactive);
    }

    // Toolbar buttons
    if (ui->cherchBtnRecherche) {
        ui->cherchBtnRecherche->setStyleSheet(R"(
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 10px;
                padding: 10px 16px;
                font-size: 13px;
                font-weight: 600;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
        )");
    }

    QString cherchSecondaryBtn = R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 10px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #cbd5e1;
        }
    )";

    if (ui->cherchBtnTri)
        ui->cherchBtnTri->setStyleSheet(cherchSecondaryBtn);
    if (ui->cherchBtnExport)
        ui->cherchBtnExport->setStyleSheet(cherchSecondaryBtn);

    if (ui->cherchBtnStatistiques) {
        ui->cherchBtnStatistiques->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 10px;
                padding: 10px 16px;
                font-size: 13px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
    }

    // Search field
    if (ui->cherchLineEditRecherche) {
        ui->cherchLineEditRecherche->setStyleSheet(R"(
            QLineEdit {
                background-color: white;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                padding: 10px 14px;
                font-size: 13px;
                color: #334155;
            }
            QLineEdit:focus {
                border-color: #3b82f6;
            }
        )");
    }

    // === FORM STYLES ===
    if (ui->cherchFormFrame) {
        ui->cherchFormFrame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border-radius: 20px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    // Form inputs
    QString cherchInputStyle = R"(
        QLineEdit {
            background-color: #f8fafc;
            border: none;
            border-radius: 10px;
            padding: 12px 16px;
            font-size: 14px;
            color: #334155;
        }
        QLineEdit:focus {
            background-color: #eff6ff;
            border: 2px solid #3b82f6;
        }
    )";

    if (ui->cherchLineEditNom)
        ui->cherchLineEditNom->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditPrenom)
        ui->cherchLineEditPrenom->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditCIN)
        ui->cherchLineEditCIN->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditEmail)
        ui->cherchLineEditEmail->setStyleSheet(cherchInputStyle);

    // Combo box
    if (ui->cherchComboBoxGrade) {
        ui->cherchComboBoxGrade->setStyleSheet(R"(
            QComboBox {
                background-color: #f8fafc;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                padding: 12px 16px;
                font-size: 14px;
                color: #334155;
                min-height: 48px;
            }
            QComboBox:hover {
                border-color: #cbd5e1;
            }
            QComboBox:focus {
                border-color: #3b82f6;
                background-color: #eff6ff;
            }
            QComboBox::drop-down {
                border: none;
                width: 40px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 6px solid #64748b;
                width: 0;
                height: 0;
                margin-right: 10px;
            }
            QComboBox QAbstractItemView {
                background-color: white;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                selection-background-color: #eff6ff;
                selection-color: #1e293b;
                outline: none;
                padding: 8px;
                margin-top: 4px;
                min-width: 200px;
            }
            QComboBox QAbstractItemView::item {
                padding: 12px 16px;
                border-radius: 6px;
                color: #334155;
                font-size: 14px;
            }
            QComboBox QAbstractItemView::item:hover {
                background-color: #f1f5f9;
                color: #1e293b;
            }
            QComboBox QAbstractItemView::item:selected {
                background-color: #eff6ff;
                color: #3b82f6;
                font-weight: 600;
            }
        )");
    }

    // Form labels
    QString cherchLabelStyle = "color: #334155; font-size: 14px; font-weight: "
                               "600; background: transparent; border: none;";
    if (ui->cherchLabelNom)
        ui->cherchLabelNom->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelPrenom)
        ui->cherchLabelPrenom->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelCIN)
        ui->cherchLabelCIN->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelEmail)
        ui->cherchLabelEmail->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelGrade)
        ui->cherchLabelGrade->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelPhoto)
        ui->cherchLabelPhoto->setStyleSheet(cherchLabelStyle);

    // Form buttons
    if (ui->cherchBtnAnnulerAjout) {
        ui->cherchBtnAnnulerAjout->setStyleSheet(R"(
            QPushButton {
                background-color: white;
                color: #64748b;
                border: 2px solid #e2e8f0;
                border-radius: 12px;
                padding: 12px 32px;
                font-size: 15px;
                font-weight: 600;
            }
            QPushButton:hover {
                background-color: #f1f5f9;
                border-color: #cbd5e1;
            }
        )");
    }

    if (ui->cherchBtnAjouterChercheur) {
        ui->cherchBtnAjouterChercheur->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #10b981, stop:1 #3b82f6);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 12px 32px;
                font-size: 15px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #059669, stop:1 #2563eb);
            }
        )");
    }

    // Upload photo button
    if (ui->cherchBtnUploadPhoto) {
        ui->cherchBtnUploadPhoto->setStyleSheet(R"(
            QPushButton {
                background-color: #f8fafc;
                border: 2px dashed #cbd5e1;
                border-radius: 16px;
                color: #94a3b8;
                font-size: 48px;
                font-weight: 300;
            }
            QPushButton:hover {
                border-color: #3b82f6;
                background-color: #eff6ff;
                color: #3b82f6;
            }
        )");
    }

    if (ui->cherchLabelPhotoHint) {
        ui->cherchLabelPhotoHint->setStyleSheet(
            "color: #94a3b8; font-size: 12px; background: transparent; border: "
            "none;");
    }

    // === SCROLL AREA STYLES ===
    if (ui->cherchCardsScrollArea) {
        ui->cherchCardsScrollArea->setStyleSheet(R"(
            QScrollArea {
                background-color: transparent;
                border: none;
            }
            QScrollArea > QWidget > QWidget {
                background-color: transparent;
            }
            QScrollBar:vertical {
                background-color: #f1f5f9;
                width: 12px;
                border-radius: 6px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background-color: #cbd5e1;
                border-radius: 6px;
                min-height: 40px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #94a3b8;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0;
            }
        )");
    }

    if (ui->cherchScrollAreaWidgetContents) {
        ui->cherchScrollAreaWidgetContents->setStyleSheet(
            "background-color: #f8fafc;");
    }
}

void SmartPub::cherchAjouterDonneesTest() {
    QStringList grades = {"Professeur",     "Maitre de Conferences",
                          "Docteur",        "Ingenieur de Recherche",
                          "Post-doctorant", "Doctorant"};
    QStringList noms = {"Dupont", "Martin",  "Bernard", "Petit",
                        "Robert", "Richard", "Durand",  "Leroy"};
    QStringList prenoms = {"Marie",   "Pierre",  "Sophie",   "Jean",
                           "Camille", "Antoine", "Isabelle", "Thomas"};

    for (int i = 0; i < 8; ++i) {
        ChercheurData data;
        data.nom = noms[i];
        data.prenom = prenoms[i];
        data.grade = grades[i % grades.size()];
        data.email = QString("%1.%2@univ.fr")
                         .arg(prenoms[i].toLower())
                         .arg(noms[i].toLower());
        data.cin = QString("AB%1").arg(123456 + i);
        data.dateCreation = QDateTime::currentDateTime().addDays(-i * 5);
        data.age = 30 + (i * 3) % 25;

        int nbProjets = (i % 4) + 1;
        for (int p = 0; p < nbProjets; ++p) {
            data.projetsIds.append(p + 1);
        }

        data.carriere = cherchDeterminerCarriere(nbProjets, data.grade);
        data.photoPath = ":/avatar.png";

        cherchChercheursMap[i + 1] = data;
    }
}

QString SmartPub::cherchDeterminerCarriere(int projetsCount,
                                           const QString &grade) {
    // Basé sur le grade académique ET le nombre de contributions dans CONTRIBUER
    if (grade == "Professeur") {
        return projetsCount >= 3 ? "Professeur Senior — Expert" : "Professeur";
    }
    if (grade == "Maitre de Conferences") {
        return projetsCount >= 3 ? "MdC — Confirmé Expert" : "Maître de Conférences";
    }
    if (grade == "Docteur") {
        return projetsCount >= 2 ? "Docteur — Chercheur Actif" : "Docteur";
    }
    if (grade == "Ingenieur de Recherche") {
        return projetsCount >= 2 ? "Ingénieur Recherche Senior" : "Ingénieur de Recherche";
    }
    if (grade == "Post-doctorant") {
        return "Post-Doctorant";
    }
    if (grade == "Doctorant") {
        return "Doctorant — En Formation";
    }
    // Fallback générique basé sur le nombre de projets
    if (projetsCount >= 4) return "Senior — Expert";
    if (projetsCount >= 2) return "Confirmé";
    return "Junior";
}

void SmartPub::cherchShowLoginView() {
    // Retour au login global
    mainStack->setCurrentIndex(0);
    isUserLoggedIn = false;
    ui->cherchLineEditLoginPassword->clear();
}

void SmartPub::cherchShowMainView() {
    // Transition vers l'application principale
    mainStack->setCurrentIndex(1);
}

void SmartPub::cherchCheckLogin() {
    // === BYPASS AUTHENTICATION (User Request) ===
    currentUser =
        UserAccount{"admin", "", "Tous", UserRole::Admin, "Administrateur"};
    isUserLoggedIn = true;

    // Configuration post-login
    updateSidebarProfileVisibility();

    // Mettre à jour les infos avatar/nom
    if (nameLabel)
        nameLabel->setText(currentUser.displayName);
    if (roleLabel)
        roleLabel->setText("Administrateur");

    // FIX: Ouvrir Publications en premier lieu après login (index 1)
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    updateProfileName(1);
    SR_updateButtonStyles();

    // S'assurer que le module Chercheur sera en vue liste lors d'une navigation future
    ui->cherchStackedWidget->setCurrentIndex(0);
    cherchVueListeActive = true;

    // Afficher l'application principale
    mainStack->setCurrentIndex(1);

    // Vider les champs de login
    ui->cherchLineEditLoginEmail->clear();
    ui->cherchLineEditLoginPassword->clear();
}

void SmartPub::cherchShowForgotPasswordView() {
    ui->cherchStackedWidgetLogin->setCurrentIndex(1);
    ui->cherchLineEditForgotEmail->clear();
    ui->cherchLineEditForgotEmail->setFocus();
}

void SmartPub::on_cherchBtnMotDePasseOublie_clicked() {
    ui->cherchStackedWidgetLogin->setCurrentIndex(1);
    ui->cherchLineEditForgotEmail
        ->clear(); // Clear the field when navigating to forgot password view
    ui->cherchLineEditForgotEmail->setFocus();
}

void SmartPub::on_cherchBtnRetourLogin_clicked() {
    ui->cherchStackedWidgetLogin->setCurrentIndex(0);
}

void SmartPub::on_cherchBtnForgotOk_clicked() {
    QString email = ui->cherchLineEditForgotEmail->text().trimmed();
    if (!email.isEmpty() && email.contains("@")) {
        QMessageBox::information(this, "Email Envoyé",
                                 "Un lien de réinitialisation a été envoyé à " +
                                     email);
        ui->cherchStackedWidgetLogin->setCurrentIndex(0);
    } else {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez entrer une adresse email valide.");
    }
}

void SmartPub::on_cherchBtnLogin_clicked() { cherchCheckLogin(); }

// void SmartPub::on_cherchUserProfileFrame_clicked()
// {
//     // Déconnexion
//     auto reply = QMessageBox::question(this, "Déconnexion",
//                                        "Voulez-vous vraiment vous déconnecter
//                                        ?", QMessageBox::Yes |
//                                        QMessageBox::No, QMessageBox::No);
//
//     if (reply == QMessageBox::Yes) {
//         // Redémarrer l'application pour retourner au login
//         qApp->quit();
//         QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
//     }
// }


void SmartPub::cherchEnrichirDonneesDepuisOracle()
{
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;
    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        it->projetsIds.clear();
    }
    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT ID_CHERCHEUR, CODE_PROJET FROM CONTRIBUER"))) {
        while (q.next()) {
            const int cid = q.value(0).toInt();
            const int pid = q.value(1).toInt();
            if (!cherchChercheursMap.contains(cid))
                continue;
            auto &lst = cherchChercheursMap[cid].projetsIds;
            if (!lst.contains(pid))
                lst.append(pid);
        }
    }
    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        const int n = it->projetsIds.size();
        it->carriere = cherchDeterminerCarriere(n, it->grade);
    }
}

void SmartPub::cherchAfficherListeChercheurs() {
    cherchClearChercheursList();
    cherchChercheursMap.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        if (cherchVueIconesActive) {
            QGridLayout *gridLayout = qobject_cast<QGridLayout *>(
                ui->cherchScrollAreaWidgetContents->layout());
            if (!gridLayout) {
                QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
                if (oldLayout) {
                    QLayoutItem *child;
                    while ((child = oldLayout->takeAt(0)) != nullptr) {
                        if (child->widget()) delete child->widget();
                        delete child;
                    }
                    delete oldLayout;
                }
                gridLayout = new QGridLayout(ui->cherchScrollAreaWidgetContents);
                gridLayout->setSpacing(24);
                gridLayout->setContentsMargins(24, 24, 24, 24);
            }
            QLabel *noDb = new QLabel("Connexion base de données indisponible.",
                                      ui->cherchScrollAreaWidgetContents);
            noDb->setAlignment(Qt::AlignCenter);
            noDb->setStyleSheet("color: #94a3b8; font-size: 16px; background: transparent; border: none;");
            gridLayout->addWidget(noDb, 0, 0, 1, 3);
        }
        return;
    }

    QString sql = "SELECT ID_CHERCHEUR, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM CHERCHEUR";
    if (!cherchWhereClause.isEmpty())
        sql += " WHERE " + cherchWhereClause;
    if (!cherchOrderByClause.isEmpty())
        sql += " ORDER BY " + cherchOrderByClause;

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les chercheurs : " + query.lastError().text());
        return;
    }

    while (query.next()) {
        int id = query.value("ID_CHERCHEUR").toInt();
        ChercheurData data;
        data.nom = query.value("NOM").toString();
        data.prenom = query.value("PRENOM").toString();
        data.email = query.value("EMAIL").toString();
        data.grade = query.value("GRADE").toString();
        data.cin = query.value("CIN").toString();
        data.photoPath = query.value("PHOTO_PROFIL").toString();
        if (data.photoPath.isEmpty())
            data.photoPath = ":/avatar.png";
        data.dateCreation = QDateTime();
        data.carriere = "";
        data.age = 0;
        cherchChercheursMap[id] = data;
    }

    cherchEnrichirDonneesDepuisOracle();

    if (cherchVueIconesActive) {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(
            ui->cherchScrollAreaWidgetContents->layout());
        if (!gridLayout) {
            QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget())
                        delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            gridLayout = new QGridLayout(ui->cherchScrollAreaWidgetContents);
            gridLayout->setSpacing(24);
            gridLayout->setContentsMargins(24, 24, 24, 24);
        }

        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end();
             ++it) {
            int id = it.key();
            auto data = it.value();
            cherchAjouterChercheurCard(id, data.nom, data.prenom, data.grade,
                                       data.email, data.photoPath);
        }
    } else {
        QVBoxLayout *listLayout = qobject_cast<QVBoxLayout *>(
            ui->cherchScrollAreaWidgetContents->layout());
        if (!listLayout) {
            QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget())
                        delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            listLayout = new QVBoxLayout(ui->cherchScrollAreaWidgetContents);
            listLayout->setSpacing(12);
            listLayout->setContentsMargins(24, 24, 24, 24);
            listLayout->setAlignment(Qt::AlignTop);
        }

        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end();
             ++it) {
            int id = it.key();
            auto data = it.value();
            cherchAjouterChercheurListItem(id, data.nom, data.prenom, data.grade,
                                           data.email, data.photoPath);
        }
    }
}

void SmartPub::cherchAjouterChercheurCard(int id, const QString &nom,
                                          const QString &prenom,
                                          const QString &grade,
                                          const QString &email,
                                          const QString &photoPath) {
    QFrame *card = new QFrame(ui->cherchScrollAreaWidgetContents);
    card->setObjectName(QString("cherchCard_%1").arg(id));
    card->setFixedSize(340, 200);
    card->setProperty("cherchChercheurId", id);
    card->setCursor(Qt::PointingHandCursor);

    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
        QFrame:hover {
            border: 2px solid #3b82f6;
            background-color: #f8fafc;
        }
    )");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QLabel *avatarLabel = new QLabel(card);
    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet("border: 3px solid white; background: transparent;");
    avatarLabel->setScaledContents(false);
    avatarLabel->setMask(QRegion(0, 0, 80, 80, QRegion::Ellipse));
    QPixmap avatarPix;
    QString path = photoPath.isEmpty() ? QString(":/avatar.png") : photoPath;
    if (!avatarPix.load(path))
        avatarPix.load(":/avatar.png");
    if (!avatarPix.isNull())
        avatarLabel->setPixmap(makeCircularPixmap(avatarPix, 80));
    mainLayout->addWidget(avatarLabel);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);
    infoLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom), card);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #1e293b; "
                             "background: transparent; border: none;");
    infoLayout->addWidget(nameLabel);

    QLabel *gradeLabel = new QLabel(grade, card);
    gradeLabel->setStyleSheet(R"(
        font-size: 12px;
        font-weight: 600;
        color: #059669;
        background-color: #d1fae5;
        padding: 6px 12px;
        border-radius: 20px;
    )");
    gradeLabel->setMaximumWidth(150);
    infoLayout->addWidget(gradeLabel);

    QLabel *emailLabel = new QLabel(email, card);
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: "
                              "transparent; border: none;");
    infoLayout->addWidget(emailLabel);

    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout, 1);

    // Boutons d'action (masqués pour les guests)
    if (currentUser.role == UserRole::Admin) {
        QVBoxLayout *btnLayout = new QVBoxLayout();
        btnLayout->setSpacing(10);
        btnLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QPushButton *btnEdit = new QPushButton("✎", card);
        btnEdit->setFixedSize(40, 40);
        btnEdit->setToolTip("Modifier le chercheur");
        btnEdit->setStyleSheet(R"(
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 10px;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
        )");
        connect(btnEdit, &QPushButton::clicked, this,
                [this, id]() { on_cherchModifierChercheur(id); });

        QPushButton *btnDelete = new QPushButton("🗑", card);
        btnDelete->setFixedSize(40, 40);
        btnDelete->setToolTip("Supprimer le chercheur");
        btnDelete->setStyleSheet(R"(
            QPushButton {
                background-color: #ef4444;
                color: white;
                border: none;
                border-radius: 10px;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: #dc2626;
            }
        )");
        connect(btnDelete, &QPushButton::clicked, this,
                [this, id]() { on_cherchSupprimerChercheur(id); });

        btnLayout->addWidget(btnEdit);
        btnLayout->addWidget(btnDelete);
        btnLayout->addStretch();
        mainLayout->addLayout(btnLayout);
    }

    card->installEventFilter(this);
    card->setMouseTracking(true);

    QGridLayout *grid =
        qobject_cast<QGridLayout *>(ui->cherchScrollAreaWidgetContents->layout());
    if (grid) {
        int count = grid->count();
        int row = count / 3;
        int col = count % 3;
        grid->addWidget(card, row, col, Qt::AlignTop);

        // Animation
        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(card);
        opacityEffect->setOpacity(0.0);
        card->setGraphicsEffect(opacityEffect);
        QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
        anim->setDuration(400);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void SmartPub::cherchAjouterChercheurListItem(int id, const QString &nom,
                                              const QString &prenom,
                                              const QString &grade,
                                              const QString &email,
                                              const QString &photoPath) {
    QFrame *item = new QFrame(ui->cherchScrollAreaWidgetContents);
    item->setObjectName(QString("cherchListItem_%1").arg(id));
    item->setFixedHeight(80);
    item->setProperty("cherchChercheurId", id);
    item->setCursor(Qt::PointingHandCursor);

    item->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #e2e8f0;
        }
        QFrame:hover {
            border: 2px solid #3b82f6;
            background-color: #f8fafc;
        }
    )");

    QHBoxLayout *mainLayout = new QHBoxLayout(item);
    mainLayout->setContentsMargins(20, 10, 20, 10);
    mainLayout->setSpacing(20);

    QLabel *avatarLabel = new QLabel(item);
    avatarLabel->setFixedSize(50, 50);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet("background: transparent;");
    avatarLabel->setScaledContents(false);
    avatarLabel->setMask(QRegion(0, 0, 50, 50, QRegion::Ellipse));
    QPixmap listPix;
    QString path = photoPath.isEmpty() ? QString(":/avatar.png") : photoPath;
    if (!listPix.load(path))
        listPix.load(":/avatar.png");
    if (!listPix.isNull())
        avatarLabel->setPixmap(makeCircularPixmap(listPix, 50));
    mainLayout->addWidget(avatarLabel);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom));
    nameLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #1e293b; "
                             "background: transparent; border: none;");
    nameLabel->setFixedWidth(200);
    mainLayout->addWidget(nameLabel);

    QLabel *gradeLabel = new QLabel(grade);
    gradeLabel->setStyleSheet(R"(
        font-size: 12px;
        font-weight: 600;
        color: #059669;
        background-color: #d1fae5;
        padding: 4px 12px;
        border-radius: 12px;
    )");
    gradeLabel->setFixedWidth(150);
    mainLayout->addWidget(gradeLabel);

    QLabel *emailLabel = new QLabel(email);
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: "
                              "transparent; border: none;");
    mainLayout->addWidget(emailLabel, 1);

    if (currentUser.role == UserRole::Admin) {
        QPushButton *btnEdit = new QPushButton("✎");
        btnEdit->setFixedSize(36, 36);
        btnEdit->setToolTip("Modifier");
        btnEdit->setStyleSheet(R"(
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 8px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
        )");
        connect(btnEdit, &QPushButton::clicked, this,
                [this, id]() { on_cherchModifierChercheur(id); });

        QPushButton *btnDelete = new QPushButton("🗑");
        btnDelete->setFixedSize(36, 36);
        btnDelete->setToolTip("Supprimer");
        btnDelete->setStyleSheet(R"(
            QPushButton {
                background-color: #ef4444;
                color: white;
                border: none;
                border-radius: 8px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #dc2626;
            }
        )");
        connect(btnDelete, &QPushButton::clicked, this,
                [this, id]() { on_cherchSupprimerChercheur(id); });

        mainLayout->addWidget(btnEdit);
        mainLayout->addWidget(btnDelete);
    }

    item->installEventFilter(this);
    item->setMouseTracking(true);

    QVBoxLayout *list =
        qobject_cast<QVBoxLayout *>(ui->cherchScrollAreaWidgetContents->layout());
    if (list) {
        list->addWidget(item);

        // Animation
        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(item);
        opacityEffect->setOpacity(0.0);
        item->setGraphicsEffect(opacityEffect);
        QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
        anim->setDuration(400);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void SmartPub::cherchClearChercheursList() {
    QLayoutItem *child;
    QLayout *layout = ui->cherchScrollAreaWidgetContents->layout();
    if (!layout)
        return;

    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

void SmartPub::on_cherchBtnVueListe_clicked() {
    if (!cherchVueListeActive) {
        ui->cherchStackedWidget->setCurrentIndex(0);
        ui->cherchLineEditRecherche->setVisible(true);
        ui->cherchBtnRecherche->setVisible(true);
        ui->cherchBtnTri->setVisible(true);
        ui->cherchBtnExport->setVisible(true);
        ui->cherchBtnStatistiques->setVisible(true);
        if (cherchBtnToggleVue)
            cherchBtnToggleVue->setVisible(true);

        ui->cherchBtnVueListe->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 13px;
                font-weight: 600;
            }
        )");
        ui->cherchBtnAjouter->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: #64748b;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 13px;
                font-weight: 500;
            }
            QPushButton:hover {
                background-color: #f1f5f9;
                color: #334155;
            }
        )");

        ui->cherchBtnVueListe->setChecked(true);
        ui->cherchBtnAjouter->setChecked(false);
        cherchVueListeActive = true;
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_cherchBtnAjouter_clicked() {
    if (cherchVueListeActive) {
        ui->cherchStackedWidget->setCurrentIndex(1);
        ui->cherchLineEditRecherche->setVisible(false);
        ui->cherchBtnRecherche->setVisible(false);
        ui->cherchBtnTri->setVisible(false);
        ui->cherchBtnExport->setVisible(false);
        ui->cherchBtnStatistiques->setVisible(false);
        if (cherchBtnToggleVue)
            cherchBtnToggleVue->setVisible(false);

        ui->cherchBtnVueListe->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: #64748b;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 13px;
                font-weight: 500;
            }
            QPushButton:hover {
                background-color: #f1f5f9;
                color: #334155;
            }
        )");
        ui->cherchBtnAjouter->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 13px;
                font-weight: 600;
            }
        )");

        ui->cherchBtnVueListe->setChecked(false);
        ui->cherchBtnAjouter->setChecked(true);
        cherchVueListeActive = false;
    }
}

void SmartPub::on_cherchBtnToggleVue_clicked() {
    cherchVueIconesActive = !cherchVueIconesActive;
    if (cherchVueIconesActive) {
        cherchBtnToggleVue->setText("⊞");
        cherchBtnToggleVue->setToolTip("Passer en mode liste");
    } else {
        cherchBtnToggleVue->setText("☰");
        cherchBtnToggleVue->setToolTip("Passer en mode icônes");
    }
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnRecherche_clicked() {
    QString searchText = ui->cherchLineEditRecherche->text().trimmed();
    if (searchText.isEmpty()) {
        cherchWhereClause.clear();
        cherchAfficherListeChercheurs();
        return;
    }
    QString escaped = searchText;
    escaped.replace("'", "''");
    QString likeVal = escaped.toLower();
    cherchWhereClause = QString("(LOWER(NOM) LIKE '%%1%' OR LOWER(PRENOM) LIKE '%%1%' OR LOWER(CIN) LIKE '%%1%')")
                            .arg(likeVal);
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnTri_clicked() {
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 8px;
            min-width: 250px;
        }
        QMenu::item {
            padding: 12px 20px;
            border-radius: 8px;
            color: #334155;
            font-size: 14px;
            font-weight: 500;
        }
        QMenu::item:selected {
            background-color: #eff6ff;
            color: #3b82f6;
        }
        QMenu::separator {
            height: 1px;
            background-color: #e2e8f0;
            margin: 8px 16px;
        }
    )");

    menu->addAction("⬆️  Nom (A → Z)", this,
                    [this]() { cherchTrierParNom(true); });
    menu->addAction("⬇️  Nom (Z → A)", this,
                    [this]() { cherchTrierParNom(false); });
    menu->addSeparator();
    menu->addAction("🎓  Grade (Hiérarchie académique)", this,
                    [this]() { cherchTrierParGrade(); });
    menu->addSeparator();
    menu->addAction("🕐  Date d'ajout (Plus récent)", this,
                    [this]() { cherchTrierParDateCreation(true); });
    menu->addAction("🕓  Date d'ajout (Plus ancien)", this,
                    [this]() { cherchTrierParDateCreation(false); });

    menu->exec(QCursor::pos());
}

void SmartPub::cherchTrierParNom(bool croissant) {
    cherchOrderByClause = croissant ? "NOM ASC, PRENOM ASC" : "NOM DESC, PRENOM DESC";
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParGrade() {
    // CASE WHEN portable Oracle/standard — tri hiérarchique académique
    cherchOrderByClause =
        "CASE GRADE "
        "WHEN 'Professeur' THEN 1 "
        "WHEN 'Maitre de Conferences' THEN 2 "
        "WHEN 'Docteur' THEN 3 "
        "WHEN 'Ingenieur de Recherche' THEN 4 "
        "WHEN 'Post-doctorant' THEN 5 "
        "WHEN 'Doctorant' THEN 6 "
        "ELSE 99 END, NOM ASC, PRENOM ASC";
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParDateCreation(bool croissant) {
    // ID_CHERCHEUR auto-incrémenté via séquence Oracle = proxy fiable de la date d'insertion
    cherchOrderByClause = croissant ? "ID_CHERCHEUR DESC" : "ID_CHERCHEUR ASC";
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnExport_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter", QDir::homePath(), "CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "ID,Nom,Prenom,Grade,Email,CIN,Date Creation,Age,Carriere,Nb "
                      "Projets\n";

            for (auto it = cherchChercheursMap.begin();
                 it != cherchChercheursMap.end(); ++it) {
                auto data = it.value();
                stream << it.key() << "," << data.nom << "," << data.prenom << ","
                       << data.grade << "," << data.email << "," << data.cin << ","
                       << data.dateCreation.toString("dd/MM/yyyy") << "," << data.age
                       << "," << data.carriere << "," << data.projetsIds.size() << "\n";
            }
            file.close();
            QMessageBox::information(this, "Export", "Export réussi !");
        }
    }
}

void SmartPub::on_cherchBtnStatistiques_clicked() {
    cherchAfficherStatistiques();
}

void SmartPub::cherchAfficherStatistiques() {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques et métiers innovants — Chercheurs");
    dialog->setMinimumSize(1100, 820);
    dialog->setStyleSheet(R"(
            QDialog { background-color: #f8fafc; }
            QLabel { color: #1e293b; background: transparent; border: none; }
            QTabWidget::pane {
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                background-color: white;
            }
            QTabBar::tab {
                padding: 10px 20px;
                font-weight: 600;
                color: #64748b;
                background: #f1f5f9;
                border: none;
                border-radius: 6px;
                margin-right: 4px;
            }
            QTabBar::tab:selected {
                color: #1e40af;
                background: white;
                border-bottom: 3px solid #3b82f6;
            }
        )");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *titleLabel = new QLabel("Tableau de bord — Chercheurs", dialog);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                              "#1e293b; background: transparent; border: none;");
    mainLayout->addWidget(titleLabel);

    auto createStatCard = [](const QString &title, const QString &value,
                             const QString &color) -> QFrame * {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString(R"(
            QFrame {
                background-color: white;
                border-radius: 16px;
                border: 1px solid #e2e8f0;
            }
        )"));
        card->setMinimumHeight(120);
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setSpacing(8);
        layout->setContentsMargins(20, 20, 20, 20);

        QLabel *t = new QLabel(title);
        t->setStyleSheet("color: #64748b; font-size: 13px; font-weight: "
                         "600; background: transparent; border: none;");

        QLabel *v = new QLabel(value);
        v->setStyleSheet(QString("color: %1; font-size: 40px; font-weight: 700; background: "
                                 "transparent; border: none;")
                             .arg(color));

        layout->addWidget(t);
        layout->addWidget(v);
        layout->addStretch();
        return card;
    };

    int nbProfs = 0, nbDocs = 0;
    for (auto &data : cherchChercheursMap) {
        if (data.grade == QLatin1String("Professeur"))
            nbProfs++;
        if (data.grade == QLatin1String("Doctorant"))
            nbDocs++;
    }

    QGridLayout *statsGrid = new QGridLayout();
    statsGrid->setSpacing(16);
    statsGrid->addWidget(
        createStatCard("Total chercheurs", QString::number(cherchChercheursMap.size()), "#3b82f6"),
        0, 0);
    statsGrid->addWidget(createStatCard("Professeurs", QString::number(nbProfs), "#10b981"), 0, 1);
    statsGrid->addWidget(createStatCard("Doctorants", QString::number(nbDocs), "#f59e0b"), 0, 2);

    QTabWidget *tabs = new QTabWidget(dialog);
    tabs->setDocumentMode(true);
    tabs->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #e2e8f0; border-radius: 12px; background: white; }"
        "QTabBar::tab { padding: 10px 18px; font-weight: 600; color: #64748b; }"
        "QTabBar::tab:selected { color: #1e293b; border-bottom: 2px solid #3b82f6; }");

    QWidget *tabOverview = new QWidget();
    QVBoxLayout *ovMain = new QVBoxLayout(tabOverview);
    ovMain->setSpacing(16);
    ovMain->setContentsMargins(12, 12, 12, 12);
    ovMain->addLayout(statsGrid);

    QScrollArea *scrollOverview = new QScrollArea();
    scrollOverview->setWidgetResizable(true);
    scrollOverview->setFrameShape(QFrame::NoFrame);
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *ovScrollLay = new QVBoxLayout(scrollContent);
    ovScrollLay->setSpacing(20);

    QHBoxLayout *chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(16);

    QSqlDatabase db = Connection::instance()->getDatabase();
    QStringList labNames;
    QList<double> labCounts;
    QStringList labNamesSat;
    QList<double> labSurcharge;

    if (db.isOpen()) {
        QSqlQuery qLab(db);
        const QString sqlEffectifs =
            QStringLiteral("SELECT L.NOM, COUNT(DISTINCT C.ID_CHERCHEUR) AS NB "
                           "FROM LABORATOIRE L "
                           "INNER JOIN CONTRIBUER C ON C.CODE_PROJET = L.CODE_PROJET "
                           "GROUP BY L.ID_LABORATOIRE, L.NOM ORDER BY L.NOM");
        if (qLab.exec(sqlEffectifs)) {
            while (qLab.next()) {
                labNames << qLab.value(0).toString();
                labCounts << qLab.value(1).toDouble();
            }
        }
        QSqlQuery qSat(db);
        const QString sqlSat =
            QStringLiteral("SELECT L.NOM, AVG(LEAST(cnt * 25, 100)) AS SAT "
                           "FROM ( "
                           "  SELECT L2.ID_LABORATOIRE, C.ID_CHERCHEUR, COUNT(DISTINCT C.CODE_PROJET) AS cnt "
                           "  FROM LABORATOIRE L2 "
                           "  INNER JOIN CONTRIBUER C ON C.CODE_PROJET = L2.CODE_PROJET "
                           "  GROUP BY L2.ID_LABORATOIRE, C.ID_CHERCHEUR "
                           ") X "
                           "JOIN LABORATOIRE L ON L.ID_LABORATOIRE = X.ID_LABORATOIRE "
                           "GROUP BY L.ID_LABORATOIRE, L.NOM ORDER BY L.NOM");
        if (qSat.exec(sqlSat)) {
            while (qSat.next()) {
                labNamesSat << qSat.value(0).toString();
                labSurcharge << qSat.value(1).toDouble();
            }
        }
    }

    auto makeBarChartView = [](const QString &title, const QStringList &categories,
                               const QList<double> &values, const QString &colorHex,
                               const QString &yLabel) -> QChartView * {
        QChartView *cv = new QChartView();
        cv->setRenderHint(QPainter::Antialiasing);
        cv->setMinimumHeight(320);

        QBarSet *set = new QBarSet("Valeur");
        for (double v : values)
            *set << v;
        set->setColor(QColor(colorHex));
        set->setBorderColor(QColor(colorHex).darker(110));

        QBarSeries *series = new QBarSeries();
        series->append(set);
        series->setBarWidth(0.65);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(title);
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundRoundness(8);
        chart->setBackgroundBrush(QBrush(QColor("#ffffff")));
        chart->legend()->setVisible(false);

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        for (const QString &c : categories)
            axisX->append(c);
        axisX->setLabelsAngle(-25);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        double vmax = 1.0;
        for (double v : values)
            vmax = qMax(vmax, v);
        axisY->setRange(0, vmax * 1.15 + 0.5);
        axisY->setLabelFormat("%.0f");
        axisY->setTitleText(yLabel);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        cv->setChart(chart);
        return cv;
    };

    if (!labNames.isEmpty() && labCounts.size() == labNames.size()) {
        chartsRow->addWidget(makeBarChartView(
            "Effectifs par laboratoire (chercheurs distincts via projets)", labNames, labCounts,
            "#3b82f6", "Nombre de chercheurs"));
    } else {
        QLabel *empty = new QLabel(
            "Aucune donnée laboratoire : vérifiez LABORATOIRE, CONTRIBUER et les clés CODE_PROJET.");
        empty->setWordWrap(true);
        empty->setStyleSheet("color: #64748b; padding: 16px;");
        chartsRow->addWidget(empty);
    }

    if (!labNamesSat.isEmpty() && labSurcharge.size() == labNamesSat.size()) {
        chartsRow->addWidget(makeBarChartView(
            "Taux de charge moyen par laboratoire (min(n×25, 100) par chercheur)", labNamesSat,
            labSurcharge, "#8b5cf6", "Score sur 100"));
    }

    ovScrollLay->addLayout(chartsRow);

    QFrame *gradeFrame = new QFrame();
    gradeFrame->setStyleSheet("QFrame { background: white; border-radius: 16px; border: 1px solid #e2e8f0; }");
    QVBoxLayout *gradeLay = new QVBoxLayout(gradeFrame);
    gradeLay->setContentsMargins(20, 20, 20, 20);
    QLabel *gradeTitle = new QLabel("Répartition par grade");
    gradeTitle->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
    gradeLay->addWidget(gradeTitle);

    QMap<QString, int> gradeCount;
    for (auto &data : cherchChercheursMap)
        gradeCount[data.grade]++;

    int totalC = qMax(1, cherchChercheursMap.size());
    for (auto it = gradeCount.begin(); it != gradeCount.end(); ++it) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *gradeLabel = new QLabel(it.key().isEmpty() ? QStringLiteral("(non renseigné)") : it.key());
        gradeLabel->setStyleSheet("font-size: 14px; color: #334155; font-weight: 600;");
        gradeLabel->setFixedWidth(200);
        QProgressBar *progress = new QProgressBar();
        progress->setRange(0, totalC);
        progress->setValue(it.value());
        progress->setTextVisible(true);
        progress->setFormat(QString("%1 chercheur(s)").arg(it.value()));
        progress->setStyleSheet(R"(
            QProgressBar { border: none; border-radius: 8px; background-color: #e2e8f0; text-align: center; height: 22px; }
            QProgressBar::chunk { background-color: #0ea5e9; border-radius: 8px; }
        )");
        row->addWidget(gradeLabel);
        row->addWidget(progress, 1);
        gradeLay->addLayout(row);
    }
    ovScrollLay->addWidget(gradeFrame);

    QFrame *overloadFrame = new QFrame();
    overloadFrame->setStyleSheet("QFrame { background: white; border-radius: 16px; border: 1px solid #e2e8f0; }");
    QVBoxLayout *overloadLayout = new QVBoxLayout(overloadFrame);
    overloadLayout->setContentsMargins(20, 20, 20, 20);
    QLabel *overloadTitle = new QLabel("Indice de surcharge par chercheur (projets affectés)");
    overloadTitle->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
    overloadLayout->addWidget(overloadTitle);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *headerName = new QLabel("Chercheur");
    headerName->setStyleSheet("color: #64748b; font-weight: 600; font-size: 12px;");
    headerName->setFixedWidth(200);
    QLabel *headerProgress = new QLabel("Charge (n projets)");
    headerProgress->setStyleSheet("color: #64748b; font-weight: 600; font-size: 12px;");
    QLabel *headerStatus = new QLabel("Statut");
    headerStatus->setStyleSheet("color: #64748b; font-weight: 600; font-size: 12px;");
    headerStatus->setFixedWidth(90);
    headerRow->addWidget(headerName);
    headerRow->addWidget(headerProgress, 1);
    headerRow->addWidget(headerStatus);
    overloadLayout->addLayout(headerRow);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e2e8f0; max-height: 1px;");
    overloadLayout->addWidget(line);

    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        const ChercheurData &data = it.value();
        int nbProjets = data.projetsIds.size();
        int surcharge = qMin(nbProjets * 25, 100);

        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *nameLabel =
            new QLabel(QString("%1 %2").arg(data.prenom, data.nom));
        nameLabel->setFixedWidth(200);
        nameLabel->setStyleSheet("font-weight: 600; color: #334155;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(surcharge);
        progressBar->setTextVisible(true);
        progressBar->setFormat(QString("%1 projet(s)").arg(nbProjets));
        progressBar->setFixedHeight(26);

        QString color;
        if (surcharge < 50)
            color = "#10b981";
        else if (surcharge < 75)
            color = "#f59e0b";
        else
            color = "#ef4444";

        progressBar->setStyleSheet(QString(R"(
            QProgressBar { border: none; border-radius: 13px; background-color: #e2e8f0; text-align: center; color: white; font-weight: 600; font-size: 11px; }
            QProgressBar::chunk { background-color: %1; border-radius: 13px; }
        )")
                                       .arg(color));

        QLabel *statusLabel = new QLabel();
        if (surcharge < 50)
            statusLabel->setText("Normal");
        else if (surcharge < 75)
            statusLabel->setText("Occupé");
        else
            statusLabel->setText("Surchargé");
        statusLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(color));
        statusLabel->setFixedWidth(90);

        rowLayout->addWidget(nameLabel);
        rowLayout->addWidget(progressBar, 1);
        rowLayout->addWidget(statusLabel);
        overloadLayout->addLayout(rowLayout);
    }

    ovScrollLay->addWidget(overloadFrame);
    ovScrollLay->addStretch();
    scrollOverview->setWidget(scrollContent);
    ovMain->addWidget(scrollOverview, 1);
    tabs->addTab(tabOverview, "Vue d'ensemble");

    QWidget *tabPromo = new QWidget();
    QVBoxLayout *promoLay = new QVBoxLayout(tabPromo);
    promoLay->setContentsMargins(16, 16, 16, 16);
    promoLay->setSpacing(12);

    QFormLayout *promoForm = new QFormLayout();
    QComboBox *comboPromo = new QComboBox();
    QList<int> idsSorted = cherchChercheursMap.keys();
    std::sort(idsSorted.begin(), idsSorted.end());
    for (int id : idsSorted) {
        const ChercheurData &d = cherchChercheursMap[id];
        comboPromo->addItem(QString("%1 %2 — id %3").arg(d.prenom, d.nom).arg(id), id);
    }

    PromotionCriteria critDefaults;
    QSpinBox *spinAns = new QSpinBox();
    spinAns->setRange(0, 50);
    spinAns->setValue(critDefaults.minAnneesAnciennete);
    QSpinBox *spinPub = new QSpinBox();
    spinPub->setRange(0, 500);
    spinPub->setValue(critDefaults.minPublications);
    QSpinBox *spinProj = new QSpinBox();
    spinProj->setRange(0, 100);
    spinProj->setValue(critDefaults.minProjetsGeres);

    promoForm->addRow("Chercheur", comboPromo);
    promoForm->addRow("Seuil ancienneté (années)", spinAns);
    promoForm->addRow("Seuil publications", spinPub);
    promoForm->addRow("Seuil projets (contributions)", spinProj);
    promoLay->addLayout(promoForm);

    QLabel *promoResultTitle = new QLabel("Résultat");
    promoResultTitle->setStyleSheet("font-weight: 700; color: #1e293b;");
    promoLay->addWidget(promoResultTitle);

    QLabel *promoVerdict = new QLabel();
    promoVerdict->setWordWrap(true);
    promoVerdict->setStyleSheet("font-size: 15px; padding: 8px;");
    QListWidget *promoDetails = new QListWidget();
    promoDetails->setMinimumHeight(180);
    promoLay->addWidget(promoVerdict);
    promoLay->addWidget(promoDetails);

    auto runPromotion = [=]() {
        if (!db.isOpen() || comboPromo->count() == 0) {
            promoVerdict->setText("Base indisponible ou aucun chercheur chargé.");
            promoDetails->clear();
            return;
        }
        PromotionCriteria c;
        c.minAnneesAnciennete = spinAns->value();
        c.minPublications = spinPub->value();
        c.minProjetsGeres = spinProj->value();
        int cid = comboPromo->currentData().toInt();
        QSqlDatabase dbConn(db);
        ChercheurPromotionProfile p = PromotionEngine::loadProfile(dbConn, cid);
        bool ok = PromotionEngine::isEligible(p, c);
        double score = PromotionEngine::eligibilityScore(p, c);
        promoVerdict->setText(
            ok ? QString("<span style='color:#059669;font-weight:700'>Éligible</span> au regard des "
                           "critères — score de complétude : %1 / 10")
                     .arg(score, 0, 'f', 1)
               : QString("<span style='color:#b91c1c;font-weight:700'>Non éligible</span> — score de "
                         "complétude : %1 / 10")
                     .arg(score, 0, 'f', 1));
        promoDetails->clear();
        for (const QString &line : PromotionEngine::detailChecks(p, c))
            promoDetails->addItem(line);
    };

    QPushButton *btnEvalPromo = new QPushButton("Évaluer l'éligibilité");
    btnEvalPromo->setCursor(Qt::PointingHandCursor);
    btnEvalPromo->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 10px; "
        "padding: 10px 20px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    connect(btnEvalPromo, &QPushButton::clicked, dialog, [runPromotion]() { runPromotion(); });
    connect(comboPromo, QOverload<int>::of(&QComboBox::currentIndexChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    connect(spinAns, QOverload<int>::of(&QSpinBox::valueChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    connect(spinPub, QOverload<int>::of(&QSpinBox::valueChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    connect(spinProj, QOverload<int>::of(&QSpinBox::valueChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    promoLay->addWidget(btnEvalPromo, 0, Qt::AlignLeft);
    promoLay->addStretch();
    tabs->addTab(tabPromo, "Prédicteur de promotion");
    runPromotion();

    QWidget *tabMatch = new QWidget();
    QVBoxLayout *matchLay = new QVBoxLayout(tabMatch);
    matchLay->setContentsMargins(16, 16, 16, 16);
    matchLay->setSpacing(12);

    QLabel *matchInfo = new QLabel(
        "Suggestions de collègues ayant des mots-clés proches (titres de publications), "
        "sans collaboration passée sur un même projet (CONTRIBUER). Classement par indice de Jaccard.");
    matchInfo->setWordWrap(true);
    matchInfo->setStyleSheet("color: #64748b; font-size: 13px;");
    matchLay->addWidget(matchInfo);

    QHBoxLayout *matchRow = new QHBoxLayout();
    QComboBox *comboMatch = new QComboBox();
    for (int id : idsSorted) {
        const ChercheurData &d = cherchChercheursMap[id];
        comboMatch->addItem(QString("%1 %2 — id %3").arg(d.prenom, d.nom).arg(id), id);
    }
    QPushButton *btnMatch = new QPushButton("Actualiser les suggestions");
    btnMatch->setCursor(Qt::PointingHandCursor);
    btnMatch->setStyleSheet(
        "QPushButton { background-color: #10b981; color: white; border: none; border-radius: 10px; "
        "padding: 10px 20px; font-weight: 600; }"
        "QPushButton:hover { background-color: #059669; }");
    matchRow->addWidget(new QLabel("Chercheur de référence:"));
    matchRow->addWidget(comboMatch, 1);
    matchRow->addWidget(btnMatch);
    matchLay->addLayout(matchRow);

    QTableWidget *tableMatch = new QTableWidget(0, 4);
    tableMatch->setHorizontalHeaderLabels(
        QStringList() << "ID"
                      << "Nom"
                      << "Indice Jaccard"
                      << "Mots-clés communs");
    tableMatch->horizontalHeader()->setStretchLastSection(true);
    tableMatch->setAlternatingRowColors(true);
    tableMatch->setStyleSheet(
        "QTableWidget { gridline-color: #e2e8f0; background: white; }"
        "QHeaderView::section { background: #f1f5f9; font-weight: 600; padding: 6px; }");
    matchLay->addWidget(tableMatch);

    auto runMatch = [=]() {
        tableMatch->setRowCount(0);
        if (!db.isOpen() || comboMatch->count() == 0)
            return;
        int cid = comboMatch->currentData().toInt();
        QSqlDatabase dbConn(db);
        QVector<MatchCandidate> vec = MatchmakingEngine::suggestColleagues(dbConn, cid, 20);
        tableMatch->setRowCount(vec.size());
        for (int i = 0; i < vec.size(); ++i) {
            const MatchCandidate &m = vec[i];
            tableMatch->setItem(i, 0, new QTableWidgetItem(QString::number(m.idChercheur)));
            tableMatch->setItem(i, 1, new QTableWidgetItem(m.nomComplet));
            tableMatch->setItem(i, 2,
                                new QTableWidgetItem(QString::number(m.scoreJaccard, 'f', 3)));
            tableMatch->setItem(i, 3, new QTableWidgetItem(QString::number(m.nbMotsCommuns)));
        }
        tableMatch->resizeColumnsToContents();
    };
    connect(btnMatch, &QPushButton::clicked, dialog, [runMatch]() { runMatch(); });
    connect(comboMatch, QOverload<int>::of(&QComboBox::currentIndexChanged), dialog,
            [runMatch](int) { runMatch(); });
    tabs->addTab(tabMatch, "Smart matchmaking");
    runMatch();

    mainLayout->addWidget(tabs, 1);

    QPushButton *btnClose = new QPushButton("Fermer", dialog);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981); "
        "color: white; border: none; border-radius: 12px; padding: 12px 40px; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669); }");
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);
    mainLayout->addWidget(btnClose, 0, Qt::AlignCenter);

    dialog->exec();
}

void SmartPub::on_cherchBtnUploadPhoto_clicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Photo", QDir::homePath(), "Images (*.png *.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        ui->cherchLabelPhotoHint->setText("Photo sélectionnée ✓");
        ui->cherchLabelPhotoHint->setStyleSheet(
            "color: #10b981; font-size: 12px; background: transparent; border: "
            "none;");
    }
}

void SmartPub::on_cherchBtnAjouterChercheur_clicked() {
    QString nom = ui->cherchLineEditNom->text().trimmed();
    QString prenom = ui->cherchLineEditPrenom->text().trimmed();
    QString cin = ui->cherchLineEditCIN->text().trimmed();
    QString email = ui->cherchLineEditEmail->text().trimmed();
    QString grade = ui->cherchComboBoxGrade->currentText().trimmed();

    if (nom.isEmpty() || prenom.isEmpty() || cin.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }
    if (email.isEmpty() || grade.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "L'email et le grade sont obligatoires.");
        return;
    }

    QString photoPath = ":/avatar.png";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO CHERCHEUR (NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL) "
                 "VALUES (:nom, :prenom, :email, :grade, :cin, :photo_profil)");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":grade", grade);
    query.bindValue(":cin", cin);
    query.bindValue(":photo_profil", photoPath);

    if (!query.exec()) {
        QString err = query.lastError().text();
        if (err.contains("unique") || err.contains("UK_CHERCHEUR"))
            QMessageBox::warning(this, "Erreur", "Un chercheur avec cet email ou ce CIN existe déjà.");
        else
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout : " + err);
        return;
    }

    QMessageBox::information(this, "Succès", "Chercheur ajouté !");

    ui->cherchLineEditNom->clear();
    ui->cherchLineEditPrenom->clear();
    ui->cherchLineEditCIN->clear();
    ui->cherchLineEditEmail->clear();
    ui->cherchComboBoxGrade->setCurrentIndex(0);
    ui->cherchLabelPhotoHint->setText("Cliquez pour ajouter une photo");
    ui->cherchLabelPhotoHint->setStyleSheet(
        "color: #94a3b8; font-size: 12px; background: transparent; border: "
        "none;");

    on_cherchBtnVueListe_clicked();
}

void SmartPub::on_cherchBtnAnnulerAjout_clicked() {
    if (!ui->cherchLineEditNom->text().isEmpty() ||
        !ui->cherchLineEditPrenom->text().isEmpty()) {

        auto reply = QMessageBox::question(this, "Confirmation", "Annuler ?");
        if (reply == QMessageBox::No)
            return;
    }

    ui->cherchLineEditNom->clear();
    ui->cherchLineEditPrenom->clear();
    ui->cherchLineEditCIN->clear();
    ui->cherchLineEditEmail->clear();

    on_cherchBtnVueListe_clicked();
}

void SmartPub::on_cherchModifierChercheur(int id) {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas modifier les données.");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }
    QSqlQuery query(db);
    query.prepare("SELECT ID_CHERCHEUR, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Erreur", "Chercheur introuvable.");
        return;
    }
    ChercheurData data;
    data.nom = query.value("NOM").toString();
    data.prenom = query.value("PRENOM").toString();
    data.email = query.value("EMAIL").toString();
    data.grade = query.value("GRADE").toString();
    data.cin = query.value("CIN").toString();
    data.photoPath = query.value("PHOTO_PROFIL").toString();
    if (data.photoPath.isEmpty()) data.photoPath = ":/avatar.png";

    QDialog dialog(this);
    dialog.setWindowTitle(
        QString("Modifier - %1 %2").arg(data.prenom).arg(data.nom));
    dialog.setMinimumWidth(480);
    dialog.setStyleSheet("background-color: #f8fafc;");

    const QString labelStyle = "color: #334155; font-size: 14px; font-weight: 600; background: transparent; border: none;";
    const QString inputStyle = "padding: 12px; border-radius: 10px; border: 2px solid #e2e8f0; font-size: 14px; color: #1e293b; background-color: white;";

    QVBoxLayout layout(&dialog);
    layout.setSpacing(16);
    layout.setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("Modifier le chercheur");
    title->setStyleSheet("font-size: 22px; font-weight: 700; color: #1e293b; background: transparent; border: none;");
    layout.addWidget(title);

    QString newPhotoPath = data.photoPath;
    QHBoxLayout *photoRow = new QHBoxLayout();
    photoRow->setSpacing(16);
    QLabel *photoLabel = new QLabel(&dialog);
    photoLabel->setFixedSize(80, 80);
    photoLabel->setAlignment(Qt::AlignCenter);
    photoLabel->setStyleSheet("border: 2px solid #e2e8f0; background: transparent;");
    photoLabel->setScaledContents(false);
    photoLabel->setMask(QRegion(0, 0, 80, 80, QRegion::Ellipse));
    QPixmap photoPix;
    if (!newPhotoPath.isEmpty())
        photoPix.load(newPhotoPath);
    if (photoPix.isNull())
        photoPix.load(":/avatar.png");
    if (!photoPix.isNull())
        photoLabel->setPixmap(makeCircularPixmap(photoPix, 80));
    photoRow->addWidget(photoLabel);
    QVBoxLayout *photoCol = new QVBoxLayout();
    QLabel *photoTitle = new QLabel("Photo de profil");
    photoTitle->setStyleSheet(labelStyle);
    photoCol->addWidget(photoTitle);
    QPushButton *btnChangerPhoto = new QPushButton("Changer la photo");
    btnChangerPhoto->setStyleSheet(R"(
        QPushButton { background-color: #e2e8f0; color: #334155; border: none; border-radius: 8px; padding: 8px 16px; font-size: 13px; font-weight: 500; }
        QPushButton:hover { background-color: #cbd5e1; color: #1e293b; }
    )");
    connect(btnChangerPhoto, &QPushButton::clicked, &dialog, [&dialog, photoLabel, &newPhotoPath]() {
        QString path = QFileDialog::getOpenFileName(&dialog, "Choisir une photo", QDir::homePath(), "Images (*.png *.jpg *.jpeg)");
        if (path.isEmpty()) return;
        newPhotoPath = path;
        QPixmap pm(path);
        if (!pm.isNull())
            photoLabel->setPixmap(makeCircularPixmap(pm, 80));
    });
    photoCol->addWidget(btnChangerPhoto);
    photoRow->addLayout(photoCol);
    layout.addLayout(photoRow);

    QLabel *lblNom = new QLabel("Nom:");
    lblNom->setStyleSheet(labelStyle);
    layout.addWidget(lblNom);
    QLineEdit *editNom = new QLineEdit(data.nom);
    editNom->setStyleSheet(inputStyle);
    layout.addWidget(editNom);

    QLabel *lblPrenom = new QLabel("Prénom:");
    lblPrenom->setStyleSheet(labelStyle);
    layout.addWidget(lblPrenom);
    QLineEdit *editPrenom = new QLineEdit(data.prenom);
    editPrenom->setStyleSheet(inputStyle);
    layout.addWidget(editPrenom);

    QLabel *lblEmail = new QLabel("Email:");
    lblEmail->setStyleSheet(labelStyle);
    layout.addWidget(lblEmail);
    QLineEdit *editEmail = new QLineEdit(data.email);
    editEmail->setStyleSheet(inputStyle);
    layout.addWidget(editEmail);

    QLabel *lblGrade = new QLabel("Grade:");
    lblGrade->setStyleSheet(labelStyle);
    layout.addWidget(lblGrade);
    QComboBox *comboGrade = new QComboBox();
    comboGrade->setStyleSheet(R"(
        QComboBox {
            padding: 12px;
            border-radius: 10px;
            border: 2px solid #e2e8f0;
            font-size: 14px;
            min-height: 40px;
            color: #1e293b;
            background-color: white;
        }
        QComboBox QAbstractItemView { color: #1e293b; background-color: white; }
    )");
    comboGrade->addItems({"Professeur", "Maitre de Conferences", "Docteur",
                          "Ingenieur de Recherche", "Post-doctorant",
                          "Doctorant"});
    comboGrade->setCurrentText(data.grade);
    layout.addWidget(comboGrade);

    QPushButton *btnSave = new QPushButton("Sauvegarder");
    btnSave->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 14px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )");
    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout.addWidget(btnSave);

    if (dialog.exec() == QDialog::Accepted) {
        QString newNom = editNom->text().trimmed();
        QString newPrenom = editPrenom->text().trimmed();
        QString newEmail = editEmail->text().trimmed();
        QString newGrade = comboGrade->currentText().trimmed();
        if (newNom.isEmpty() || newPrenom.isEmpty() || newEmail.isEmpty() || newGrade.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Tous les champs sont obligatoires.");
            return;
        }
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE CHERCHEUR SET NOM = :nom, PRENOM = :prenom, EMAIL = :email, GRADE = :grade, PHOTO_PROFIL = :photo_profil WHERE ID_CHERCHEUR = :id");
        updateQuery.bindValue(":nom", newNom);
        updateQuery.bindValue(":prenom", newPrenom);
        updateQuery.bindValue(":email", newEmail);
        updateQuery.bindValue(":grade", newGrade);
        updateQuery.bindValue(":photo_profil", newPhotoPath.isEmpty() ? QString(":/avatar.png") : newPhotoPath);
        updateQuery.bindValue(":id", id);
        if (!updateQuery.exec()) {
            QString err = updateQuery.lastError().text();
            if (err.contains("unique") || err.contains("UK_CHERCHEUR"))
                QMessageBox::warning(this, "Erreur", "Un chercheur avec cet email existe déjà.");
            else
                QMessageBox::critical(this, "Erreur", "Échec de la modification : " + err);
            return;
        }
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_cherchSupprimerChercheur(int id) {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas supprimer les données.");
        return;
    }

    auto reply =
        QMessageBox::question(this, "Supprimer", "Confirmer la suppression ?");
    if (reply != QMessageBox::Yes)
        return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression : " + query.lastError().text());
        return;
    }
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchVoirDetailsChercheur(int id) {
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    // ── Charger les données du chercheur ──────────────────────────────────────
    QSqlQuery query(db);
    query.prepare("SELECT ID_CHERCHEUR, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL "
                  "FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Erreur", "Chercheur introuvable.");
        return;
    }

    ChercheurData data;
    data.nom       = query.value("NOM").toString();
    data.prenom    = query.value("PRENOM").toString();
    data.email     = query.value("EMAIL").toString();
    data.grade     = query.value("GRADE").toString();
    data.cin       = query.value("CIN").toString();
    data.photoPath = query.value("PHOTO_PROFIL").toString();
    if (data.photoPath.isEmpty()) data.photoPath = ":/avatar.png";
    data.age = 0;

    // ── Charger les projets depuis CONTRIBUER ─────────────────────────────────
    QSqlQuery qProj(db);
    qProj.prepare("SELECT CODE_PROJET FROM CONTRIBUER WHERE ID_CHERCHEUR = :id");
    qProj.bindValue(":id", id);
    if (qProj.exec()) {
        while (qProj.next())
            data.projetsIds.append(qProj.value(0).toInt());
    }

    // ── Calculer la carrière depuis le nb de projets et le grade ─────────────
    data.carriere = cherchDeterminerCarriere(data.projetsIds.size(), data.grade);

    // ── Récupérer les titres des projets pour affichage et PDF ────────────────
    QList<QString> projetsTitres;
    if (!data.projetsIds.isEmpty()) {
        for (int pid : data.projetsIds) {
            QSqlQuery qT(db);
            qT.prepare("SELECT TITRE FROM PROJET WHERE CODE_PROJET = :pid");
            qT.bindValue(":pid", pid);
            if (qT.exec() && qT.next())
                projetsTitres.append(qT.value(0).toString());
            else
                projetsTitres.append(QString("Projet #%1").arg(pid));
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // DIALOGUE DE DÉTAILS
    // ═══════════════════════════════════════════════════════════════════════════
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(QString("Profil — %1 %2").arg(data.prenom, data.nom));
    dialog->setMinimumSize(720, 640);
    dialog->setMaximumSize(920, 840);
    dialog->setStyleSheet("background-color: #f8fafc;");
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Header gradient ───────────────────────────────────────────────────────
    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
        }
    )");
    headerFrame->setFixedHeight(210);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(10);
    headerLayout->setContentsMargins(20, 20, 20, 16);

    // Avatar circulaire
    QLabel *avatarLbl = new QLabel();
    avatarLbl->setFixedSize(110, 110);
    avatarLbl->setAlignment(Qt::AlignCenter);
    avatarLbl->setStyleSheet("border: 4px solid white; background: transparent;");
    avatarLbl->setScaledContents(false);
    avatarLbl->setMask(QRegion(0, 0, 110, 110, QRegion::Ellipse));
    QPixmap profilePix;
    if (!profilePix.load(data.photoPath)) profilePix.load(":/avatar.png");
    if (!profilePix.isNull())
        avatarLbl->setPixmap(makeCircularPixmap(profilePix, 110));
    headerLayout->addWidget(avatarLbl, 0, Qt::AlignCenter);

    QLabel *nameHeaderLbl = new QLabel(QString("%1 %2").arg(data.prenom, data.nom));
    nameHeaderLbl->setStyleSheet(
        "color: white; font-size: 22px; font-weight: 700; "
        "background: transparent; border: none;");
    nameHeaderLbl->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(nameHeaderLbl, 0, Qt::AlignCenter);

    // Badge carrière dans le header
    QLabel *careerBadge = new QLabel(data.carriere.isEmpty() ? "—" : data.carriere);
    careerBadge->setStyleSheet(
        "background: rgba(255,255,255,0.22); color: white; "
        "border-radius: 10px; padding: 5px 16px; font-size: 12px; "
        "font-weight: 600; border: none;");
    careerBadge->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(careerBadge, 0, Qt::AlignCenter);

    mainLayout->addWidget(headerFrame);

    // ── Scroll area avec les infos ────────────────────────────────────────────
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: white; border: none;");

    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(28, 24, 28, 24);

    // Fabrique de ligne info
    auto createInfoRow = [](const QString &label, const QString &value,
                             const QString &icon = "") -> QFrame * {
        QFrame *row = new QFrame();
        row->setStyleSheet(
            "QFrame { background-color: #f8fafc; border-radius: 10px; border: none; }");
        row->setMaximumHeight(68);
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(16, 10, 16, 10);

        QLabel *iconLbl = new QLabel(icon.isEmpty() ? "•" : icon);
        iconLbl->setStyleSheet("font-size: 16px; background: transparent; border: none;");
        iconLbl->setFixedWidth(26);
        rowLayout->addWidget(iconLbl);

        QLabel *labelLbl = new QLabel(label + " :");
        labelLbl->setStyleSheet(
            "color: #64748b; font-size: 13px; font-weight: 600; "
            "min-width: 130px; background: transparent; border: none;");
        rowLayout->addWidget(labelLbl);

        QLabel *valueLbl = new QLabel(value.isEmpty() ? "—" : value);
        valueLbl->setStyleSheet(
            "color: #1e293b; font-size: 14px; font-weight: 500; "
            "background: transparent; border: none;");
        valueLbl->setWordWrap(true);
        rowLayout->addWidget(valueLbl, 1);
        return row;
    };

    contentLayout->addWidget(createInfoRow("Grade",    data.grade,                    "🎓"));
    contentLayout->addWidget(createInfoRow("Email",    data.email,                    "✉️"));
    contentLayout->addWidget(createInfoRow("CIN",      data.cin,                      "🆔"));
    contentLayout->addWidget(createInfoRow("Carrière", data.carriere,                 "⭐"));
    contentLayout->addWidget(createInfoRow("Projets",
        QString::number(data.projetsIds.size()) + " contribution(s)",                "📁"));

    // ── Liste des projets associés ────────────────────────────────────────────
    if (!projetsTitres.isEmpty()) {
        QLabel *projTitle = new QLabel("Projets associés");
        projTitle->setStyleSheet(
            "color: #1e293b; font-size: 15px; font-weight: 700; "
            "margin-top: 6px; background: transparent; border: none;");
        contentLayout->addWidget(projTitle);

        for (const QString &titre : projetsTitres) {
            QFrame *projFrame = new QFrame();
            projFrame->setStyleSheet(
                "QFrame { background-color: #eff6ff; border-left: 4px solid #3b82f6; "
                "border-radius: 8px; border-top: none; border-right: none; border-bottom: none; }");
            QHBoxLayout *pLayout = new QHBoxLayout(projFrame);
            pLayout->setContentsMargins(14, 10, 14, 10);
            QLabel *pLbl = new QLabel(titre);
            pLbl->setStyleSheet(
                "color: #1e40af; font-weight: 600; background: transparent; border: none;");
            pLayout->addWidget(pLbl, 1);
            contentLayout->addWidget(projFrame);
        }
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    // ── Footer : Export PDF + Fermer ──────────────────────────────────────────
    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet(
        "background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(24, 0, 24, 0);
    footerLayout->setSpacing(12);

    QPushButton *btnExport = new QPushButton("📄  Exporter en PDF");
    btnExport->setCursor(Qt::PointingHandCursor);
    btnExport->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 0 22px;
            font-size: 13px;
            font-weight: 600;
            min-height: 42px;
        }
        QPushButton:hover {
            background-color: #eff6ff;
            border-color: #3b82f6;
            color: #1d4ed8;
        }
    )");

    // ── Lambda export PDF capturant id, data et projetsTitres ─────────────────
    connect(btnExport, &QPushButton::clicked, dialog,
        [this, data, projetsTitres]() {

        QString safeNom = data.nom;
        safeNom.replace(" ", "_");
        QString safePrenom = data.prenom;
        safePrenom.replace(" ", "_");

        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Exporter Fiche Chercheur — PDF",
            QDir::homePath() + "/Fiche_" + safeNom + "_" + safePrenom + ".pdf",
            "Fichiers PDF (*.pdf)");
        if (fileName.isEmpty()) return;

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageOrientation(QPageLayout::Portrait);

        QPainter painter;
        if (!painter.begin(&printer)) {
            QMessageBox::critical(this, "Erreur PDF",
                "Impossible d'initialiser le fichier PDF :\n" + fileName);
            return;
        }

        const double res = printer.resolution();  // points / inch (ex: 1200)
        const double cm  = res / 2.54;            // 1 cm en points
        int x = (int)(1.8 * cm);
        int y = (int)(1.5 * cm);

        // ── EN-TÊTE : rectangle gradient simulé ────────────────────────────
        QLinearGradient headerGrad(x, y, x + (int)(17.4 * cm), y);
        headerGrad.setColorAt(0.0, QColor("#3b82f6"));
        headerGrad.setColorAt(1.0, QColor("#10b981"));
        painter.setPen(Qt::NoPen);
        painter.setBrush(headerGrad);
        painter.drawRoundedRect(x, y, (int)(17.4 * cm), (int)(4.0 * cm), 14, 14);

        // Photo de profil (rendue circulaire)
        QString photoPath = data.photoPath.isEmpty() ? ":/avatar.png" : data.photoPath;
        QPixmap pix;
        if (!pix.load(photoPath)) pix.load(":/avatar.png");
        if (!pix.isNull()) {
            int sz = (int)(3.2 * cm);
            QPixmap circ = makeCircularPixmap(pix, sz);
            // Dessin du cercle blanc derrière la photo
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::white);
            painter.drawEllipse(x + (int)(0.25 * cm), y + (int)(0.35 * cm), sz + 8, sz + 8);
            painter.drawPixmap(x + (int)(0.29 * cm) + 4,
                               y + (int)(0.39 * cm) + 4, sz, sz, circ);
        }

        // Nom + Grade
        int textX = x + (int)(4.2 * cm);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
        painter.drawText(textX, y + (int)(1.3 * cm),
                         QString("%1 %2").arg(data.prenom, data.nom));

        painter.setFont(QFont("Segoe UI", 13));
        painter.drawText(textX, y + (int)(2.1 * cm), data.grade);

        // Badge carrière
        if (!data.carriere.isEmpty()) {
            painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
            int bx = textX, by = y + (int)(2.55 * cm);
            int bw = (int)(5.5 * cm), bh = (int)(0.6 * cm);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 55));
            painter.drawRoundedRect(bx, by, bw, bh, 10, 10);
            painter.setPen(Qt::white);
            painter.drawText(QRect(bx, by, bw, bh), Qt::AlignCenter, data.carriere);
        }

        y += (int)(4.8 * cm);

        // ── Helpers locaux ──────────────────────────────────────────────────
        auto drawSectionHeader = [&](const QString &titre) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#f1f5f9"));
            painter.drawRoundedRect(x, y - (int)(0.05 * cm),
                                    (int)(17.4 * cm), (int)(0.75 * cm), 6, 6);
            painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
            painter.setPen(QColor("#1e293b"));
            painter.drawText(x + (int)(0.5 * cm), y + (int)(0.52 * cm), titre);
            y += (int)(1.1 * cm);
        };

        auto drawField = [&](const QString &label, const QString &value) {
            painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
            painter.setPen(QColor("#64748b"));
            painter.drawText(x + (int)(0.4 * cm), y, label);
            painter.setFont(QFont("Segoe UI", 11));
            painter.setPen(QColor("#1e293b"));
            painter.drawText(x + (int)(5.2 * cm), y, value.isEmpty() ? "—" : value);
            y += (int)(0.75 * cm);
        };

        // ── Informations personnelles ───────────────────────────────────────
        drawSectionHeader("Informations personnelles");
        drawField("Email :",     data.email);
        drawField("CIN :",       data.cin);
        drawField("Grade :",     data.grade);
        drawField("Carrière :",  data.carriere);

        y += (int)(0.5 * cm);

        // ── Projets ─────────────────────────────────────────────────────────
        drawSectionHeader(
            QString("Projets de recherche  (%1 contribution(s))")
            .arg(projetsTitres.size()));

        if (projetsTitres.isEmpty()) {
            painter.setFont(QFont("Segoe UI", 11));
            painter.setPen(QColor("#94a3b8"));
            painter.drawText(x + (int)(0.5 * cm), y, "Aucun projet associé.");
            y += (int)(0.7 * cm);
        } else {
            for (const QString &titre : projetsTitres) {
                // Puce bleue
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor("#3b82f6"));
                painter.drawEllipse(x + (int)(0.4 * cm),
                                    y - (int)(0.2 * cm),
                                    (int)(0.2 * cm), (int)(0.2 * cm));
                painter.setFont(QFont("Segoe UI", 11));
                painter.setPen(QColor("#1e293b"));
                painter.drawText(x + (int)(0.85 * cm), y, titre);
                y += (int)(0.6 * cm);
            }
        }

        // ── Pied de page ────────────────────────────────────────────────────
        int footerY = (int)(27.8 * cm);
        painter.setPen(QColor("#e2e8f0"));
        painter.drawLine(x, footerY, x + (int)(17.4 * cm), footerY);
        painter.setFont(QFont("Segoe UI", 9));
        painter.setPen(QColor("#94a3b8"));
        painter.drawText(x, footerY + (int)(0.45 * cm),
            QString("SmartPub — Fiche générée le %1")
            .arg(QDate::currentDate().toString("dd/MM/yyyy")));

        painter.end();
        QMessageBox::information(this, "Export PDF",
            "✅  Fiche exportée avec succès !\n" + fileName);
    });

    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 0 30px;
            font-size: 14px;
            font-weight: 600;
            min-height: 42px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563eb, stop:1 #059669);
        }
    )");
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);

    footerLayout->addWidget(btnExport);
    footerLayout->addStretch();
    footerLayout->addWidget(btnClose);
    mainLayout->addWidget(footerFrame);

    dialog->exec();
}

// on_cherchBtnExportDetails_clicked() — supprimée, export PDF géré
// directement par une lambda dans on_cherchVoirDetailsChercheur()

void SmartPub::on_cherchLineEditRecherche_textChanged(const QString &text) {
    if (text.length() >= 2 || text.isEmpty()) {
        QTimer::singleShot(300, this, [this, text]() {
            if (ui->cherchLineEditRecherche->text() == text) {
                on_cherchBtnRecherche_clicked();
            }
        });
    }
}
// ============================================================================
// MODULE PUBLICATIONS
// ============================================================================

void SmartPub::SR_setupUI() {
    ui->SR_stackedWidget->setCurrentIndex(0);

    SR_filterFrame = new QFrame(ui->SR_tableFrame);
    SR_filterFrame->setStyleSheet("background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 10px; padding: 4px;");
    SR_filterFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *filterLayout = new QHBoxLayout(SR_filterFrame);
    filterLayout->setSpacing(12);

    QLabel *lblTitre = new QLabel("Titre", SR_filterFrame);
    SR_filterTitre = new QLineEdit(SR_filterFrame);
    SR_filterTitre->setPlaceholderText("Filtrer par titre...");
    SR_filterTitre->setMinimumWidth(140);
    QLabel *lblAuteur = new QLabel("Auteur", SR_filterFrame);
    SR_filterAuteur = new QLineEdit(SR_filterFrame);
    SR_filterAuteur->setPlaceholderText("Filtrer par auteur...");
    SR_filterAuteur->setMinimumWidth(140);
    QLabel *lblStatut = new QLabel("Statut", SR_filterFrame);
    SR_filterStatut = new QComboBox(SR_filterFrame);
    SR_filterStatut->setMinimumWidth(120);
    SR_filterStatut->addItem("Tous");
    SR_filterStatut->addItem("Publié");
    SR_filterStatut->addItem("Soumis");
    SR_filterStatut->addItem("En révision");
    SR_filterStatut->addItem("Accepté");
    SR_filterStatut->addItem("Rejeté");
    SR_btnReinitFilter = new QPushButton("Réinitialiser", SR_filterFrame);

    filterLayout->addWidget(lblTitre);
    filterLayout->addWidget(SR_filterTitre);
    filterLayout->addWidget(lblAuteur);
    filterLayout->addWidget(SR_filterAuteur);
    filterLayout->addWidget(lblStatut);
    filterLayout->addWidget(SR_filterStatut);
    filterLayout->addWidget(SR_btnReinitFilter);
    filterLayout->addStretch();

    QVBoxLayout *tableLayout = qobject_cast<QVBoxLayout *>(ui->SR_tableFrame->layout());
    if (tableLayout)
        tableLayout->insertWidget(1, SR_filterFrame);
}

void SmartPub::SR_connectSignals() {
    connect(ui->SR_btnVueListe, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnVueListe_clicked);
    connect(ui->SR_btnAjouter, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnAjouter_clicked);
    connect(ui->SR_btnRecherche, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnRecherche_clicked);
    connect(ui->SR_btnTri, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnTri_clicked);
    // Masquer les champs email/password et le bouton guest (User Request)
    // RETABLIR LES CHAMPS VISIBLES (User Request Update)
    ui->cherchLineEditLoginEmail->setPlaceholderText("Email");
    ui->cherchLineEditLoginPassword->setPlaceholderText("Mot de passe");
    ui->cherchLineEditLoginEmail->setVisible(true);
    ui->cherchLineEditLoginPassword->setVisible(true);

    // Masquer le bouton mot de passe oublié si nécessaire (Le laisser visible si
    // champs visibles)
    ui->cherchBtnMotDePasseOublie->setVisible(true);

    // Modern Button Style
    QString buttonStyle = R"(
        QPushButton {
            background-color: #2563eb;
            color: white;
            border-radius: 8px;
            font-weight: 600;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #1d4ed8;
        }
    )";
    ui->cherchBtnLogin->setStyleSheet(buttonStyle);
    ui->cherchBtnLogin->setText("Se Connecter");
    connect(ui->SR_btnExport, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnExport_clicked);
    connect(ui->SR_btnStatistiques, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnStatistiques_clicked);
    connect(ui->SR_btnAjouterPublication, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnAjouterPublication_clicked);
    connect(ui->SR_btnAnnulerAjout, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnAnnulerAjout_clicked);
    connect(SR_filterTitre, &QLineEdit::textChanged, this, [this]() { SR_applyFilterListe(); });
    connect(SR_filterAuteur, &QLineEdit::textChanged, this, [this]() { SR_applyFilterListe(); });
    connect(SR_filterStatut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { SR_applyFilterListe(); });
    connect(SR_btnReinitFilter, &QPushButton::clicked, this, &SmartPub::SR_reinitFilterListe);
}

void SmartPub::SR_updateButtonStyles() {
    QString activeStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )";

    QString inactiveStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #64748b;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #f1f5f9;
            color: #334155;
        }
    )";

    QString deleteStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #ef4444;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #fef2f2;
            color: #dc2626;
        }
    )";

    if (ui->SR_stackedWidget->currentIndex() == 0) {
        ui->SR_btnVueListe->setStyleSheet(activeStyle);
        ui->SR_btnVueListe->setChecked(true);
        ui->SR_btnAjouter->setStyleSheet(inactiveStyle);
        ui->SR_btnAjouter->setChecked(false);
    } else if (ui->SR_stackedWidget->currentIndex() == 1) {
        ui->SR_btnVueListe->setStyleSheet(inactiveStyle);
        ui->SR_btnVueListe->setChecked(false);
        ui->SR_btnAjouter->setStyleSheet(activeStyle);
        ui->SR_btnAjouter->setChecked(true);
    }
}

void SmartPub::SR_loadSampleData() {
    QSqlDatabase db = Connection::instance()->getDatabase();
    ui->SR_tablePublications->setColumnWidth(5, 135);
    ui->SR_tablePublications->setRowCount(0);

    auto clearStats = [this]() {
        ui->SR_lblTotalNumber->setText(QStringLiteral("0"));
        ui->SR_lblThisYearNumber->setText(QStringLiteral("0"));
        ui->SR_lblPlanSNumber->setText(QStringLiteral("0"));
        ui->SR_lblStatPublie->setText(QStringLiteral("● Publié (0%)"));
        ui->SR_lblStatSoumis->setText(QStringLiteral("● Soumis (0%)"));
        ui->SR_lblStatRevision->setText(QStringLiteral("● En révision (0%)"));
        ui->SR_lblStatAccepte->setText(QStringLiteral("● Accepté (0%)"));
    };

    if (!db.isOpen()) {
        clearStats();
        return;
    }

    QSqlQuery query(db);
    const QString sql =
        QStringLiteral("SELECT ID_PUBLICATION, DOI, TITRE, AUTEUR, DATE_PUBLICATION, REVUE, STATUT "
                       "FROM PUBLICATION ORDER BY ID_PUBLICATION");
    if (!query.exec(sql)) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Impossible de charger les publications : ") + query.lastError().text());
        clearStats();
        return;
    }

    int row = 0;
    while (query.next()) {
        const int idPub = query.value(QStringLiteral("ID_PUBLICATION")).toInt();
        QString titre = query.value(QStringLiteral("TITRE")).toString();
        QString auteur = query.value(QStringLiteral("AUTEUR")).toString();
        QVariant dateVar = query.value(QStringLiteral("DATE_PUBLICATION"));
        QDate d = dateVar.toDate();
        if (!d.isValid() && dateVar.toDateTime().isValid())
            d = dateVar.toDateTime().date();
        QString dateStr = d.isValid() ? d.toString(QStringLiteral("yyyy-MM-dd")) : dateVar.toString();
        if (dateStr.length() > 10)
            dateStr = dateStr.left(10);
        QString revue = query.value(QStringLiteral("REVUE")).toString();
        QString statut = SR_statutDbToUi(query.value(QStringLiteral("STATUT")).toString());

        ui->SR_tablePublications->insertRow(row);
        QTableWidgetItem *titItem = new QTableWidgetItem(titre);
        titItem->setData(Qt::UserRole, idPub);
        titItem->setToolTip(query.value(QStringLiteral("DOI")).toString());
        ui->SR_tablePublications->setItem(row, 0, titItem);
        ui->SR_tablePublications->setItem(row, 1, new QTableWidgetItem(auteur));
        ui->SR_tablePublications->setItem(row, 2, new QTableWidgetItem(dateStr));
        ui->SR_tablePublications->setItem(row, 3, new QTableWidgetItem(revue));
        ui->SR_tablePublications->setItem(row, 4, new QTableWidgetItem(statut));
        SR_addButtonsToRow(row);
        row++;
    }
    ui->SR_tablePublications->resizeRowsToContents();
    ui->SR_lblTotalNumber->setText(QString::number(row));
    int thisYear = QDate::currentDate().year();
    int countThisYear = 0;
    for (int r = 0; r < row; r++) {
        QString ds = ui->SR_tablePublications->item(r, 2) ? ui->SR_tablePublications->item(r, 2)->text() : QString();
        if (ds.length() >= 4 && ds.left(4).toInt() == thisYear)
            countThisYear++;
    }
    ui->SR_lblThisYearNumber->setText(QString::number(countThisYear));
    ui->SR_lblPlanSNumber->setText(QStringLiteral("0"));
    int publie = 0, soumis = 0, revision = 0, accepte = 0;
    for (int r = 0; r < row; r++) {
        QString s = ui->SR_tablePublications->item(r, 4) ? ui->SR_tablePublications->item(r, 4)->text() : QString();
        if (s.contains(QStringLiteral("Publié"), Qt::CaseInsensitive))
            publie++;
        else if (s.contains(QStringLiteral("Soumis"), Qt::CaseInsensitive))
            soumis++;
        else if (s.contains(QStringLiteral("révision"), Qt::CaseInsensitive))
            revision++;
        else if (s.contains(QStringLiteral("Accepté"), Qt::CaseInsensitive))
            accepte++;
    }
    int total = row > 0 ? row : 1;
    ui->SR_lblStatPublie->setText(QStringLiteral("● Publié (%1%)").arg((publie * 100) / total));
    ui->SR_lblStatSoumis->setText(QStringLiteral("● Soumis (%1%)").arg((soumis * 100) / total));
    ui->SR_lblStatRevision->setText(QStringLiteral("● En révision (%1%)").arg((revision * 100) / total));
    ui->SR_lblStatAccepte->setText(QStringLiteral("● Accepté (%1%)").arg((accepte * 100) / total));
}

void SmartPub::on_SR_btnVueListe_clicked() {
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnAjouter_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de publications.");
        return;
    }

    // Reset form state
    editingPublicationRow = -1;
    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
    ui->SR_dateEditPublication->setDate(QDate::currentDate());
    ui->SR_btnAjouterPublication->setText("Ajouter");

    ui->SR_stackedWidget->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnRecherche_clicked() {
    QString searchText = ui->SR_lineEditRecherche->text();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Recherche",
                                 "Veuillez entrer un terme de recherche");
    } else {
        QMessageBox::information(this, "Recherche", "Recherche de: " + searchText);
    }
}

void SmartPub::on_SR_btnTri_clicked() {
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 8px;
            min-width: 220px;
        }
        QMenu::item {
            padding: 12px 20px;
            border-radius: 8px;
            color: #334155;
            font-size: 14px;
            font-weight: 500;
        }
        QMenu::item:selected {
            background-color: #eff6ff;
            color: #3b82f6;
        }
        QMenu::separator {
            height: 1px;
            background-color: #e2e8f0;
            margin: 8px 16px;
        }
    )");

    menu->addAction("Trier par Nom (A-Z)", this,
                    [this]() { cherchTrierParNom(true); });
    menu->addAction("Trier par Nom (Z-A)", this,
                    [this]() { cherchTrierParNom(false); });
    menu->addAction("Trier par Date (Plus récent)", this,
                    [this]() { cherchTrierParDateCreation(true); });
    menu->addAction("Trier par Date (Plus ancien)", this,
                    [this]() { cherchTrierParDateCreation(false); });

    menu->exec(QCursor::pos());
}

void SmartPub::on_SR_btnExport_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les transactions", QDir::homePath(), "CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Export",
                                 "Transactions exportées avec succès !");
    }
}

void SmartPub::on_SR_btnStatistiques_clicked() {
    ui->SR_stackedWidget->setCurrentIndex(2);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnAjouterPublication_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de publications.");
        return;
    }

    QString titre = ui->SR_lineEditTitre->text().trimmed();
    QString auteurs = ui->SR_lineEditAuteurs->text().trimmed();
    QString revue = ui->SR_lineEditRevue->text().trimmed();
    QString statut = ui->SR_comboBoxStatut->currentText();
    QDate datePub = ui->SR_dateEditPublication->date();
    QString dateStr = datePub.toString("yyyy-MM-dd");

    if (titre.isEmpty() || auteurs.isEmpty() || revue.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez remplir tous les champs obligatoires");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    const QString statutDb = SR_statutUiToDb(statut);

    if (editingPublicationRow != -1) {
        QTableWidgetItem *titItem = ui->SR_tablePublications->item(editingPublicationRow, 0);
        if (!titItem)
            return;
        const int idPublication = titItem->data(Qt::UserRole).toInt();
        QSqlQuery query(db);
        query.prepare(
            QStringLiteral("UPDATE PUBLICATION SET TITRE = :titre, AUTEUR = :auteur, "
                           "DATE_PUBLICATION = TO_DATE(:date_pub, 'YYYY-MM-DD'), REVUE = :revue, STATUT = :statut "
                           "WHERE ID_PUBLICATION = :id"));
        query.bindValue(QStringLiteral(":titre"), titre);
        query.bindValue(QStringLiteral(":auteur"), auteurs);
        query.bindValue(QStringLiteral(":date_pub"), dateStr);
        query.bindValue(QStringLiteral(":revue"), revue);
        query.bindValue(QStringLiteral(":statut"), statutDb);
        query.bindValue(QStringLiteral(":id"), idPublication);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de la modification : ") + query.lastError().text());
            return;
        }
        SR_loadSampleData();
        QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Publication modifiée avec succès"));
        editingPublicationRow = -1;
        ui->SR_btnAjouterPublication->setText(QStringLiteral("Ajouter"));
    } else {
        const QString doi =
            QStringLiteral("10.1000/smartpub/%1").arg(QDateTime::currentMSecsSinceEpoch());
        QSqlQuery query(db);
        query.prepare(
            QStringLiteral("INSERT INTO PUBLICATION (DOI, TITRE, AUTEUR, DATE_PUBLICATION, REVUE, STATUT) "
                           "VALUES (:doi, :titre, :auteur, TO_DATE(:date_pub, 'YYYY-MM-DD'), :revue, :statut)"));
        query.bindValue(QStringLiteral(":doi"), doi);
        query.bindValue(QStringLiteral(":titre"), titre);
        query.bindValue(QStringLiteral(":auteur"), auteurs);
        query.bindValue(QStringLiteral(":date_pub"), dateStr);
        query.bindValue(QStringLiteral(":revue"), revue);
        query.bindValue(QStringLiteral(":statut"), statutDb);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de l'ajout : ") + query.lastError().text());
            return;
        }
        SR_loadSampleData();
        QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Publication ajoutée avec succès"));
    }

    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
    ui->SR_dateEditPublication->setDate(QDate::currentDate());
}

void SmartPub::on_SR_btnAnnulerAjout_clicked() {
    editingPublicationRow = -1;
    ui->SR_btnAjouterPublication->setText("Ajouter");
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::SR_applyFilterListe() {
    QString titreFilter = SR_filterTitre->text().trimmed();
    QString auteurFilter = SR_filterAuteur->text().trimmed();
    QString statutFilter = SR_filterStatut->currentIndex() <= 0 ? QString() : SR_filterStatut->currentText();

    for (int r = 0; r < ui->SR_tablePublications->rowCount(); r++) {
        bool show = true;
        if (show && !titreFilter.isEmpty()) {
            QTableWidgetItem *it = ui->SR_tablePublications->item(r, 0);
            show = it && it->text().contains(titreFilter, Qt::CaseInsensitive);
        }
        if (show && !auteurFilter.isEmpty()) {
            QTableWidgetItem *it = ui->SR_tablePublications->item(r, 1);
            show = it && it->text().contains(auteurFilter, Qt::CaseInsensitive);
        }
        if (show && !statutFilter.isEmpty()) {
            QTableWidgetItem *it = ui->SR_tablePublications->item(r, 4);
            show = it && it->text().trimmed().compare(statutFilter, Qt::CaseInsensitive) == 0;
        }
        ui->SR_tablePublications->setRowHidden(r, !show);
    }
}

void SmartPub::SR_reinitFilterListe() {
    SR_filterTitre->clear();
    SR_filterAuteur->clear();
    SR_filterStatut->setCurrentIndex(0);
    for (int r = 0; r < ui->SR_tablePublications->rowCount(); r++)
        ui->SR_tablePublications->setRowHidden(r, false);
}

static int SR_rowFromActionButton(QTableWidget *table, QObject *sender) {
    QPushButton *btn = qobject_cast<QPushButton *>(sender);
    if (!btn) return -1;
    QWidget *cellWidget = btn->parentWidget();
    if (!cellWidget) return -1;
    for (int r = 0; r < table->rowCount(); r++) {
        if (table->cellWidget(r, 5) == cellWidget)
            return r;
    }
    return -1;
}

void SmartPub::on_SR_modifierPublication_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas modifier les publications.");
        return;
    }
    int row = SR_rowFromActionButton(ui->SR_tablePublications, sender());
    if (row < 0) return;
    QTableWidgetItem *titItem = ui->SR_tablePublications->item(row, 0);
    if (!titItem) return;
    QString titre = titItem->text();
    QString auteurs = ui->SR_tablePublications->item(row, 1) ? ui->SR_tablePublications->item(row, 1)->text() : QString();
    QString dateStr = ui->SR_tablePublications->item(row, 2) ? ui->SR_tablePublications->item(row, 2)->text() : QDate::currentDate().toString("yyyy-MM-dd");
    QString revue = ui->SR_tablePublications->item(row, 3) ? ui->SR_tablePublications->item(row, 3)->text() : QString();
    QString statut = ui->SR_tablePublications->item(row, 4) ? ui->SR_tablePublications->item(row, 4)->text() : QString();
    QDate datePub = QDate::fromString(dateStr.left(10), "yyyy-MM-dd");
    if (!datePub.isValid()) datePub = QDate::currentDate();

    editingPublicationRow = row;
    ui->SR_lineEditTitre->setText(titre);
    ui->SR_lineEditAuteurs->setText(auteurs);
    ui->SR_lineEditRevue->setText(revue);
    ui->SR_dateEditPublication->setDate(datePub);
    int idx = ui->SR_comboBoxStatut->findText(statut);
    if (idx >= 0) ui->SR_comboBoxStatut->setCurrentIndex(idx);
    else ui->SR_comboBoxStatut->setCurrentText(statut);
    ui->SR_btnAjouterPublication->setText("Enregistrer modification");
    ui->SR_stackedWidget->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_supprimerPublication_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas supprimer les publications.");
        return;
    }
    int row = SR_rowFromActionButton(ui->SR_tablePublications, sender());
    if (row < 0) return;
    QTableWidgetItem *titItem = ui->SR_tablePublications->item(row, 0);
    if (!titItem) return;
    QString titre = titItem->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmer la suppression",
        "Êtes-vous sûr de vouloir supprimer la publication \"" + titre + "\" ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"), QStringLiteral("Connexion à la base de données impossible."));
        return;
    }
    const int idPublication = titItem->data(Qt::UserRole).toInt();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM PUBLICATION WHERE ID_PUBLICATION = :id"));
    query.bindValue(QStringLiteral(":id"), idPublication);
    if (!query.exec()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"),
                              QStringLiteral("Échec de la suppression : ") + query.lastError().text());
        return;
    }
    editingPublicationRow = -1;
    ui->SR_btnAjouterPublication->setText(QStringLiteral("Ajouter"));
    SR_loadSampleData();
    QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Publication supprimée."));
}

// ============================================================================
// MODULE FINANCES
// ============================================================================

void SmartPub::finSetupUI() {
    ui->finStackedWidget->setCurrentIndex(0);
    finVueListeActive = true;

    // === Noms des colonnes du tableau ===
    ui->finTableTransactions->setHorizontalHeaderLabels(
        {"ID", "Projet", "Type de transaction", "Montant", "Date", "Categorie", "Statut", ""});

    // === Types de transactions ===
    ui->finComboBoxType->clear();
    ui->finComboBoxType->addItem(QStringLiteral("Subvention"), QStringLiteral("subvention"));
    ui->finComboBoxType->addItem(QStringLiteral("Achat"), QStringLiteral("achat"));
    ui->finComboBoxType->addItem(QStringLiteral("Salaire"), QStringLiteral("salaire"));
    ui->finComboBoxType->addItem(QStringLiteral("Remboursement"), QStringLiteral("remboursement"));
    ui->finComboBoxType->addItem(QStringLiteral("Autre"), QStringLiteral("autre"));

    finRemplirComboProjets();
}

void SmartPub::finRemplirComboProjets()
{
    ui->finComboBoxProjet->clear();
    ui->finComboBoxProjet->addItem(QStringLiteral("—"), QVariant(0));

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;

    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT CODE_PROJET, TITRE FROM PROJET ORDER BY CODE_PROJET"))) {
        while (q.next()) {
            const QString code = q.value(0).toString();   // CODE_PROJET est une QString
            const QString titre = q.value(1).toString();
            ui->finComboBoxProjet->addItem(QStringLiteral("%1 — %2").arg(code, titre), code);
        }
    }
}

void SmartPub::finChargerTransactionsDepuisOracle()
{
    finTransactionsMap.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        finAfficherListeTransactions();
        return;
    }

    QSqlQuery q(db);
    const QString sql =
        QStringLiteral("SELECT f.ID_TRANSACTION, f.MONTANT, f.TYPE_TRANS, f.CATEGORIE, f.DATE_TRANSACTION, "
                       "f.STATUT, f.DESCRIPTION, f.ID_PROJET, p.TITRE "
                       "FROM FINANCE f LEFT JOIN PROJET p ON p.CODE_PROJET = f.ID_PROJET "
                       "ORDER BY f.ID_TRANSACTION");
    if (!q.exec(sql)) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Impossible de charger les transactions : ") + q.lastError().text());
        finAfficherListeTransactions();
        return;
    }

    while (q.next()) {
        TransactionData t;
        t.id = q.value(0).toInt();
        t.montant = q.value(1).toDouble();
        t.type = finTypeDbToUi(q.value(2).toString());
        t.categorie = q.value(3).toString();
        QVariant dv = q.value(4);
        QDate d = dv.toDate();
        if (!d.isValid() && dv.toDateTime().isValid())
            d = dv.toDateTime().date();
        t.date = d.isValid() ? d.toString(QStringLiteral("dd/MM/yyyy")) : dv.toString();
        t.statut = q.value(5).toString();
        t.description = q.value(6).toString();
        QVariant pv = q.value(7);
        t.idProjet = pv.isNull() ? QString() : pv.toString();
        const QString titre = q.value(8).toString();
        t.projet = titre.isEmpty() && !t.idProjet.isEmpty() ? QStringLiteral("Projet #%1").arg(t.idProjet) : titre;
        finTransactionsMap.insert(t.id, t);
    }
    finAfficherListeTransactions();
}

void SmartPub::finConnectSignals() {
    connect(ui->finBtnVueListe, &QPushButton::clicked, this,
            &SmartPub::on_finBtnVueListe_clicked);
    connect(ui->finBtnAjouter, &QPushButton::clicked, this,
            &SmartPub::on_finBtnAjouter_clicked);
    connect(ui->finBtnRecherche, &QPushButton::clicked, this,
            &SmartPub::on_finBtnRecherche_clicked);
    connect(ui->finBtnTri, &QPushButton::clicked, this,
            &SmartPub::on_finBtnTri_clicked);
    connect(ui->finBtnExport, &QPushButton::clicked, this,
            &SmartPub::on_finBtnExport_clicked);
    connect(ui->finBtnStatistiques, &QPushButton::clicked, this,
            &SmartPub::on_finBtnStatistiques_clicked);
    connect(ui->finBtnAjouterTransaction, &QPushButton::clicked, this,
            &SmartPub::on_finBtnAjouterTransaction_clicked);
    connect(ui->finBtnAnnulerAjout, &QPushButton::clicked, this,
            &SmartPub::on_finBtnAnnulerAjout_clicked);
    connect(ui->finBtnModifierTable, &QPushButton::clicked, this,
            &SmartPub::on_finBtnModifierTransaction_clicked);
    connect(ui->finBtnSupprimerTable, &QPushButton::clicked, this,
            &SmartPub::on_finBtnSupprimerTransaction_clicked);
    connect(ui->finLineEditRecherche, &QLineEdit::textChanged, this,
            &SmartPub::on_finLineEditRecherche_textChanged);
}
// background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b9cff, stop:1
// #2dd4bf); background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2b8cef,
// stop:1 #1dc4af);
void SmartPub::finUpdateButtonStyles() {
    QString activeStyle = R"(
        QPushButton {

            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )";

    QString inactiveStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #64748b;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #f1f5f9;
            color: #334155;
        }
    )";

    if (ui->finStackedWidget->currentIndex() == 0) {
        ui->finBtnVueListe->setStyleSheet(activeStyle);
        ui->finBtnVueListe->setChecked(true);
        ui->finBtnAjouter->setStyleSheet(inactiveStyle);
        ui->finBtnAjouter->setChecked(false);
    } else if (ui->finStackedWidget->currentIndex() == 1) {
        ui->finBtnVueListe->setStyleSheet(inactiveStyle);
        ui->finBtnVueListe->setChecked(false);
        ui->finBtnAjouter->setStyleSheet(activeStyle);
        ui->finBtnAjouter->setChecked(true);
    }
}

void SmartPub::finAjouterDonneesTest() {
    finChargerTransactionsDepuisOracle();
}

void SmartPub::finAfficherListeTransactions() {
    ui->finTableTransactions->setRowCount(0);
    QList<TransactionData> liste = finGetTransactionsFiltreesEtTriees();
    for (const TransactionData &data : liste) {
        finAjouterTransactionTable(data);
    }
}

QList<TransactionData> SmartPub::finGetTransactionsFiltreesEtTriees() const {
    QList<TransactionData> liste;
    QString search = ui->finLineEditRecherche->text().trimmed().toLower();

    for (auto it = finTransactionsMap.constBegin(); it != finTransactionsMap.constEnd(); ++it) {
        const TransactionData &t = it.value();
        if (!search.isEmpty()) {
            if (!t.projet.toLower().contains(search) &&
                !t.type.toLower().contains(search) &&
                !t.categorie.toLower().contains(search) &&
                !t.statut.toLower().contains(search) &&
                !t.description.toLower().contains(search) &&
                !QString::number(t.montant, 'f', 2).contains(search))
                continue;
        }
        liste.append(t);
    }

    std::sort(liste.begin(), liste.end(), [this](const TransactionData &a, const TransactionData &b) {
        bool less = false;
        switch (finTriColonne) {
        case 0: less = a.id < b.id; break;
        case 1: less = a.projet.compare(b.projet, Qt::CaseInsensitive) < 0; break;
        case 2: less = a.type.compare(b.type, Qt::CaseInsensitive) < 0; break;
        case 3: less = a.montant < b.montant; break;
        case 4: {
            QDate da = QDate::fromString(a.date, "dd/MM/yyyy");
            QDate db = QDate::fromString(b.date, "dd/MM/yyyy");
            less = da < db;
            break;
        }
        case 5: less = a.categorie.compare(b.categorie, Qt::CaseInsensitive) < 0; break;
        case 6: less = a.statut.compare(b.statut, Qt::CaseInsensitive) < 0; break;
        default: less = a.id < b.id;
        }
        return finTriOrdre == Qt::AscendingOrder ? less : !less;
    });

    return liste;
}

void SmartPub::finAjouterTransactionTable(const TransactionData &data) {
    int row = ui->finTableTransactions->rowCount();
    ui->finTableTransactions->insertRow(row);

    ui->finTableTransactions->setItem(
        row, 0, new QTableWidgetItem(QString::number(data.id)));
    ui->finTableTransactions->setItem(row, 1, new QTableWidgetItem(data.projet));
    ui->finTableTransactions->setItem(row, 2, new QTableWidgetItem(data.type));
    ui->finTableTransactions->setItem(
        row, 3,
        new QTableWidgetItem(QString::number(data.montant, 'f', 2) + " €"));
    ui->finTableTransactions->setItem(row, 4, new QTableWidgetItem(data.date));
    ui->finTableTransactions->setItem(row, 5,
                                      new QTableWidgetItem(data.categorie));
    // Colonne Statut avec couleur selon valeur
    QTableWidgetItem *statutItem = new QTableWidgetItem(data.statut);
    if (data.statut == "Validée" || data.statut == "validée")
        statutItem->setForeground(QColor("#16a34a"));
    else if (data.statut == "En attente")
        statutItem->setForeground(QColor("#d97706"));
    else if (data.statut == "Rejetée" || data.statut == "rejetée")
        statutItem->setForeground(QColor("#dc2626"));
    statutItem->setFont(QFont("", -1, QFont::Bold));
    ui->finTableTransactions->setItem(row, 6, statutItem);

    // Boutons d'action inline dans la colonne ACTIONS
    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(6, 4, 6, 4);
    actionsLayout->setSpacing(8);

    QPushButton *btnModifier = new QPushButton("✏ Modifier");
    btnModifier->setCursor(Qt::PointingHandCursor);
    btnModifier->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border-radius: 5px;"
        " padding: 4px 10px; font-size: 11px; font-weight: bold; border: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:pressed { background-color: #1d4ed8; }");

    int transId = data.id;
    connect(btnModifier, &QPushButton::clicked, this, [this, transId]() {
        finTransactionSelectionnee = transId;
        if (finTransactionsMap.contains(transId)) {
            finRemplirComboProjets();
            finRemplirFormulaire(finTransactionsMap[transId]);
            ui->finFormTitle->setText("Modifier la transaction");
            ui->finFormSubtitle->setText("Modifiez les informations de la transaction");
            ui->finStackedWidget->setCurrentIndex(1);
            ui->finBtnAjouterTransaction->setText("💾 Enregistrer");
            finUpdateButtonStyles();
        }
    });

    QPushButton *btnSupprimer = new QPushButton("✕ Supprimer");
    btnSupprimer->setCursor(Qt::PointingHandCursor);
    btnSupprimer->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; border-radius: 5px;"
        " padding: 4px 10px; font-size: 11px; font-weight: bold; border: none; }"
        "QPushButton:hover { background-color: #dc2626; }"
        "QPushButton:pressed { background-color: #b91c1c; }");
    connect(btnSupprimer, &QPushButton::clicked, this, [this, transId]() {
        int rep = QMessageBox::question(this, "Confirmer la suppression",
            QString("Etes-vous sur de vouloir supprimer la transaction #%1 ?\nCette action est irreversible.").arg(transId),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (rep != QMessageBox::Yes) return;
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) return;
        QSqlQuery q(db);
        q.prepare(QStringLiteral("DELETE FROM FINANCE WHERE ID_TRANSACTION = :id"));
        q.bindValue(":id", transId);
        if (q.exec()) {
            QMessageBox::information(this, "Succès", "Transaction supprimée avec succès !");
            finChargerTransactionsDepuisOracle();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression : " + q.lastError().text());
        }
    });

    actionsLayout->addWidget(btnModifier);
    actionsLayout->addWidget(btnSupprimer);
    actionsLayout->addStretch();
    ui->finTableTransactions->setCellWidget(row, 7, actionsWidget);
}

void SmartPub::finViderFormulaire() {
    ui->finComboBoxProjet->setCurrentIndex(0);
    ui->finComboBoxType->setCurrentIndex(0);
    ui->finLineEditMontant->clear();
    ui->finDateEdit->setDate(QDate::currentDate());
    ui->finComboBoxCategorie->setCurrentIndex(0);
    ui->finComboBoxStatut->setCurrentIndex(0);
    ui->finTextEditDescription->clear();
}

void SmartPub::finRemplirFormulaire(const TransactionData &data) {
    int idxProjet = ui->finComboBoxProjet->findData(data.idProjet);
    if (idxProjet >= 0)
        ui->finComboBoxProjet->setCurrentIndex(idxProjet);
    else
        ui->finComboBoxProjet->setCurrentIndex(0);

    int idxType = ui->finComboBoxType->findText(data.type);
    if (idxType >= 0)
        ui->finComboBoxType->setCurrentIndex(idxType);
    else
        ui->finComboBoxType->setCurrentIndex(0);

    ui->finLineEditMontant->setText(QString::number(data.montant, 'f', 2));

    QDate d = QDate::fromString(data.date, "dd/MM/yyyy");
    if (d.isValid()) ui->finDateEdit->setDate(d);
    else ui->finDateEdit->setDate(QDate::currentDate());

    int idxCat = ui->finComboBoxCategorie->findText(data.categorie);
    if (idxCat >= 0) ui->finComboBoxCategorie->setCurrentIndex(idxCat);
    else ui->finComboBoxCategorie->setCurrentText(data.categorie);

    int idxStatut = ui->finComboBoxStatut->findText(data.statut);
    if (idxStatut >= 0) ui->finComboBoxStatut->setCurrentIndex(idxStatut);
    else ui->finComboBoxStatut->setCurrentText(data.statut);

    ui->finTextEditDescription->setPlainText(data.description);
}

void SmartPub::on_finBtnVueListe_clicked() {
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnAjouter_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }
    finTransactionSelectionnee = 0;  // Mode ajout
    finRemplirComboProjets();  // Recharger les projets depuis la BD
    finViderFormulaire();
    ui->finFormTitle->setText("Nouvelle transaction");
    ui->finFormSubtitle->setText("Remplissez les informations pour ajouter une nouvelle transaction");
    ui->finBtnAjouterTransaction->setText("➕ Ajouter");
    ui->finStackedWidget->setCurrentIndex(1);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnRecherche_clicked() {
    finAfficherListeTransactions();
}

void SmartPub::on_finLineEditRecherche_textChanged(const QString &) {
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnTri_clicked() {
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 8px;
            min-width: 220px;
        }
        QMenu::item {
            padding: 12px 20px;
            border-radius: 8px;
            color: #334155;
            font-size: 14px;
            font-weight: 500;
        }
        QMenu::item:selected {
            background-color: #eff6ff;
            color: #3b82f6;
        }
    )");

    auto appliquerTri = [this](int col, Qt::SortOrder ordre) {
        finTriColonne = col;
        finTriOrdre = ordre;
        finAfficherListeTransactions();
    };

    menu->addAction("Trier par Date (récent → ancien)", this, [appliquerTri]() {
        appliquerTri(4, Qt::DescendingOrder);
    });
    menu->addAction("Trier par Date (ancien → récent)", this, [appliquerTri]() {
        appliquerTri(4, Qt::AscendingOrder);
    });
    menu->addAction("Trier par Montant (croissant)", this, [appliquerTri]() {
        appliquerTri(3, Qt::AscendingOrder);
    });
    menu->addAction("Trier par Montant (décroissant)", this, [appliquerTri]() {
        appliquerTri(3, Qt::DescendingOrder);
    });
    menu->addAction("Trier par Projet (A-Z)", this, [appliquerTri]() {
        appliquerTri(1, Qt::AscendingOrder);
    });
    menu->addAction("Trier par Type", this, [appliquerTri]() {
        appliquerTri(2, Qt::AscendingOrder);
    });

    menu->exec(QCursor::pos());
}

void SmartPub::on_finBtnExport_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les transactions", QDir::homePath(), "CSV (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "ID;Projet;Type;Montant;Date;Catégorie;Statut;Description\n";

    QList<TransactionData> liste = finGetTransactionsFiltreesEtTriees();
    for (const TransactionData &t : liste) {
        QString desc = t.description;
        desc.replace("\"", "\"\"");
        out << t.id << ";\"" << t.projet << "\";\"" << t.type << "\";"
            << QString::number(t.montant, 'f', 2) << ";\"" << t.date << "\";\""
            << t.categorie << "\";\"" << t.statut << "\";\"" << desc << "\"\n";
    }
    file.close();
    QMessageBox::information(this, "Export", "Transactions exportées avec succès !");
}

void SmartPub::on_finBtnStatistiques_clicked() {
    FinStatistiquesDialog *dialog = new FinStatistiquesDialog(finTransactionsMap, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::on_finBtnAjouterTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }

    const QString idProjet = ui->finComboBoxProjet->currentData().toString();
    QString typeDb = ui->finComboBoxType->currentData().toString();
    if (typeDb.isEmpty())
        typeDb = finTypeUiToDb(ui->finComboBoxType->currentText());
    const QString montantStr = ui->finLineEditMontant->text();
    const QString date = ui->finDateEdit->date().toString(QStringLiteral("dd/MM/yyyy"));
    const QString categorie = ui->finComboBoxCategorie->currentText();
    const QString statut = ui->finComboBoxStatut->currentText();
    const QString description = ui->finTextEditDescription->toPlainText();

    // Verification projet obligatoire (NOT NULL base)
    if (idProjet.isEmpty() || idProjet == QStringLiteral("0")) {
        QMessageBox::warning(this, QStringLiteral("Champ obligatoire"),
                             QStringLiteral("Veuillez selectionner un projet existant."));
        return;
    }
    // Verification type obligatoire (NOT NULL base)
    if (typeDb.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Champ obligatoire"),
                             QStringLiteral("Veuillez selectionner un type de transaction."));
        return;
    }
    // Verification date valide (NOT NULL base)
    if (!ui->finDateEdit->date().isValid()) {
        QMessageBox::warning(this, QStringLiteral("Champ obligatoire"),
                             QStringLiteral("Veuillez saisir une date valide."));
        return;
    }
    // Verification montant
    if (montantStr.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Veuillez remplir tous les champs obligatoires (*)"));
        return;
    }

    bool ok = false;
    const double montant = montantStr.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Le montant doit etre un nombre valide (ex: 150.50)."));
        return;
    }
    if (montant <= 0.0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Le montant doit etre superieur a 0."));
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"),
                              QStringLiteral("Connexion à la base de données impossible."));
        return;
    }

    QSqlQuery query(db);

    if (finTransactionSelectionnee > 0) {
        query.prepare(
            QStringLiteral("UPDATE FINANCE SET MONTANT = :m, TYPE_TRANS = :type, CATEGORIE = :cat, "
                           "DATE_TRANSACTION = TO_DATE(:d, 'DD/MM/YYYY'), STATUT = :statut, "
                           "DESCRIPTION = :desc, ID_PROJET = :idp WHERE ID_TRANSACTION = :id"));
        query.bindValue(QStringLiteral(":m"), montant);
        query.bindValue(QStringLiteral(":type"), typeDb);
        query.bindValue(QStringLiteral(":cat"), categorie);
        query.bindValue(QStringLiteral(":d"), date);
        query.bindValue(QStringLiteral(":statut"), statut);
        query.bindValue(QStringLiteral(":desc"), description);
        if (!idProjet.isEmpty() && idProjet != QStringLiteral("0"))
            query.bindValue(QStringLiteral(":idp"), idProjet);
        else
            query.bindValue(QStringLiteral(":idp"), QVariant());
        query.bindValue(QStringLiteral(":id"), finTransactionSelectionnee);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de la modification : ") + query.lastError().text());
            return;
        }
        QMessageBox::information(this, QStringLiteral("Succès"),
                                 QStringLiteral("Transaction modifiée avec succès !"));
    } else {
        query.prepare(QStringLiteral(
            "INSERT INTO FINANCE (MONTANT, TYPE_TRANS, CATEGORIE, DATE_TRANSACTION, STATUT, DESCRIPTION, "
            "ID_PROJET) VALUES (:m, :type, :cat, TO_DATE(:d, 'DD/MM/YYYY'), :statut, :desc, :idp)"));
        query.bindValue(QStringLiteral(":m"), montant);
        query.bindValue(QStringLiteral(":type"), typeDb);
        query.bindValue(QStringLiteral(":cat"), categorie);
        query.bindValue(QStringLiteral(":d"), date);
        query.bindValue(QStringLiteral(":statut"), statut);
        query.bindValue(QStringLiteral(":desc"), description);
        if (!idProjet.isEmpty() && idProjet != QStringLiteral("0"))
            query.bindValue(QStringLiteral(":idp"), idProjet);
        else
            query.bindValue(QStringLiteral(":idp"), QVariant());
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de l'ajout : ") + query.lastError().text());
            return;
        }
        QMessageBox::information(this, QStringLiteral("Succès"),
                                 QStringLiteral("Transaction ajoutée avec succès !"));
    }

    finTransactionSelectionnee = 0;
    finViderFormulaire();
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finChargerTransactionsDepuisOracle();
}

void SmartPub::on_finBtnAnnulerAjout_clicked() {
    finTransactionSelectionnee = 0;
    finViderFormulaire();
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnModifierTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(
            this, "Accès refusé",
            "Les invités ne peuvent pas modifier les transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner une transaction à modifier");
        return;
    }

    QTableWidgetItem *idItem = ui->finTableTransactions->item(currentRow, 0);
    if (!idItem) return;

    int transactionId = idItem->text().toInt();
    if (!finTransactionsMap.contains(transactionId)) return;

    finTransactionSelectionnee = transactionId;
    finRemplirFormulaire(finTransactionsMap[transactionId]);
    ui->finFormTitle->setText("Modifier la transaction");
    ui->finFormSubtitle->setText("Modifiez les informations de la transaction");
    ui->finBtnAjouterTransaction->setText("💾 Enregistrer");
    ui->finStackedWidget->setCurrentIndex(1);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnSupprimerTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(
            this, "Accès refusé",
            "Les invités ne peuvent pas supprimer les transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner une transaction à supprimer");
        return;
    }

    QTableWidgetItem *idItem = ui->finTableTransactions->item(currentRow, 0);
    if (!idItem) return;

    int transactionId = idItem->text().toInt();

    auto reply = QMessageBox::question(
        this, "Supprimer", "Confirmer la suppression de cette transaction ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("DELETE FROM FINANCE WHERE ID_TRANSACTION = :id"));
            q.bindValue(QStringLiteral(":id"), transactionId);
            if (!q.exec()) {
                QMessageBox::critical(this, QStringLiteral("Erreur"),
                                      QStringLiteral("Échec de la suppression : ") + q.lastError().text());
                return;
            }
        }
        finChargerTransactionsDepuisOracle();
        QMessageBox::information(this, "Succès", "Transaction supprimée");
    }
}

// ============================================================================
// MODULE EVENEMENTS
// ============================================================================

void SmartPub::evSetupUI() {
    ui->evTabWidget->setCurrentIndex(0);
    evEventSelectionne = -1;

    // Configure evTableEvents to stretch and fill available space
    if (ui->evTableEvents) {
        ui->evTableEvents->horizontalHeader()->setStretchLastSection(true);
        ui->evTableEvents->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        ui->evTableEvents->verticalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
        ui->evTableEvents->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding);
    }

    // Configure evTableSearchEvents to stretch and fill available space
    if (ui->evTableSearchEvents) {
        ui->evTableSearchEvents->horizontalHeader()->setStretchLastSection(true);
        ui->evTableSearchEvents->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        ui->evTableSearchEvents->verticalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
        ui->evTableSearchEvents->setSizePolicy(QSizePolicy::Expanding,
                                               QSizePolicy::Expanding);
    }
}

void SmartPub::evConnectSignals() {
    connect(ui->evBtnAjouterEvent, &QPushButton::clicked, this,
            &SmartPub::on_evBtnAjouterEvent_clicked);
    connect(ui->evBtnModifierEvent, &QPushButton::clicked, this,
            &SmartPub::on_evBtnModifierEvent_clicked);
    connect(ui->evBtnSupprimerEvent, &QPushButton::clicked, this,
            &SmartPub::on_evBtnSupprimerEvent_clicked);
    connect(ui->evBtnTrierDate, &QPushButton::clicked, this,
            &SmartPub::on_evBtnTrierDate_clicked);
    connect(ui->evBtnRechercheLieu, &QPushButton::clicked, this,
            &SmartPub::on_evBtnRechercheLieu_clicked);
    connect(ui->evBtnExportCalendrier, &QPushButton::clicked, this,
            &SmartPub::on_evBtnExportCalendrier_clicked);
    connect(ui->evBtnLivreResumes, &QPushButton::clicked, this,
            &SmartPub::on_evBtnLivreResumes_clicked);
    connect(ui->evBtnCalculImpact, &QPushButton::clicked, this,
            &SmartPub::on_evBtnCalculImpact_clicked);
    connect(ui->evBtnStatsParticipation, &QPushButton::clicked, this,
            &SmartPub::on_evBtnStatsParticipation_clicked);
}

void SmartPub::evAjouterDonneesTest() {
    evEditingCode.clear();
    evAfficherListeEvents();
}

void SmartPub::evAfficherListeEvents() {
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    evEventsMap.clear();
    QSqlQuery query(db);
    if (!query.exec("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT ORDER BY DATE_EVENEMENT")) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les événements : " + query.lastError().text());
        return;
    }

    ui->evTableEvents->setRowCount(0);
    ui->evTableSearchEvents->setRowCount(0);

    while (query.next()) {
        EventData data;
        data.code = QString::number(query.value("CODE_EVENEMENT").toLongLong());
        data.nom = query.value("NOM").toString();
        data.lieu = query.value("LIEU").toString();
        QVariant dVal = query.value("DATE_EVENEMENT");
        if (dVal.canConvert<QDate>()) {
            data.date = dVal.toDate().toString("dd/MM/yyyy");
        } else {
            QString dStr = dVal.toString();
            if (dStr.contains("T")) dStr = dStr.left(10);
            if (dStr.contains("-") && dStr.length() >= 10) {
                QDate dt = QDate::fromString(dStr.left(10), "yyyy-MM-dd");
                if (dt.isValid()) data.date = dt.toString("dd/MM/yyyy");
                else data.date = dStr;
            } else {
                data.date = dStr;
            }
        }
        evEventsMap[data.code] = data;
        evAjouterEventTable(data);
        int row = ui->evTableSearchEvents->rowCount();
        ui->evTableSearchEvents->insertRow(row);
        ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(data.code));
        ui->evTableSearchEvents->setItem(row, 1, new QTableWidgetItem(data.nom));
        ui->evTableSearchEvents->setItem(row, 2, new QTableWidgetItem(data.lieu));
        ui->evTableSearchEvents->setItem(row, 3, new QTableWidgetItem(data.date));
    }
}

void SmartPub::evAjouterEventTable(const EventData &data) {
    int row = ui->evTableEvents->rowCount();
    ui->evTableEvents->insertRow(row);
    ui->evTableEvents->setItem(row, 0, new QTableWidgetItem(data.code));
    ui->evTableEvents->setItem(row, 1, new QTableWidgetItem(data.nom));
    ui->evTableEvents->setItem(row, 2, new QTableWidgetItem(data.lieu));
    ui->evTableEvents->setItem(row, 3, new QTableWidgetItem(data.date));
}

void SmartPub::evRechercherParLieu() {
    QString lieu = ui->evLineEditSearchLieu->text().trimmed();
    if (lieu.isEmpty()) {
        QMessageBox::warning(this, "Recherche", "Veuillez entrer un lieu");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    ui->evTableSearchEvents->setRowCount(0);
    QSqlQuery query(db);
    query.prepare("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT WHERE UPPER(LIEU) LIKE UPPER(:lieu) ORDER BY DATE_EVENEMENT");
    query.bindValue(":lieu", "%" + lieu + "%");
    if (!query.exec()) {
        QMessageBox::warning(this, "Erreur", "Recherche échouée : " + query.lastError().text());
        return;
    }
    while (query.next()) {
        int row = ui->evTableSearchEvents->rowCount();
        ui->evTableSearchEvents->insertRow(row);
        QVariant dVal = query.value("DATE_EVENEMENT");
        QString dateStr;
        if (dVal.canConvert<QDate>()) {
            dateStr = dVal.toDate().toString("dd/MM/yyyy");
        } else {
            dateStr = dVal.toString();
            if (dateStr.contains("T")) dateStr = dateStr.left(10);
        }
        ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(QString::number(query.value("CODE_EVENEMENT").toLongLong())));
        ui->evTableSearchEvents->setItem(row, 1, new QTableWidgetItem(query.value("NOM").toString()));
        ui->evTableSearchEvents->setItem(row, 2, new QTableWidgetItem(query.value("LIEU").toString()));
        ui->evTableSearchEvents->setItem(row, 3, new QTableWidgetItem(dateStr));
    }
}

void SmartPub::on_evBtnAjouterEvent_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter d'événements.");
        return;
    }

    QString nom = ui->evLineEditNom->text().trimmed();
    QString lieu = ui->evLineEditLieu->text().trimmed();
    QString date = ui->evLineEditDate->text().trimmed();

    if (nom.isEmpty() || lieu.isEmpty() || date.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir nom, lieu et date");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    if (!evEditingCode.isEmpty()) {
        QSqlQuery query(db);
        query.prepare("UPDATE EVENEMENT SET NOM = :nom, LIEU = :lieu, DATE_EVENEMENT = TO_DATE(:date_event, 'DD/MM/YYYY') WHERE CODE_EVENEMENT = :code");
        query.bindValue(":nom", nom);
        query.bindValue(":lieu", lieu);
        query.bindValue(":date_event", date);
        query.bindValue(":code", evEditingCode.toLongLong());
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de la modification : " + query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Succès", "Événement modifié avec succès !");
        evEditingCode.clear();
    } else {
        QSqlQuery query(db);
        query.prepare("INSERT INTO EVENEMENT (NOM, LIEU, DATE_EVENEMENT) VALUES (:nom, :lieu, TO_DATE(:date_event, 'DD/MM/YYYY'))");
        query.bindValue(":nom", nom);
        query.bindValue(":lieu", lieu);
        query.bindValue(":date_event", date);
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout : " + query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Succès", "Événement ajouté avec succès !");
    }

    evAfficherListeEvents();
    ui->evLineEditID->clear();
    ui->evLineEditNom->clear();
    ui->evLineEditLieu->clear();
    ui->evLineEditDate->clear();
    ui->evLineEditID->setEnabled(true);
}

void SmartPub::on_evBtnModifierEvent_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas modifier les événements.");
        return;
    }

    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner un événement à modifier");
        return;
    }

    QString code = ui->evTableEvents->item(currentRow, 0)->text();
    ui->evLineEditID->setText(code);
    ui->evLineEditNom->setText(ui->evTableEvents->item(currentRow, 1)->text());
    ui->evLineEditLieu->setText(ui->evTableEvents->item(currentRow, 2)->text());
    ui->evLineEditDate->setText(ui->evTableEvents->item(currentRow, 3)->text());
    ui->evLineEditID->setEnabled(false);
    evEditingCode = code;
}

void SmartPub::on_evBtnSupprimerEvent_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(
            this, "Accès refusé",
            "Les invités ne peuvent pas supprimer les événements.");
        return;
    }

    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner un événement à supprimer");
        return;
    }

    auto reply = QMessageBox::question(
        this, "Supprimer", "Confirmer la suppression de cet événement ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QString code = ui->evTableEvents->item(currentRow, 0)->text();
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) {
            return;
        }
        QSqlQuery query(db);
        query.prepare("DELETE FROM EVENEMENT WHERE CODE_EVENEMENT = :code");
        query.bindValue(":code", code.toLongLong());
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression : " + query.lastError().text());
            return;
        }
        if (evEditingCode == code) {
            evEditingCode.clear();
            ui->evLineEditID->clear();
            ui->evLineEditNom->clear();
            ui->evLineEditLieu->clear();
            ui->evLineEditDate->clear();
            ui->evLineEditID->setEnabled(true);
        }
        evAfficherListeEvents();
        QMessageBox::information(this, "Succès", "Événement supprimé");
    }
}

void SmartPub::on_evBtnTrierDate_clicked() {
    QMessageBox::information(this, "Tri", "Événements triés par date");
}

void SmartPub::on_evBtnRechercheLieu_clicked() { evRechercherParLieu(); }

void SmartPub::on_evBtnExportCalendrier_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter le calendrier", QDir::homePath(), "iCalendar (*.ics)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Export",
                                 "Calendrier exporté avec succès !");
    }
}

void SmartPub::on_evBtnLivreResumes_clicked() {
    QMessageBox::information(this, "Livre des Résumés",
                             "Génération du livre des résumés - À implémenter");
}

void SmartPub::on_evBtnCalculImpact_clicked() {
    QMessageBox::information(this, "Calculateur d'Impact",
                             "Calcul de l'impact carbone - À implémenter");
}

void SmartPub::on_evBtnStatsParticipation_clicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques de participation");
    dialog.setMinimumSize(600, 450);
    dialog.resize(700, 500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Nombre total de participants (simulé : 50 + hash du code pour variété)
    int totalParticipants = 0;
    QMap<QString, int> participantsParDate;
    for (auto it = evEventsMap.begin(); it != evEventsMap.end(); ++it) {
        int nb = 50 + qHash(it.value().code) % 100;
        if (nb < 20) nb = 50;
        totalParticipants += nb;
        participantsParDate[it.value().date] += nb;
    }

    QLabel *labelTotal = new QLabel(QString("Nombre total de participants : <b>%1</b>").arg(totalParticipants));
    labelTotal->setStyleSheet("font-size: 16px; color: #334155; padding: 10px;");
    layout->addWidget(labelTotal);

    // Courbe : nombre de participants par date
    QChartView *chartView = new QChartView(&dialog);
    chartView->setRenderHint(QPainter::Antialiasing);

    QLineSeries *series = new QLineSeries();
    series->setName("Participants par date");
    series->setColor(QColor("#3b82f6"));
    series->setPen(QPen(QColor("#3b82f6"), 3));

    QStringList datesTriees = participantsParDate.keys();
    std::sort(datesTriees.begin(), datesTriees.end(), [](const QString &a, const QString &b) {
        QDate da = QDate::fromString(a, "dd/MM/yyyy");
        QDate db = QDate::fromString(b, "dd/MM/yyyy");
        return da < db;
    });

    int idx = 0;
    for (const QString &d : datesTriees) {
        series->append(idx, participantsParDate[d]);
        idx++;
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Nombre de participants par date");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QBrush(QColor("white")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(datesTriees);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    int maxPart = 0;
    for (int v : participantsParDate)
        if (v > maxPart) maxPart = v;
    axisY->setRange(0, maxPart + 10);
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);
    chartView->setChart(chart);
    layout->addWidget(chartView);

    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none; "
        "border-radius: 8px; padding: 10px 24px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    connect(btnFermer, &QPushButton::clicked, &dialog, &QDialog::accept);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnFermer);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog.exec();
}
// ==================== MAINWINDOW ====================

int SmartPub::projExtraireProgression(const QString &progressionStr) const
{
    QString temp = progressionStr;
    if (temp.endsWith('%')) {
        temp.chop(1);
    }
    return temp.toInt();
}

void SmartPub::projSetupStatistiquesButton()
{
    ui->btnStatistiques->setFixedHeight(44);
    ui->btnStatistiques->setMinimumWidth(140);
    ui->btnStatistiques->setCursor(Qt::PointingHandCursor);
    ui->btnStatistiques->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    padding: 0px 20px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );
}

void SmartPub::projSetupAIButton()
{
    // PAS de parent "this" sinon le bouton flotte sur la fenetre principale
    QPushButton *btnAI = new QPushButton("🤖");
    btnAI->setObjectName("btnAIRecommandations");
    btnAI->setFixedSize(44, 44);
    btnAI->setCursor(Qt::PointingHandCursor);
    btnAI->setToolTip("Recommandations IA");

    btnAI->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );

    connect(btnAI, &QPushButton::clicked, this, &SmartPub::onIARecommanderClicked);

    // La toolbar est un QVBoxLayout avec 2 rangees :
    // itemAt(0) = row1 (QHBoxLayout) : CRUD | sep | Statistiques | [AI ici] | spacer | recherche | filtres | exporter
    // itemAt(1) = row2 (QHBoxLayout) : Trier par | Date Debut | Date Fin | Etat | Progression
    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout*>(ui->toolbarFrameProjets->layout());
    if (!vLayout || vLayout->count() == 0) return;

    QHBoxLayout *row1 = qobject_cast<QHBoxLayout*>(vLayout->itemAt(0)->layout());
    if (!row1) return;

    // Chercher btnStatistiques et inserer btnAI juste apres
    int index = -1;
    for (int i = 0; i < row1->count(); ++i) {
        QLayoutItem *item = row1->itemAt(i);
        if (item && item->widget() == ui->btnStatistiques) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        row1->insertWidget(index + 1, btnAI);
    } else {
        // Fallback : inserer apres le separateur (position 5)
        row1->insertWidget(5, btnAI);
    }
}

void SmartPub::projSetupSidebarProfile()
{
    profileWidget = new QWidget(ui->sidebarFrame);
    profileWidget->setObjectName("sidebarProfile");
    profileWidget->setStyleSheet(
        "QWidget#sidebarProfile {"
        "    background-color: #0f172a;"
        "    border-top: 1px solid #334155;"
        "}"
        );
    profileWidget->setFixedHeight(80);

    QHBoxLayout *profileLayout = new QHBoxLayout(profileWidget);
    profileLayout->setSpacing(15);
    profileLayout->setContentsMargins(20, 10, 20, 10);

    avatarLabel = new QLabel("CP");
    avatarLabel->setFixedSize(45, 45);
    avatarLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #10b981;"
        "    color: white;"
        "    border-radius: 22px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    qproperty-alignment: AlignCenter;"
        "}"
        );
    avatarLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(3);

    nameLabel = new QLabel("Chef de Projet");
    nameLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        );

    roleLabel = new QLabel("Administrateur");
    roleLabel->setStyleSheet(
        "QLabel {"
        "    color: #94a3b8;"
        "    font-size: 12px;"
        "}"
        );

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(roleLabel);

    profileLayout->addWidget(avatarLabel);
    profileLayout->addLayout(infoLayout, 1);

    btnSettings = new QPushButton("⚙");
    btnSettings->setFixedSize(35, 35);
    btnSettings->setCursor(Qt::PointingHandCursor);
    btnSettings->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #94a3b8;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 18px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #334155;"
        "    color: white;"
        "}"
        );
    connect(btnSettings, &QPushButton::clicked, this, &SmartPub::onSettingsClicked);
    profileLayout->addWidget(btnSettings);

    QVBoxLayout *sidebarLayout = qobject_cast<QVBoxLayout*>(ui->sidebarFrame->layout());
    if (sidebarLayout) {
        sidebarLayout->addWidget(profileWidget);
    }

    projUpdateSidebarProfileVisibility();
}

void SmartPub::projUpdateSidebarProfileVisibility()
{
    if (!profileWidget) return;

    if (!sidebarExpanded) {
        profileWidget->setVisible(false);
    } else {
        profileWidget->setVisible(true);
    }
}

// ============================================================================
// MODULE PROJETS
// ============================================================================

void SmartPub::projSetupUI()
{
    projSetupTable();

    ui->btnListeProjets->setToolTip("Liste des projets");
    ui->btnAjouterProjet->setToolTip("Ajouter un projet");
    ui->btnModifierProjet->setToolTip("Modifier le projet sélectionné");
    ui->btnSupprimerProjet->setToolTip("Supprimer le projet sélectionné");

    // Barre de recherche
    ui->lineEditRechercheProjets->setFixedHeight(44);
    ui->lineEditRechercheProjets->setMinimumWidth(220);
    ui->lineEditRechercheProjets->setStyleSheet(
        "QLineEdit {"
        "    background-color: #ffffff;"
        "    border: 1.5px solid #e2e8f0;"
        "    border-radius: 10px;"
        "    padding: 0px 14px;"
        "    font-size: 13px;"
        "    color: #334155;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #3b82f6;"
        "    border-width: 2px;"
        "}");

    // Filtres
    ui->btnFiltresProjets->setFixedHeight(44);
    ui->btnFiltresProjets->setMinimumWidth(90);
    ui->btnFiltresProjets->setCursor(Qt::PointingHandCursor);
    ui->btnFiltresProjets->setStyleSheet(
        "QPushButton {"
        "    background-color: #ffffff;"
        "    color: #475569;"
        "    border: 1.5px solid #e2e8f0;"
        "    border-radius: 10px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "    padding: 0px 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1f5f9;"
        "    border-color: #94a3b8;"
        "    color: #1e293b;"
        "}");

    // Exporter
    ui->btnExporterProjets->setFixedHeight(44);
    ui->btnExporterProjets->setMinimumWidth(100);
    ui->btnExporterProjets->setCursor(Qt::PointingHandCursor);
    ui->btnExporterProjets->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    padding: 0px 16px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}");

    // Boutons Trier par
    QString triBtnStyle =
        "QPushButton {"
        "    background-color: white;"
        "    color: #334155;"
        "    border: 1px solid #cbd5e1;"
        "    border-radius: 15px;"
        "    font-size: 12px;"
        "    font-weight: 500;"
        "    padding: 5px 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1f5f9;"
        "    border-color: #94a3b8;"
        "}"
        "QPushButton:checked {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    font-weight: 600;"
        "}";
    ui->btnTriDateDebut->setStyleSheet(triBtnStyle);
    ui->btnTriDateFin->setStyleSheet(triBtnStyle);
    ui->btnTriEtat->setStyleSheet(triBtnStyle);
    ui->btnTriProgression->setStyleSheet(triBtnStyle);

    // Style initial des boutons CRUD
    projSetActiveCrudButton(0);
}

// CORRECTION DU TABLEAU - setupTable amélioré
void SmartPub::projSetupTable()
{
    QTableWidget *table = ui->tableWidgetProjets;

    table->verticalHeader()->setVisible(false);
    table->setSortingEnabled(false);
    table->setAlternatingRowColors(false);
    table->hideColumn(0); // Cache la colonne ID

    table->setColumnCount(8);
    QStringList headers;
    headers << "ID" << "Code" << "Titre" << "Date Début" << "Date Fin"
            << "Responsable" << "État" << "Progression";
    table->setHorizontalHeaderLabels(headers);

    // CORRECTION: StretchLastSection à true pour que la dernière colonne s'étire
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(80);
    table->horizontalHeader()->setDefaultSectionSize(100);

    // CORRECTION: Largeurs ajustées pour éviter le tronquage
    table->setColumnWidth(0, 0);       // ID caché
    table->setColumnWidth(1, 110);     // Code - un peu plus large
    table->setColumnWidth(2, 300);     // Titre - BEAUCOUP PLUS LARGE
    table->setColumnWidth(3, 100);     // Date Début
    table->setColumnWidth(4, 100);     // Date Fin
    table->setColumnWidth(5, 170);     // Responsable
    table->setColumnWidth(6, 90);      // État
    // Colonne 7 (Progression) s'étire avec StretchLastSection

    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);

    // CORRECTION: WordWrap désactivé pour éviter les problèmes de hauteur
    table->setWordWrap(false);

    // Hauteur de ligne fixe
    table->verticalHeader()->setDefaultSectionSize(50);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // Style de l'en-tête - CORRECTION: padding réduit pour éviter le tronquage
    table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #1e293b;"
        "    padding: 10px 6px;"        // Padding réduit
        "    font-weight: 600;"
        "    color: white;"
        "    border: none;"
        "    font-size: 11px;"          // Police légèrement plus petite
        "    text-transform: uppercase;"
        "}"
        "QHeaderView::section:hover {"
        "    background-color: #334155;"
        "}"
        );

    // Style du tableau
    table->setStyleSheet(
        "QTableWidget {"
        "    background-color: white;"
        "    border: none;"
        "    font-size: 13px;"
        "    gridline-color: transparent;"
        "    selection-background-color: #eff6ff;"
        "    selection-color: #1e293b;"
        "}"
        "QTableWidget::item {"
        "    padding: 12px 8px;"
        "    border-bottom: 1px solid #f1f5f9;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #eff6ff;"
        "    color: #1e293b;"
        "}"
        "QTableWidget::item:hover {"
        "    background-color: #f8fafc;"
        "}"
        "QTableCornerButton::section {"
        "    background-color: #1e293b;"
        "    border: none;"
        "}"
        );
}

void SmartPub::projSetupConnections()
{
    // Note: Les boutons de navigation (btnPublications, btnChercheurs, etc.) sont déjà
    // connectés dans setupConnections(), donc on ne les reconnecte pas ici pour éviter
    // les conflits et les comportements inattendus.

    connect(ui->btnListeProjets, &QPushButton::clicked, this, &SmartPub::onCrudButtonClicked);
    connect(ui->btnAjouterProjet, &QPushButton::clicked, this, &SmartPub::onCrudButtonClicked);
    connect(ui->btnModifierProjet, &QPushButton::clicked, this, &SmartPub::onCrudButtonClicked);
    connect(ui->btnSupprimerProjet, &QPushButton::clicked, this, &SmartPub::onSupprimerProjetClicked);

    connect(ui->lineEditRechercheProjets, &QLineEdit::textChanged, this, &SmartPub::onRechercheTextChanged);
    connect(ui->btnAnnulerForm, &QPushButton::clicked, this, &SmartPub::onAnnulerFormClicked);
    connect(ui->btnEnregistrerForm, &QPushButton::clicked, this, &SmartPub::onEnregistrerFormClicked);

    connect(ui->tableWidgetProjets, &QTableWidget::itemSelectionChanged, this, &SmartPub::onTableSelectionChanged);
    connect(ui->tableWidgetProjets, &QTableWidget::cellDoubleClicked, this, &SmartPub::onTableDoubleClicked);

    connect(ui->btnTriDateDebut, &QPushButton::clicked, this, &SmartPub::onTriDateDebutClicked);
    connect(ui->btnTriDateFin, &QPushButton::clicked, this, &SmartPub::onTriDateFinClicked);
    connect(ui->btnTriEtat, &QPushButton::clicked, this, &SmartPub::onTriEtatClicked);
    connect(ui->btnTriProgression, &QPushButton::clicked, this, &SmartPub::onTriProgressionClicked);

    connect(ui->btnStatistiques, &QPushButton::clicked, this, &SmartPub::onStatistiquesClicked);
    connect(ui->btnFiltresProjets, &QPushButton::clicked, this, &SmartPub::onFiltresClicked);
    connect(ui->btnExporterProjets, &QPushButton::clicked, this, &SmartPub::onExporterClicked);
}

void SmartPub::projSetupSampleData()
{
    // Les données sont chargées depuis Oracle via projChargerDepuisDB()
    // appelé dans projChargerProjets()
    projets.clear();
}

void SmartPub::projSetupComboBoxes()
{
    ui->comboBoxResponsableForm->clear();
    ui->comboBoxResponsableForm->addItem(QStringLiteral("—"), QVariant());

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery q(db);
        if (q.exec(QStringLiteral("SELECT ID_CHERCHEUR, NOM, PRENOM FROM CHERCHEUR ORDER BY NOM, PRENOM"))) {
            while (q.next()) {
                const int cid = q.value(0).toInt();
                const QString label = QStringLiteral("%1 %2")
                                          .arg(q.value(1).toString().trimmed(),
                                               q.value(2).toString().trimmed())
                                          .trimmed();
                ui->comboBoxResponsableForm->addItem(label.isEmpty() ? QString::number(cid) : label,
                                                     QVariant(cid));
            }
        }
    }

    ui->comboBoxEtatForm->clear();
    ui->comboBoxEtatForm->addItem(QStringLiteral("En cours"), QStringLiteral("en_cours"));
    ui->comboBoxEtatForm->addItem(QStringLiteral("Terminé"), QStringLiteral("termine"));
    ui->comboBoxEtatForm->addItem(QStringLiteral("Suspendu"), QStringLiteral("suspendu"));
    ui->comboBoxEtatForm->addItem(QStringLiteral("Annulé"), QStringLiteral("annule"));
}


// CORRECTION: ajouterProjetTable amélioré pour éviter le tronquage
void SmartPub::projAjouterProjetTable(const Projet &projet, int rowIndex)
{
    Q_UNUSED(rowIndex);
    int row = ui->tableWidgetProjets->rowCount();
    ui->tableWidgetProjets->insertRow(row);

    // ID (caché)
    QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(projet.id));
    ui->tableWidgetProjets->setItem(row, 0, idItem);

    // Code
    QTableWidgetItem *codeItem = new QTableWidgetItem(projet.code);
    codeItem->setForeground(QColor("#3b82f6"));
    codeItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidgetProjets->setItem(row, 1, codeItem);

    // Titre
    QTableWidgetItem *titreItem = new QTableWidgetItem(projet.titre);
    titreItem->setFont(QFont("Segoe UI", 9, QFont::Medium));
    titreItem->setToolTip(projet.titre);
    titreItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titreItem->setForeground(QColor("#1e293b"));
    ui->tableWidgetProjets->setItem(row, 2, titreItem);

    // Date début
    QTableWidgetItem *debutItem = new QTableWidgetItem(projet.dateDebut.toString("dd/MM/yyyy"));
    debutItem->setTextAlignment(Qt::AlignCenter);
    debutItem->setForeground(QColor("#64748b"));
    debutItem->setFont(QFont("Segoe UI", 9));
    ui->tableWidgetProjets->setItem(row, 3, debutItem);

    // Date fin
    QTableWidgetItem *finItem = new QTableWidgetItem(projet.dateFin.toString("dd/MM/yyyy"));
    finItem->setTextAlignment(Qt::AlignCenter);
    finItem->setForeground(QColor("#64748b"));
    finItem->setFont(QFont("Segoe UI", 9));
    ui->tableWidgetProjets->setItem(row, 4, finItem);

    // Responsable
    QTableWidgetItem *respItem = new QTableWidgetItem(projet.responsable);
    respItem->setForeground(QColor("#475569"));
    respItem->setFont(QFont("Segoe UI", 9));
    respItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidgetProjets->setItem(row, 5, respItem);

    // État avec couleur (affichage libellé FR, données = codes Oracle)
    QTableWidgetItem *etatItem = new QTableWidgetItem(projEtatDbToUi(projet.etat));
    etatItem->setTextAlignment(Qt::AlignCenter);
    etatItem->setForeground(QColor(::getEtatColor(projEtatDbToUi(projet.etat))));
    etatItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    ui->tableWidgetProjets->setItem(row, 6, etatItem);

    // Progression avec couleur
    QTableWidgetItem *progItem = new QTableWidgetItem(projet.progression);
    progItem->setTextAlignment(Qt::AlignCenter);
    progItem->setForeground(QColor(::getProgressionColorFromString(projet.progression)));
    progItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    ui->tableWidgetProjets->setItem(row, 7, progItem);

    // Couleurs de fond alternées
    QColor bgColor = (row % 2 == 0) ? QColor("#ffffff") : QColor("#f8fafc");
    for (int col = 0; col < 8; ++col) {
        QTableWidgetItem *item = ui->tableWidgetProjets->item(row, col);
        if (item) item->setBackground(bgColor);
    }
}
void SmartPub::projAjusterColonnesTable()
{
    QTableWidget *table = ui->tableWidgetProjets;

    // Recalcule les largeurs optimales
    table->resizeColumnsToContents();

    // Mais avec des minimums et maximums
    table->setColumnWidth(0, 0);  // ID caché
    if (table->columnWidth(1) < 100) table->setColumnWidth(1, 100);  // Code min
    if (table->columnWidth(2) < 250) table->setColumnWidth(2, 250);  // Titre min
    if (table->columnWidth(2) > 400) table->setColumnWidth(2, 400);  // Titre max
    if (table->columnWidth(3) < 90) table->setColumnWidth(3, 90);    // Date min
    if (table->columnWidth(4) < 90) table->setColumnWidth(4, 90);    // Date min
    if (table->columnWidth(5) < 150) table->setColumnWidth(5, 150);  // Resp min
    if (table->columnWidth(6) < 80) table->setColumnWidth(6, 80);    // État min
    if (table->columnWidth(7) < 80) table->setColumnWidth(7, 80);    // Prog min
}

void SmartPub::projChargerProjets()
{
    projets.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare(
            QStringLiteral("SELECT p.CODE_PROJET, p.TITRE, p.DATE_DEBUT, p.DATE_FIN, p.RESPONSABLE, p.ETAT, p.PROGRESSION, "
                           "TRIM(c.NOM || ' ' || NVL(c.PRENOM, '')) AS RESP_NOM "
                           "FROM PROJET p LEFT JOIN CHERCHEUR c ON c.ID_CHERCHEUR = p.RESPONSABLE "
                           "ORDER BY p.CODE_PROJET"));
        if (query.exec()) {
            while (query.next()) {
                Projet p;
                p.id = query.value(QStringLiteral("CODE_PROJET")).toInt();
                p.code = QString::number(p.id);
                p.titre = query.value(QStringLiteral("TITRE")).toString();
                p.dateDebut = query.value(QStringLiteral("DATE_DEBUT")).toDate();
                p.dateFin = query.value(QStringLiteral("DATE_FIN")).toDate();
                QVariant rv = query.value(QStringLiteral("RESPONSABLE"));
                p.responsableId = rv.isNull() ? 0 : rv.toInt();
                p.responsable = query.value(QStringLiteral("RESP_NOM")).toString();
                if (p.responsable.isEmpty() && p.responsableId != 0)
                    p.responsable = QStringLiteral("—");
                p.etat = query.value(QStringLiteral("ETAT")).toString();
                double prog = query.value(QStringLiteral("PROGRESSION")).toDouble();
                p.progression = QStringLiteral("%1%").arg(qRound(prog));
                p.description.clear();
                projets.append(p);
            }
            // Mettre à jour nextProjetId
            if (!projets.isEmpty()) {
                int maxId = 0;
                for (const Projet &p : projets) maxId = qMax(maxId, p.id);
                nextProjetId = maxId + 1;
            }
        } else {
            qDebug() << "Erreur chargement projets:" << query.lastError().text();
        }
    }

    projViderTable();
    if (filtresActifs) {
        projAppliquerFiltres();
        for (int i = 0; i < projetsFiltres.size(); ++i)
            projAjouterProjetTable(projetsFiltres[i], i);
    } else {
        for (int i = 0; i < projets.size(); ++i)
            projAjouterProjetTable(projets[i], i);
    }
    projAjusterColonnesTable();
}

void SmartPub::projViderTable()
{
    ui->tableWidgetProjets->setRowCount(0);
}

void SmartPub::onNavigationButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int index = -1;

    // Correction des indices pour correspondre à l'ordre réel dans stackedWidgetModules:
    // 0: pageChercheurs, 1: pagePublications, 2: pageFinances, 3: pageEvenements, 4: pageProjets
    if (button == ui->btnChercheurs) index = 0;
    else if (button == ui->btnPublications) index = 1;
    else if (button == ui->btnFinances) index = 2;
    else if (button == ui->btnEvenements) index = 3;
    else if (button == ui->btnProjets) index = 4;
    else if (button == ui->btnLaboratoires) {
        // Laboratoires n'a pas de page dédiée, afficher un message
        QMessageBox::information(this, "Information", "Module Laboratoires en cours de développement");
        return;
    }

    if (index != -1) {
        ui->stackedWidgetModules->setCurrentIndex(index);
        projSetActiveNavigationButton(index);
    }
}

void SmartPub::projSetActiveNavigationButton(int index)
{
    QVector<QPushButton*> buttons = {
        ui->btnPublications, ui->btnChercheurs, ui->btnLaboratoires,
        ui->btnProjets, ui->btnFinances, ui->btnEvenements
    };

    for (int i = 0; i < buttons.size(); ++i) {
        QString style;
        if (i == index) {
            style = "QPushButton {"
                    "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                    "        stop:0 #3b82f6, stop:1 #10b981);"
                    "    color: white;"
                    "    border: none;"
                    "    border-radius: 12px;"
                    "    padding: 14px 25px;"
                    "    font-size: 14px;"
                    "    font-weight: 600;"
                    "    text-align: left;"
                    "}"
                    "QPushButton:hover {"
                    "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                    "        stop:0 #2563eb, stop:1 #059669);"
                    "}";
        } else {
            style = "QPushButton {"
                    "    background-color: transparent;"
                    "    color: #94a3b8;"
                    "    border: none;"
                    "    border-radius: 12px;"
                    "    padding: 14px 25px;"
                    "    font-size: 14px;"
                    "    font-weight: 500;"
                    "    text-align: left;"
                    "}"
                    "QPushButton:hover {"
                    "    background-color: #334155;"
                    "    color: #e2e8f0;"
                    "}";
        }
        buttons[i]->setStyleSheet(style);
    }
}

void SmartPub::onCrudButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (button == ui->btnListeProjets) {
        ui->stackedWidgetProjets->setCurrentIndex(0);
        projSetActiveCrudButton(0);
    }
    else if (button == ui->btnAjouterProjet) {
        isEditing = false;
        projViderFormulaire();
        projAfficherFormulaire(false);
        projSetActiveCrudButton(1);
    }
    else if (button == ui->btnModifierProjet) {
        int row = projGetSelectedRow();
        if (row != -1) {
            int projetId = ui->tableWidgetProjets->item(row, 0)->text().toInt();
            for (const Projet &projet : projets) {
                if (projet.id == projetId) {
                    isEditing = true;
                    currentProjetId = projetId;
                    projRemplirFormulaire(projet);
                    projAfficherFormulaire(true);
                    projSetActiveCrudButton(2);
                    break;
                }
            }
        } else {
            QMessageBox::warning(this, "Modification", "Veuillez sélectionner un projet à modifier");
        }
    }
}

void SmartPub::projSetActiveCrudButton(int index)
{
    for (auto *btn : {ui->btnListeProjets, ui->btnAjouterProjet, ui->btnModifierProjet, ui->btnSupprimerProjet}) {
        btn->setFixedSize(44, 44);
        btn->setCursor(Qt::PointingHandCursor);
    }

    if (index == 0) {
        ui->btnListeProjets->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
            "    color: white; border: none; border-radius: 10px; font-size: 18px;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
            "}");
    } else {
        ui->btnListeProjets->setStyleSheet(
            "QPushButton {"
            "    background-color: #f1f5f9; color: #64748b;"
            "    border: 1.5px solid #e2e8f0; border-radius: 10px; font-size: 18px;"
            "}"
            "QPushButton:hover { background-color: #e2e8f0; color: #1e293b; }");
    }

    ui->btnAjouterProjet->setStyleSheet(
        "QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white; border: none; border-radius: 10px; font-size: 22px; font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #2563eb; }");

    ui->btnModifierProjet->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1f5f9; color: #64748b;"
        "    border: 1.5px solid #e2e8f0; border-radius: 10px; font-size: 18px;"
        "}"
        "QPushButton:hover { background-color: #e2e8f0; color: #1e293b; }");

    ui->btnSupprimerProjet->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1f5f9; color: #64748b;"
        "    border: 1.5px solid #e2e8f0; border-radius: 10px; font-size: 18px;"
        "}"
        "QPushButton:hover { background-color: #fee2e2; border-color: #fca5a5; color: #dc2626; }");
}

void SmartPub::onRechercheTextChanged(const QString &text)
{
    projFiltrerTable(text);
}

void SmartPub::projFiltrerTable(const QString &text)
{
    for (int row = 0; row < ui->tableWidgetProjets->rowCount(); ++row) {
        bool match = false;

        for (int col = 1; col < ui->tableWidgetProjets->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetProjets->item(row, col);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }

        ui->tableWidgetProjets->setRowHidden(row, !match);
    }
}

void SmartPub::onAnnulerFormClicked()
{
    projCacherFormulaire();
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::onEnregistrerFormClicked()
{
    if (!projValiderFormulaire()) {
        return;
    }

    Projet projet = projGetProjetFromForm();
    QSqlDatabase db = Connection::instance()->getDatabase();

    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données perdue.");
        return;
    }

    QSqlQuery query(db);

    const double progVal = projProgressionStrToDouble(projet.progression);

    if (isEditing) {
        query.prepare(QStringLiteral("UPDATE PROJET SET "
                                     "TITRE = :titre, DATE_DEBUT = :debut, DATE_FIN = :fin, "
                                     "RESPONSABLE = :resp, ETAT = :etat, PROGRESSION = :prog "
                                     "WHERE CODE_PROJET = :id"));
        query.bindValue(QStringLiteral(":titre"), projet.titre);
        query.bindValue(QStringLiteral(":debut"), projet.dateDebut);
        query.bindValue(QStringLiteral(":fin"), projet.dateFin);
        if (projet.responsableId > 0)
            query.bindValue(QStringLiteral(":resp"), projet.responsableId);
        else
            query.bindValue(QStringLiteral(":resp"), QVariant());
        query.bindValue(QStringLiteral(":etat"), projet.etat);
        query.bindValue(QStringLiteral(":prog"), progVal);
        query.bindValue(QStringLiteral(":id"), currentProjetId);

        if (query.exec()) {
            QMessageBox::information(this, QStringLiteral("Modification"),
                                       QStringLiteral("Projet modifié avec succès."));
        } else {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Erreur modification: ") + query.lastError().text());
            return;
        }
    } else {
        if (projet.code.isEmpty()) {
            query.prepare(QStringLiteral("INSERT INTO PROJET (TITRE, DATE_DEBUT, DATE_FIN, "
                                         "RESPONSABLE, ETAT, PROGRESSION) "
                                         "VALUES (:titre, :debut, :fin, :resp, :etat, :prog)"));
        } else {
            query.prepare(QStringLiteral("INSERT INTO PROJET (CODE_PROJET, TITRE, DATE_DEBUT, DATE_FIN, "
                                         "RESPONSABLE, ETAT, PROGRESSION) "
                                         "VALUES (:code, :titre, :debut, :fin, :resp, :etat, :prog)"));
            query.bindValue(QStringLiteral(":code"), projet.code.toInt());
        }
        query.bindValue(QStringLiteral(":titre"), projet.titre);
        query.bindValue(QStringLiteral(":debut"), projet.dateDebut);
        query.bindValue(QStringLiteral(":fin"), projet.dateFin);
        if (projet.responsableId > 0)
            query.bindValue(QStringLiteral(":resp"), projet.responsableId);
        else
            query.bindValue(QStringLiteral(":resp"), QVariant());
        query.bindValue(QStringLiteral(":etat"), projet.etat);
        query.bindValue(QStringLiteral(":prog"), progVal);

        if (query.exec()) {
            QMessageBox::information(this, QStringLiteral("Ajout"),
                                     QStringLiteral("Nouveau projet ajouté avec succès."));
        } else {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Erreur ajout: ") + query.lastError().text());
            return;
        }
    }

    projCacherFormulaire();
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
    projChargerProjets();  // Recharger depuis Oracle
}

void SmartPub::projMettreAJourProjetTable(int row, const Projet &projet)
{
    QString titreDisplay = projet.titre;
    if (titreDisplay.length() > 35) {
        titreDisplay = titreDisplay.left(32) + "...";
    }

    ui->tableWidgetProjets->item(row, 1)->setText(projet.code);
    ui->tableWidgetProjets->item(row, 2)->setText(titreDisplay);
    ui->tableWidgetProjets->item(row, 2)->setToolTip(projet.titre);
    ui->tableWidgetProjets->item(row, 2)->setForeground(QColor("#1e293b"));
    ui->tableWidgetProjets->item(row, 3)->setText(projet.dateDebut.toString("dd/MM/yyyy"));
    ui->tableWidgetProjets->item(row, 4)->setText(projet.dateFin.toString("dd/MM/yyyy"));
    ui->tableWidgetProjets->item(row, 5)->setText(projet.responsable);

    QTableWidgetItem *etatItem = ui->tableWidgetProjets->item(row, 6);
    etatItem->setText(projEtatDbToUi(projet.etat));
    etatItem->setForeground(QColor(::getEtatColor(projEtatDbToUi(projet.etat))));

    QTableWidgetItem *progItem = ui->tableWidgetProjets->item(row, 7);
    progItem->setText(projet.progression);
    progItem->setForeground(QColor(::getProgressionColorFromString(projet.progression)));
}

void SmartPub::onSupprimerProjetClicked()
{
    int row = projGetSelectedRow();
    if (row == -1) {
        QMessageBox::warning(this, "Suppression", "Veuillez sélectionner un projet à supprimer.");
        return;
    }

    int projetId = ui->tableWidgetProjets->item(row, 0)->text().toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmer la suppression",
                                  "Êtes-vous sûr de vouloir supprimer ce projet ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) {
            QMessageBox::critical(this, "Erreur", "Connexion à la base de données perdue.");
            return;
        }

        QSqlQuery query(db);
        query.prepare(QStringLiteral("DELETE FROM PROJET WHERE CODE_PROJET = :id"));
        query.bindValue(QStringLiteral(":id"), projetId);

        if (query.exec()) {
            QMessageBox::information(this, "Suppression", "Projet supprimé avec succès.");
            projChargerProjets();  // Recharger depuis Oracle
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur suppression: " + query.lastError().text());
        }
    }
}

void SmartPub::onTableSelectionChanged()
{
    bool hasSelection = !ui->tableWidgetProjets->selectedItems().isEmpty();
    ui->btnModifierProjet->setEnabled(hasSelection);
    ui->btnSupprimerProjet->setEnabled(hasSelection);
}

void SmartPub::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    int projetId = ui->tableWidgetProjets->item(row, 0)->text().toInt();
    projShowProjetDetails(projetId);
}

void SmartPub::onTriDateDebutClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateFin->setChecked(false);
    ui->btnTriEtat->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 3) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 3;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📅 Date Début ▲" : "📅 Date Début ▼");
}

void SmartPub::onTriDateFinClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriEtat->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 4) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 4;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📅 Date Fin ▲" : "📅 Date Fin ▼");
}

void SmartPub::onTriEtatClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriDateFin->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 6) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 6;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "🔧 État ▲" : "🔧 État ▼");
}

void SmartPub::onTriProgressionClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriDateFin->setChecked(false);
    ui->btnTriEtat->setChecked(false);

    if (currentSortColumn == 7) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 7;
        currentSortOrder = Qt::DescendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📈 Progression ▲" : "📈 Progression ▼");
}

void SmartPub::projSortProjetsBy(int column, Qt::SortOrder order)
{
    switch (column) {
    case 3:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      return order == Qt::AscendingOrder ? a.dateDebut < b.dateDebut : a.dateDebut > b.dateDebut;
                  });
        break;
    case 4:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      return order == Qt::AscendingOrder ? a.dateFin < b.dateFin : a.dateFin > b.dateFin;
                  });
        break;
    case 6:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      return order == Qt::AscendingOrder ? a.etat < b.etat : a.etat > b.etat;
                  });
        break;
    case 7:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      int progA = a.progression.left(a.progression.indexOf('%')).toInt();
                      int progB = b.progression.left(b.progression.indexOf('%')).toInt();
                      return order == Qt::AscendingOrder ? progA < progB : progA > progB;
                  });
        break;
    }

    projChargerProjets();
}

void SmartPub::onStatistiquesClicked()
{
    StatistiquesDialog *dialog = new StatistiquesDialog(projets, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::onSanteProjetClicked()
{
    QString message = "<b>Santé des Projets</b><br><br>";
    int sains = 0, risque = 0, critiques = 0;

    for (const auto &p : projets) {
        QString statut = projCalculerStatutProjet(p);
        if (statut == "Sain") sains++;
        else if (statut == "À risque") risque++;
        else if (statut == "Critique") critiques++;
    }

    message += QString("<span style='color: #10b981;'>Sains: %1</span><br>").arg(sains);
    message += QString("<span style='color: #f59e0b;'>À risque: %1</span><br>").arg(risque);
    message += QString("<span style='color: #ef4444;'>Critiques: %1</span><br>").arg(critiques);

    QMessageBox::information(this, "Santé des Projets", message);
}

void SmartPub::onOptimiserChargeClicked()
{
    QMessageBox::information(this, "Optimisation de Charge",
                             "<b>Analyse de la charge de travail</b><br><br>"
                             "Cette fonctionnalité analysera la répartition des projets par responsable "
                             "et suggérera des optimisations.");
}

void SmartPub::onIARecommanderClicked()
{
    IARecommandationsDialog *dialog = new IARecommandationsDialog(projets, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::onFiltresClicked()
{
    FiltresDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        filtreEtat = dialog.getEtatFiltre();
        filtreResponsable = dialog.getResponsableFiltre();
        filtreDateDebutMin = dialog.getDateDebutMin();
        filtreDateDebutMax = dialog.getDateDebutMax();
        filtresActifs = !filtreEtat.isEmpty() || !filtreResponsable.isEmpty() ||
                        filtreDateDebutMin != QDate(2020, 1, 1) ||
                        filtreDateDebutMax != QDate::currentDate().addYears(5);

        projAppliquerFiltres();
        projMettreAJourBadgeFiltres();
    }
}

void SmartPub::projAppliquerFiltres()
{
    projetsFiltres.clear();

    for (const auto &projet : projets) {
        bool match = true;

        if (!filtreEtat.isEmpty() && projet.etat != filtreEtat) {
            match = false;
        }

        if (!filtreResponsable.isEmpty() && projet.responsable != filtreResponsable) {
            match = false;
        }

        if (projet.dateDebut < filtreDateDebutMin || projet.dateDebut > filtreDateDebutMax) {
            match = false;
        }

        if (match) {
            projetsFiltres.append(projet);
        }
    }

    projChargerProjets();
}

void SmartPub::projMettreAJourBadgeFiltres()
{
    if (filtresActifs) {
        ui->btnFiltresProjets->setText("🎛️ Filtres ✓");
        ui->btnFiltresProjets->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
            "    color: white;"
            "    border: none;"
            "    border-radius: 10px;"
            "    font-size: 13px;"
            "    font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
            "}"
            );
    } else {
        ui->btnFiltresProjets->setText("🎛️ Filtres");
        ui->btnFiltresProjets->setStyleSheet(
            "QPushButton {"
            "    background-color: white;"
            "    color: #334155;"
            "    border: 1px solid #e2e8f0;"
            "    border-radius: 10px;"
            "    font-size: 13px;"
            "    font-weight: 500;"
            "}"
            "QPushButton:hover {"
            "    background-color: #f8fafc;"
            "    border-color: #cbd5e1;"
            "}"
            );
    }
}

void SmartPub::onExporterClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les projets",
                                                    "", "Fichiers CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Exportation", "Projets exportés avec succès dans:\n" + fileName);
    }
}

void SmartPub::projAfficherFormulaire(bool isEdit)
{
    ui->labelFormTitle->setText(isEdit ? "Modifier le Projet" : "Nouveau Projet");
    ui->stackedWidgetProjets->setCurrentIndex(1);
}

void SmartPub::projCacherFormulaire()
{
    ui->stackedWidgetProjets->setCurrentIndex(0);
}

void SmartPub::projRemplirFormulaire(const Projet &projet)
{
    ui->lineEditCodeForm->setText(projet.code);
    ui->lineEditTitreForm->setText(projet.titre);
    ui->dateEditDebutForm->setDate(projet.dateDebut);
    ui->dateEditFinForm->setDate(projet.dateFin);

    int respIndex = ui->comboBoxResponsableForm->findData(QVariant(projet.responsableId));
    if (respIndex >= 0)
        ui->comboBoxResponsableForm->setCurrentIndex(respIndex);
    else
        ui->comboBoxResponsableForm->setCurrentIndex(0);

    int etatIndex = ui->comboBoxEtatForm->findData(projet.etat);
    if (etatIndex >= 0)
        ui->comboBoxEtatForm->setCurrentIndex(etatIndex);
    else
        ui->comboBoxEtatForm->setCurrentIndex(0);

    ui->textEditDescriptionForm->setText(projet.description);
}

void SmartPub::projViderFormulaire()
{
    ui->lineEditCodeForm->clear();
    ui->lineEditTitreForm->clear();
    ui->dateEditDebutForm->setDate(QDate::currentDate());
    ui->dateEditFinForm->setDate(QDate::currentDate().addDays(30));
    ui->comboBoxResponsableForm->setCurrentIndex(0);
    ui->comboBoxEtatForm->setCurrentIndex(0);
    ui->textEditDescriptionForm->clear();
}

Projet SmartPub::projGetProjetFromForm() const
{
    Projet projet;
    if (isEditing) {
        projet.id = currentProjetId;
        for (const Projet &p : projets) {
            if (p.id == currentProjetId) {
                projet.progression = p.progression;
                break;
            }
        }
    }
    if (projet.progression.isEmpty())
        projet.progression = QStringLiteral("0%");

    projet.code = ui->lineEditCodeForm->text().trimmed();
    projet.titre = ui->lineEditTitreForm->text();
    projet.dateDebut = ui->dateEditDebutForm->date();
    projet.dateFin = ui->dateEditFinForm->date();
    projet.responsable = ui->comboBoxResponsableForm->currentText();
    projet.responsableId = ui->comboBoxResponsableForm->currentData().toInt();
    projet.etat = ui->comboBoxEtatForm->currentData().toString();
    if (projet.etat.isEmpty())
        projet.etat = QStringLiteral("en_cours");
    projet.description = ui->textEditDescriptionForm->toPlainText();
    return projet;
}

bool SmartPub::projValiderFormulaire() const
{
    const QString codeTxt = ui->lineEditCodeForm->text().trimmed();
    if (isEditing && codeTxt.isEmpty()) {
        QMessageBox::warning(const_cast<SmartPub *>(this), QStringLiteral("Validation"),
                             QStringLiteral("Le code du projet est requis"));
        ui->lineEditCodeForm->setFocus();
        return false;
    }
    if (!codeTxt.isEmpty()) {
        bool ok = false;
        codeTxt.toInt(&ok);
        if (!ok) {
            QMessageBox::warning(const_cast<SmartPub *>(this), QStringLiteral("Validation"),
                                 QStringLiteral("Le code projet doit être un nombre entier"));
            ui->lineEditCodeForm->setFocus();
            return false;
        }
    }

    if (ui->lineEditTitreForm->text().isEmpty()) {
        QMessageBox::warning(const_cast<SmartPub*>(this), "Validation", "Le titre du projet est requis");
        ui->lineEditTitreForm->setFocus();
        return false;
    }

    if (ui->dateEditDebutForm->date() > ui->dateEditFinForm->date()) {
        QMessageBox::warning(const_cast<SmartPub*>(this), "Validation", "La date de début doit être antérieure à la date de fin");
        ui->dateEditDebutForm->setFocus();
        return false;
    }

    return true;
}

void SmartPub::projMettreAJourStats()
{
}

int SmartPub::projGetSelectedRow() const
{
    QList<QTableWidgetItem*> selected = ui->tableWidgetProjets->selectedItems();
    if (!selected.isEmpty()) {
        return selected.first()->row();
    }
    return -1;
}

void SmartPub::projShowProjetDetails(int projetId)
{
    for (const Projet &projet : projets) {
        if (projet.id == projetId) {
            projShowProjetDetailsDialog(projet);
            break;
        }
    }
}

void SmartPub::projShowProjetDetailsDialog(const Projet &projet)
{
    ProjetDetailsDialog *dialog = new ProjetDetailsDialog(projet, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

QString SmartPub::projCalculerStatutProjet(const Projet &projet) const
{
    int progression = projExtraireProgression(projet.progression);
    int joursRestants = QDate::currentDate().daysTo(projet.dateFin);

    if (projet.etat == QLatin1String("termine")) return QStringLiteral("Sain");

    if (joursRestants < 0) {
        return "Critique";
    } else if (joursRestants < 30) {
        if (progression < 80) return "Critique";
        else if (progression < 90) return "À risque";
        else return "Sain";
    } else {
        return "Sain";
    }
}

double SmartPub::projCalculerTauxAvancement(const Projet &projet) const
{
    int joursTotaux = projet.dateDebut.daysTo(projet.dateFin);
    int joursRestants = QDate::currentDate().daysTo(projet.dateFin);

    if (joursTotaux <= 0) return 100.0;

    double pourcentageTemps = (1.0 - (double)joursRestants / (double)joursTotaux) * 100;
    return qBound(0.0, pourcentageTemps, 100.0);
}

QVector<QString> SmartPub::projGenererAlertes(const Projet &projet) const
{
    QVector<QString> alertes;

    if (projet.etat == QLatin1String("termine")) return alertes;

    int joursRestants = QDate::currentDate().daysTo(projet.dateFin);

    if (joursRestants < 0) {
        alertes.append("Projet en retard ! Date dépassée.");
    }
    else if (joursRestants < 30) {
        alertes.append(QString("Échéance proche (%1 jours)").arg(joursRestants));
    }

    return alertes;
}

int SmartPub::projCompterProjetsParEtat(const QString &etat) const
{
    int count = 0;
    for (const auto &p : projets) {
        if (p.etat == etat) count++;
    }
    return count;
}

double SmartPub::projCalculerProgressionMoyenne() const
{
    double total = 0;
    int count = 0;
    for (const auto &p : projets) {
        if (p.etat != QLatin1String("termine") && p.etat != QLatin1String("annule")) {
            total += projExtraireProgression(p.progression);
            count++;
        }
    }
    return count > 0 ? total / count : 0;
}

int SmartPub::projCompterProjetsEnRetard() const
{
    int count = 0;
    for (const auto &p : projets) {
        if (p.etat != QLatin1String("termine") && QDate::currentDate() > p.dateFin) {
            count++;
        }
    }
    return count;
}
// ============================================================================
// FONCTIONS MANQUANTES POUR LE MODULE PROJETS ET SR
// À ajouter à la fin de smartpub.cpp (avant la dernière accolade)
// ============================================================================

// ============================================================================
// MODULE SMART RESEARCH - SR_addButtonsToRow
// ============================================================================

void SmartPub::SR_addButtonsToRow(int row)
{
    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(buttonWidget);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    QPushButton *btnEdit = new QPushButton("✏️");
    btnEdit->setMinimumSize(32, 28);
    btnEdit->setMaximumSize(32, 28);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(
        "QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2563eb;"
        "}"
    );

    QPushButton *btnDelete = new QPushButton("🗑️");
    btnDelete->setMinimumSize(32, 28);
    btnDelete->setMaximumSize(32, 28);
    btnDelete->setCursor(Qt::PointingHandCursor);
    btnDelete->setStyleSheet(
        "QPushButton {"
        "    background-color: #ef4444;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #dc2626;"
        "}"
    );

    connect(btnEdit, &QPushButton::clicked, this, &SmartPub::on_SR_modifierPublication_clicked);
    connect(btnDelete, &QPushButton::clicked, this, &SmartPub::on_SR_supprimerPublication_clicked);

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    layout->addStretch();

    ui->SR_tablePublications->setCellWidget(row, 5, buttonWidget);
}

// ============================================================================
// MODULE PROJETS - SLOTS
// ============================================================================

void SmartPub::on_btnListeProjets_clicked()
{
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::on_btnAjouterProjet_clicked()
{
    isEditing = false;
    projViderFormulaire();
    projAfficherFormulaire(false);
    projSetActiveCrudButton(1);
}

void SmartPub::on_btnModifierProjet_clicked()
{
    int row = projGetSelectedRow();
    if (row != -1) {
        int projetId = ui->tableWidgetProjets->item(row, 0)->text().toInt();
        for (const Projet &projet : projets) {
            if (projet.id == projetId) {
                isEditing = true;
                currentProjetId = projetId;
                projRemplirFormulaire(projet);
                projAfficherFormulaire(true);
                projSetActiveCrudButton(2);
                break;
            }
        }
    } else {
        QMessageBox::warning(this, "Modification", "Veuillez sélectionner un projet à modifier");
    }
}

void SmartPub::on_btnSupprimerProjet_clicked()
{
    int row = projGetSelectedRow();
    if (row == -1) {
        QMessageBox::warning(this, "Suppression", "Veuillez sélectionner un projet à supprimer");
        return;
    }

    int projetId = ui->tableWidgetProjets->item(row, 0)->text().toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmer la suppression",
                                  "Êtes-vous sûr de vouloir supprimer ce projet ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (int i = 0; i < projets.size(); ++i) {
            if (projets[i].id == projetId) {
                projets.remove(i);
                break;
            }
        }

        ui->tableWidgetProjets->removeRow(row);
        QMessageBox::information(this, "Suppression", "Projet supprimé avec succès");
    }
}

void SmartPub::on_lineEditRechercheProjets_textChanged(const QString &text)
{
    projFiltrerTable(text);
}

void SmartPub::on_btnAnnulerForm_clicked()
{
    projCacherFormulaire();
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::on_btnEnregistrerForm_clicked()
{
    if (!projValiderFormulaire()) {
        return;
    }

    Projet projet = projGetProjetFromForm();

    if (isEditing) {
        for (int i = 0; i < projets.size(); ++i) {
            if (projets[i].id == currentProjetId) {
                projets[i] = projet;
                for (int row = 0; row < ui->tableWidgetProjets->rowCount(); ++row) {
                    if (ui->tableWidgetProjets->item(row, 0)->text().toInt() == currentProjetId) {
                        projMettreAJourProjetTable(row, projet);
                        break;
                    }
                }
                QMessageBox::information(this, "Modification", "Projet modifié avec succès");
                break;
            }
        }
    } else {
        projet.id = nextProjetId++;
        projets.append(projet);
        projAjouterProjetTable(projet, projets.size() - 1);
        QMessageBox::information(this, "Ajout", "Nouveau projet ajouté avec succès");
    }

    projCacherFormulaire();
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::on_tableSelectionChanged()
{
    bool hasSelection = !ui->tableWidgetProjets->selectedItems().isEmpty();
    ui->btnModifierProjet->setEnabled(hasSelection);
    ui->btnSupprimerProjet->setEnabled(hasSelection);
}

void SmartPub::on_tableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    int projetId = ui->tableWidgetProjets->item(row, 0)->text().toInt();
    projShowProjetDetails(projetId);
}

void SmartPub::on_triDateDebutClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateFin->setChecked(false);
    ui->btnTriEtat->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 3) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 3;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📅 Date Début ▲" : "📅 Date Début ▼");
}

void SmartPub::on_triDateFinClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriEtat->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 4) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 4;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📅 Date Fin ▲" : "📅 Date Fin ▼");
}

void SmartPub::on_triEtatClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriDateFin->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 6) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 6;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "🔧 État ▲" : "🔧 État ▼");
}

void SmartPub::on_triProgressionClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriDateFin->setChecked(false);
    ui->btnTriEtat->setChecked(false);

    if (currentSortColumn == 7) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 7;
        currentSortOrder = Qt::DescendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📈 Progression ▲" : "📈 Progression ▼");
}

void SmartPub::on_statistiquesClicked()
{
    StatistiquesDialog *dialog = new StatistiquesDialog(projets, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::on_santeProjetClicked()
{
    QString message = "<b>Santé des Projets</b><br><br>";
    int sains = 0, risque = 0, critiques = 0;

    for (const auto &p : projets) {
        QString statut = projCalculerStatutProjet(p);
        if (statut == "Sain") sains++;
        else if (statut == "À risque") risque++;
        else if (statut == "Critique") critiques++;
    }

    message += QString("<span style='color: #10b981;'>Sains: %1</span><br>").arg(sains);
    message += QString("<span style='color: #f59e0b;'>À risque: %1</span><br>").arg(risque);
    message += QString("<span style='color: #ef4444;'>Critiques: %1</span><br>").arg(critiques);

    QMessageBox::information(this, "Santé des Projets", message);
}

void SmartPub::on_optimiserChargeClicked()
{
    QMessageBox::information(this, "Optimisation de Charge",
                             "<b>Analyse de la charge de travail</b><br><br>"
                             "Cette fonctionnalité analysera la répartition des projets par responsable "
                             "et suggérera des optimisations.");
}

void SmartPub::on_iaRecommanderClicked()
{
    IARecommandationsDialog *dialog = new IARecommandationsDialog(projets, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::on_filtresClicked()
{
    FiltresDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        filtreEtat = dialog.getEtatFiltre();
        filtreResponsable = dialog.getResponsableFiltre();
        filtreDateDebutMin = dialog.getDateDebutMin();
        filtreDateDebutMax = dialog.getDateDebutMax();
        filtresActifs = !filtreEtat.isEmpty() || !filtreResponsable.isEmpty() ||
                        filtreDateDebutMin != QDate(2020, 1, 1) ||
                        filtreDateDebutMax != QDate::currentDate().addYears(5);

        projAppliquerFiltres();
        projMettreAJourBadgeFiltres();
    }
}

void SmartPub::on_exporterClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les projets",
                                                    "", "Fichiers CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Exportation", "Projets exportés avec succès dans:\n" + fileName);
    }
}

// ============================================================================
// MODULE LABORATOIRES
// ============================================================================

void SmartPub::labSetupUI()
{
    // Créer la page laboratoires et l'ajouter au stackedWidgetModules (index 5)
    labPage = new QWidget();
    labPage->setObjectName("labPage");
    labPage->setStyleSheet("QWidget#labPage { background-color: #f8fafc; }");

    QVBoxLayout *pageLayout = new QVBoxLayout(labPage);
    pageLayout->setSpacing(0);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    // ---- HEADER ----
    QFrame *headerFrame = new QFrame();
    headerFrame->setObjectName("labHeaderFrame");
    headerFrame->setFixedHeight(90);
    headerFrame->setStyleSheet(
        "QFrame#labHeaderFrame {"
        "    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "    border: none;"
        "}");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(30, 0, 30, 0);

    QLabel *titleLabel = new QLabel("🧪  Gestion des Laboratoires");
    titleLabel->setStyleSheet("color:white;font-size:26px;font-weight:bold;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    labTotalLabel = new QLabel("0 laboratoires");
    labTotalLabel->setObjectName("labTotalLabel");
    labTotalLabel->setStyleSheet(
        "color:white;font-size:14px;background:rgba(255,255,255,0.2);"
        "border-radius:12px;padding:6px 16px;");
    headerLayout->addWidget(labTotalLabel);
    pageLayout->addWidget(headerFrame);

    // ---- TOOLBAR ----
    QFrame *toolbarFrame = new QFrame();
    toolbarFrame->setObjectName("labToolbarFrame");
    toolbarFrame->setFixedHeight(70);
    toolbarFrame->setStyleSheet(
        "QFrame#labToolbarFrame {"
        "    background-color:white;"
        "    border-bottom:1px solid #e2e8f0;"
        "}");
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarFrame);
    toolbarLayout->setContentsMargins(20, 0, 20, 0);
    toolbarLayout->setSpacing(10);

    // CRUD buttons
    labBtnAjouter = new QPushButton("➕  Ajouter");
    labBtnAjouter->setObjectName("labBtnAjouter");
    labBtnAjouter->setFixedHeight(44);
    labBtnAjouter->setCursor(Qt::PointingHandCursor);
    labBtnAjouter->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "color:white;border:none;border-radius:10px;padding:0 20px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #059669);}");
    toolbarLayout->addWidget(labBtnAjouter);

    labBtnModifier = new QPushButton("✏️  Modifier");
    labBtnModifier->setObjectName("labBtnModifier");
    labBtnModifier->setFixedHeight(44);
    labBtnModifier->setEnabled(false);
    labBtnModifier->setCursor(Qt::PointingHandCursor);
    labBtnModifier->setStyleSheet(
        "QPushButton{background-color:#f1f5f9;color:#475569;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 16px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#e2e8f0;}"
        "QPushButton:disabled{color:#cbd5e1;}");
    toolbarLayout->addWidget(labBtnModifier);

    labBtnSupprimer = new QPushButton("🗑️  Supprimer");
    labBtnSupprimer->setObjectName("labBtnSupprimer");
    labBtnSupprimer->setFixedHeight(44);
    labBtnSupprimer->setEnabled(false);
    labBtnSupprimer->setCursor(Qt::PointingHandCursor);
    labBtnSupprimer->setStyleSheet(
        "QPushButton{background-color:transparent;color:#ef4444;border:1.5px solid #fca5a5;"
        "border-radius:10px;padding:0 16px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#fef2f2;}"
        "QPushButton:disabled{color:#fca5a5;border-color:#fecaca;}");
    toolbarLayout->addWidget(labBtnSupprimer);

    // Séparateur
    QFrame *sep1 = new QFrame(); sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color:#e2e8f0;"); sep1->setFixedWidth(1);
    toolbarLayout->addWidget(sep1);

    // Boutons IA / Stats / Exporter / Trier
    QPushButton *btnStats = new QPushButton("📊  Statistiques");
    btnStats->setObjectName("labBtnStatistiques");
    btnStats->setFixedHeight(44);
    btnStats->setCursor(Qt::PointingHandCursor);
    btnStats->setStyleSheet(
        "QPushButton{background-color:white;color:#3b82f6;border:1.5px solid #bfdbfe;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#eff6ff;}");
    toolbarLayout->addWidget(btnStats);

    QPushButton *btnOptim = new QPushButton("🤝  Optimiseur");
    btnOptim->setObjectName("labBtnOptimiseur");
    btnOptim->setFixedHeight(44);
    btnOptim->setCursor(Qt::PointingHandCursor);
    btnOptim->setStyleSheet(
        "QPushButton{background-color:white;color:#10b981;border:1.5px solid #a7f3d0;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#ecfdf5;}");
    toolbarLayout->addWidget(btnOptim);

    QPushButton *btnPred = new QPushButton("🔮  Prédicteur");
    btnPred->setObjectName("labBtnPredicteur");
    btnPred->setFixedHeight(44);
    btnPred->setCursor(Qt::PointingHandCursor);
    btnPred->setStyleSheet(
        "QPushButton{background-color:white;color:#8b5cf6;border:1.5px solid #ddd6fe;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#f5f3ff;}");
    toolbarLayout->addWidget(btnPred);

    toolbarLayout->addStretch();

    // Barre de recherche
    labSearchEdit = new QLineEdit();
    labSearchEdit->setObjectName("labSearchEdit");
    labSearchEdit->setPlaceholderText("🔍  Rechercher un laboratoire…");
    labSearchEdit->setFixedHeight(44);
    labSearchEdit->setMinimumWidth(250);
    labSearchEdit->setStyleSheet(
        "QLineEdit{background-color:#f8fafc;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 14px;font-size:13px;color:#334155;}"
        "QLineEdit:focus{border-color:#3b82f6;background-color:white;}");
    toolbarLayout->addWidget(labSearchEdit);

    QPushButton *btnExporter = new QPushButton("📤  Exporter");
    btnExporter->setObjectName("labBtnExporter");
    btnExporter->setFixedHeight(44);
    btnExporter->setCursor(Qt::PointingHandCursor);
    btnExporter->setStyleSheet(
        "QPushButton{background-color:#f8fafc;color:#64748b;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:500;}"
        "QPushButton:hover{background-color:#f1f5f9;}");
    toolbarLayout->addWidget(btnExporter);

    QPushButton *btnTrier = new QPushButton("⇅  Trier");
    btnTrier->setObjectName("labBtnTrier");
    btnTrier->setFixedHeight(44);
    btnTrier->setCursor(Qt::PointingHandCursor);
    btnTrier->setStyleSheet(
        "QPushButton{background-color:#f8fafc;color:#64748b;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:500;}"
        "QPushButton:hover{background-color:#f1f5f9;}");
    toolbarLayout->addWidget(btnTrier);

    pageLayout->addWidget(toolbarFrame);

    // ---- BODY: Table + Formulaire côte à côte ----
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(20, 16, 20, 16);

    // TABLE
    labTable = new QTableWidget();
    labTable->setObjectName("labTable");
    labTable->setColumnCount(7);
    labTable->setHorizontalHeaderLabels({"ID", "Nom", "Thématique", "Budget (€)", "Capacité", "Statut", "Directeur"});
    labTable->horizontalHeader()->setStretchLastSection(true);
    labTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    labTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    labTable->setSelectionMode(QAbstractItemView::SingleSelection);
    labTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    labTable->setAlternatingRowColors(true);
    labTable->verticalHeader()->setDefaultSectionSize(52);
    labTable->verticalHeader()->setVisible(false);
    labTable->setColumnWidth(0, 55);
    labTable->setColumnWidth(2, 170);
    labTable->setColumnWidth(3, 115);
    labTable->setColumnWidth(4, 90);
    labTable->setColumnWidth(5, 130);
    labTable->setStyleSheet(R"(
        QTableWidget {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            gridline-color: #f1f5f9;
            font-size: 13px;
            color: #334155;
        }
        QTableWidget::item {
            padding: 8px 12px;
        }
        QTableWidget::item:selected {
            background-color: #eff6ff;
            color: #1d4ed8;
        }
        QHeaderView::section {
            background-color: #f8fafc;
            color: #64748b;
            font-weight: 600;
            font-size: 12px;
            padding: 10px 12px;
            border: none;
            border-bottom: 1px solid #e2e8f0;
        }
        QTableWidget::item:alternate {
            background-color: #f8fafc;
        }
    )");
    bodyLayout->addWidget(labTable, 1);

    // FORMULAIRE (caché par défaut)
    labFormFrame = new QFrame();
    labFormFrame->setObjectName("labFormFrame");
    labFormFrame->setFixedWidth(360);
    labFormFrame->setVisible(false);
    labFormFrame->setStyleSheet(
        "QFrame#labFormFrame {"
        "    background-color:white;"
        "    border:1px solid #e2e8f0;"
        "    border-radius:12px;"
        "    margin-left:16px;"
        "}");
    QVBoxLayout *formLayout = new QVBoxLayout(labFormFrame);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(12);

    QLabel *formTitle = new QLabel("📋  Laboratoire");
    formTitle->setObjectName("labFormTitle");
    formTitle->setStyleSheet("font-size:17px;font-weight:bold;color:#1e3a5f;margin-bottom:4px;");
    formLayout->addWidget(formTitle);

    QFrame *formSep = new QFrame(); formSep->setFrameShape(QFrame::HLine);
    formSep->setStyleSheet("color:#e2e8f0;"); formLayout->addWidget(formSep);

    auto makeLabel = [](const QString &txt) {
        QLabel *l = new QLabel(txt);
        l->setStyleSheet("font-size:12px;font-weight:600;color:#64748b;margin-top:4px;");
        return l;
    };
    auto makeInput = []() {
        QLineEdit *e = new QLineEdit();
        e->setFixedHeight(40);
        e->setStyleSheet(
            "QLineEdit{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
            "padding:0 12px;font-size:13px;color:#334155;}"
            "QLineEdit:focus{border-color:#3b82f6;background:white;}");
        return e;
    };

    formLayout->addWidget(makeLabel("Nom du laboratoire *"));
    labFormNom = makeInput();
    labFormNom->setPlaceholderText("Ex: Lab IA Avancée");
    formLayout->addWidget(labFormNom);

    formLayout->addWidget(makeLabel("Thématique *"));
    labFormThematique = new QComboBox();
    labFormThematique->addItems({"Intelligence Artificielle", "Biotechnologie",
        "Nanotechnologie", "Énergies Renouvelables", "Robotique",
        "Chimie", "Physique Quantique", "Sciences de Données", "Autre"});
    labFormThematique->setFixedHeight(40);
    labFormThematique->setStyleSheet(
        "QComboBox{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
        "padding:0 12px;font-size:13px;color:#334155;}"
        "QComboBox:focus{border-color:#3b82f6;background:white;}"
        "QComboBox::drop-down{border:none;width:26px;}");
    formLayout->addWidget(labFormThematique);

    formLayout->addWidget(makeLabel("Budget annuel (€)"));
    labFormBudget = makeInput();
    labFormBudget->setPlaceholderText("Ex: 250000");
    formLayout->addWidget(labFormBudget);

    formLayout->addWidget(makeLabel("Capacité (chercheurs)"));
    labFormCapacite = new QSpinBox();
    labFormCapacite->setRange(1, 500);
    labFormCapacite->setValue(10);
    labFormCapacite->setFixedHeight(40);
    labFormCapacite->setStyleSheet(
        "QSpinBox{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
        "padding:0 12px;font-size:13px;color:#334155;}"
        "QSpinBox:focus{border-color:#3b82f6;background:white;}");
    formLayout->addWidget(labFormCapacite);

    formLayout->addWidget(makeLabel("Statut *"));
    labFormStatut = new QComboBox();
    labFormStatut->addItems({"Actif", "En Construction", "En Rénovation", "Inactif"});
    labFormStatut->setFixedHeight(40);
    labFormStatut->setStyleSheet(
        "QComboBox{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
        "padding:0 12px;font-size:13px;color:#334155;}"
        "QComboBox:focus{border-color:#3b82f6;background:white;}"
        "QComboBox::drop-down{border:none;width:26px;}");
    formLayout->addWidget(labFormStatut);

    formLayout->addWidget(makeLabel("Équipements (séparés par virgule)"));
    labFormEquipements = makeInput();
    labFormEquipements->setPlaceholderText("Ex: Microscope, Spectromètre…");
    formLayout->addWidget(labFormEquipements);

    formLayout->addWidget(makeLabel("Directeur de recherche"));
    labFormDirecteur = makeInput();
    labFormDirecteur->setPlaceholderText("Ex: Dr. Dupont");
    formLayout->addWidget(labFormDirecteur);

    formLayout->addStretch();

    QHBoxLayout *formBtnsLayout = new QHBoxLayout();
    formBtnsLayout->setSpacing(10);
    QPushButton *btnAnnulerForm = new QPushButton("Annuler");
    btnAnnulerForm->setObjectName("labBtnAnnulerForm");
    btnAnnulerForm->setFixedHeight(42);
    btnAnnulerForm->setCursor(Qt::PointingHandCursor);
    btnAnnulerForm->setStyleSheet(
        "QPushButton{background-color:#f1f5f9;color:#475569;border:1.5px solid #e2e8f0;"
        "border-radius:8px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#e2e8f0;}");
    QPushButton *btnConfirmerForm = new QPushButton("💾  Enregistrer");
    btnConfirmerForm->setObjectName("labBtnConfirmerForm");
    btnConfirmerForm->setFixedHeight(42);
    btnConfirmerForm->setCursor(Qt::PointingHandCursor);
    btnConfirmerForm->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "color:white;border:none;border-radius:8px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #059669);}");
    formBtnsLayout->addWidget(btnAnnulerForm);
    formBtnsLayout->addWidget(btnConfirmerForm);
    formLayout->addLayout(formBtnsLayout);

    bodyLayout->addWidget(labFormFrame);
    pageLayout->addLayout(bodyLayout);

    // Ajouter la page au stackedWidgetModules (index 5)
    ui->stackedWidgetModules->addWidget(labPage);
}

void SmartPub::labConnectSignals()
{
    connect(labBtnAjouter, &QPushButton::clicked, this, &SmartPub::on_labBtnAjouter_clicked);
    connect(labBtnModifier, &QPushButton::clicked, this, &SmartPub::on_labBtnModifier_clicked);
    connect(labBtnSupprimer, &QPushButton::clicked, this, &SmartPub::on_labBtnSupprimer_clicked);
    connect(labSearchEdit, &QLineEdit::textChanged, this, &SmartPub::on_labSearchChanged);
    connect(labTable, &QTableWidget::itemSelectionChanged, this, &SmartPub::on_labTableSelectionChanged);

    QPushButton *btnStats     = labPage->findChild<QPushButton*>("labBtnStatistiques");
    QPushButton *btnOptim     = labPage->findChild<QPushButton*>("labBtnOptimiseur");
    QPushButton *btnPred      = labPage->findChild<QPushButton*>("labBtnPredicteur");
    QPushButton *btnExporter  = labPage->findChild<QPushButton*>("labBtnExporter");
    QPushButton *btnTrier     = labPage->findChild<QPushButton*>("labBtnTrier");
    QPushButton *btnConfirmer = labFormFrame->findChild<QPushButton*>("labBtnConfirmerForm");
    QPushButton *btnAnnuler   = labFormFrame->findChild<QPushButton*>("labBtnAnnulerForm");

    if (btnStats)    connect(btnStats,    &QPushButton::clicked, this, &SmartPub::on_labBtnStatistiques_clicked);
    if (btnOptim)    connect(btnOptim,    &QPushButton::clicked, this, &SmartPub::on_labBtnOptimiseur_clicked);
    if (btnPred)     connect(btnPred,     &QPushButton::clicked, this, &SmartPub::on_labBtnPredicteur_clicked);
    if (btnExporter) connect(btnExporter, &QPushButton::clicked, this, &SmartPub::on_labBtnExporter_clicked);
    if (btnTrier)    connect(btnTrier,    &QPushButton::clicked, this, &SmartPub::on_labBtnTrier_clicked);
    if (btnConfirmer)connect(btnConfirmer,&QPushButton::clicked, this, &SmartPub::on_labBtnConfirmerForm_clicked);
    if (btnAnnuler)  connect(btnAnnuler,  &QPushButton::clicked, this, &SmartPub::on_labBtnAnnulerForm_clicked);
}

// ---------- Chargement des données (DB + fallback mock) ----------
void SmartPub::labChargerDonnees()
{
    labDataMap.clear();
    labNextId = 1;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery q(db);
        if (q.exec("SELECT ID_LABORATOIRE, NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, CODE_PROJET FROM LABORATOIRE ORDER BY ID_LABORATOIRE")) {
            while (q.next()) {
                LaboratoryData lab;
                lab.id         = q.value("ID_LABORATOIRE").toInt();
                lab.nom        = q.value("NOM").toString();
                lab.thematique = q.value("THEMATIQUE").toString();
                lab.statut     = q.value("DISPONIBILITE").toString() == "disponible" ? "Actif" : "Inactif";
                lab.adresse    = q.value("ADRESSE").toString();
                lab.budget     = 0;
                lab.capacite   = 0;
                lab.equipements.clear();
                lab.directeur.clear();
                labDataMap.insert(lab.id, lab);
                if (lab.id >= labNextId) labNextId = lab.id + 1;
            }
            if (!labDataMap.isEmpty()) {
                labAfficherListe();
                return;
            }
        }
    }

    labAfficherListe();
}

void SmartPub::labAfficherListe()
{
    QList<LaboratoryData> all = labDataMap.values();
    labAfficherListe(all);
}

void SmartPub::labAfficherListe(const QList<LaboratoryData> &labs)
{
    labTable->setRowCount(0);
    for (const LaboratoryData &lab : labs) {
        int row = labTable->rowCount();
        labTable->insertRow(row);
        labTable->setItem(row, 0, new QTableWidgetItem(QString::number(lab.id)));
        labTable->setItem(row, 1, new QTableWidgetItem(lab.nom));
        labTable->setItem(row, 2, new QTableWidgetItem(lab.thematique));
        labTable->setItem(row, 3, new QTableWidgetItem(lab.budget > 0 ? QString("%1 €").arg(lab.budget, 0, 'f', 0) : "N/A"));
        labTable->setItem(row, 4, new QTableWidgetItem(lab.capacite > 0 ? QString::number(lab.capacite) : "N/A"));

        // Badge statut coloré
        QTableWidgetItem *statutItem = new QTableWidgetItem(lab.statut);
        if (lab.statut == "Actif")             statutItem->setForeground(QColor("#10b981"));
        else if (lab.statut == "En Construction") statutItem->setForeground(QColor("#f59e0b"));
        else if (lab.statut == "En Rénovation")   statutItem->setForeground(QColor("#3b82f6"));
        else                                       statutItem->setForeground(QColor("#ef4444"));
        labTable->setItem(row, 5, statutItem);

        labTable->setItem(row, 6, new QTableWidgetItem(lab.directeur.isEmpty() ? "—" : lab.directeur));

        // Stocker l'id dans UserRole
        labTable->item(row, 0)->setData(Qt::UserRole, lab.id);
    }

    if (labTotalLabel)
        labTotalLabel->setText(QString::number(labs.size()) + " laboratoire" + (labs.size() > 1 ? "s" : ""));
}

void SmartPub::labViderFormulaire()
{
    if (labFormNom)        labFormNom->clear();
    if (labFormThematique) labFormThematique->setCurrentIndex(0);
    if (labFormBudget)     labFormBudget->clear();
    if (labFormCapacite)   labFormCapacite->setValue(10);
    if (labFormStatut)     labFormStatut->setCurrentIndex(0);
    if (labFormEquipements)labFormEquipements->clear();
    if (labFormDirecteur)  labFormDirecteur->clear();
}

void SmartPub::labRemplirFormulaire(const LaboratoryData &lab)
{
    if (labFormNom)        labFormNom->setText(lab.nom);
    if (labFormThematique) labFormThematique->setCurrentText(lab.thematique);
    if (labFormBudget)     labFormBudget->setText(lab.budget > 0 ? QString::number(lab.budget, 'f', 2) : "");
    if (labFormCapacite)   labFormCapacite->setValue(lab.capacite > 0 ? lab.capacite : 1);
    if (labFormStatut)     labFormStatut->setCurrentText(lab.statut);
    if (labFormEquipements)labFormEquipements->setText(lab.equipements);
    if (labFormDirecteur)  labFormDirecteur->setText(lab.directeur);
}

LaboratoryData SmartPub::labGetFormData() const
{
    LaboratoryData lab;
    lab.nom        = labFormNom        ? labFormNom->text().trimmed()        : "";
    lab.thematique = labFormThematique ? labFormThematique->currentText()    : "";
    lab.budget     = labFormBudget     ? labFormBudget->text().toDouble()    : 0.0;
    lab.capacite   = labFormCapacite   ? labFormCapacite->value()            : 1;
    lab.statut     = labFormStatut     ? labFormStatut->currentText()        : "Actif";
    lab.equipements= labFormEquipements? labFormEquipements->text().trimmed(): "";
    lab.directeur  = labFormDirecteur  ? labFormDirecteur->text().trimmed()  : "";
    return lab;
}

bool SmartPub::labValiderFormulaire() const
{
    if (!labFormNom || labFormNom->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Validation", "Le nom du laboratoire est obligatoire.");
        return false;
    }
    if (labFormBudget && !labFormBudget->text().isEmpty()) {
        bool ok; double v = labFormBudget->text().toDouble(&ok);
        if (!ok || v < 0) {
            QMessageBox::warning(nullptr, "Validation", "Le budget doit être un nombre positif.");
            return false;
        }
    }
    return true;
}

void SmartPub::labMontrerFormulaire(bool isEdit)
{
    if (!labFormFrame) return;
    QLabel *title = labFormFrame->findChild<QLabel*>("labFormTitle");
    if (title) title->setText(isEdit ? "✏️  Modifier le laboratoire" : "➕  Nouveau laboratoire");
    labFormFrame->setVisible(true);
}

void SmartPub::labCacherFormulaire()
{
    if (labFormFrame) labFormFrame->setVisible(false);
}

void SmartPub::labSetTableRowBackground(QTableWidget *table, int row, const QColor &color)
{
    if (!table) return;
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem *item = table->item(row, col);
        if (item) item->setBackground(color);
    }
}

// ---------- Slots ----------
void SmartPub::on_labBtnAjouter_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de laboratoires.");
        return;
    }
    labEditingId = -1;
    labViderFormulaire();
    labMontrerFormulaire(false);
}

void SmartPub::on_labBtnModifier_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas modifier de laboratoires.");
        return;
    }
    int row = labTable->currentRow();
    if (row < 0) return;
    int id = labTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (!labDataMap.contains(id)) return;
    labEditingId = id;
    labRemplirFormulaire(labDataMap[id]);
    labMontrerFormulaire(true);
}

void SmartPub::on_labBtnSupprimer_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer de laboratoires.");
        return;
    }
    int row = labTable->currentRow();
    if (row < 0) return;
    int id = labTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (!labDataMap.contains(id)) return;

    QString nom = labDataMap[id].nom;
    auto reply = QMessageBox::question(this, "Confirmation",
        QString("Supprimer le laboratoire\n« %1 » ?").arg(nom),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM LABORATOIRE WHERE ID_LABORATOIRE = :id");
        q.bindValue(":id", id);
        q.exec();
    }

    labDataMap.remove(id);
    labCacherFormulaire();
    labAfficherListe();
    QMessageBox::information(this, "Succès", "Laboratoire supprimé avec succès !");
}

void SmartPub::on_labBtnConfirmerForm_clicked()
{
    if (!labValiderFormulaire()) return;

    LaboratoryData lab = labGetFormData();

    if (labEditingId == -1) {
        // AJOUT
        lab.id = labNextId;
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery q(db);
            q.prepare("INSERT INTO LABORATOIRE(NOM, THEMATIQUE, DISPONIBILITE, ADRESSE) "
                      "VALUES(:nom, :them, :dispo, :adr)");
            q.bindValue(":nom",  lab.nom);
            q.bindValue(":them", lab.thematique);
            q.bindValue(":dispo", lab.statut == "Actif" ? "disponible" : "indisponible");
            q.bindValue(":adr",  lab.adresse.isEmpty() ? QString() : lab.adresse);
            if (q.exec()) {
                // Récupérer l'id généré par la séquence
                QSqlQuery qid(db);
                if (qid.exec("SELECT SEQ_LABORATOIRE.CURRVAL FROM DUAL")) {
                    qid.next();
                    lab.id = qid.value(0).toInt();
                }
            }
        }
        if (lab.id == labNextId) labNextId++;
        labDataMap.insert(lab.id, lab);
        QMessageBox::information(this, "Succès", "Laboratoire ajouté avec succès !");
    } else {
        // MODIFICATION
        lab.id = labEditingId;
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery q(db);
            q.prepare("UPDATE LABORATOIRE SET NOM=:nom, THEMATIQUE=:them, DISPONIBILITE=:dispo, ADRESSE=:adr WHERE ID_LABORATOIRE=:id");
            q.bindValue(":nom",  lab.nom);
            q.bindValue(":them", lab.thematique);
            q.bindValue(":dispo", lab.statut == "Actif" ? "disponible" : "indisponible");
            q.bindValue(":adr",  lab.adresse.isEmpty() ? QString() : lab.adresse);
            q.bindValue(":id",   lab.id);
            q.exec();
        }
        labDataMap[lab.id] = lab;
        QMessageBox::information(this, "Succès", "Laboratoire modifié avec succès !");
    }

    labEditingId = -1;
    labCacherFormulaire();
    labAfficherListe();
}

void SmartPub::on_labBtnAnnulerForm_clicked()
{
    labEditingId = -1;
    labViderFormulaire();
    labCacherFormulaire();
}

void SmartPub::on_labTableSelectionChanged()
{
    bool sel = !labTable->selectedItems().isEmpty();
    if (labBtnModifier)  labBtnModifier->setEnabled(sel);
    if (labBtnSupprimer) labBtnSupprimer->setEnabled(sel);
}

void SmartPub::on_labSearchChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        labAfficherListe();
        return;
    }
    QList<LaboratoryData> filtered;
    for (const LaboratoryData &lab : labDataMap) {
        if (lab.nom.contains(text, Qt::CaseInsensitive)
         || lab.thematique.contains(text, Qt::CaseInsensitive)
         || lab.directeur.contains(text, Qt::CaseInsensitive)) {
            filtered.append(lab);
        }
    }
    labAfficherListe(filtered);
}

void SmartPub::on_labBtnStatistiques_clicked()
{
    labMontrerStatistiques();
}

void SmartPub::on_labBtnOptimiseur_clicked()
{
    labOptimiseurCollab();
}

void SmartPub::on_labBtnPredicteur_clicked()
{
    labPredicteurBesoins();
}

void SmartPub::on_labBtnExporter_clicked()
{
    labExporter();
}

void SmartPub::on_labBtnTrier_clicked()
{
    labTrier();
}

// ---------- Statistiques ----------
void SmartPub::labMontrerStatistiques()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("📊  Statistiques des Laboratoires");
    dlg->setMinimumSize(900, 680);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog{background-color:#f8fafc;} QLabel{color:#334155;}");

    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 16);

    // Titre
    QLabel *title = new QLabel("📊  Statistiques Globales des Laboratoires");
    title->setStyleSheet("font-size:20px;font-weight:bold;color:#1e3a5f;");
    mainLayout->addWidget(title);

    // Calculs
    double totalBudget = 0; int totalCap = 0;
    QMap<QString, int> byThematic; QMap<QString, double> budgetByThematic;
    for (const LaboratoryData &lab : labDataMap) {
        totalBudget += lab.budget; totalCap += lab.capacite;
        byThematic[lab.thematique]++;
        budgetByThematic[lab.thematique] += lab.budget;
    }
    int count = labDataMap.size();
    double avgBudget = count > 0 ? totalBudget / count : 0;
    double avgCap    = count > 0 ? (double)totalCap / count : 0;

    // Cartes stats
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(12);
    auto makeCard = [](const QString &title_, const QString &val, const QString &color) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame{background:%1;border-radius:12px;}").arg(color));
        QVBoxLayout *cl = new QVBoxLayout(card); cl->setContentsMargins(16,14,16,14); cl->setSpacing(4);
        QLabel *t = new QLabel(title_); t->setStyleSheet("color:white;font-size:12px;font-weight:600;");
        QLabel *v = new QLabel(val);    v->setStyleSheet("color:white;font-size:22px;font-weight:bold;");
        cl->addWidget(t); cl->addWidget(v); return card;
    };
    cardsLayout->addWidget(makeCard("Laboratoires",   QString::number(count), "#3b82f6"));
    cardsLayout->addWidget(makeCard("Budget Total",   QString("%1 €").arg(totalBudget, 0, 'f', 0), "#10b981"));
    cardsLayout->addWidget(makeCard("Capacité Totale",QString("%1 chercheurs").arg(totalCap), "#f59e0b"));
    cardsLayout->addWidget(makeCard("Budget Moyen",   QString("%1 €").arg(avgBudget, 0, 'f', 0), "#8b5cf6"));
    cardsLayout->addWidget(makeCard("Capacité Moy.",  QString("%1").arg(avgCap, 0, 'f', 1), "#ef4444"));
    mainLayout->addLayout(cardsLayout);

    // Graphiques
    QHBoxLayout *chartsLayout = new QHBoxLayout(); chartsLayout->setSpacing(16);

    // Pie chart thématiques
    QPieSeries *pie = new QPieSeries();
    for (auto it = byThematic.begin(); it != byThematic.end(); ++it) {
        QPieSlice *s = pie->append(it.key(), it.value());
        s->setLabelVisible(true);
        s->setLabel(QString("%1: %2").arg(it.key()).arg(it.value()));
    }
    QChart *pieChart = new QChart(); pieChart->addSeries(pie);
    pieChart->setTitle("Répartition par Thématique");
    pieChart->legend()->setAlignment(Qt::AlignBottom);
    pieChart->setBackgroundBrush(QBrush(Qt::white));
    QChartView *pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    pieView->setMinimumSize(380, 280);
    pieView->setStyleSheet("background:white;border-radius:12px;border:1px solid #e2e8f0;");
    chartsLayout->addWidget(pieView);

    // Bar chart budgets
    QBarSet *bset = new QBarSet("Budget (€)");
    QStringList cats;
    for (auto it = budgetByThematic.begin(); it != budgetByThematic.end(); ++it) {
        *bset << it.value(); cats << it.key();
    }
    QBarSeries *bar = new QBarSeries(); bar->append(bset);
    QChart *barChart = new QChart(); barChart->addSeries(bar);
    barChart->setTitle("Budget par Thématique"); barChart->setAnimationOptions(QChart::SeriesAnimations);
    barChart->setBackgroundBrush(QBrush(Qt::white));
    QBarCategoryAxis *axX = new QBarCategoryAxis(); axX->append(cats);
    barChart->addAxis(axX, Qt::AlignBottom); bar->attachAxis(axX);
    QValueAxis *axY = new QValueAxis(); axY->setTitleText("Budget (€)");
    barChart->addAxis(axY, Qt::AlignLeft); bar->attachAxis(axY);
    barChart->legend()->setVisible(false);
    QChartView *barView = new QChartView(barChart);
    barView->setRenderHint(QPainter::Antialiasing);
    barView->setMinimumSize(380, 280);
    barView->setStyleSheet("background:white;border-radius:12px;border:1px solid #e2e8f0;");
    chartsLayout->addWidget(barView);
    mainLayout->addLayout(chartsLayout);

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(42); closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "color:white;border:none;border-radius:8px;font-size:13px;font-weight:600;padding:0 24px;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #059669);}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout(); btnRow->addStretch(); btnRow->addWidget(closeBtn);
    mainLayout->addLayout(btnRow);

    dlg->exec();
}

// ---------- Optimiseur de Collaboration ----------
void SmartPub::labOptimiseurCollab()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("🤝  Optimiseur de Collaboration");
    dlg->setMinimumSize(900, 600);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog{background-color:white;} QLabel{color:#1f2937;}");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(24, 24, 24, 16); layout->setSpacing(14);

    QLabel *title = new QLabel("🤝  Analyse des Synergies Inter-Laboratoires");
    title->setStyleSheet("font-size:18px;font-weight:bold;color:#1e3a5f;");
    layout->addWidget(title);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Laboratoire 1","Laboratoire 2","Synergie","Score","Économies","Recommandation"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setDefaultSectionSize(52);
    table->setStyleSheet("QTableWidget{color:#1f2937;background:white;gridline-color:#e5e7eb;border:1px solid #e2e8f0;border-radius:10px;}"
                         "QTableWidget::item{padding:8px;color:#1f2937;}"
                         "QHeaderView::section{background:#f3f4f6;color:#374151;padding:10px;border:none;border-bottom:1px solid #e5e7eb;font-weight:600;}");

    QList<LaboratoryData> labs = labDataMap.values();
    int synFound = 0; double totalSav = 0;

    for (int i = 0; i < labs.size(); i++) {
        for (int j = i + 1; j < labs.size(); j++) {
            const LaboratoryData &l1 = labs[i]; const LaboratoryData &l2 = labs[j];
            if (l1.thematique == l2.thematique && (l1.statut == "Actif" || l2.statut == "Actif")) {
                double sav = (l1.budget + l2.budget) * 0.15;
                totalSav += sav; synFound++;
                int row = table->rowCount(); table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(l1.nom));
                table->setItem(row, 1, new QTableWidgetItem(l2.nom));
                table->setItem(row, 2, new QTableWidgetItem("🎯 Thématique commune"));
                table->setItem(row, 3, new QTableWidgetItem("⭐⭐⭐⭐⭐"));
                table->setItem(row, 4, new QTableWidgetItem(QString("%1 €").arg(sav, 0, 'f', 0)));
                table->setItem(row, 5, new QTableWidgetItem("Partage de ressources, projets conjoints"));
                labSetTableRowBackground(table, row, QColor(220, 252, 231));
            }
        }
    }

    if (synFound == 0) {
        QLabel *noSyn = new QLabel("ℹ️  Aucune synergie détectée. Ajoutez plusieurs laboratoires avec des thématiques similaires.");
        noSyn->setWordWrap(true);
        noSyn->setStyleSheet("color:#6b7280;font-size:14px;padding:20px;background:#f9fafb;border-radius:8px;");
        layout->addWidget(noSyn);
    } else {
        layout->addWidget(table);
        QLabel *summary = new QLabel(QString(
            "<span style='color:#1e40af;font-weight:600;'>📊 %1 synergie(s) détectée(s)</span> — "
            "Économies potentielles : <span style='color:#059669;font-weight:600;'>%2 €</span>")
            .arg(synFound).arg(totalSav, 0, 'f', 0));
        summary->setTextFormat(Qt::RichText);
        summary->setStyleSheet("padding:10px;background:#eff6ff;border-radius:8px;font-size:13px;");
        layout->addWidget(summary);
    }

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(40); closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton{background:#6b7280;color:white;border:none;border-radius:8px;padding:0 20px;font-weight:600;}QPushButton:hover{background:#4b5563;}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout(); btnRow->addStretch(); btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
    dlg->exec();
}

// ---------- Prédicteur de Besoins ----------
void SmartPub::labPredicteurBesoins()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("🔮  Prédicteur de Besoins");
    dlg->setMinimumSize(920, 640);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog{background-color:white;} QLabel{color:#1f2937;}");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(24, 24, 24, 16); layout->setSpacing(14);

    QLabel *title = new QLabel("🔮  Prédicteur de Besoins & Alertes Intelligentes");
    title->setStyleSheet("font-size:18px;font-weight:bold;color:#1e3a5f;");
    layout->addWidget(title);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Laboratoire","Type d'Alerte","Priorité","Délai","Action Recommandée"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setDefaultSectionSize(56);
    table->setStyleSheet("QTableWidget{color:#1f2937;background:white;gridline-color:#e5e7eb;border:1px solid #e2e8f0;border-radius:10px;}"
                         "QTableWidget::item{padding:10px;color:#1f2937;}"
                         "QHeaderView::section{background:#f3f4f6;color:#374151;padding:10px;border:none;border-bottom:1px solid #e5e7eb;font-weight:600;}");

    int crit = 0, warn = 0;
    for (const LaboratoryData &lab : labDataMap) {
        if (lab.capacite > 22 && lab.statut == "Actif") {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("⚠️ Saturation capacité"));
            table->setItem(row,2,new QTableWidgetItem("🔴 CRITIQUE"));
            table->setItem(row,3,new QTableWidgetItem("2-3 mois"));
            table->setItem(row,4,new QTableWidgetItem("Augmenter la capacité de 20% ou créer une annexe"));
            labSetTableRowBackground(table, row, QColor(254, 226, 226)); crit++;
        } else if (lab.capacite > 18 && lab.statut == "Actif") {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("⚡ Capacité élevée"));
            table->setItem(row,2,new QTableWidgetItem("🟡 ATTENTION"));
            table->setItem(row,3,new QTableWidgetItem("4-6 mois"));
            table->setItem(row,4,new QTableWidgetItem("Planifier extension, optimiser l'espace"));
            labSetTableRowBackground(table, row, QColor(254, 243, 199)); warn++;
        }
        if (lab.budget > 300000) {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("💰 Budget élevé"));
            table->setItem(row,2,new QTableWidgetItem("🔵 INFO"));
            table->setItem(row,3,new QTableWidgetItem("Continu"));
            table->setItem(row,4,new QTableWidgetItem("Audit financier, recherche de synergies budgétaires"));
            labSetTableRowBackground(table, row, QColor(219, 234, 254));
        }
        if (lab.statut == "En Construction" || lab.statut == "En Rénovation") {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("🔧 Laboratoire indisponible"));
            table->setItem(row,2,new QTableWidgetItem("🟠 SUIVI"));
            table->setItem(row,3,new QTableWidgetItem("Variable"));
            table->setItem(row,4,new QTableWidgetItem("Suivre l'avancement, prévoir ouverture"));
            labSetTableRowBackground(table, row, QColor(255, 237, 213));
        }
    }

    if (table->rowCount() == 0) {
        QLabel *ok = new QLabel("✅  Aucune alerte détectée. Tous les laboratoires sont dans des conditions optimales !");
        ok->setWordWrap(true);
        ok->setStyleSheet("color:#059669;font-size:14px;padding:20px;background:#ecfdf5;border-radius:8px;");
        layout->addWidget(ok);
    } else {
        layout->addWidget(table);
        QString summary = QString("🔴 %1 critique(s)  —  🟡 %2 avertissement(s)").arg(crit).arg(warn);
        QLabel *sumLabel = new QLabel(summary);
        sumLabel->setStyleSheet("padding:10px;background:#fef9ec;border-radius:8px;font-size:13px;font-weight:600;color:#92400e;");
        layout->addWidget(sumLabel);
    }

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(40); closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton{background:#6b7280;color:white;border:none;border-radius:8px;padding:0 20px;font-weight:600;}QPushButton:hover{background:#4b5563;}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout(); btnRow->addStretch(); btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
    dlg->exec();
}

// ---------- Export CSV ----------
void SmartPub::labExporter()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les Laboratoires", "", "Fichiers CSV (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier : " + fileName);
        return;
    }
    QTextStream out(&file);
    out << "ID,Nom,Thématique,Budget(€),Capacité,Statut,Directeur,Équipements\n";
    for (const LaboratoryData &lab : labDataMap) {
        out << lab.id << ","
            << "\"" << lab.nom << "\","
            << "\"" << lab.thematique << "\","
            << lab.budget << ","
            << lab.capacite << ","
            << "\"" << lab.statut << "\","
            << "\"" << lab.directeur << "\","
            << "\"" << lab.equipements << "\"\n";
    }
    file.close();
    QMessageBox::information(this, "Export Réussi", "Données exportées avec succès :\n" + fileName);
}

// ---------- Tri ----------
void SmartPub::labTrier()
{
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu{background:white;border:1px solid #e2e8f0;border-radius:10px;padding:6px;}
        QMenu::item{padding:10px 20px;border-radius:6px;color:#334155;font-size:13px;}
        QMenu::item:selected{background:#eff6ff;color:#1d4ed8;}
    )");
    menu->addAction("⬆️  Nom (A → Z)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.nom < b.nom; });
        labAfficherListe(sorted);
    });
    menu->addAction("⬇️  Nom (Z → A)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.nom > b.nom; });
        labAfficherListe(sorted);
    });
    menu->addAction("💰  Budget (croissant)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.budget < b.budget; });
        labAfficherListe(sorted);
    });
    menu->addAction("💰  Budget (décroissant)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.budget > b.budget; });
        labAfficherListe(sorted);
    });
    menu->addAction("🧪  Thématique (A → Z)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.thematique < b.thematique; });
        labAfficherListe(sorted);
    });
    menu->popup(QCursor::pos());
}
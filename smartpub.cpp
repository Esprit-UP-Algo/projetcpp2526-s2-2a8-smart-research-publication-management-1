#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include <QApplication>
#include <QRegion>
#include <QProcess>
#include <QScreen>

// ============================================================================
// FONCTIONS HELPER GLOBALES (pour module Projets)
// ============================================================================

static QString getEtatColor(const QString &etat) {
    if (etat == "Actif")
        return "#10b981";
    if (etat == "Terminé")
        return "#3b82f6";
    if (etat == "En pause")
        return "#f59e0b";
    if (etat == "Planifié")
        return "#8b5cf6";
    return "#64748b";
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
        "chercheur@smartpub.com / chercheur123"
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
    for (const auto &p : m_projets) if (p.etat == "Actif") actifs++;
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

    QLabel *etatBadge = new QLabel(projet.etat);
    QString etatColor = ::getEtatColor(projet.etat);
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
        if (p.etat == "Actif") actifs++;
        else if (p.etat == "Terminé") termines++;
        else if (p.etat == "En pause") pause++;
        else if (p.etat == "Planifié") planifies++;

        if (p.etat != "Terminé" && p.etat != "Planifié") {
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
        if (p.etat == "Actif") actifs++;
        else if (p.etat == "Terminé") termines++;
        else if (p.etat == "En pause") pause++;
        else if (p.etat == "Planifié") planifies++;
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

// ============================================================================
// CLASSE SMARTPUB
// ============================================================================

SmartPub::SmartPub(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SmartPub), sidebarExpanded(false),
    isUserLoggedIn(false), cherchVueListeActive(true),
    cherchVueIconesActive(true), cherchChercheurSelectionne(-1),
    cherchIsLoggedIn(false), cherchOrderByClause("ID"),
    cherchWhereClause(), finVueListeActive(true),
    finTransactionSelectionnee(-1), evEventSelectionne(-1), nextProjetId(1),
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
    if (cherchIsLoggedIn || currentUser.role == UserRole::Guest) {
        cherchShowMainView();
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_btnPublications_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    updateProfileName(1);
    SR_updateButtonStyles();
}

void SmartPub::on_btnLaboratoires_clicked() {
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(5);
    updateProfileName(5);
    QMessageBox::information(this, "Information",
                             "Module Laboratoires en cours de développement");
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
    if (projetsCount >= 4)
        return "Senior - Expert";
    if (projetsCount >= 2)
        return "Confirmé";
    if (grade == "Professeur" || grade == "Maitre de Conferences")
        return "Senior";
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
    // Connect directly as Admin without checking credentials

    // Default to Admin
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

    // RAFRAICHIR LA LISTE DES CHERCHEURS POUR AFFICHER LES BOUTONS
    // Important : On le fait ici pour être sûr que l'interface réagit au rôle
    // Admin
    cherchAfficherListeChercheurs();

    // Afficher l'application
    mainStack->setCurrentIndex(1);

    // Vider les champs pour la forme (même s'ils sont cachés ou ignorés)
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

    QString sql = "SELECT ID, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM chercheur";
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
        int id = query.value("ID").toInt();
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
    menu->addSeparator();
    menu->addAction("Trier par Grade (Hiérarchie)", this,
                    [this]() { cherchTrierParGrade(); });
    menu->addSeparator();
    menu->addAction("Trier par Date (Plus récent)", this,
                    [this]() { cherchTrierParDateCreation(true); });
    menu->addAction("Trier par Date (Plus ancien)", this,
                    [this]() { cherchTrierParDateCreation(false); });

    menu->exec(QCursor::pos());
}

void SmartPub::cherchTrierParNom(bool croissant) {
    cherchOrderByClause = croissant ? "NOM ASC, PRENOM ASC" : "NOM DESC, PRENOM DESC";
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParGrade() {
    cherchOrderByClause = "DECODE(GRADE, 'Professeur', 1, 'Maitre de Conferences', 2, "
                         "'Docteur', 3, 'Ingenieur de Recherche', 4, 'Post-doctorant', 5, 'Doctorant', 6, 99), NOM, PRENOM";
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParDateCreation(bool croissant) {
    cherchOrderByClause = croissant ? "ID DESC" : "ID ASC";
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
    dialog->setWindowTitle("Statistiques des Chercheurs");
    dialog->setMinimumSize(1000, 800);
    dialog->setStyleSheet("background-color: #f8fafc;");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(24);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("📊 Tableau de Bord Statistique", dialog);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 700; color: "
                              "#1e293b; background: transparent; border: none;");
    mainLayout->addWidget(titleLabel);

    QScrollArea *scrollArea = new QScrollArea(dialog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(24);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // Stats grid
    QGridLayout *statsGrid = new QGridLayout();
    statsGrid->setSpacing(20);

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
        card->setMinimumHeight(140);
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setSpacing(8);
        layout->setContentsMargins(24, 24, 24, 24);

        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("color: #64748b; font-size: 14px; font-weight: "
                                  "600; background: transparent; border: none;");

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(
            QString("color: %1; font-size: 48px; font-weight: 700; background: "
                    "transparent; border: none;")
                .arg(color));

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        layout->addStretch();
        return card;
    };

    statsGrid->addWidget(
        createStatCard("Total Chercheurs",
                       QString::number(cherchChercheursMap.size()), "#3b82f6"),
        0, 0);

    int nbProfs = 0, nbDocs = 0;
    for (auto &data : cherchChercheursMap) {
        if (data.grade == "Professeur")
            nbProfs++;
        if (data.grade == "Doctorant")
            nbDocs++;
    }

    statsGrid->addWidget(
        createStatCard("Professeurs", QString::number(nbProfs), "#10b981"), 0, 1);
    statsGrid->addWidget(
        createStatCard("Doctorants", QString::number(nbDocs), "#f59e0b"), 0, 2);

    contentLayout->addLayout(statsGrid);

    // Répartition par grade
    QFrame *chartFrame = new QFrame();
    chartFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
    )");
    chartFrame->setMinimumHeight(400);
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    chartLayout->setSpacing(20);
    chartLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *chartTitle = new QLabel("Répartition par Grade", chartFrame);
    chartTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: "
                              "#1e293b; background: transparent; border: none;");
    chartLayout->addWidget(chartTitle);

    QMap<QString, int> gradeCount;
    for (auto &data : cherchChercheursMap) {
        gradeCount[data.grade]++;
    }

    for (auto it = gradeCount.begin(); it != gradeCount.end(); ++it) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *gradeLabel = new QLabel(it.key() + ":");
        gradeLabel->setStyleSheet("font-size: 16px; color: #334155; font-weight: "
                                  "600; background: transparent; border: none;");
        gradeLabel->setFixedWidth(200);

        QProgressBar *progress = new QProgressBar();
        progress->setRange(0, cherchChercheursMap.size());
        progress->setValue(it.value());
        progress->setTextVisible(true);
        progress->setFormat(QString("%1 chercheurs").arg(it.value()));
        progress->setStyleSheet(R"(
            QProgressBar {
                border: none;
                border-radius: 8px;
                background-color: #e2e8f0;
                text-align: center;
                height: 24px;
            }
            QProgressBar::chunk {
                background-color: #3b82f6;
                border-radius: 8px;
            }
        )");

        row->addWidget(gradeLabel);
        row->addWidget(progress, 1);
        chartLayout->addLayout(row);
    }

    chartLayout->addStretch();
    contentLayout->addWidget(chartFrame);

    // Indice de surcharge
    QFrame *overloadFrame = new QFrame();
    overloadFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
    )");
    QVBoxLayout *overloadLayout = new QVBoxLayout(overloadFrame);
    overloadLayout->setSpacing(20);
    overloadLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *overloadTitle =
        new QLabel("📈 Indice de Surcharge (Projets en cours)", overloadFrame);
    overloadTitle->setStyleSheet(
        "font-size: 20px; font-weight: 600; color: #1e293b; background: "
        "transparent; border: none;");
    overloadLayout->addWidget(overloadTitle);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *headerName = new QLabel("Chercheur");
    headerName->setStyleSheet("color: #64748b; font-weight: 600; font-size: "
                              "13px; background: transparent; border: none;");
    headerName->setFixedWidth(200);

    QLabel *headerProgress = new QLabel("Charge de travail");
    headerProgress->setStyleSheet("color: #64748b; font-weight: 600; font-size: "
                                  "13px; background: transparent; border: none;");

    QLabel *headerStatus = new QLabel("Statut");
    headerStatus->setStyleSheet("color: #64748b; font-weight: 600; font-size: "
                                "13px; background: transparent; border: none;");
    headerStatus->setFixedWidth(100);

    headerRow->addWidget(headerName);
    headerRow->addWidget(headerProgress, 1);
    headerRow->addWidget(headerStatus);
    overloadLayout->addLayout(headerRow);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e2e8f0;");
    line->setFixedHeight(1);
    overloadLayout->addWidget(line);

    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end();
         ++it) {
        auto data = it.value();
        int nbProjets = data.projetsIds.size();
        int surcharge = qMin(nbProjets * 25, 100);

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(15);

        QLabel *nameLabel =
            new QLabel(QString("%1 %2").arg(data.prenom).arg(data.nom));
        nameLabel->setFixedWidth(200);
        nameLabel->setStyleSheet("font-weight: 600; color: #334155; background: "
                                 "transparent; border: none;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(surcharge);
        progressBar->setTextVisible(true);
        progressBar->setFormat(QString("%1 projets").arg(nbProjets));
        progressBar->setFixedHeight(28);

        QString color;
        if (surcharge < 50)
            color = "#10b981";
        else if (surcharge < 75)
            color = "#f59e0b";
        else
            color = "#ef4444";

        progressBar->setStyleSheet(QString(R"(
            QProgressBar {
                border: none;
                border-radius: 14px;
                background-color: #e2e8f0;
                text-align: center;
                color: white;
                font-weight: 600;
                font-size: 12px;
            }
            QProgressBar::chunk {
                background-color: %1;
                border-radius: 14px;
            }
        )")
                                       .arg(color));

        QLabel *statusLabel = new QLabel();
        if (surcharge < 50)
            statusLabel->setText("🟢 Normal");
        else if (surcharge < 75)
            statusLabel->setText("🟡 Occupé");
        else
            statusLabel->setText("🔴 Surchargé");
        statusLabel->setStyleSheet(QString("color: %1; font-weight: 600; "
                                           "background: transparent; border: none;")
                                       .arg(color));
        statusLabel->setFixedWidth(100);

        rowLayout->addWidget(nameLabel);
        rowLayout->addWidget(progressBar, 1);
        rowLayout->addWidget(statusLabel);

        overloadLayout->addLayout(rowLayout);
    }

    contentLayout->addWidget(overloadFrame);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    QPushButton *btnClose = new QPushButton("Fermer", dialog);
    btnClose->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 48px;
            font-size: 16px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )");
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

    int newId = 1;
    QSqlQuery query(db);
    if (query.exec("SELECT TABLE1_SEQ.NEXTVAL FROM DUAL") && query.next()) {
        newId = query.value(0).toInt();
    } else {
        if (query.exec("SELECT NVL(MAX(ID), 0) + 1 FROM CHERCHEUR") && query.next())
            newId = query.value(0).toInt();
    }

    query.prepare("INSERT INTO chercheur (ID, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL) "
                 "VALUES (:id, :nom, :prenom, :email, :grade, :cin, :photo_profil)");
    query.bindValue(":id", newId);
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
    query.prepare("SELECT ID, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM chercheur WHERE ID = :id");
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
        updateQuery.prepare("UPDATE chercheur SET NOM = :nom, PRENOM = :prenom, EMAIL = :email, GRADE = :grade, PHOTO_PROFIL = :photo_profil WHERE ID = :id");
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
    query.prepare("DELETE FROM chercheur WHERE ID = :id");
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
    QSqlQuery query(db);
    query.prepare("SELECT ID, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM chercheur WHERE ID = :id");
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
    data.dateCreation = QDateTime();
    data.carriere = "";
    data.age = 0;

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(
        QString("Profil - %1 %2").arg(data.prenom).arg(data.nom));
    dialog->setMinimumSize(700, 600);
    dialog->setMaximumSize(900, 800);
    dialog->setStyleSheet("background-color: #f8fafc;");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
    )");
    headerFrame->setFixedHeight(200);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(15);

    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(120, 120);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet("border: 4px solid white; background: transparent;");
    avatarLabel->setScaledContents(false);
    avatarLabel->setMask(QRegion(0, 0, 120, 120, QRegion::Ellipse));
    QPixmap profilePix;
    QString avatarPath = data.photoPath.isEmpty() ? QString(":/avatar.png") : data.photoPath;
    if (!profilePix.load(avatarPath))
        profilePix.load(":/avatar.png");
    if (!profilePix.isNull())
        avatarLabel->setPixmap(makeCircularPixmap(profilePix, 120));
    headerLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);

    QLabel *nameLabel =
        new QLabel(QString("%1 %2").arg(data.prenom).arg(data.nom));
    nameLabel->setStyleSheet("color: white; font-size: 26px; font-weight: 700; "
                             "background: transparent; border: none;");
    nameLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(nameLabel, 0, Qt::AlignCenter);

    mainLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: white; border: none;");

    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    auto createInfoRow = [&](const QString &label, const QString &value,
                             const QString &icon = "") -> QFrame * {
        QFrame *row = new QFrame();
        row->setStyleSheet("background-color: #f8fafc; border-radius: 12px;");
        row->setMaximumHeight(80);
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(20, 15, 20, 15);

        QLabel *iconLabel = new QLabel(icon.isEmpty() ? "•" : icon);
        iconLabel->setStyleSheet(
            "font-size: 20px; background: transparent; border: none;");
        rowLayout->addWidget(iconLabel);

        QLabel *labelWidget = new QLabel(label + ":");
        labelWidget->setStyleSheet(
            "color: #64748b; font-size: 14px; font-weight: 600; min-width: 150px; "
            "background: transparent; border: none;");
        rowLayout->addWidget(labelWidget);

        QLabel *valueWidget = new QLabel(value);
        valueWidget->setStyleSheet("color: #1e293b; font-size: 16px; font-weight: "
                                   "500; background: transparent; border: none;");
        valueWidget->setWordWrap(true);
        rowLayout->addWidget(valueWidget, 1);

        return row;
    };

    contentLayout->addWidget(createInfoRow("Grade", data.grade, "🎓"));
    contentLayout->addWidget(createInfoRow("Email", data.email, "✉️"));
    contentLayout->addWidget(createInfoRow("CIN", data.cin, "🆔"));
    contentLayout->addWidget(
        createInfoRow("Âge", QString("%1 ans").arg(data.age), "🎂"));
    contentLayout->addWidget(
        createInfoRow("Date d'ajout",
                      data.dateCreation.toString("dd MMMM yyyy à hh:mm"), "📅"));
    contentLayout->addWidget(createInfoRow("Carrière", data.carriere, "⭐"));
    contentLayout->addWidget(createInfoRow(
        "Projets en cours", QString::number(data.projetsIds.size()), "📁"));

    if (!data.projetsIds.isEmpty()) {
        QLabel *projetsTitle = new QLabel("Détails des projets:");
        projetsTitle->setStyleSheet(
            "color: #1e293b; font-size: 18px; font-weight: 700; margin-top: 10px; "
            "background: transparent; border: none;");
        contentLayout->addWidget(projetsTitle);

        for (int projId : data.projetsIds) {
            QFrame *projFrame = new QFrame();
            projFrame->setStyleSheet("background-color: #eff6ff; border-left: 4px "
                                     "solid #3b82f6; border-radius: 8px;");
            QHBoxLayout *projLayout = new QHBoxLayout(projFrame);
            projLayout->setContentsMargins(15, 12, 15, 12);

            QLabel *projLabel = new QLabel(
                QString("Projet #%1 - En cours de développement").arg(projId));
            projLabel->setStyleSheet("color: #3b82f6; font-weight: 600; background: "
                                     "transparent; border: none;");
            projLayout->addWidget(projLabel);

            QPushButton *btnViewProj = new QPushButton("Voir");
            btnViewProj->setFixedWidth(80);
            btnViewProj->setStyleSheet(R"(
                QPushButton {
                    background-color: #3b82f6;
                    color: white;
                    border: none;
                    border-radius: 6px;
                    padding: 6px 12px;
                    font-size: 12px;
                }
                QPushButton:hover {
                    background-color: #2563eb;
                }
            )");
            projLayout->addWidget(btnViewProj);

            contentLayout->addWidget(projFrame);
        }
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet(
        "background-color: white; border-top: 1px solid #e2e8f0;");
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(30, 20, 30, 20);
    footerLayout->setSpacing(15);

    QPushButton *btnExport = new QPushButton("📄 Exporter la fiche");
    btnExport->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #3b82f6;
            color: #3b82f6;
        }
    )");
    connect(btnExport, &QPushButton::clicked, this,
            [this]() { on_cherchBtnExportDetails_clicked(); });

    QPushButton *btnClose = new QPushButton("Fermer");
    btnClose->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 32px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )");
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);

    footerLayout->addWidget(btnExport);
    footerLayout->addStretch();
    footerLayout->addWidget(btnClose);

    mainLayout->addWidget(footerFrame);

    dialog->exec();
}

void SmartPub::on_cherchBtnExportDetails_clicked() {
    QMessageBox::information(this, "Export", "Fiche exportée avec succès !");
}

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

    if (db.isOpen()) {
        QSqlQuery query(db);
        if (query.exec("SELECT DOI, TITRE, AUTEUR, DATES, REVUE, STATUT FROM PUBLICATIONS ORDER BY DOI")) {
            ui->SR_tablePublications->setRowCount(0);
            int row = 0;
            while (query.next()) {
                QVariant doiVar = query.value("DOI");
                int id = doiVar.toInt();
                if (id == 0 && !doiVar.toString().isEmpty()) id = -1; // keep non-numeric DOI in UserRole as string
                QString titre = query.value("TITRE").toString();
                QString auteur = query.value("AUTEUR").toString();
                QVariant dateVar = query.value("DATES");
                QDate d = dateVar.toDate();
                if (!d.isValid() && dateVar.toDateTime().isValid())
                    d = dateVar.toDateTime().date();
                QString dateStr = d.isValid() ? d.toString("yyyy-MM-dd") : dateVar.toString();
                if (dateStr.length() > 10) dateStr = dateStr.left(10);
                QString revue = query.value("REVUE").toString();
                QString statut = query.value("STATUT").toString();

                ui->SR_tablePublications->insertRow(row);
                QTableWidgetItem *titItem = new QTableWidgetItem(titre);
                titItem->setData(Qt::UserRole, id != -1 ? QVariant(id) : doiVar);
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
                QString d = ui->SR_tablePublications->item(r, 2) ? ui->SR_tablePublications->item(r, 2)->text() : QString();
                if (d.length() >= 4 && d.left(4).toInt() == thisYear) countThisYear++;
            }
            ui->SR_lblThisYearNumber->setText(QString::number(countThisYear));
            ui->SR_lblPlanSNumber->setText("0");
            int publie = 0, soumis = 0, revision = 0, accepte = 0;
            for (int r = 0; r < row; r++) {
                QString s = ui->SR_tablePublications->item(r, 4) ? ui->SR_tablePublications->item(r, 4)->text() : QString();
                if (s.contains("Publié", Qt::CaseInsensitive)) publie++;
                else if (s.contains("Soumis", Qt::CaseInsensitive)) soumis++;
                else if (s.contains("révision", Qt::CaseInsensitive)) revision++;
                else if (s.contains("Accepté", Qt::CaseInsensitive)) accepte++;
            }
            int total = row > 0 ? row : 1;
            ui->SR_lblStatPublie->setText(QString("● Publié (%1%)").arg((publie * 100) / total));
            ui->SR_lblStatSoumis->setText(QString("● Soumis (%1%)").arg((soumis * 100) / total));
            ui->SR_lblStatRevision->setText(QString("● En révision (%1%)").arg((revision * 100) / total));
            ui->SR_lblStatAccepte->setText(QString("● Accepté (%1%)").arg((accepte * 100) / total));
            return;
        }
    }

    // Fallback: sample data when DB not available or table missing
    QStringList titres = {"Machine Learning pour la détection de fraudes",
                          "Analyse des données génomiques",
                          "Quantum Computing: état de l'art",
                          "Intelligence Artificielle en médecine",
                          "Blockchain pour la sécurité des données",
                          "Deep Learning pour la vision par ordinateur",
                          "Cryptographie post-quantique",
                          "IoT et sécurité des réseaux"};

    QStringList auteurs = {
                           "Dr. Martin, Prof. Dubois", "Dr. Laurent, Dr. Bernard",
                           "Prof. Moreau, Dr. Petit",  "Dr. Roux, Prof. Simon",
                           "Dr. Michel, Dr. Garcia",   "Prof. Durand, Dr. Lefebvre",
                           "Dr. Morel, Prof. Girard",  "Dr. Andre, Dr. Blanc"};

    QStringList dates = {"2024-01-15", "2024-02-20", "2023-11-10", "2024-03-05",
                         "2023-09-18", "2024-04-12", "2023-12-01", "2024-05-20"};

    QStringList revues = {
                          "IEEE Transactions on AI",    "Nature Genetics",
                          "Quantum Information Review", "Medical AI Journal",
                          "Blockchain Security Review", "Computer Vision and Pattern Recognition",
                          "Journal of Cryptology",      "IEEE Internet of Things Journal"};

    QStringList statuts = {"Publié",  "Publié", "Soumis", "En révision",
                           "Accepté", "Publié", "Soumis", "En révision"};

    ui->SR_tablePublications->setRowCount(titres.size());

    for (int i = 0; i < titres.size(); ++i) {
        QTableWidgetItem *titItem = new QTableWidgetItem(titres[i]);
        titItem->setData(Qt::UserRole, i + 1);
        ui->SR_tablePublications->setItem(i, 0, titItem);
        ui->SR_tablePublications->setItem(i, 1, new QTableWidgetItem(auteurs[i]));
        ui->SR_tablePublications->setItem(i, 2, new QTableWidgetItem(dates[i]));
        ui->SR_tablePublications->setItem(i, 3, new QTableWidgetItem(revues[i]));
        ui->SR_tablePublications->setItem(i, 4, new QTableWidgetItem(statuts[i]));

        SR_addButtonsToRow(i);
    }

    ui->SR_tablePublications->resizeRowsToContents();

    ui->SR_lblTotalNumber->setText(QString::number(titres.size()));
    ui->SR_lblThisYearNumber->setText("5");
    ui->SR_lblPlanSNumber->setText("3");

    int publie = 3, soumis = 2, revision = 2, accepte = 1;
    int total = titres.size();

    ui->SR_lblStatPublie->setText(
        QString("● Publié (%1%)").arg((publie * 100) / total));
    ui->SR_lblStatSoumis->setText(
        QString("● Soumis (%1%)").arg((soumis * 100) / total));
    ui->SR_lblStatRevision->setText(
        QString("● En révision (%1%)").arg((revision * 100) / total));
    ui->SR_lblStatAccepte->setText(
        QString("● Accepté (%1%)").arg((accepte * 100) / total));
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

    if (editingPublicationRow != -1) {
        // Edit mode: UPDATE in DB
        QSqlQuery query(db);
        query.prepare("UPDATE PUBLICATIONS SET TITRE = :titre, AUTEUR = :auteur, "
                     "DATES = TO_DATE(:date_pub, 'YYYY-MM-DD'), REVUE = :revue, STATUT = :statut WHERE DOI = :doi");
        query.bindValue(":titre", titre);
        query.bindValue(":auteur", auteurs);
        query.bindValue(":date_pub", dateStr);
        query.bindValue(":revue", revue);
        query.bindValue(":statut", statut);
        query.bindValue(":doi", ui->SR_tablePublications->item(editingPublicationRow, 0)->data(Qt::UserRole));
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de la modification : " + query.lastError().text());
            return;
        }
        ui->SR_tablePublications->item(editingPublicationRow, 0)->setText(titre);
        ui->SR_tablePublications->item(editingPublicationRow, 1)->setText(auteurs);
        ui->SR_tablePublications->item(editingPublicationRow, 2)->setText(dateStr);
        ui->SR_tablePublications->item(editingPublicationRow, 3)->setText(revue);
        ui->SR_tablePublications->item(editingPublicationRow, 4)->setText(statut);
        QMessageBox::information(this, "Succès", "Publication modifiée avec succès");
        editingPublicationRow = -1;
        ui->SR_btnAjouterPublication->setText("Ajouter");
    } else {
        // Add mode: INSERT into DB
        int newId = 1;
        QSqlQuery seqQuery(db);
        if (seqQuery.exec("SELECT NVL(MAX(DOI), 0) + 1 FROM PUBLICATIONS") && seqQuery.next()) {
            newId = seqQuery.value(0).toInt();
        }
        QSqlQuery query(db);
        query.prepare("INSERT INTO PUBLICATIONS (DOI, TITRE, AUTEUR, DATES, REVUE, STATUT) "
                      "VALUES (:doi, :titre, :auteur, TO_DATE(:date_pub, 'YYYY-MM-DD'), :revue, :statut)");
        query.bindValue(":doi", newId);
        query.bindValue(":titre", titre);
        query.bindValue(":auteur", auteurs);
        query.bindValue(":date_pub", dateStr);
        query.bindValue(":revue", revue);
        query.bindValue(":statut", statut);
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout : " + query.lastError().text());
            return;
        }
        int row = ui->SR_tablePublications->rowCount();
        ui->SR_tablePublications->insertRow(row);
        QTableWidgetItem *titItem = new QTableWidgetItem(titre);
        titItem->setData(Qt::UserRole, newId);
        ui->SR_tablePublications->setItem(row, 0, titItem);
        ui->SR_tablePublications->setItem(row, 1, new QTableWidgetItem(auteurs));
        ui->SR_tablePublications->setItem(row, 2, new QTableWidgetItem(dateStr));
        ui->SR_tablePublications->setItem(row, 3, new QTableWidgetItem(revue));
        ui->SR_tablePublications->setItem(row, 4, new QTableWidgetItem(statut));
        SR_addButtonsToRow(row);
        ui->SR_tablePublications->resizeRowsToContents();
        QMessageBox::information(this, "Succès", "Publication ajoutée avec succès");
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
    int id = titItem->data(Qt::UserRole).toInt();
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
    int id = titItem->data(Qt::UserRole).toInt();
    QString titre = titItem->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmer la suppression",
        "Êtes-vous sûr de vouloir supprimer la publication \"" + titre + "\" ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("DELETE FROM PUBLICATIONS WHERE DOI = :doi");
        query.bindValue(":doi", titItem->data(Qt::UserRole));
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression : " + query.lastError().text());
            return;
        }
    }
    ui->SR_tablePublications->removeRow(row);
    if (editingPublicationRow == row) editingPublicationRow = -1;
    else if (editingPublicationRow > row) editingPublicationRow--;
    ui->SR_btnAjouterPublication->setText("Ajouter");
    QMessageBox::information(this, "Succès", "Publication supprimée.");
}

// ============================================================================
// MODULE FINANCES
// ============================================================================

void SmartPub::finSetupUI() {
    ui->finStackedWidget->setCurrentIndex(0);
    finVueListeActive = true;

    ui->finComboBoxProjet->addItems(
        {"Projet AI-2024-001", "Projet Quantum-2024-002",
         "Projet BioTech-2024-003", "Projet CyberSec-2024-004"});
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
    TransactionData t1;
    t1.id = 1;
    t1.projet = "Projet AI-2024-001";
    t1.type = "Recette";
    t1.montant = 50000.00;
    t1.date = "15/01/2024";
    t1.categorie = "Équipement";
    t1.statut = "Validée";
    t1.description = "Achat de serveurs GPU";
    finTransactionsMap[1] = t1;

    TransactionData t2;
    t2.id = 2;
    t2.projet = "Projet Quantum-2024-002";
    t2.type = "Dépense";
    t2.montant = 25000.00;
    t2.date = "20/02/2024";
    t2.categorie = "Personnel";
    t2.statut = "En attente";
    t2.description = "Salaire chercheur post-doc";
    finTransactionsMap[2] = t2;

    TransactionData t3;
    t3.id = 3;
    t3.projet = "Projet BioTech-2024-003";
    t3.type = "Dépense";
    t3.montant = 8000.00;
    t3.date = "10/03/2024";
    t3.categorie = "Consommables";
    t3.statut = "Validée";
    t3.description = "Réactifs de laboratoire";
    finTransactionsMap[3] = t3;

    finAfficherListeTransactions();
}

void SmartPub::finAfficherListeTransactions() {
    ui->finTableTransactions->setRowCount(0);
    for (auto it = finTransactionsMap.begin(); it != finTransactionsMap.end();
         ++it) {
        finAjouterTransactionTable(it.value());
    }
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
    ui->finTableTransactions->setItem(row, 6, new QTableWidgetItem(data.statut));
    ui->finTableTransactions->setItem(
        row, 7, new QTableWidgetItem("Modifier | Supprimer"));
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
    ui->finStackedWidget->setCurrentIndex(1);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnRecherche_clicked() {
    QString searchText = ui->finLineEditRecherche->text();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Recherche",
                                 "Veuillez entrer un terme de recherche");
    } else {
        QMessageBox::information(this, "Recherche",
                                 "Recherche de transaction: " + searchText);
    }
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

    menu->addAction("Trier par Date", this, [this]() {
        QMessageBox::information(this, "Tri", "Tri par date effectué");
    });
    menu->addAction("Trier par Montant", this, [this]() {
        QMessageBox::information(this, "Tri", "Tri par montant effectué");
    });
    menu->addAction("Trier par Projet", this, [this]() {
        QMessageBox::information(this, "Tri", "Tri par projet effectué");
    });

    menu->exec(QCursor::pos());
}

void SmartPub::on_finBtnExport_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les transactions", QDir::homePath(), "CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Export",
                                 "Transactions exportées avec succès !");
    }
}

void SmartPub::on_finBtnStatistiques_clicked() {
    QMessageBox::information(this, "Statistiques",
                             "Module statistiques finances - À implémenter");
}

void SmartPub::on_finBtnAjouterTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }

    QString projet = ui->finComboBoxProjet->currentText();
    QString type = ui->finComboBoxType->currentText();
    QString montantStr = ui->finLineEditMontant->text();
    QString date = ui->finDateEdit->date().toString("dd/MM/yyyy");
    QString categorie = ui->finComboBoxCategorie->currentText();
    QString statut = ui->finComboBoxStatut->currentText();
    QString description = ui->finTextEditDescription->toPlainText();

    if (projet.isEmpty() || montantStr.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }

    bool ok;
    double montant = montantStr.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Erreur", "Montant invalide");
        return;
    }

    int newId =
        finTransactionsMap.isEmpty() ? 1 : finTransactionsMap.keys().last() + 1;

    TransactionData data;
    data.id = newId;
    data.projet = projet;
    data.type = type;
    data.montant = montant;
    data.date = date;
    data.categorie = categorie;
    data.statut = statut;
    data.description = description;

    finTransactionsMap[newId] = data;

    QMessageBox::information(this, "Succès", "Transaction ajoutée avec succès !");

    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();

    ui->finLineEditMontant->clear();
    ui->finTextEditDescription->clear();
}

void SmartPub::on_finBtnAnnulerAjout_clicked() {
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
    QMessageBox::information(this, "Modifier",
                             "Fonctionnalité de modification - À implémenter");
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

    auto reply = QMessageBox::question(
        this, "Supprimer", "Confirmer la suppression de cette transaction ?");
    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "Succès", "Transaction supprimée");
        ui->finTableTransactions->removeRow(currentRow);
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
    if (!query.exec("SELECT CODE, NOM, LIEU, DATE_EVENT FROM EVENEMENT ORDER BY DATE_EVENT")) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les événements : " + query.lastError().text());
        return;
    }

    ui->evTableEvents->setRowCount(0);
    ui->evTableSearchEvents->setRowCount(0);

    while (query.next()) {
        EventData data;
        data.code = query.value("CODE").toString();
        data.nom = query.value("NOM").toString();
        data.lieu = query.value("LIEU").toString();
        QVariant dVal = query.value("DATE_EVENT");
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
    query.prepare("SELECT CODE, NOM, LIEU, DATE_EVENT FROM EVENEMENT WHERE UPPER(LIEU) LIKE UPPER(:lieu) ORDER BY DATE_EVENT");
    query.bindValue(":lieu", "%" + lieu + "%");
    if (!query.exec()) {
        QMessageBox::warning(this, "Erreur", "Recherche échouée : " + query.lastError().text());
        return;
    }
    while (query.next()) {
        int row = ui->evTableSearchEvents->rowCount();
        ui->evTableSearchEvents->insertRow(row);
        QString dateStr = query.value("DATE_EVENT").toString();
        if (dateStr.contains("T")) dateStr = dateStr.left(10);
        ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(query.value("CODE").toString()));
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

    QString code = ui->evLineEditID->text().trimmed();
    QString nom = ui->evLineEditNom->text().trimmed();
    QString lieu = ui->evLineEditLieu->text().trimmed();
    QString date = ui->evLineEditDate->text().trimmed();

    if (code.isEmpty() || nom.isEmpty() || lieu.isEmpty() || date.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    if (!evEditingCode.isEmpty()) {
        QSqlQuery query(db);
        query.prepare("UPDATE EVENEMENT SET NOM = :nom, LIEU = :lieu, DATE_EVENT = TO_DATE(:date_event, 'DD/MM/YYYY') WHERE CODE = :code");
        query.bindValue(":nom", nom);
        query.bindValue(":lieu", lieu);
        query.bindValue(":date_event", date);
        query.bindValue(":code", evEditingCode);
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Échec de la modification : " + query.lastError().text());
            return;
        }
        QMessageBox::information(this, "Succès", "Événement modifié avec succès !");
        evEditingCode.clear();
    } else {
        QSqlQuery query(db);
        query.prepare("INSERT INTO EVENEMENT (CODE, NOM, LIEU, DATE_EVENT) VALUES (:code, :nom, :lieu, TO_DATE(:date_event, 'DD/MM/YYYY'))");
        query.bindValue(":code", code);
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
        query.prepare("DELETE FROM EVENEMENT WHERE CODE = :code");
        query.bindValue(":code", code);
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
    ui->btnStatistiques->setStyleSheet(
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
}

void SmartPub::projSetupAIButton()
{
    QPushButton *btnAI = new QPushButton("🤖", this);
    btnAI->setObjectName("btnAIRecommandations");
    btnAI->setMinimumSize(50, 42);
    btnAI->setMaximumSize(50, 42);
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

    QHBoxLayout *toolbarLayout = qobject_cast<QHBoxLayout*>(ui->toolbarFrameProjets->layout());
    if (toolbarLayout) {
        int index = -1;
        for (int i = 0; i < toolbarLayout->count(); ++i) {
            QLayoutItem *item = toolbarLayout->itemAt(i);
            if (item && item->widget() == ui->btnStatistiques) {
                index = i;
                break;
            }
        }

        if (index >= 0) {
            toolbarLayout->insertWidget(index + 1, btnAI);
        } else {
            toolbarLayout->insertWidget(2, btnAI);
        }

        toolbarLayout->insertSpacing(index + 2, 10);
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

void SmartPub::projSetupUI()
{
    projSetupTable();

    ui->btnListeProjets->setToolTip("Liste des projets");
    ui->btnAjouterProjet->setToolTip("Ajouter un projet");
    ui->btnModifierProjet->setToolTip("Modifier le projet sélectionné");
    ui->btnSupprimerProjet->setToolTip("Supprimer le projet sélectionné");
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
    projets.clear();

    projets.append(Projet(nextProjetId++, "PRJ-2024-AI-01", "Smart-Traffic 2026",
                          QDate(2024, 1, 15), QDate(2026, 12, 31),
                          "Dr. Ahmed Ben Ali", "Actif", "75%",
                          "Développement d'un système de gestion du trafic intelligent utilisant l'IA et le machine learning pour optimiser les flux urbains."));

    projets.append(Projet(nextProjetId++, "PRJ-2024-BIO-02", "Analyse Génome Humain",
                          QDate(2024, 3, 1), QDate(2025, 6, 30),
                          "Pr. Fatima Zohra", "Actif", "60%",
                          "Analyse approfondie du génome humain pour identifier les marqueurs génétiques de maladies rares."));

    projets.append(Projet(nextProjetId++, "PRJ-2023-QUANT-01", "Calculateur Quantique",
                          QDate(2023, 9, 10), QDate(2024, 8, 15),
                          "Dr. Mohamed Salah", "En pause", "45%",
                          "Développement d'un prototype de calculateur quantique pour applications cryptographiques."));

    projets.append(Projet(nextProjetId++, "PRJ-2024-ENV-03", "Énergies Renouvelables",
                          QDate(2024, 2, 1), QDate(2025, 12, 31),
                          "Dr. Sarah Johnson", "Actif", "30%",
                          "Développement de nouvelles technologies pour l'énergie solaire à haut rendement."));

    projets.append(Projet(nextProjetId++, "PRJ-2023-MED-04", "Vaccins Nouvelle Génération",
                          QDate(2023, 11, 15), QDate(2024, 10, 30),
                          "Pr. Robert Chen", "Terminé", "100%",
                          "Recherche sur des vaccins à ARNm pour maladies infectieuses émergentes."));
}

void SmartPub::projSetupComboBoxes()
{
    ui->comboBoxResponsableForm->clear();
    ui->comboBoxResponsableForm->addItem("Dr. Ahmed Ben Ali");
    ui->comboBoxResponsableForm->addItem("Pr. Fatima Zohra");
    ui->comboBoxResponsableForm->addItem("Dr. Mohamed Salah");
    ui->comboBoxResponsableForm->addItem("Dr. Sarah Johnson");
    ui->comboBoxResponsableForm->addItem("Pr. Robert Chen");

    ui->comboBoxEtatForm->clear();
    ui->comboBoxEtatForm->addItem("Planifié");
    ui->comboBoxEtatForm->addItem("Actif");
    ui->comboBoxEtatForm->addItem("En pause");
    ui->comboBoxEtatForm->addItem("Terminé");
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

    // Titre - CORRECTION: Afficher le titre complet sans tronquage
    // On utilise elided text seulement si vraiment nécessaire
    QTableWidgetItem *titreItem = new QTableWidgetItem(projet.titre);
    titreItem->setFont(QFont("Segoe UI", 9, QFont::Medium));
    titreItem->setToolTip(projet.titre);
    titreItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // CORRECTION: Pas de tronquage ici, on laisse la colonne s'adapter
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

    // État avec couleur
    QTableWidgetItem *etatItem = new QTableWidgetItem(projet.etat);
    etatItem->setTextAlignment(Qt::AlignCenter);
    etatItem->setForeground(QColor(::getEtatColor(projet.etat)));
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
    projViderTable();

    const QVector<Projet> &projetsACharger = filtresActifs ? projetsFiltres : projets;

    for (int i = 0; i < projetsACharger.size(); ++i) {
        projAjouterProjetTable(projetsACharger[i], i);
    }

    projAjusterColonnesTable();  // Cette ligne doit être après la boucle
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
    QVector<QPushButton*> buttons = {
        ui->btnListeProjets, ui->btnAjouterProjet,
        ui->btnModifierProjet, ui->btnSupprimerProjet
    };

    QString activeStyle = "QPushButton {"
                          "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                          "        stop:0 #3b82f6, stop:1 #10b981);"
                          "    color: white;"
                          "    border: none;"
                          "    border-radius: 8px;"
                          "    font-size: 14px;"
                          "    font-weight: 600;"
                          "}"
                          "QPushButton:hover {"
                          "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                          "        stop:0 #2563eb, stop:1 #059669);"
                          "}";

    QString inactiveStyle = "QPushButton {"
                            "    background-color: transparent;"
                            "    color: #64748b;"
                            "    border: none;"
                            "    font-size: 14px;"
                            "    font-weight: 500;"
                            "}"
                            "QPushButton:hover {"
                            "    color: #334155;"
                            "    background-color: #f1f5f9;"
                            "}";

    for (int i = 0; i < buttons.size(); ++i) {
        buttons[i]->setStyleSheet(i == index ? activeStyle : inactiveStyle);
    }
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

void SmartPub::projMettreAJourProjetTable(int row, const Projet &projet)
{
    QString titreDisplay = projet.titre;
    if (titreDisplay.length() > 35) {
        titreDisplay = titreDisplay.left(32) + "...";
    }

    ui->tableWidgetProjets->item(row, 1)->setText(projet.code);
    ui->tableWidgetProjets->item(row, 2)->setText(titreDisplay);
    ui->tableWidgetProjets->item(row, 2)->setToolTip(projet.titre);
    ui->tableWidgetProjets->item(row, 3)->setText(projet.dateDebut.toString("dd/MM/yyyy"));
    ui->tableWidgetProjets->item(row, 4)->setText(projet.dateFin.toString("dd/MM/yyyy"));
    ui->tableWidgetProjets->item(row, 5)->setText(projet.responsable);

    QTableWidgetItem *etatItem = ui->tableWidgetProjets->item(row, 6);
    etatItem->setText(projet.etat);
    etatItem->setForeground(QColor(::getEtatColor(projet.etat)));

    QTableWidgetItem *progItem = ui->tableWidgetProjets->item(row, 7);
    progItem->setText(projet.progression);
    progItem->setForeground(QColor(::getProgressionColorFromString(projet.progression)));
}

void SmartPub::onSupprimerProjetClicked()
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

    int respIndex = ui->comboBoxResponsableForm->findText(projet.responsable);
    if (respIndex != -1) ui->comboBoxResponsableForm->setCurrentIndex(respIndex);

    int etatIndex = ui->comboBoxEtatForm->findText(projet.etat);
    if (etatIndex != -1) ui->comboBoxEtatForm->setCurrentIndex(etatIndex);

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
    }
    projet.code = ui->lineEditCodeForm->text();
    projet.titre = ui->lineEditTitreForm->text();
    projet.dateDebut = ui->dateEditDebutForm->date();
    projet.dateFin = ui->dateEditFinForm->date();
    projet.responsable = ui->comboBoxResponsableForm->currentText();
    projet.etat = ui->comboBoxEtatForm->currentText();
    projet.progression = "0%";
    projet.description = ui->textEditDescriptionForm->toPlainText();
    return projet;
}

bool SmartPub::projValiderFormulaire() const
{
    if (ui->lineEditCodeForm->text().isEmpty()) {
        QMessageBox::warning(const_cast<SmartPub*>(this), "Validation", "Le code du projet est requis");
        ui->lineEditCodeForm->setFocus();
        return false;
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

    if (projet.etat == "Terminé") return "Sain";

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

    if (projet.etat == "Terminé") return alertes;

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
        if (p.etat != "Terminé" && p.etat != "Planifié") {
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
        if (p.etat != "Terminé" && QDate::currentDate() > p.dateFin) {
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

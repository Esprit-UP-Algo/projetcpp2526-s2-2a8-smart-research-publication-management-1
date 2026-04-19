#include "smartpub.h"
#include "ui_smartpub.h"
#include <QFocusEvent>
#include <QRandomGenerator>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>

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
    setupUI();
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
        "Comptes disponibles:\n"
        "admin@gmail.com / admin123\n"
        "smartpub.chercheur@gmail.com / chercheur123\n"
        "smartpub.publications@gmail.com / pub123\n"
        "smartpub.evenement@gmail.com / evenement123\n"
        "smartpub.finance@gmail.com / fin123\n"
        "smartpub.laboratoire@gmail.com / lab123\n"
        "smartpub.projet@gmail.com / projet123\n"
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
    errorLabel->hide();

    AppUserAccount acc;
    QString errMsg;

    if (m_authService.authenticate(emailEdit->text(), passwordEdit->text(),
                                   &acc, &errMsg))
    {
        loggedInUser = UserAccount{
            acc.email,
            acc.password,
            acc.allowedModule,
            UserRole::Admin,
            acc.displayName,
            acc.moduleIndex
        };
        loggedIn = true;
        accept();
        return;
    }

    errorLabel->setText("❌ " + (errMsg.isEmpty()
                                     ? QStringLiteral("Email ou mot de passe incorrect")
                                     : errMsg));
    errorLabel->show();
    passwordEdit->clear();
    passwordEdit->setFocus();
}

void LoginDialog::onForgotPasswordClicked()
{
    ForgotPasswordSmsDialog dlg(this);
    dlg.exec();
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
// DIALOG MOT DE PASSE OUBLIÉ PAR SMS — 3 étapes
// ============================================================================

QString ForgotPasswordSmsDialog::genererOtp()
{
    int code = QRandomGenerator::global()->bounded(100000, 999999);
    return QString::number(code);
}

bool ForgotPasswordSmsDialog::validerNumeroTunisien(const QString &phone)
{
    // Format attendu : +216 suivi de 8 chiffres (ex: +21698765432)
    static const QRegularExpression re(QStringLiteral("^\\+216[0-9]{8}$"));
    return re.match(phone).hasMatch();
}

ForgotPasswordSmsDialog::ForgotPasswordSmsDialog(QWidget *parent)
    : QDialog(parent), m_attempts(0)
{
    setWindowTitle(QString::fromUtf8("R\u00e9initialisation par SMS"));
    setMinimumSize(460, 420);
    resize(460, 420);
    setModal(true);
    m_nam = new QNetworkAccessManager(this);
    setupUI();
}

void ForgotPasswordSmsDialog::setupUI()
{
    setStyleSheet(
        "QDialog { background: white; }"
        "QLabel { color: #1e293b; font-size: 14px; background: transparent; }"
        "QLineEdit {"
        "    padding: 11px; border: 2px solid #e2e8f0; border-radius: 8px;"
        "    font-size: 14px; background: #f8fafc; color: #1e293b; }"
        "QLineEdit:focus { border-color: #3b82f6; background: #ffffff; color: #1e293b; }"
        "QPushButton { border-radius: 8px; font-size: 14px; font-weight: 600;"
        "    padding: 12px; border: none; }"
    );

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(36, 32, 36, 32);
    root->setSpacing(0);

    auto *titleLbl = new QLabel(QString::fromUtf8("Mot de passe oubli\u00e9"));
    titleLbl->setAlignment(Qt::AlignCenter);
    titleLbl->setStyleSheet("font-size: 22px; font-weight: bold; color: #3b82f6; margin-bottom: 4px;");
    root->addWidget(titleLbl);

    auto *subLbl = new QLabel(QString::fromUtf8("R\u00e9initialisation via SMS (num\u00e9ros tunisiens +216)"));
    subLbl->setAlignment(Qt::AlignCenter);
    subLbl->setStyleSheet("font-size: 12px; color: #64748b; margin-bottom: 20px;");
    root->addWidget(subLbl);

    m_stack = new QStackedWidget(this);
    root->addWidget(m_stack);

    // ── ÉTAPE 1 : email + téléphone ─────────────────────────────────────────
    auto *page1 = new QWidget();
    auto *lay1  = new QVBoxLayout(page1);
    lay1->setSpacing(12);
    lay1->setContentsMargins(0, 0, 0, 0);

    lay1->addWidget(new QLabel(QString::fromUtf8("Adresse email du compte :")));
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("votre.email@smartpub.com");
    lay1->addWidget(m_emailEdit);

    lay1->addWidget(new QLabel(QString::fromUtf8("Num\u00e9ro tunisien (+216XXXXXXXX) :")));
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("+21698765432");
    m_phoneEdit->setMaxLength(13);
    lay1->addWidget(m_phoneEdit);

    auto *hintLbl = new QLabel(QString::fromUtf8(
        "\u2139 1 SMS gratuit/jour via TextBelt (essai gratuit inclus)"));
    hintLbl->setStyleSheet("font-size: 11px; color: #64748b;");
    lay1->addWidget(hintLbl);

    m_error1 = new QLabel();
    m_error1->setStyleSheet("color:#ef4444;font-size:12px;padding:6px;"
                             "background:#fee2e2;border-radius:6px;");
    m_error1->setWordWrap(true);
    m_error1->hide();
    lay1->addWidget(m_error1);

    lay1->addSpacing(8);
    auto *btnEnvoyer = new QPushButton(QString::fromUtf8("Envoyer le code SMS"));
    btnEnvoyer->setStyleSheet("QPushButton{background:#3b82f6;color:white;}"
                               "QPushButton:hover{background:#2563eb;}");
    connect(btnEnvoyer, &QPushButton::clicked, this, &ForgotPasswordSmsDialog::onEnvoyerCode);
    lay1->addWidget(btnEnvoyer);

    auto *btnAnnuler1 = new QPushButton("Annuler");
    btnAnnuler1->setStyleSheet("QPushButton{background:#f1f5f9;color:#475569;}"
                                "QPushButton:hover{background:#e2e8f0;}");
    connect(btnAnnuler1, &QPushButton::clicked, this, &QDialog::reject);
    lay1->addWidget(btnAnnuler1);
    m_stack->addWidget(page1);

    // ── ÉTAPE 2 : saisie OTP ────────────────────────────────────────────────
    auto *page2 = new QWidget();
    auto *lay2  = new QVBoxLayout(page2);
    lay2->setSpacing(12);
    lay2->setContentsMargins(0, 0, 0, 0);

    m_infoLabel2 = new QLabel();
    m_infoLabel2->setWordWrap(true);
    m_infoLabel2->setAlignment(Qt::AlignCenter);
    m_infoLabel2->setStyleSheet("font-size:13px;color:#334155;padding:10px;"
                                 "background:#eff6ff;border-radius:8px;");
    lay2->addWidget(m_infoLabel2);

    lay2->addSpacing(8);
    lay2->addWidget(new QLabel(QString::fromUtf8("Code re\u00e7u par SMS (6 chiffres) :")));
    m_otpEdit = new QLineEdit();
    m_otpEdit->setPlaceholderText("_ _ _ _ _ _");
    m_otpEdit->setMaxLength(6);
    m_otpEdit->setAlignment(Qt::AlignCenter);
    m_otpEdit->setValidator(new QIntValidator(0, 999999, this));
    m_otpEdit->setStyleSheet(
        "QLineEdit{padding:14px;letter-spacing:8px;font-size:22px;font-weight:bold;"
        "border:2px solid #e2e8f0;border-radius:8px;background:#f8fafc;color:#1e293b;}"
        "QLineEdit:focus{border-color:#3b82f6;background:#ffffff;color:#1e293b;}");
    lay2->addWidget(m_otpEdit);

    m_error2 = new QLabel();
    m_error2->setStyleSheet("color:#ef4444;font-size:12px;padding:6px;"
                             "background:#fee2e2;border-radius:6px;");
    m_error2->setWordWrap(true);
    m_error2->hide();
    lay2->addWidget(m_error2);

    lay2->addSpacing(8);
    auto *btnValider = new QPushButton(QString::fromUtf8("V\u00e9rifier le code"));
    btnValider->setStyleSheet("QPushButton{background:#10b981;color:white;}"
                               "QPushButton:hover{background:#059669;}");
    connect(btnValider, &QPushButton::clicked, this, &ForgotPasswordSmsDialog::onValiderCode);
    lay2->addWidget(btnValider);

    auto *btnRetour2 = new QPushButton(QString::fromUtf8("Retour"));
    btnRetour2->setStyleSheet("QPushButton{background:#f1f5f9;color:#475569;}"
                               "QPushButton:hover{background:#e2e8f0;}");
    connect(btnRetour2, &QPushButton::clicked, this, [this]() {
        m_stack->setCurrentIndex(0);
        m_error2->hide();
        m_otpEdit->clear();
        m_attempts = 0;
    });
    lay2->addWidget(btnRetour2);
    m_stack->addWidget(page2);

    // ── ÉTAPE 3 : nouveau mot de passe ──────────────────────────────────────
    auto *page3 = new QWidget();
    auto *lay3  = new QVBoxLayout(page3);
    lay3->setSpacing(12);
    lay3->setContentsMargins(0, 0, 0, 0);

    auto *step3Info = new QLabel(QString::fromUtf8(
        "\u2705 Identit\u00e9 v\u00e9rifi\u00e9e !\n"
        "Choisissez votre nouveau mot de passe."));
    step3Info->setWordWrap(true);
    step3Info->setAlignment(Qt::AlignCenter);
    step3Info->setStyleSheet("font-size:13px;color:#16a34a;padding:10px;"
                              "background:#f0fdf4;border-radius:8px;font-weight:600;");
    lay3->addWidget(step3Info);

    lay3->addSpacing(8);
    lay3->addWidget(new QLabel(QString::fromUtf8("Nouveau mot de passe (min. 8 caract\u00e8res) :")));
    m_newPassEdit = new QLineEdit();
    m_newPassEdit->setEchoMode(QLineEdit::Password);
    m_newPassEdit->setPlaceholderText(QString::fromUtf8("\u2022\u2022\u2022\u2022\u2022\u2022\u2022\u2022"));
    lay3->addWidget(m_newPassEdit);

    lay3->addWidget(new QLabel(QString::fromUtf8("Confirmer le mot de passe :")));
    m_confirmPassEdit = new QLineEdit();
    m_confirmPassEdit->setEchoMode(QLineEdit::Password);
    m_confirmPassEdit->setPlaceholderText(QString::fromUtf8("\u2022\u2022\u2022\u2022\u2022\u2022\u2022\u2022"));
    lay3->addWidget(m_confirmPassEdit);

    m_error3 = new QLabel();
    m_error3->setStyleSheet("color:#ef4444;font-size:12px;padding:6px;"
                             "background:#fee2e2;border-radius:6px;");
    m_error3->setWordWrap(true);
    m_error3->hide();
    lay3->addWidget(m_error3);

    lay3->addSpacing(8);
    auto *btnSave = new QPushButton(QString::fromUtf8("Enregistrer le mot de passe"));
    btnSave->setStyleSheet("QPushButton{background:#3b82f6;color:white;}"
                            "QPushButton:hover{background:#2563eb;}");
    connect(btnSave, &QPushButton::clicked, this,
            &ForgotPasswordSmsDialog::onEnregistrerMotDePasse);
    lay3->addWidget(btnSave);
    m_stack->addWidget(page3);

    m_stack->setCurrentIndex(0);
}

// ── Étape 1 ──────────────────────────────────────────────────────────────────

void ForgotPasswordSmsDialog::onEnvoyerCode()
{
    const QString email = m_emailEdit->text().trimmed();
    const QString phone = m_phoneEdit->text().trimmed();

    if (email.isEmpty() || !email.contains('@')) {
        m_error1->setText(QString::fromUtf8("\u274c Email invalide."));
        m_error1->show();
        return;
    }

    AppAuthService auth;
    if (!auth.emailExists(email)) {
        m_error1->setText(QString::fromUtf8(
            "\u274c Aucun compte trouv\u00e9 pour cet email."));
        m_error1->show();
        return;
    }

    if (!validerNumeroTunisien(phone)) {
        m_error1->setText(QString::fromUtf8(
            "\u274c Format invalide. Utilisez +216 suivi de 8 chiffres.\n"
            "Exemple : +21698765432"));
        m_error1->show();
        return;
    }

    m_error1->hide();
    m_email    = email;
    m_otp      = genererOtp();
    m_attempts = 0;
    m_otpExpiry = QDateTime::currentDateTime().addSecs(600); // 10 minutes

    qDebug() << "[SMS-OTP] Code:" << m_otp << "| Tel:" << phone << "| Email:" << email;

    sendSmsTextBelt(phone, m_otp);
    goToStep2(phone);
}

// ── Envoi SMS via TextBelt (1 SMS gratuit/jour/IP, supporte +216) ────────────

void ForgotPasswordSmsDialog::sendSmsTextBelt(const QString &phone, const QString &otp)
{
    // Clé "textbelt" = 1 SMS gratuit par jour et par IP.
    // Pour un usage intensif, achetez une clé sur textbelt.com.
    const QString apiKey = QStringLiteral("textbelt");

    const QString message = QString(
        "SmartPub - Code de reinitialisation: %1\n"
        "Valable 10 minutes. Ne le partagez pas."
    ).arg(otp);

    QNetworkRequest request(QUrl("https://textbelt.com/text"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    QUrlQuery params;
    params.addQueryItem("phone",   phone);
    params.addQueryItem("message", message);
    params.addQueryItem("key",     apiKey);

    QNetworkReply *reply = m_nam->post(request,
                                       params.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        qDebug().noquote() << "[TextBelt]" << reply->readAll();
        reply->deleteLater();
    });
}

// ── Transition étape 2 ───────────────────────────────────────────────────────

void ForgotPasswordSmsDialog::goToStep2(const QString &phone)
{
    QString masked = phone;
    if (phone.length() >= 4)
        masked = phone.left(phone.length() - 4).replace(
                     QRegularExpression("[0-9]"), "*") + phone.right(4);

    m_infoLabel2->setText(
        QString::fromUtf8(
            "Code envoy\u00e9 au %1\n\n"
            "Expire dans 10 minutes. Max 3 tentatives.\n"
            "(Code visible dans la console Qt si SMS non re\u00e7u)")
        .arg(masked));

    m_stack->setCurrentIndex(1);
    m_otpEdit->clear();
    m_otpEdit->setFocus();
}

// ── Étape 2 : vérification OTP ───────────────────────────────────────────────

void ForgotPasswordSmsDialog::onValiderCode()
{
    const QString saisie = m_otpEdit->text().trimmed();

    if (saisie.isEmpty()) {
        m_error2->setText(QString::fromUtf8("\u274c Veuillez saisir le code SMS."));
        m_error2->show();
        return;
    }

    // Vérification expiration
    if (QDateTime::currentDateTime() > m_otpExpiry) {
        m_error2->setText(QString::fromUtf8(
            "\u274c Code expir\u00e9. Recommencez depuis l'\u00e9tape 1."));
        m_error2->show();
        m_stack->setCurrentIndex(0);
        m_attempts = 0;
        return;
    }

    // Limite de tentatives
    m_attempts++;
    if (m_attempts > 3) {
        m_error2->setText(QString::fromUtf8(
            "\u274c Trop de tentatives. Recommencez depuis l'\u00e9tape 1."));
        m_error2->show();
        m_stack->setCurrentIndex(0);
        m_attempts = 0;
        return;
    }

    if (saisie != m_otp) {
        const int restants = 3 - m_attempts;
        m_error2->setText(QString::fromUtf8(
            "\u274c Code incorrect. %1 tentative(s) restante(s).")
            .arg(restants));
        m_error2->show();
        return;
    }

    // Code correct → étape 3
    m_error2->hide();
    goToStep3();
}

// ── Transition étape 3 ───────────────────────────────────────────────────────

void ForgotPasswordSmsDialog::goToStep3()
{
    m_newPassEdit->clear();
    m_confirmPassEdit->clear();
    m_error3->hide();
    m_stack->setCurrentIndex(2);
    m_newPassEdit->setFocus();
}

// ── Étape 3 : enregistrement du nouveau mot de passe ────────────────────────

void ForgotPasswordSmsDialog::onEnregistrerMotDePasse()
{
    const QString newPass     = m_newPassEdit->text();
    const QString confirmPass = m_confirmPassEdit->text();

    if (newPass.length() < 8) {
        m_error3->setText(QString::fromUtf8(
            "\u274c Le mot de passe doit contenir au moins 8 caract\u00e8res."));
        m_error3->show();
        return;
    }

    // Force minimale : au moins une lettre et un chiffre
    const bool hasLetter = newPass.contains(QRegularExpression("[A-Za-z]"));
    const bool hasDigit  = newPass.contains(QRegularExpression("[0-9]"));
    if (!hasLetter || !hasDigit) {
        m_error3->setText(QString::fromUtf8(
            "\u274c Le mot de passe doit contenir au moins une lettre et un chiffre."));
        m_error3->show();
        return;
    }

    if (newPass != confirmPass) {
        m_error3->setText(QString::fromUtf8(
            "\u274c Les mots de passe ne correspondent pas."));
        m_error3->show();
        return;
    }

    AppAuthService auth;
    if (!auth.updatePassword(m_email, newPass)) {
        m_error3->setText(QString::fromUtf8(
            "\u274c Erreur lors de la sauvegarde. R\u00e9essayez."));
        m_error3->show();
        return;
    }

    qDebug() << "[SMS-OTP] Mot de passe mis a jour pour :" << m_email;

    QMessageBox::information(
        this,
        QString::fromUtf8("Succ\u00e8s"),
        QString::fromUtf8(
            "\u2705 Mot de passe modifi\u00e9 avec succ\u00e8s !\n\n"
            "Vous pouvez maintenant vous connecter\n"
            "avec votre nouveau mot de passe."));
    accept();
}

// ============================================================================
// MODULE PUBLICATIONS — Upload PDF
// ============================================================================

void SmartPub::on_btnUploadPDF_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        QString::fromUtf8("S\u00e9lectionner un fichier PDF"),
        QString(),
        "PDF Files (*.pdf)");

    if (filePath.isEmpty()) return;

    PDFUploadHandler uploadHandler;
    if (!uploadHandler.processUploadedFile(filePath)) {
        QMessageBox::critical(this, "Erreur", uploadHandler.getLastError());
        return;
    }

    QMap<QString, QString> fields = uploadHandler.getExtractedFields();
    if (fields.isEmpty()) {
        QMessageBox::information(this, "Information",
            QString::fromUtf8("Aucun champ reconnu trouv\u00e9 dans le PDF."));
        return;
    }

    QString confirm = QString::fromUtf8("Informations extraites du PDF :\n\n");
    if (fields.contains("titre"))
        confirm += QString("Titre: %1\n").arg(fields["titre"]);
    if (fields.contains("date_de_publication"))
        confirm += QString("Date: %1\n").arg(fields["date_de_publication"]);
    if (fields.contains("revue_journal"))
        confirm += QString("Revue/Journal: %1\n").arg(fields["revue_journal"]);
    if (fields.contains("statut"))
        confirm += QString("Statut: %1\n").arg(fields["statut"]);
    confirm += QString::fromUtf8("\nVoulez-vous remplir le formulaire avec ces informations ?");

    if (QMessageBox::question(this,
            QString::fromUtf8("Confirmer les informations"),
            confirm,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes) != QMessageBox::Yes)
        return;

    if (fields.contains("titre"))
        ui->SR_lineEditTitre->setText(fields["titre"]);
    if (fields.contains("revue_journal"))
        ui->SR_lineEditRevue->setText(fields["revue_journal"]);
    if (fields.contains("date_de_publication")) {
        const QString dateStr = fields["date_de_publication"];
        QDate parsed;
        for (const QString &fmt : QStringList{"yyyy-MM-dd","dd/MM/yyyy","MM/dd/yyyy","dd-MM-yyyy","yyyy/MM/dd"}) {
            parsed = QDate::fromString(dateStr, fmt);
            if (parsed.isValid()) break;
        }
        if (parsed.isValid() && ui->SR_dateEditPublication)
            ui->SR_dateEditPublication->setDate(parsed);
    }
    if (fields.contains("statut") && ui->SR_comboBoxStatut) {
        int idx = ui->SR_comboBoxStatut->findText(fields["statut"], Qt::MatchFixedString);
        if (idx >= 0) ui->SR_comboBoxStatut->setCurrentIndex(idx);
    }
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

    // Style complet — écrase le thème global du QMainWindow
    setStyleSheet(
        "QDialog {"
        "    background-color: #f8fafc;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel { color: #1e293b; font-size: 14px; background: transparent; }"
        "QPushButton { border: none; border-radius: 8px; padding: 12px 24px;"
        "    font-size: 14px; font-weight: 600; }"
        "QComboBox {"
        "    background-color: #f8fafc; border: 2px solid #e2e8f0;"
        "    border-radius: 8px; padding: 10px; min-height: 40px;"
        "    font-size: 13px; color: #1e293b; }"
        "QComboBox:focus { border-color: #3b82f6; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView {"
        "    background-color: white; color: #1e293b;"
        "    selection-background-color: #eff6ff; border: 1px solid #e2e8f0; }"
        "QCheckBox { spacing: 8px; font-size: 13px; color: #334155; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px;"
        "    border: 2px solid #cbd5e1; background-color: white; }"
        "QCheckBox::indicator:checked { background-color: #3b82f6; border-color: #3b82f6; }"
        "QCheckBox::indicator:unchecked:hover { border-color: #3b82f6; }"
        "QSpinBox {"
        "    background-color: #f8fafc; border: 2px solid #e2e8f0;"
        "    border-radius: 8px; padding: 8px; min-height: 40px;"
        "    min-width: 80px; font-size: 13px; color: #1e293b; }"
        "QSpinBox:focus { border-color: #3b82f6; }"
        "QGroupBox {"
        "    font-weight: bold; border: 1px solid #e2e8f0; border-radius: 12px;"
        "    margin-top: 15px; padding: 20px; background-color: white; }"
        "QGroupBox::title {"
        "    subcontrol-origin: margin; left: 15px; padding: 0 10px;"
        "    color: #3b82f6; font-size: 14px; }"
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


// ============================================================================
// CLASSE SMARTPUB
// ============================================================================

SmartPub::SmartPub(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SmartPub), sidebarExpanded(false),
    isUserLoggedIn(false), cherchVueListeActive(true),
    cherchVueIconesActive(true), cherchChercheurSelectionne(-1),
    cherchIsLoggedIn(false), cherchOrderByClause("ID_CHERCHEUR"),
    cherchWhereClause(),
    finVueListeActive(true),
    finTransactionSelectionnee(-1), finTriColonne(4),
    finTriOrdre(Qt::DescendingOrder), evEventSelectionne(-1),
    currentProjetId(-1), isEditing(false), currentSortColumn(-1),
    currentSortOrder(Qt::AscendingOrder), filtresActifs(false),
    cherchBtnSelectProjets(nullptr),
    cherchProjetsListWidget(nullptr), cherchLabelProjetsSelec(nullptr),
    editingPublicationRow(-1),
    SR_filterFrame(nullptr), SR_filterTitre(nullptr), SR_filterAuteur(nullptr),
    SR_filterStatut(nullptr), SR_btnReinitFilter(nullptr),
    SR_sortColumn(2), SR_sortOrder(Qt::DescendingOrder) {
    ui->setupUi(this);
    // Configurer les dimensions de la fenêtre
    this->setMinimumSize(1280, 720);
    this->resize(1400, 800);

    // Initialiser les notifications OS (System Tray)
    OsNotification::instance()->initialize(QApplication::windowIcon());

    // Initialiser l'interface utilisateur
    setupUI();


    // =====================================================================
    // ARDUINO — Scenario 1 : Controle d'acces RFID au laboratoire
    //           (Entree / Sortie avec pointage dans la BD Oracle)
    // =====================================================================

    arduino       = nullptr;
    scenarioAcces = nullptr;
    scenarioProgramme = nullptr;

    arduino = new Arduino();
    if (arduino->connect_arduino() == 0) {
        qDebug() << "[SmartPub] Arduino connecte sur"
                 << arduino->getarduino_port_name();

        // --- Scenario 1 : controle acces laboratoire (ID labo = 1) ---
        scenarioAcces = new Scenario1(arduino, 1);

        // Traitement de chaque trame RFID recue depuis l'Arduino
        connect(arduino->getserial(), &QSerialPort::readyRead,
                this, [this]() {
                    scenarioAcces->processAccess();
                });

        // --- Demi Scenario 2 : affichage programme LED ---
        scenarioProgramme = new DemiScenario2(arduino);

    } else {
        qDebug() << "[SmartPub] Echec de connexion a l'Arduino";
    }

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
        // Réinitialiser l'utilisateur courant
        currentUser = UserAccount{
            QString(), QString(), QStringLiteral("ALL"),
            UserRole::Guest, QStringLiteral("Invité"), -1
        };
        isUserLoggedIn = false;

        // Réactiver tous les boutons sidebar avant de retourner au login
        const QList<QPushButton*> sidebarBtns = {
            ui->btnChercheurs, ui->btnPublications, ui->btnFinances,
            ui->btnEvenements, ui->btnProjets, ui->btnLaboratoires
        };
        for (QPushButton *btn : sidebarBtns) {
            if (btn) {
                btn->setEnabled(true);
                btn->setToolTip(QString());
            }
        }

        // Vider les champs de login
        if (ui->cherchLineEditLoginEmail)    ui->cherchLineEditLoginEmail->clear();
        if (ui->cherchLineEditLoginPassword) ui->cherchLineEditLoginPassword->clear();

        mainStack->setCurrentIndex(0);
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

void SmartPub::checkPermissions()
{
    applyModuleRestrictions();
}

void SmartPub::applyModuleRestrictions()
{
    const bool fullAccess = (currentUser.module == QStringLiteral("ALL")
                             || currentUser.moduleIndex == -1);

    // Correspondance bouton sidebar → index module
    // IMPORTANT : cet ordre doit correspondre exactement à stackedWidgetModules
    struct SidebarEntry { QPushButton *btn; int moduleIdx; };
    const QList<SidebarEntry> entries = {
                                         { ui->btnChercheurs,   0 },
                                         { ui->btnPublications, 1 },
                                         { ui->btnFinances,     2 },
                                         { ui->btnEvenements,   3 },
                                         { ui->btnProjets,      4 },
                                         { ui->btnLaboratoires, 5 },
                                         };

    const QString disabledStyle = QStringLiteral(
        "QPushButton { color: #475569; background-color: transparent;"
        " border: none; border-radius: 12px; padding: 14px 20px;"
        " font-size: 16px; font-weight: 500; text-align: center; }"
        );

    for (const SidebarEntry &e : entries) {
        if (!e.btn) continue;
        const bool allowed = fullAccess || (currentUser.moduleIndex == e.moduleIdx);
        e.btn->setEnabled(allowed);
        if (!allowed) {
            e.btn->setStyleSheet(disabledStyle);
            e.btn->setToolTip(QStringLiteral("Accès restreint à votre module"));
        } else {
            e.btn->setToolTip(QString());
            // Le style actif/inactif sera re-appliqué par setActiveNavigationButton
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

    if (event->type() == QEvent::FocusOut) {
        auto *fe = static_cast<QFocusEvent *>(event);
        if (fe->reason() == Qt::PopupFocusReason
            || fe->reason() == Qt::ActiveWindowFocusReason) {
            return QMainWindow::eventFilter(obj, event);
        }
        if (obj == ui->lineEditCodeForm && !isEditing) {
            if (projCodeDebounceTimer)
                projCodeDebounceTimer->stop();
            projTouchedCode = true;
            projValidateCode(false);
        } else if (obj == ui->lineEditTitreForm) {
            if (projTitreDebounceTimer)
                projTitreDebounceTimer->stop();
            projTouchedTitre = true;
            projValidateTitre(false);
        } else if (obj == ui->dateEditDebutForm || obj == ui->dateEditFinForm) {
            projTouchedDates = true;
            projValidateDates(false);
        } else if (obj == ui->comboBoxResponsableForm) {
            projTouchedResponsable = true;
            projValidateResponsable(false);
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
    if (dialog.exec() == QDialog::Accepted) {
        const QString theme = dialog.getSelectedTheme();
        if (theme == QStringLiteral("dark")) {
            // ── Thème sombre ──────────────────────────────────────────────────
            qApp->setStyleSheet(
                "QMainWindow, QWidget { background-color: #0f172a; color: #e2e8f0; }"
                "QFrame { background-color: #1e293b; border-color: #334155; }"
                "QTableWidget { background-color: #1e293b; color: #e2e8f0;"
                "    gridline-color: #334155; selection-background-color: #3b82f6; }"
                "QTableWidget::item { color: #e2e8f0; border-bottom: 1px solid #334155; }"
                "QHeaderView::section { background-color: #0f172a; color: #94a3b8;"
                "    border: none; padding: 8px; }"
                "QLineEdit, QTextEdit, QComboBox, QDateEdit, QSpinBox {"
                "    background-color: #1e293b; color: #e2e8f0;"
                "    border: 1.5px solid #334155; border-radius: 8px; padding: 8px; }"
                "QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QDateEdit:focus {"
                "    border-color: #3b82f6; }"
                "QComboBox QAbstractItemView { background-color: #1e293b; color: #e2e8f0;"
                "    selection-background-color: #3b82f6; border: 1px solid #334155; }"
                "QScrollBar:vertical { background: #1e293b; width: 8px; border-radius: 4px; }"
                "QScrollBar::handle:vertical { background: #475569; border-radius: 4px; }"
                "QLabel { color: #e2e8f0; background: transparent; }"
                "QPushButton { border-radius: 8px; }"
                "QGroupBox { background-color: #1e293b; border: 1px solid #334155;"
                "    border-radius: 10px; color: #e2e8f0; }"
                "QGroupBox::title { color: #3b82f6; }"
                "QCheckBox { color: #e2e8f0; }"
                "QCheckBox::indicator { background-color: #1e293b; border: 2px solid #475569;"
                "    border-radius: 4px; width: 16px; height: 16px; }"
                "QCheckBox::indicator:checked { background-color: #3b82f6; border-color: #3b82f6; }"
                "QListWidget { background-color: #1e293b; color: #e2e8f0;"
                "    border: 1px solid #334155; }"
                "QListWidget::item:selected { background-color: #3b82f6; }"
                "QToolTip { background-color: #1e293b; color: #e2e8f0;"
                "    border: 1px solid #334155; }"
            );
        } else {
            // ── Thème clair (défaut) ──────────────────────────────────────────
            qApp->setStyleSheet(
                "QMainWindow { background-color: #f8fafc; }"
                "QWidget { font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif; }"
                "QScrollArea { border: none; background-color: transparent; }"
                "QScrollBar:vertical { border: none; background: #f1f5f9; width: 8px;"
                "    border-radius: 4px; }"
                "QScrollBar::handle:vertical { background: #cbd5e1; border-radius: 4px; }"
                "QScrollBar::handle:vertical:hover { background: #94a3b8; }"
                "QScrollBar:horizontal { border: none; background: #f1f5f9; height: 8px;"
                "    border-radius: 4px; }"
                "QScrollBar::handle:horizontal { background: #cbd5e1; border-radius: 4px; }"
                "QScrollBar::handle:horizontal:hover { background: #94a3b8; }"
            );
        }
    }
}

void SmartPub::on_btnChercheurs_clicked() { handleChercheursNavigation(); }

void SmartPub::on_btnPublications_clicked() { handlePublicationsNavigation(); }

void SmartPub::on_btnFinances_clicked() { handleFinancesNavigation(); }

void SmartPub::on_btnEvenements_clicked() { handleEvenementsNavigation(); }

void SmartPub::on_btnProjets_clicked() { handleProjetsNavigation(); }

void SmartPub::on_btnLaboratoires_clicked() { handleLaboratoiresNavigation(); }

// ============================================================================
// SLOTS UI — délégation vers handle* (logique métier dans les modules *.cpp)
// ============================================================================


void SmartPub::on_SR_btnVueListe_clicked() { handleSRBtnVueListeClicked(); }
void SmartPub::on_SR_btnAjouter_clicked() { handleSRBtnAjouterClicked(); }
void SmartPub::on_SR_btnRecherche_clicked() { handleSRBtnRechercheClicked(); }
void SmartPub::on_SR_btnTri_clicked() { handleSRBtnTriClicked(); }
void SmartPub::on_SR_btnExport_clicked() { handleSRBtnExportClicked(); }
void SmartPub::on_SR_btnStatistiques_clicked() {
    handleSRBtnStatistiquesClicked();
}
void SmartPub::on_SR_btnAjouterPublication_clicked() {
    handleSRBtnAjouterPublicationClicked();
}
void SmartPub::on_SR_btnAnnulerAjout_clicked() {
    handleSRBtnAnnulerAjoutClicked();
}
void SmartPub::on_SR_modifierPublication_clicked() {
    handleSRModifierPublicationClicked();
}
void SmartPub::on_SR_supprimerPublication_clicked() {
    handleSRSupprimerPublicationClicked();
}
void SmartPub::SR_applyFilterListe() { handleSRApplyFilterListe(); }
void SmartPub::SR_reinitFilterListe() { handleSRReinitFilterListe(); }

void SmartPub::on_finBtnVueListe_clicked() { handleFinBtnVueListeClicked(); }
void SmartPub::on_finBtnAjouter_clicked() { handleFinBtnAjouterClicked(); }
void SmartPub::on_finBtnRecherche_clicked() { handleFinBtnRechercheClicked(); }
void SmartPub::on_finBtnTri_clicked() { handleFinBtnTriClicked(); }
void SmartPub::on_finBtnExport_clicked() { handleFinBtnExportClicked(); }
void SmartPub::on_finBtnStatistiques_clicked() {
    handleFinBtnStatistiquesClicked();
}
void SmartPub::on_finBtnAjouterTransaction_clicked() {
    handleFinBtnAjouterTransactionClicked();
}
void SmartPub::on_finBtnAnnulerAjout_clicked() {
    handleFinBtnAnnulerAjoutClicked();
}
void SmartPub::on_finBtnModifierTransaction_clicked() {
    handleFinBtnModifierTransactionClicked();
}
void SmartPub::on_finBtnSupprimerTransaction_clicked() {
    handleFinBtnSupprimerTransactionClicked();
}
void SmartPub::on_finLineEditRecherche_textChanged(const QString &text) {
    handleFinLineEditRechercheTextChanged(text);
}

void SmartPub::on_evBtnAjouterEvent_clicked() {
    handleEvBtnAjouterEventClicked();
}
void SmartPub::on_evBtnModifierEvent_clicked() {
    handleEvBtnModifierEventClicked();
}
void SmartPub::on_evBtnSupprimerEvent_clicked() {
    handleEvBtnSupprimerEventClicked();
}
void SmartPub::on_evBtnTrierDate_clicked() { handleEvBtnTrierDateClicked(); }
void SmartPub::on_evBtnRechercheLieu_clicked() {
    handleEvBtnRechercheLieuClicked();
}
void SmartPub::on_evBtnExportCalendrier_clicked() {
    handleEvBtnExportCalendrierClicked();
}
void SmartPub::on_evBtnLivreResumes_clicked() {
    handleEvBtnLivreResumesClicked();
}
void SmartPub::on_evBtnCalculImpact_clicked() {
    handleEvBtnCalculImpactClicked();
}
void SmartPub::on_evBtnStatsParticipation_clicked() {
    handleEvBtnStatsParticipationClicked();
}

void SmartPub::on_labBtnAjouter_clicked() { handleLabBtnAjouterClicked(); }
void SmartPub::on_labBtnModifier_clicked() { handleLabBtnModifierClicked(); }
void SmartPub::on_labBtnSupprimer_clicked() { handleLabBtnSupprimerClicked(); }
void SmartPub::on_labBtnConfirmerForm_clicked() {
    handleLabBtnConfirmerFormClicked();
}
void SmartPub::on_labBtnAnnulerForm_clicked() {
    handleLabBtnAnnulerFormClicked();
}
void SmartPub::on_labBtnStatistiques_clicked() {
    handleLabBtnStatistiquesClicked();
}
void SmartPub::on_labBtnOptimiseur_clicked() {
    handleLabBtnOptimiseurClicked();
}
void SmartPub::on_labBtnPredicteur_clicked() {
    handleLabBtnPredicteurClicked();
}
void SmartPub::on_labBtnExporter_clicked() { handleLabBtnExporterClicked(); }
void SmartPub::on_labBtnTrier_clicked() { handleLabBtnTrierClicked(); }
void SmartPub::on_labTableSelectionChanged() {
    handleLabTableSelectionChanged();
}
void SmartPub::on_labSearchChanged(const QString &text) {
    handleLabSearchChanged(text);
}

void SmartPub::on_btnListeProjets_clicked() { handleProjetListeProjets(); }
void SmartPub::on_btnAjouterProjet_clicked() { handleProjetAjouterProjet(); }
void SmartPub::on_btnModifierProjet_clicked() { handleProjetModifierProjet(); }
void SmartPub::on_btnSupprimerProjet_clicked() {
    handleProjetSupprimerProjet();
}
void SmartPub::on_lineEditRechercheProjets_textChanged(const QString &text) {
    handleProjetRechercheChanged(text);
}
void SmartPub::on_btnAnnulerForm_clicked() { handleProjetAnnulerForm(); }
void SmartPub::on_btnEnregistrerForm_clicked() {
    handleProjetEnregistrerForm();
}
void SmartPub::on_tableSelectionChanged() {
    handleProjetTableSelectionChanged();
}
void SmartPub::on_tableDoubleClicked(int row, int column) {
    handleProjetTableDoubleClicked(row, column);
}
void SmartPub::on_triDateDebutClicked() { handleProjetTriDateDebut(); }
void SmartPub::on_triDateFinClicked() { handleProjetTriDateFin(); }
void SmartPub::on_triEtatClicked() { handleProjetTriEtat(); }
void SmartPub::on_triProgressionClicked() { handleProjetTriProgression(); }
void SmartPub::on_statistiquesClicked() { handleProjetStatistiques(); }
void SmartPub::on_santeProjetClicked() { handleProjetSante(); }
void SmartPub::on_optimiserChargeClicked() {
    handleProjetOptimiserCharge();
}
void SmartPub::on_iaRecommanderClicked() { handleProjetIARecommander(); }
void SmartPub::on_filtresClicked() { handleProjetFiltres(); }
void SmartPub::on_exporterClicked() { handleProjetExporter(); }

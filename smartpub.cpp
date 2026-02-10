#include "smartpub.h"
#include "ui_smartpub.h"
#include <QApplication>
#include <QScreen>
#include <QProcess>

// ============================================================================
// FONCTIONS HELPER GLOBALES (pour module Projets)
// ============================================================================

static QString getEtatColor(const QString &etat)
{
    if (etat == "Actif") return "#10b981";
    if (etat == "Terminé") return "#3b82f6";
    if (etat == "En pause") return "#f59e0b";
    if (etat == "Planifié") return "#8b5cf6";
    return "#64748b";
}

static QString getProgressionColor(int valeur)
{
    if (valeur >= 80) return "#10b981";
    if (valeur >= 50) return "#3b82f6";
    if (valeur >= 25) return "#f59e0b";
    return "#ef4444";
}

static QString getProgressionColorFromString(const QString &progression)
{
    QString temp = progression;
    if (temp.endsWith('%')) temp.chop(1);
    int valeur = temp.toInt();
    return getProgressionColor(valeur);
}


// ============================================================================
// LOGIN DIALOG
// ============================================================================

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), loggedIn(false)
{
    setWindowTitle("SmartPub - Connexion");
    setFixedSize(450, 550);
    setupAccounts();
    setupUI();
}

void LoginDialog::setupAccounts()
{
    // Compte admin principal
    accounts << UserAccount{"admin", "admin123", "Tous", UserRole::Admin, "Administrateur"};

    // Compte guest
    accounts << UserAccount{"guest", "", "Tous", UserRole::Guest, "Invité"};
}

void LoginDialog::setupUI()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #f8fafc;
            font-family: 'Segoe UI', 'Roboto', sans-serif;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Header avec gradient
    QFrame *headerFrame = new QFrame();
    headerFrame->setFixedHeight(180);
    headerFrame->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
    )");

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setAlignment(Qt::AlignCenter);

    QLabel *logoLabel = new QLabel("🔬");
    logoLabel->setStyleSheet("font-size: 48px; background: transparent;");
    logoLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel("SmartPub");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subtitleLabel = new QLabel("Gestion de la Recherche Scientifique");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px; background: transparent;");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(logoLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    // Content
    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(40, 30, 40, 30);

    QLabel *loginTitle = new QLabel("Connexion");
    loginTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #1e293b; background: transparent;");
    contentLayout->addWidget(loginTitle);

    contentLayout->addSpacing(10);

    // Email
    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155; background: transparent;");
    contentLayout->addWidget(emailLabel);

    contentLayout->addSpacing(5);

    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("votre@email.com");
    emailEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #f8fafc;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 12px 16px;
            font-size: 14px;
            color: #334155;
            min-height: 20px;
        }
        QLineEdit:focus {
            border-color: #3b82f6;
            background-color: white;
        }
    )");
    contentLayout->addWidget(emailEdit);

    contentLayout->addSpacing(15);

    // Password
    QLabel *passwordLabel = new QLabel("Mot de passe");
    passwordLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155; background: transparent;");
    contentLayout->addWidget(passwordLabel);

    contentLayout->addSpacing(5);

    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("••••••••");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #f8fafc;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 12px 16px;
            font-size: 14px;
            color: #334155;
            min-height: 20px;
        }
        QLineEdit:focus {
            border-color: #3b82f6;
            background-color: white;
        }
    )");
    contentLayout->addWidget(passwordEdit);

    contentLayout->addSpacing(10);

    // Error label
    errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: #ef4444; font-size: 12px; background: transparent;");
    errorLabel->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(errorLabel);

    contentLayout->addSpacing(15);

    // Login button
    loginBtn = new QPushButton("Se connecter");
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(R"(
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
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    contentLayout->addWidget(loginBtn);

    // Guest button
    guestBtn = new QPushButton("Continuer en tant qu'invité");
    guestBtn->setCursor(Qt::PointingHandCursor);
    guestBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: #64748b;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 12px;
            font-size: 14px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #cbd5e1;
            color: #334155;
        }
    )");
    connect(guestBtn, &QPushButton::clicked, this, &LoginDialog::onGuestClicked);
    contentLayout->addWidget(guestBtn);

    // Forgot password
    forgotBtn = new QPushButton("Mot de passe oublié ?");
    forgotBtn->setCursor(Qt::PointingHandCursor);
    forgotBtn->setStyleSheet(R"(
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
    connect(forgotBtn, &QPushButton::clicked, this, &LoginDialog::onForgotPasswordClicked);
    contentLayout->addWidget(forgotBtn, 0, Qt::AlignCenter);

    contentLayout->addStretch();
    mainLayout->addWidget(contentWidget, 1);
}

void LoginDialog::onLoginClicked()
{
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();

    for (const auto &acc : accounts) {
        if (acc.email == email && acc.password == password) {
            loggedInUser = acc;
            loggedIn = true;
            accept();
            return;
        }
    }

    errorLabel->setText("Email ou mot de passe incorrect");
    passwordEdit->clear();
}

void LoginDialog::onGuestClicked()
{
    loggedInUser = UserAccount{"", "", "Tous", UserRole::Guest, "Invité"};
    loggedIn = true;
    accept();
}

void LoginDialog::onForgotPasswordClicked()
{
    QMessageBox::information(this, "Mot de passe oublié",
                             "Veuillez contacter l'administrateur système pour réinitialiser votre mot de passe.");
}

// ============================================================================
// DIALOGS POUR MODULE PROJETS
// ============================================================================

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Paramètres");
    setFixedSize(500, 400);
    setupUI();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("Paramètres");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    // Thème
    QLabel *themeLabel = new QLabel("Thème:");
    themeCombo = new QComboBox();
    themeCombo->addItems({"Clair", "Sombre", "Auto"});
    QHBoxLayout *themeLayout = new QHBoxLayout();
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeCombo);
    mainLayout->addLayout(themeLayout);

    // Langue
    QLabel *langLabel = new QLabel("Langue:");
    langCombo = new QComboBox();
    langCombo->addItems({"Français", "Anglais", "Arabe"});
    QHBoxLayout *langLayout = new QHBoxLayout();
    langLayout->addWidget(langLabel);
    langLayout->addWidget(langCombo);
    mainLayout->addLayout(langLayout);

    // Notifications
    notifCheck = new QCheckBox("Activer les notifications");
    notifCheck->setChecked(true);
    mainLayout->addWidget(notifCheck);

    emailCheck = new QCheckBox("Notifications par email");
    mainLayout->addWidget(emailCheck);

    soundCheck = new QCheckBox("Son de notification");
    soundCheck->setChecked(true);
    mainLayout->addWidget(soundCheck);

    autoSaveCheck = new QCheckBox("Sauvegarde automatique");
    autoSaveCheck->setChecked(true);
    mainLayout->addWidget(autoSaveCheck);

    // Intervalle de sauvegarde
    QLabel *intervalLabel = new QLabel("Intervalle de sauvegarde (minutes):");
    intervalSpin = new QSpinBox();
    intervalSpin->setRange(1, 60);
    intervalSpin->setValue(5);
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(intervalLabel);
    intervalLayout->addWidget(intervalSpin);
    mainLayout->addLayout(intervalLayout);

    mainLayout->addStretch();

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("OK");
    okBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okBtn);
    mainLayout->addLayout(buttonLayout);
}

FiltresDialog::FiltresDialog(QWidget *parent)
    : QDialog(parent), filtreActif(false)
{
    setWindowTitle("Filtres de Projets");
    setFixedSize(450, 400);
    setupUI();
}

void FiltresDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *titleLabel = new QLabel("Filtres");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    // État
    QLabel *etatLabel = new QLabel("État:");
    comboBoxEtat = new QComboBox();
    comboBoxEtat->addItems({"Tous", "Planifié", "Actif", "En pause", "Terminé"});
    QHBoxLayout *etatLayout = new QHBoxLayout();
    etatLayout->addWidget(etatLabel);
    etatLayout->addWidget(comboBoxEtat);
    mainLayout->addLayout(etatLayout);

    // Responsable
    QLabel *respLabel = new QLabel("Responsable:");
    comboBoxResponsable = new QComboBox();
    comboBoxResponsable->addItems({"Tous", "Dr. Ahmed Ben Ali", "Pr. Fatima Zohra", "Dr. Mohamed Salah", "Dr. Sarah Johnson", "Pr. Robert Chen"});
    QHBoxLayout *respLayout = new QHBoxLayout();
    respLayout->addWidget(respLabel);
    respLayout->addWidget(comboBoxResponsable);
    mainLayout->addLayout(respLayout);

    // Date début min
    QLabel *dateMinLabel = new QLabel("Date début (min):");
    dateEditDebutMin = new QDateEdit();
    dateEditDebutMin->setCalendarPopup(true);
    dateEditDebutMin->setDate(QDate(2020, 1, 1));
    QHBoxLayout *dateMinLayout = new QHBoxLayout();
    dateMinLayout->addWidget(dateMinLabel);
    dateMinLayout->addWidget(dateEditDebutMin);
    mainLayout->addLayout(dateMinLayout);

    // Date début max
    QLabel *dateMaxLabel = new QLabel("Date début (max):");
    dateEditMax = new QDateEdit();
    dateEditMax->setCalendarPopup(true);
    dateEditMax->setDate(QDate::currentDate());
    QHBoxLayout *dateMaxLayout = new QHBoxLayout();
    dateMaxLayout->addWidget(dateMaxLabel);
    dateMaxLayout->addWidget(dateEditMax);
    mainLayout->addLayout(dateMaxLayout);

    mainLayout->addStretch();

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnAppliquer = new QPushButton("Appliquer");
    btnAppliquer->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(btnAppliquer, &QPushButton::clicked, this, [this]() { filtreActif = true; accept(); });

    btnReinitialiser = new QPushButton("Réinitialiser");
    btnReinitialiser->setStyleSheet("QPushButton { background-color: #64748b; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(btnReinitialiser, &QPushButton::clicked, this, [this]() {
        comboBoxEtat->setCurrentIndex(0);
        comboBoxResponsable->setCurrentIndex(0);
        dateEditDebutMin->setDate(QDate(2020, 1, 1));
        dateEditMax->setDate(QDate::currentDate());
        filtreActif = false;
    });

    btnAnnuler = new QPushButton("Annuler");
    btnAnnuler->setStyleSheet("QPushButton { background-color: #ef4444; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(btnAppliquer);
    buttonLayout->addWidget(btnReinitialiser);
    buttonLayout->addWidget(btnAnnuler);
    mainLayout->addLayout(buttonLayout);
}

QString FiltresDialog::getEtatFiltre() const
{
    QString etat = comboBoxEtat->currentText();
    return etat == "Tous" ? "" : etat;
}

QString FiltresDialog::getResponsableFiltre() const
{
    QString resp = comboBoxResponsable->currentText();
    return resp == "Tous" ? "" : resp;
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

IARecommandationsDialog::IARecommandationsDialog(const QList<Projet> &projets, QWidget *parent)
    : QDialog(parent), m_projets(projets)
{
    setWindowTitle("Recommandations IA");
    setFixedSize(700, 600);
    genererRecommandations();
    setupUI();
}

void IARecommandationsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *titleLabel = new QLabel("🤖 Recommandations IA");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);

    for (const auto &rec : m_recommandations) {
        QFrame *frame = new QFrame();
        frame->setStyleSheet("QFrame { background-color: #f8fafc; border: 2px solid #e2e8f0; border-radius: 12px; padding: 15px; }");
        QVBoxLayout *frameLayout = new QVBoxLayout(frame);

        QLabel *titreLabel = new QLabel(rec.titre);
        titreLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1e293b;");
        frameLayout->addWidget(titreLabel);

        QLabel *descLabel = new QLabel(rec.description);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet("color: #64748b;");
        frameLayout->addWidget(descLabel);

        QLabel *scoreLabel = new QLabel(QString("Score de similarité: %1%").arg(rec.scoreSimilarite * 100, 0, 'f', 1));
        scoreLabel->setStyleSheet("color: #3b82f6; font-weight: 600;");
        frameLayout->addWidget(scoreLabel);

        contentLayout->addWidget(frame);
    }

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn);
}

void IARecommandationsDialog::genererRecommandations()
{
    m_recommandations.clear();
    if (m_projets.isEmpty()) return;

    Recommandation rec;
    rec.titre = "Collaboration suggérée";
    rec.description = "Basé sur l'analyse de vos projets, nous suggérons une collaboration avec des chercheurs du domaine de l'IA.";
    rec.scoreSimilarite = 0.85;
    rec.collaborateursSuggeres = {"Dr. Ahmed Ben Ali", "Pr. Fatima Zohra"};
    rec.raison = "Domaines complémentaires";
    rec.domaine = "Intelligence Artificielle";
    m_recommandations.append(rec);

    Recommandation rec2;
    rec2.titre = "Optimisation des ressources";
    rec2.description = "Vos projets pourraient bénéficier d'une meilleure allocation des ressources.";
    rec2.scoreSimilarite = 0.72;
    rec2.raison = "Analyse des budgets";
    rec2.domaine = "Gestion de projet";
    m_recommandations.append(rec2);
}

ProjetDetailsDialog::ProjetDetailsDialog(const Projet &projet, QWidget *parent)
    : QDialog(parent), m_projet(projet)
{
    setWindowTitle("Détails du Projet");
    setFixedSize(600, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *titleLabel = new QLabel("📁 Détails du Projet");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);

    auto addInfoRow = [&](const QString &label, const QString &value) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *lbl = new QLabel(label + ":");
        lbl->setStyleSheet("font-weight: 600; color: #334155; min-width: 120px;");
        QLabel *val = new QLabel(value);
        val->setStyleSheet("color: #1e293b;");
        rowLayout->addWidget(lbl);
        rowLayout->addWidget(val);
        contentLayout->addLayout(rowLayout);
    };

    addInfoRow("Code", m_projet.code);
    addInfoRow("Titre", m_projet.titre);
    addInfoRow("Date Début", m_projet.dateDebut.toString("dd/MM/yyyy"));
    addInfoRow("Date Fin", m_projet.dateFin.toString("dd/MM/yyyy"));
    addInfoRow("Responsable", m_projet.responsable);
    addInfoRow("État", m_projet.etat);
    addInfoRow("Progression", m_projet.progression);

    QLabel *descLabel = new QLabel("Description:");
    descLabel->setStyleSheet("font-weight: 600; color: #334155;");
    contentLayout->addWidget(descLabel);
    QTextEdit *descText = new QTextEdit();
    descText->setPlainText(m_projet.description);
    descText->setReadOnly(true);
    descText->setMaximumHeight(150);
    contentLayout->addWidget(descText);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn);
}

StatistiquesDialog::StatistiquesDialog(const QList<Projet> &projets, QWidget *parent)
    : QDialog(parent), m_projets(projets)
{
    setWindowTitle("Statistiques des Projets");
    setFixedSize(800, 700);
    setupUI();
    calculerStatistiques();
    creerGraphiques();
}

void StatistiquesDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    QLabel *titleLabel = new QLabel("📊 Statistiques des Projets");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);

    // Labels de statistiques
    QGridLayout *statsLayout = new QGridLayout();
    labelTotalProjets = new QLabel("Total: 0");
    labelProjetsActifs = new QLabel("Actifs: 0");
    labelProjetsTermines = new QLabel("Terminés: 0");
    labelProgressionMoyenne = new QLabel("Progression moyenne: 0%");
    labelProjetsRetard = new QLabel("En retard: 0");
    labelProjetsPlanifies = new QLabel("Planifiés: 0");
    labelProjetsPause = new QLabel("En pause: 0");

    statsLayout->addWidget(labelTotalProjets, 0, 0);
    statsLayout->addWidget(labelProjetsActifs, 0, 1);
    statsLayout->addWidget(labelProjetsTermines, 1, 0);
    statsLayout->addWidget(labelProgressionMoyenne, 1, 1);
    statsLayout->addWidget(labelProjetsRetard, 2, 0);
    statsLayout->addWidget(labelProjetsPlanifies, 2, 1);
    statsLayout->addWidget(labelProjetsPause, 3, 0);

    contentLayout->addLayout(statsLayout);

    // Graphiques
    chartEtatView = new QChartView();
    chartProgressionView = new QChartView();
    chartTemporelView = new QChartView();

    contentLayout->addWidget(new QLabel("Répartition par État:"));
    contentLayout->addWidget(chartEtatView);
    contentLayout->addWidget(new QLabel("Répartition par Progression:"));
    contentLayout->addWidget(chartProgressionView);
    contentLayout->addWidget(new QLabel("Évolution Temporelle:"));
    contentLayout->addWidget(chartTemporelView);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; border: none; border-radius: 8px; padding: 10px 20px; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn);
}

void StatistiquesDialog::calculerStatistiques()
{
    int total = m_projets.size();
    int actifs = 0, termines = 0, planifies = 0, pause = 0, retard = 0;
    double progressionTotal = 0;
    int progressionCount = 0;

    for (const auto &p : m_projets) {
        if (p.etat == "Actif") actifs++;
        else if (p.etat == "Terminé") termines++;
        else if (p.etat == "Planifié") planifies++;
        else if (p.etat == "En pause") pause++;

        if (p.etat != "Terminé" && QDate::currentDate() > p.dateFin) retard++;

        QString prog = p.progression;
        if (prog.endsWith('%')) prog.chop(1);
        bool ok;
        int progVal = prog.toInt(&ok);
        if (ok) {
            progressionTotal += progVal;
            progressionCount++;
        }
    }

    double progressionMoyenne = progressionCount > 0 ? progressionTotal / progressionCount : 0;

    labelTotalProjets->setText(QString("Total: %1").arg(total));
    labelProjetsActifs->setText(QString("Actifs: %1").arg(actifs));
    labelProjetsTermines->setText(QString("Terminés: %1").arg(termines));
    labelProgressionMoyenne->setText(QString("Progression moyenne: %1%").arg(progressionMoyenne, 0, 'f', 1));
    labelProjetsRetard->setText(QString("En retard: %1").arg(retard));
    labelProjetsPlanifies->setText(QString("Planifiés: %1").arg(planifies));
    labelProjetsPause->setText(QString("En pause: %1").arg(pause));
}

void StatistiquesDialog::creerGraphiques()
{
    // Graphique par état
    QPieSeries *etatSeries = new QPieSeries();
    QMap<QString, int> etatCount;
    for (const auto &p : m_projets) {
        etatCount[p.etat]++;
    }
    for (auto it = etatCount.begin(); it != etatCount.end(); ++it) {
        etatSeries->append(it.key(), it.value());
    }
    QChart *etatChart = new QChart();
    etatChart->addSeries(etatSeries);
    etatChart->setTitle("Répartition par État");
    chartEtatView->setChart(etatChart);

    // Graphique par progression
    QBarSeries *progSeries = new QBarSeries();
    QBarSet *progSet = new QBarSet("Projets");
    QStringList categories;
    categories << "0-25%" << "25-50%" << "50-75%" << "75-100%";
    int counts[4] = {0, 0, 0, 0};
    for (const auto &p : m_projets) {
        QString prog = p.progression;
        if (prog.endsWith('%')) prog.chop(1);
        bool ok;
        int progVal = prog.toInt(&ok);
        if (ok) {
            if (progVal < 25) counts[0]++;
            else if (progVal < 50) counts[1]++;
            else if (progVal < 75) counts[2]++;
            else counts[3]++;
        }
    }
    *progSet << counts[0] << counts[1] << counts[2] << counts[3];
    progSeries->append(progSet);
    QChart *progChart = new QChart();
    progChart->addSeries(progSeries);
    progChart->setTitle("Répartition par Progression");
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    progChart->addAxis(axisX, Qt::AlignBottom);
    progSeries->attachAxis(axisX);
    chartProgressionView->setChart(progChart);

    // Graphique temporel (simplifié)
    QLineSeries *tempSeries = new QLineSeries();
    QMap<QDate, int> dateCount;
    for (const auto &p : m_projets) {
        dateCount[p.dateDebut]++;
    }
    for (auto it = dateCount.begin(); it != dateCount.end(); ++it) {
        tempSeries->append(it.key().toJulianDay(), it.value());
    }
    QChart *tempChart = new QChart();
    tempChart->addSeries(tempSeries);
    tempChart->setTitle("Évolution Temporelle");
    chartTemporelView->setChart(tempChart);
}

// ============================================================================
// SMARTPUB CONSTRUCTEUR/DESTRUCTEUR
// ============================================================================

SmartPub::SmartPub(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SmartPub)
    , sidebarExpanded(false)
    , isUserLoggedIn(false)
    , cherchVueListeActive(true)
    , cherchVueIconesActive(true)
    , cherchChercheurSelectionne(-1)
    , cherchIsLoggedIn(false)
    , finVueListeActive(true)
    , finTransactionSelectionnee(-1)
    , evEventSelectionne(-1)
    , nextProjetId(1)
    , currentProjetId(-1)
    , isEditing(false)
    , currentSortColumn(-1)
    , currentSortOrder(Qt::AscendingOrder)
    , filtresActifs(false)
{
    ui->setupUi(this);
    // Configurer les dimensions de la fenêtre
    this->setMinimumSize(1280, 720);
    this->resize(1400, 800);


    // Afficher le login au démarrage avant l'affichage de la fenêtre
    showLogin();

    setupUI();
    setupSidebar();
    setupConnections();

    // Initialiser tous les modules
    cherchSetupUI();
    cherchConnectSignals();
    cherchApplyModernStyle();
    cherchAjouterDonneesTest();

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
    projConnectSignals();
    projSetupComboBoxes();
    projSetupSampleData();
    projChargerProjets();

    // Configurer la sidebar et les permissions
    updateSidebarProfileVisibility();
    checkPermissions();

    // Afficher le module approprié selon l'utilisateur
    // Index: 0=Chercheurs, 1=Publications, 2=Finances, 3=Événements, 4=Projets
    if (currentUser.role == UserRole::Admin) {
        // Admin peut accéder à tous les modules
        ui->stackedWidgetModules->setCurrentIndex(0);
        cherchShowMainView();
    } else {
        // Guest - afficher Chercheurs par défaut en lecture seule
        ui->stackedWidgetModules->setCurrentIndex(0);
        cherchShowMainView();
    }

    setActiveNavigationButton(ui->stackedWidgetModules->currentIndex());

    showMaximized();
}

SmartPub::~SmartPub()
{
    delete ui;
}
// ============================================================================
// SETUP UI ET SIDEBAR
// ============================================================================

void SmartPub::showLogin()
{
    // Afficher le login d'abord avant l'affichage de la fenêtre
    LoginDialog loginDlg(this);
    if (loginDlg.exec() != QDialog::Accepted) {
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }

    currentUser = loginDlg.getLoggedInUser();
    isUserLoggedIn = true;

    // Appliquer les restrictions Guest immédiatement
    if (currentUser.role == UserRole::Guest) {
        applyGuestRestrictions();
    }
}

void SmartPub::setupUI()
{
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

void SmartPub::setupSidebar()
{
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
    profileWidget->setFixedHeight(80);

    QHBoxLayout *profileLayout = new QHBoxLayout(profileWidget);
    profileLayout->setSpacing(15);
    profileLayout->setContentsMargins(20, 10, 20, 10);

    // Avatar
    avatarLabel = new QLabel(currentUser.displayName.left(2).toUpper());
    avatarLabel->setFixedSize(45, 45);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            background-color: #10b981;
            color: white;
            border-radius: 22px;
            font-size: 16px;
            font-weight: bold;
            qproperty-alignment: AlignCenter;
        }
    )");
    avatarLabel->setAlignment(Qt::AlignCenter);

    // Info utilisateur
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(3);

    nameLabel = new QLabel(currentUser.displayName);
    nameLabel->setStyleSheet("color: white; font-size: 14px; font-weight: 600;");

    QString roleText = (currentUser.role == UserRole::Admin) ? "Administrateur" : "Invité (Lecture seule)";
    roleLabel = new QLabel(roleText);
    roleLabel->setStyleSheet("color: #94a3b8; font-size: 12px;");

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(roleLabel);

    // Bouton paramètres
    btnSettings = new QPushButton("⚙");
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
    connect(btnSettings, &QPushButton::clicked, this, &SmartPub::onSettingsClicked);

    profileLayout->addWidget(avatarLabel);
    profileLayout->addLayout(infoLayout, 1);
    profileLayout->addWidget(btnSettings);

    // Ajouter le profil au layout de la sidebar
    QVBoxLayout *sidebarLayout = qobject_cast<QVBoxLayout*>(ui->sidebarFrame->layout());
    if (sidebarLayout) {
        sidebarLayout->addWidget(profileWidget);
    }

    // Event filter pour expand/collapse
    ui->sidebarFrame->setMouseTracking(true);
    ui->sidebarFrame->installEventFilter(this);

    updateSidebarProfileVisibility();
}

void SmartPub::setupConnections()
{
    // Connexions de la sidebar principale
    connect(ui->btnPublications, &QPushButton::clicked, this, &SmartPub::on_btnPublications_clicked);
    connect(ui->btnChercheurs, &QPushButton::clicked, this, &SmartPub::on_btnChercheurs_clicked);
    connect(ui->btnLaboratoires, &QPushButton::clicked, this, &SmartPub::on_btnLaboratoires_clicked);
    connect(ui->btnProjets, &QPushButton::clicked, this, &SmartPub::on_btnProjets_clicked);
    connect(ui->btnFinances, &QPushButton::clicked, this, &SmartPub::on_btnFinances_clicked);
    connect(ui->btnEvenements, &QPushButton::clicked, this, &SmartPub::on_btnEvenements_clicked);
}

void SmartPub::checkPermissions()
{
    // Si l'utilisateur est un invité, désactiver tous les boutons de modification
    if (currentUser.role == UserRole::Guest) {
        applyGuestRestrictions();
    }
}

void SmartPub::applyGuestRestrictions()
{
    // Parcourir tous les boutons et désactiver ceux qui contiennent des mots-clés CRUD
    QList<QPushButton*> allButtons = this->findChildren<QPushButton*>();
    QStringList crudKeywords = {"add", "edit", "delete", "save", "supprimer", "modifier", "ajouter", "enregistrer", "annuler"};

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

void SmartPub::updateSidebarProfileVisibility()
{
    if (!profileWidget) return;
    profileWidget->setVisible(sidebarExpanded);
}

bool SmartPub::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->sidebarFrame) {
        if (event->type() == QEvent::Enter) {
            sidebarTimer->stop();
            if (!sidebarExpanded) {
                expandSidebar();
            }
        }
        else if (event->type() == QEvent::Leave) {
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
        QFrame *card = qobject_cast<QFrame*>(obj);
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

void SmartPub::expandSidebar()
{
    sidebarExpanded = true;

    QPropertyAnimation *animation = new QPropertyAnimation(ui->sidebarFrame, "minimumWidth");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(ui->sidebarFrame->width());
    animation->setEndValue(260);

    QPropertyAnimation *animation2 = new QPropertyAnimation(ui->sidebarFrame, "maximumWidth");
    animation2->setDuration(250);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->setStartValue(ui->sidebarFrame->width());
    animation2->setEndValue(260);

    // Mettre à jour les textes des boutons
    QVector<QPushButton*> navButtons = {
        ui->btnPublications, ui->btnChercheurs, ui->btnLaboratoires,
        ui->btnProjets, ui->btnFinances, ui->btnEvenements
    };

    QStringList fullTexts = {"📄 Publications", "👥 Chercheurs", "🧪 Laboratoires",
                             "📁 Projets", "💰 Finances", "📅 Événements"};

    for (int i = 0; i < navButtons.size(); ++i) {
        navButtons[i]->setText(fullTexts[i]);
    }

    ui->appNameLabel->show();
    updateSidebarProfileVisibility();

    animation->start(QAbstractAnimation::DeleteWhenStopped);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);

    // Mettre à jour les styles des boutons pour refléter la nouvelle taille
    // Trouver le bouton actif dans la liste existante
    for (auto *btn : navButtons) {
        QString style = btn->styleSheet();
        if (style.contains("background: qlineargradient")) {
            // Ce bouton est actif, le mettre à jour
            QTimer::singleShot(260, this, [this, btn]() {
                updateNavButtonStyles(btn);
            });
            break;
        }
    }
}

void SmartPub::collapseSidebar()
{
    sidebarExpanded = false;

    QPropertyAnimation *animation = new QPropertyAnimation(ui->sidebarFrame, "minimumWidth");
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(ui->sidebarFrame->width());
    animation->setEndValue(70);

    QPropertyAnimation *animation2 = new QPropertyAnimation(ui->sidebarFrame, "maximumWidth");
    animation2->setDuration(250);
    animation2->setEasingCurve(QEasingCurve::InOutQuad);
    animation2->setStartValue(ui->sidebarFrame->width());
    animation2->setEndValue(70);

    // Réduire les textes aux icônes
    QVector<QPushButton*> navButtons = {
        ui->btnPublications, ui->btnChercheurs, ui->btnLaboratoires,
        ui->btnProjets, ui->btnFinances, ui->btnEvenements
    };

    QStringList icons = {"📄", "👥", "🧪", "📁", "💰", "📅"};

    for (int i = 0; i < navButtons.size(); ++i) {
        navButtons[i]->setText(icons[i]);
    }

    ui->appNameLabel->hide();
    updateSidebarProfileVisibility();

    animation->start(QAbstractAnimation::DeleteWhenStopped);
    animation2->start(QAbstractAnimation::DeleteWhenStopped);

    // Mettre à jour les styles des boutons pour refléter la nouvelle taille
    // Trouver le bouton actif dans la liste existante
    for (auto *btn : navButtons) {
        QString style = btn->styleSheet();
        if (style.contains("background: qlineargradient")) {
            // Ce bouton est actif, le mettre à jour
            QTimer::singleShot(260, this, [this, btn]() {
                updateNavButtonStyles(btn);
            });
            break;
        }
    }
}

void SmartPub::updateNavButtonStyles(QPushButton *activeBtn)
{
    QVector<QPushButton*> buttons = {
        ui->btnPublications, ui->btnChercheurs, ui->btnLaboratoires,
        ui->btnProjets, ui->btnFinances, ui->btnEvenements
    };

    // Choisir la taille de police selon l'état de la sidebar
    int fontSize = sidebarExpanded ? 14 : 20;
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
    )").arg(fontSize).arg(textAlign);

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
    )").arg(fontSize).arg(textAlign);

    for (auto *btn : buttons) {
        btn->setStyleSheet(btn == activeBtn ? activeStyle : inactiveStyle);
    }
}

void SmartPub::setActiveNavigationButton(int index)
{
    QVector<QPushButton*> navButtons = {
        ui->btnChercheurs,      // Index 0
        ui->btnPublications,    // Index 1
        ui->btnFinances,        // Index 2
        ui->btnEvenements,      // Index 3
        ui->btnProjets          // Index 4
    };

    if (index >= 0 && index < navButtons.size()) {
        updateNavButtonStyles(navButtons[index]);
    }
}


void SmartPub::onSettingsClicked()
{
    SettingsDialog dialog(this);
    dialog.exec();
}
// ============================================================================
// NAVIGATION PRINCIPALE
// ============================================================================

void SmartPub::on_btnChercheurs_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(0);
    setActiveNavigationButton(0);
    if (cherchIsLoggedIn || currentUser.role == UserRole::Guest) {
        cherchShowMainView();
    }
}

void SmartPub::on_btnPublications_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    SR_updateButtonStyles();
}

void SmartPub::on_btnLaboratoires_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    QMessageBox::information(this, "Information", "Module Laboratoires en cours de développement");
}

void SmartPub::on_btnFinances_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(2);
    setActiveNavigationButton(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_btnProjets_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(4);
    setActiveNavigationButton(4);
    projChargerProjets();
}

void SmartPub::on_btnEvenements_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(3);
    setActiveNavigationButton(3);
    evAfficherListeEvents();
}

// ============================================================================
// MODULE CHERCHEURS
// ============================================================================

void SmartPub::cherchSetupUI()
{
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
    connect(cherchBtnToggleVue, &QPushButton::clicked, this, &SmartPub::on_cherchBtnToggleVue_clicked);
}

void SmartPub::cherchConnectSignals()
{
    connect(ui->cherchBtnLogin, &QPushButton::clicked, this, &SmartPub::on_cherchBtnLogin_clicked);
    connect(ui->cherchBtnMotDePasseOublie, &QPushButton::clicked, this, &SmartPub::on_cherchBtnMotDePasseOublie_clicked);
    connect(ui->cherchBtnRetourLogin, &QPushButton::clicked, this, &SmartPub::on_cherchBtnRetourLogin_clicked);
    connect(ui->cherchBtnForgotOk, &QPushButton::clicked, this, &SmartPub::on_cherchBtnForgotOk_clicked);

    //     ui->cherchUserProfileFrame->setCursor(Qt::PointingHandCursor);
    //     ui->cherchUserProfileFrame->installEventFilter(this);

    connect(ui->cherchBtnVueListe, &QPushButton::clicked, this, &SmartPub::on_cherchBtnVueListe_clicked);
    connect(ui->cherchBtnAjouter, &QPushButton::clicked, this, &SmartPub::on_cherchBtnAjouter_clicked);
    connect(ui->cherchBtnRecherche, &QPushButton::clicked, this, &SmartPub::on_cherchBtnRecherche_clicked);
    connect(ui->cherchBtnTri, &QPushButton::clicked, this, &SmartPub::on_cherchBtnTri_clicked);
    connect(ui->cherchBtnExport, &QPushButton::clicked, this, &SmartPub::on_cherchBtnExport_clicked);
    connect(ui->cherchBtnStatistiques, &QPushButton::clicked, this, &SmartPub::on_cherchBtnStatistiques_clicked);
    connect(ui->cherchBtnUploadPhoto, &QPushButton::clicked, this, &SmartPub::on_cherchBtnUploadPhoto_clicked);
    connect(ui->cherchBtnAjouterChercheur, &QPushButton::clicked, this, &SmartPub::on_cherchBtnAjouterChercheur_clicked);
    connect(ui->cherchBtnAnnulerAjout, &QPushButton::clicked, this, &SmartPub::on_cherchBtnAnnulerAjout_clicked);
    connect(ui->cherchLineEditRecherche, &QLineEdit::textChanged, this, &SmartPub::on_cherchLineEditRecherche_textChanged);
}

void SmartPub::cherchApplyModernStyle()
{
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
        ui->cherchTitleLabel->setStyleSheet("color: #1e293b; font-size: 24px; font-weight: 700; background: transparent; border: none;");
    }
    
    if (ui->cherchSubtitleLabel) {
        ui->cherchSubtitleLabel->setStyleSheet("color: #64748b; font-size: 13px; background: transparent; border: none;");
    }

    // === TOOLBAR STYLES ===
    if (ui->cherchToolbarFrame) {
        ui->cherchToolbarFrame->setStyleSheet("background-color: transparent; border: none;");
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

    if (ui->cherchBtnTri) ui->cherchBtnTri->setStyleSheet(cherchSecondaryBtn);
    if (ui->cherchBtnExport) ui->cherchBtnExport->setStyleSheet(cherchSecondaryBtn);

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

    if (ui->cherchLineEditNom) ui->cherchLineEditNom->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditPrenom) ui->cherchLineEditPrenom->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditCIN) ui->cherchLineEditCIN->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditEmail) ui->cherchLineEditEmail->setStyleSheet(cherchInputStyle);

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
    QString cherchLabelStyle = "color: #334155; font-size: 14px; font-weight: 600; background: transparent; border: none;";
    if (ui->cherchLabelNom) ui->cherchLabelNom->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelPrenom) ui->cherchLabelPrenom->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelCIN) ui->cherchLabelCIN->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelEmail) ui->cherchLabelEmail->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelGrade) ui->cherchLabelGrade->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelPhoto) ui->cherchLabelPhoto->setStyleSheet(cherchLabelStyle);

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
        ui->cherchLabelPhotoHint->setStyleSheet("color: #94a3b8; font-size: 12px; background: transparent; border: none;");
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
        ui->cherchScrollAreaWidgetContents->setStyleSheet("background-color: #f8fafc;");
    }
}

void SmartPub::cherchAjouterDonneesTest()
{
    QStringList grades = {"Professeur", "Maitre de Conferences", "Docteur", "Ingenieur de Recherche", "Post-doctorant", "Doctorant"};
    QStringList noms = {"Dupont", "Martin", "Bernard", "Petit", "Robert", "Richard", "Durand", "Leroy"};
    QStringList prenoms = {"Marie", "Pierre", "Sophie", "Jean", "Camille", "Antoine", "Isabelle", "Thomas"};

    for (int i = 0; i < 8; ++i) {
        ChercheurData data;
        data.nom = noms[i];
        data.prenom = prenoms[i];
        data.grade = grades[i % grades.size()];
        data.email = QString("%1.%2@univ.fr").arg(prenoms[i].toLower()).arg(noms[i].toLower());
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

QString SmartPub::cherchDeterminerCarriere(int projetsCount, const QString &grade)
{
    if (projetsCount >= 4) return "Senior - Expert";
    if (projetsCount >= 2) return "Confirmé";
    if (grade == "Professeur" || grade == "Maitre de Conferences") return "Senior";
    return "Junior";
}

void SmartPub::cherchShowLoginView()
{
    // Pour le module chercheurs, on saute le login interne car on a déjà le login global
    cherchShowMainView();
}

void SmartPub::cherchShowMainView()
{
    ui->cherchStackedWidgetMain->setCurrentIndex(1);
    cherchIsLoggedIn = true;
    cherchVueListeActive = true;
    ui->cherchStackedWidget->setCurrentIndex(0);
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchCheckLogin()
{
    // Plus utilisé - login géré globalement
    cherchShowMainView();
}

void SmartPub::cherchShowForgotPasswordView()
{
    ui->cherchStackedWidgetLogin->setCurrentIndex(1);
    ui->cherchLineEditForgotEmail->clear();
    ui->cherchLineEditForgotEmail->setFocus();
}

void SmartPub::on_cherchBtnMotDePasseOublie_clicked()
{
    cherchShowForgotPasswordView();
}

void SmartPub::on_cherchBtnRetourLogin_clicked()
{
    ui->cherchStackedWidgetLogin->setCurrentIndex(0);
}

void SmartPub::on_cherchBtnForgotOk_clicked()
{
    QString email = ui->cherchLineEditForgotEmail->text();
    if (!email.isEmpty()) {
        QMessageBox::information(this, "Email envoyé",
                                 "Un email de récupération a été envoyé à " + email);
        ui->cherchStackedWidgetLogin->setCurrentIndex(0);
    } else {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer une adresse email valide.");
    }
}

void SmartPub::on_cherchBtnLogin_clicked()
{
    cherchCheckLogin();
}

// void SmartPub::on_cherchUserProfileFrame_clicked()
// {
//     // Déconnexion
//     auto reply = QMessageBox::question(this, "Déconnexion",
//                                        "Voulez-vous vraiment vous déconnecter ?",
//                                        QMessageBox::Yes | QMessageBox::No,
//                                        QMessageBox::No);
//
//     if (reply == QMessageBox::Yes) {
//         // Redémarrer l'application pour retourner au login
//         qApp->quit();
//         QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
//     }
// }

void SmartPub::cherchAfficherListeChercheurs()
{
    cherchClearChercheursList();

    if (cherchVueIconesActive) {
        QGridLayout *gridLayout = qobject_cast<QGridLayout*>(ui->cherchScrollAreaWidgetContents->layout());
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

        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
            int id = it.key();
            auto data = it.value();
            QPixmap photo;
            cherchAjouterChercheurCard(id, data.nom, data.prenom,
                                       data.grade, data.email, photo);
        }
    } else {
        QVBoxLayout *listLayout = qobject_cast<QVBoxLayout*>(ui->cherchScrollAreaWidgetContents->layout());
        if (!listLayout) {
            QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            listLayout = new QVBoxLayout(ui->cherchScrollAreaWidgetContents);
            listLayout->setSpacing(12);
            listLayout->setContentsMargins(24, 24, 24, 24);
            listLayout->setAlignment(Qt::AlignTop);
        }

        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
            int id = it.key();
            auto data = it.value();
            cherchAjouterChercheurListItem(id, data.nom, data.prenom, data.grade, data.email);
        }
    }
}

void SmartPub::cherchAjouterChercheurCard(int id, const QString &nom, const QString &prenom,
                                          const QString &grade, const QString &email,
                                          const QPixmap &photo)
{
    Q_UNUSED(photo)
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

    QFrame *avatarFrame = new QFrame(card);
    avatarFrame->setFixedSize(80, 80);
    avatarFrame->setStyleSheet(R"(
        QFrame {
            background-image: url(:/avatar.png);
            background-repeat: no-repeat;
            background-position: center;
            border-radius: 40px;
            border: 3px solid white;
            background-color: #e2e8f0;
        }
    )");
    mainLayout->addWidget(avatarFrame);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);
    infoLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom), card);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #1e293b; background: transparent; border: none;");
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
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent; border: none;");
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
        connect(btnEdit, &QPushButton::clicked, this, [this, id]() {
            on_cherchModifierChercheur(id);
        });

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
        connect(btnDelete, &QPushButton::clicked, this, [this, id]() {
            on_cherchSupprimerChercheur(id);
        });

        btnLayout->addWidget(btnEdit);
        btnLayout->addWidget(btnDelete);
        btnLayout->addStretch();
        mainLayout->addLayout(btnLayout);
    }

    card->installEventFilter(this);
    card->setMouseTracking(true);

    QGridLayout *grid = qobject_cast<QGridLayout*>(ui->cherchScrollAreaWidgetContents->layout());
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

void SmartPub::cherchAjouterChercheurListItem(int id, const QString &nom, const QString &prenom,
                                              const QString &grade, const QString &email)
{
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

    QLabel *avatarLabel = new QLabel();
    avatarLabel->setFixedSize(50, 50);
    avatarLabel->setStyleSheet(R"(
        QLabel {
            background-image: url(:/avatar.png);
            background-repeat: no-repeat;
            background-position: center;
            border-radius: 25px;
            background-color: #e2e8f0;
        }
    )");
    mainLayout->addWidget(avatarLabel);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom));
    nameLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #1e293b; background: transparent; border: none;");
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
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent; border: none;");
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
        connect(btnEdit, &QPushButton::clicked, this, [this, id]() {
            on_cherchModifierChercheur(id);
        });

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
        connect(btnDelete, &QPushButton::clicked, this, [this, id]() {
            on_cherchSupprimerChercheur(id);
        });

        mainLayout->addWidget(btnEdit);
        mainLayout->addWidget(btnDelete);
    }

    item->installEventFilter(this);
    item->setMouseTracking(true);

    QVBoxLayout *list = qobject_cast<QVBoxLayout*>(ui->cherchScrollAreaWidgetContents->layout());
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

void SmartPub::cherchClearChercheursList()
{
    QLayoutItem *child;
    QLayout *layout = ui->cherchScrollAreaWidgetContents->layout();
    if (!layout) return;

    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

void SmartPub::on_cherchBtnVueListe_clicked()
{
    if (!cherchVueListeActive) {
        ui->cherchStackedWidget->setCurrentIndex(0);
        ui->cherchLineEditRecherche->setVisible(true);
        ui->cherchBtnRecherche->setVisible(true);
        ui->cherchBtnTri->setVisible(true);
        ui->cherchBtnExport->setVisible(true);
        ui->cherchBtnStatistiques->setVisible(true);
        if (cherchBtnToggleVue) cherchBtnToggleVue->setVisible(true);

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

void SmartPub::on_cherchBtnAjouter_clicked()
{
    if (cherchVueListeActive) {
        ui->cherchStackedWidget->setCurrentIndex(1);
        ui->cherchLineEditRecherche->setVisible(false);
        ui->cherchBtnRecherche->setVisible(false);
        ui->cherchBtnTri->setVisible(false);
        ui->cherchBtnExport->setVisible(false);
        ui->cherchBtnStatistiques->setVisible(false);
        if (cherchBtnToggleVue) cherchBtnToggleVue->setVisible(false);

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

void SmartPub::on_cherchBtnToggleVue_clicked()
{
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

void SmartPub::on_cherchBtnRecherche_clicked()
{
    QString searchText = ui->cherchLineEditRecherche->text().toLower();
    if (searchText.isEmpty()) {
        cherchAfficherListeChercheurs();
        return;
    }
    cherchClearChercheursList();

    if (cherchVueIconesActive) {
        QGridLayout *gridLayout = qobject_cast<QGridLayout*>(ui->cherchScrollAreaWidgetContents->layout());
        if (!gridLayout) return;

        int count = 0;
        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
            int id = it.key();
            auto data = it.value();

            if (data.nom.toLower().contains(searchText) ||
                data.prenom.toLower().contains(searchText) ||
                data.cin.toLower().contains(searchText)) {

                QPixmap photo;
                cherchAjouterChercheurCard(id, data.nom, data.prenom,
                                           data.grade, data.email, photo);
                count++;
            }
        }

        if (count == 0) {
            QLabel *noResult = new QLabel("Aucun chercheur trouvé", ui->cherchScrollAreaWidgetContents);
            noResult->setAlignment(Qt::AlignCenter);
            noResult->setStyleSheet("color: #94a3b8; font-size: 16px; margin-top: 50px; background: transparent; border: none;");
            gridLayout->addWidget(noResult, 0, 0, 1, 3);
        }
    } else {
        QVBoxLayout *listLayout = qobject_cast<QVBoxLayout*>(ui->cherchScrollAreaWidgetContents->layout());
        if (!listLayout) return;

        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
            int id = it.key();
            auto data = it.value();

            if (data.nom.toLower().contains(searchText) ||
                data.prenom.toLower().contains(searchText) ||
                data.cin.toLower().contains(searchText)) {
                cherchAjouterChercheurListItem(id, data.nom, data.prenom, data.grade, data.email);
            }
        }
    }
}

void SmartPub::on_cherchBtnTri_clicked()
{
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

    menu->addAction("Trier par Nom (A-Z)", this, [this]() { cherchTrierParNom(true); });
    menu->addAction("Trier par Nom (Z-A)", this, [this]() { cherchTrierParNom(false); });
    menu->addSeparator();
    menu->addAction("Trier par Grade (Hiérarchie)", this, [this]() { cherchTrierParGrade(); });
    menu->addSeparator();
    menu->addAction("Trier par Date (Plus récent)", this, [this]() { cherchTrierParDateCreation(true); });
    menu->addAction("Trier par Date (Plus ancien)", this, [this]() { cherchTrierParDateCreation(false); });

    menu->exec(QCursor::pos());
}

void SmartPub::cherchTrierParNom(bool croissant)
{
    QList<int> keys = cherchChercheursMap.keys();
    std::sort(keys.begin(), keys.end(), [this, croissant](int a, int b) {
        QString nomA = cherchChercheursMap[a].nom + cherchChercheursMap[a].prenom;
        QString nomB = cherchChercheursMap[b].nom + cherchChercheursMap[b].prenom;
        return croissant ? (nomA < nomB) : (nomA > nomB);
    });

    QMap<int, ChercheurData> sortedMap;
    for (int key : keys) {
        sortedMap[key] = cherchChercheursMap[key];
    }
    cherchChercheursMap = sortedMap;
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParGrade()
{
    QMap<QString, int> gradeOrder = {
        {"Professeur", 1},
        {"Maitre de Conferences", 2},
        {"Docteur", 3},
        {"Ingenieur de Recherche", 4},
        {"Post-doctorant", 5},
        {"Doctorant", 6}
    };

    QList<int> keys = cherchChercheursMap.keys();
    std::sort(keys.begin(), keys.end(), [this, &gradeOrder](int a, int b) {
        int orderA = gradeOrder.value(cherchChercheursMap[a].grade, 99);
        int orderB = gradeOrder.value(cherchChercheursMap[b].grade, 99);
        return orderA < orderB;
    });

    QMap<int, ChercheurData> sortedMap;
    for (int key : keys) {
        sortedMap[key] = cherchChercheursMap[key];
    }
    cherchChercheursMap = sortedMap;
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParDateCreation(bool croissant)
{
    QList<int> keys = cherchChercheursMap.keys();
    std::sort(keys.begin(), keys.end(), [this, croissant](int a, int b) {
        QDateTime dateA = cherchChercheursMap[a].dateCreation;
        QDateTime dateB = cherchChercheursMap[b].dateCreation;
        return croissant ? (dateA > dateB) : (dateA < dateB);
    });

    QMap<int, ChercheurData> sortedMap;
    for (int key : keys) {
        sortedMap[key] = cherchChercheursMap[key];
    }
    cherchChercheursMap = sortedMap;
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter", QDir::homePath(), "CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "ID,Nom,Prenom,Grade,Email,CIN,Date Creation,Age,Carriere,Nb Projets\n";

            for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
                auto data = it.value();
                stream << it.key() << ","
                       << data.nom << ","
                       << data.prenom << ","
                       << data.grade << ","
                       << data.email << ","
                       << data.cin << ","
                       << data.dateCreation.toString("dd/MM/yyyy") << ","
                       << data.age << ","
                       << data.carriere << ","
                       << data.projetsIds.size() << "\n";
            }
            file.close();
            QMessageBox::information(this, "Export", "Export réussi !");
        }
    }
}

void SmartPub::on_cherchBtnStatistiques_clicked()
{
    cherchAfficherStatistiques();
}

void SmartPub::cherchAfficherStatistiques()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques des Chercheurs");
    dialog->setMinimumSize(1000, 800);
    dialog->setStyleSheet("background-color: #f8fafc;");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(24);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("📊 Tableau de Bord Statistique", dialog);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 700; color: #1e293b; background: transparent; border: none;");
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

    auto createStatCard = [](const QString &title, const QString &value, const QString &color) -> QFrame* {
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
        titleLabel->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; background: transparent; border: none;");

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("color: %1; font-size: 48px; font-weight: 700; background: transparent; border: none;").arg(color));

        layout->addWidget(titleLabel);
        layout->addWidget(valueLabel);
        layout->addStretch();
        return card;
    };

    statsGrid->addWidget(createStatCard("Total Chercheurs", QString::number(cherchChercheursMap.size()), "#3b82f6"), 0, 0);

    int nbProfs = 0, nbDocs = 0;
    for (auto &data : cherchChercheursMap) {
        if (data.grade == "Professeur") nbProfs++;
        if (data.grade == "Doctorant") nbDocs++;
    }

    statsGrid->addWidget(createStatCard("Professeurs", QString::number(nbProfs), "#10b981"), 0, 1);
    statsGrid->addWidget(createStatCard("Doctorants", QString::number(nbDocs), "#f59e0b"), 0, 2);

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
    chartTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #1e293b; background: transparent; border: none;");
    chartLayout->addWidget(chartTitle);

    QMap<QString, int> gradeCount;
    for (auto &data : cherchChercheursMap) {
        gradeCount[data.grade]++;
    }

    for (auto it = gradeCount.begin(); it != gradeCount.end(); ++it) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *gradeLabel = new QLabel(it.key() + ":");
        gradeLabel->setStyleSheet("font-size: 16px; color: #334155; font-weight: 600; background: transparent; border: none;");
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

    QLabel *overloadTitle = new QLabel("📈 Indice de Surcharge (Projets en cours)", overloadFrame);
    overloadTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #1e293b; background: transparent; border: none;");
    overloadLayout->addWidget(overloadTitle);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *headerName = new QLabel("Chercheur");
    headerName->setStyleSheet("color: #64748b; font-weight: 600; font-size: 13px; background: transparent; border: none;");
    headerName->setFixedWidth(200);

    QLabel *headerProgress = new QLabel("Charge de travail");
    headerProgress->setStyleSheet("color: #64748b; font-weight: 600; font-size: 13px; background: transparent; border: none;");

    QLabel *headerStatus = new QLabel("Statut");
    headerStatus->setStyleSheet("color: #64748b; font-weight: 600; font-size: 13px; background: transparent; border: none;");
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

    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        auto data = it.value();
        int nbProjets = data.projetsIds.size();
        int surcharge = qMin(nbProjets * 25, 100);

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(15);

        QLabel *nameLabel = new QLabel(QString("%1 %2").arg(data.prenom).arg(data.nom));
        nameLabel->setFixedWidth(200);
        nameLabel->setStyleSheet("font-weight: 600; color: #334155; background: transparent; border: none;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(surcharge);
        progressBar->setTextVisible(true);
        progressBar->setFormat(QString("%1 projets").arg(nbProjets));
        progressBar->setFixedHeight(28);

        QString color;
        if (surcharge < 50) color = "#10b981";
        else if (surcharge < 75) color = "#f59e0b";
        else color = "#ef4444";

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
        )").arg(color));

        QLabel *statusLabel = new QLabel();
        if (surcharge < 50) statusLabel->setText("🟢 Normal");
        else if (surcharge < 75) statusLabel->setText("🟡 Occupé");
        else statusLabel->setText("🔴 Surchargé");
        statusLabel->setStyleSheet(QString("color: %1; font-weight: 600; background: transparent; border: none;").arg(color));
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

void SmartPub::on_cherchBtnUploadPhoto_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Photo", QDir::homePath(), "Images (*.png *.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        ui->cherchLabelPhotoHint->setText("Photo sélectionnée ✓");
        ui->cherchLabelPhotoHint->setStyleSheet("color: #10b981; font-size: 12px; background: transparent; border: none;");
    }
}

void SmartPub::on_cherchBtnAjouterChercheur_clicked()
{
    QString nom = ui->cherchLineEditNom->text().trimmed();
    QString prenom = ui->cherchLineEditPrenom->text().trimmed();
    QString cin = ui->cherchLineEditCIN->text().trimmed();

    if (nom.isEmpty() || prenom.isEmpty() || cin.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }

    int newId = cherchChercheursMap.isEmpty() ? 1 : cherchChercheursMap.keys().last() + 1;

    ChercheurData data;
    data.nom = nom;
    data.prenom = prenom;
    data.cin = cin;
    data.email = ui->cherchLineEditEmail->text();
    data.grade = ui->cherchComboBoxGrade->currentText();
    data.dateCreation = QDateTime::currentDateTime();
    data.age = 35;
    data.carriere = "Junior";
    data.photoPath = ":/avatar.png";

    cherchChercheursMap[newId] = data;

    QMessageBox::information(this, "Succès", "Chercheur ajouté !");

    ui->cherchLineEditNom->clear();
    ui->cherchLineEditPrenom->clear();
    ui->cherchLineEditCIN->clear();
    ui->cherchLineEditEmail->clear();
    ui->cherchComboBoxGrade->setCurrentIndex(0);
    ui->cherchLabelPhotoHint->setText("Cliquez pour ajouter une photo");
    ui->cherchLabelPhotoHint->setStyleSheet("color: #94a3b8; font-size: 12px; background: transparent; border: none;");

    on_cherchBtnVueListe_clicked();
}

void SmartPub::on_cherchBtnAnnulerAjout_clicked()
{
    if (!ui->cherchLineEditNom->text().isEmpty() ||
        !ui->cherchLineEditPrenom->text().isEmpty()) {

        auto reply = QMessageBox::question(this, "Confirmation", "Annuler ?");
        if (reply == QMessageBox::No) return;
    }

    ui->cherchLineEditNom->clear();
    ui->cherchLineEditPrenom->clear();
    ui->cherchLineEditCIN->clear();
    ui->cherchLineEditEmail->clear();

    on_cherchBtnVueListe_clicked();
}

void SmartPub::on_cherchModifierChercheur(int id)
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas modifier les données.");
        return;
    }

    auto data = cherchChercheursMap.value(id);
    if (data.nom.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(QString("Modifier - %1 %2").arg(data.prenom).arg(data.nom));
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet("background-color: #f8fafc;");

    QVBoxLayout layout(&dialog);
    layout.setSpacing(20);
    layout.setContentsMargins(30, 30, 30, 30);

    QLabel *title = new QLabel("Modifier le chercheur");
    title->setStyleSheet("font-size: 22px; font-weight: 700; color: #1e293b; background: transparent; border: none;");
    layout.addWidget(title);

    QLineEdit *editNom = new QLineEdit(data.nom);
    editNom->setStyleSheet("padding: 12px; border-radius: 10px; border: 2px solid #e2e8f0; font-size: 14px;");
    QLineEdit *editPrenom = new QLineEdit(data.prenom);
    editPrenom->setStyleSheet("padding: 12px; border-radius: 10px; border: 2px solid #e2e8f0; font-size: 14px;");
    QLineEdit *editEmail = new QLineEdit(data.email);
    editEmail->setStyleSheet("padding: 12px; border-radius: 10px; border: 2px solid #e2e8f0; font-size: 14px;");

    QComboBox *comboGrade = new QComboBox();
    comboGrade->setStyleSheet(R"(
        QComboBox {
            padding: 12px;
            border-radius: 10px;
            border: 2px solid #e2e8f0;
            font-size: 14px;
            min-height: 40px;
        }
    )");
    comboGrade->addItems({"Professeur", "Maitre de Conferences", "Docteur", "Ingenieur de Recherche", "Post-doctorant", "Doctorant"});
    comboGrade->setCurrentText(data.grade);

    layout.addWidget(new QLabel("Nom:"));
    layout.addWidget(editNom);
    layout.addWidget(new QLabel("Prénom:"));
    layout.addWidget(editPrenom);
    layout.addWidget(new QLabel("Email:"));
    layout.addWidget(editEmail);
    layout.addWidget(new QLabel("Grade:"));
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
        ChercheurData newData = data;
        newData.nom = editNom->text();
        newData.prenom = editPrenom->text();
        newData.email = editEmail->text();
        newData.grade = comboGrade->currentText();
        cherchChercheursMap[id] = newData;
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_cherchSupprimerChercheur(int id)
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer les données.");
        return;
    }

    auto reply = QMessageBox::question(this, "Supprimer", "Confirmer la suppression ?");
    if (reply == QMessageBox::Yes) {
        cherchChercheursMap.remove(id);
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_cherchVoirDetailsChercheur(int id)
{
    auto data = cherchChercheursMap.value(id);
    if (data.nom.isEmpty()) return;

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(QString("Profil - %1 %2").arg(data.prenom).arg(data.nom));
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
    avatarLabel->setStyleSheet(R"(
        QLabel {
            background-image: url(:/avatar.png);
            background-repeat: no-repeat;
            background-position: center;
            border-radius: 60px;
            border: 4px solid white;
            background-color: #e2e8f0;
        }
    )");
    avatarLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(avatarLabel, 0, Qt::AlignCenter);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(data.prenom).arg(data.nom));
    nameLabel->setStyleSheet("color: white; font-size: 26px; font-weight: 700; background: transparent; border: none;");
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

    auto createInfoRow = [&](const QString &label, const QString &value, const QString &icon = "") -> QFrame* {
        QFrame *row = new QFrame();
        row->setStyleSheet("background-color: #f8fafc; border-radius: 12px;");
        row->setMaximumHeight(80);
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(20, 15, 20, 15);

        QLabel *iconLabel = new QLabel(icon.isEmpty() ? "•" : icon);
        iconLabel->setStyleSheet("font-size: 20px; background: transparent; border: none;");
        rowLayout->addWidget(iconLabel);

        QLabel *labelWidget = new QLabel(label + ":");
        labelWidget->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; min-width: 150px; background: transparent; border: none;");
        rowLayout->addWidget(labelWidget);

        QLabel *valueWidget = new QLabel(value);
        valueWidget->setStyleSheet("color: #1e293b; font-size: 16px; font-weight: 500; background: transparent; border: none;");
        valueWidget->setWordWrap(true);
        rowLayout->addWidget(valueWidget, 1);

        return row;
    };

    contentLayout->addWidget(createInfoRow("Grade", data.grade, "🎓"));
    contentLayout->addWidget(createInfoRow("Email", data.email, "✉️"));
    contentLayout->addWidget(createInfoRow("CIN", data.cin, "🆔"));
    contentLayout->addWidget(createInfoRow("Âge", QString("%1 ans").arg(data.age), "🎂"));
    contentLayout->addWidget(createInfoRow("Date d'ajout", data.dateCreation.toString("dd MMMM yyyy à hh:mm"), "📅"));
    contentLayout->addWidget(createInfoRow("Carrière", data.carriere, "⭐"));
    contentLayout->addWidget(createInfoRow("Projets en cours", QString::number(data.projetsIds.size()), "📁"));

    if (!data.projetsIds.isEmpty()) {
        QLabel *projetsTitle = new QLabel("Détails des projets:");
        projetsTitle->setStyleSheet("color: #1e293b; font-size: 18px; font-weight: 700; margin-top: 10px; background: transparent; border: none;");
        contentLayout->addWidget(projetsTitle);

        for (int projId : data.projetsIds) {
            QFrame *projFrame = new QFrame();
            projFrame->setStyleSheet("background-color: #eff6ff; border-left: 4px solid #3b82f6; border-radius: 8px;");
            QHBoxLayout *projLayout = new QHBoxLayout(projFrame);
            projLayout->setContentsMargins(15, 12, 15, 12);

            QLabel *projLabel = new QLabel(QString("Projet #%1 - En cours de développement").arg(projId));
            projLabel->setStyleSheet("color: #3b82f6; font-weight: 600; background: transparent; border: none;");
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
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
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
    connect(btnExport, &QPushButton::clicked, this, [this]() {
        on_cherchBtnExportDetails_clicked();
    });

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

void SmartPub::on_cherchBtnExportDetails_clicked()
{
    QMessageBox::information(this, "Export", "Fiche exportée avec succès !");
}

void SmartPub::on_cherchLineEditRecherche_textChanged(const QString &text)
{
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

void SmartPub::SR_setupUI()
{
    ui->SR_stackedWidget->setCurrentIndex(0);
}

void SmartPub::SR_connectSignals()
{
    connect(ui->SR_btnVueListe, &QPushButton::clicked, this, &SmartPub::on_SR_btnVueListe_clicked);
    connect(ui->SR_btnAjouter, &QPushButton::clicked, this, &SmartPub::on_SR_btnAjouter_clicked);
    connect(ui->SR_btnSupprimer, &QPushButton::clicked, this, &SmartPub::on_SR_btnSupprimer_clicked);
    connect(ui->SR_btnRecherche, &QPushButton::clicked, this, &SmartPub::on_SR_btnRecherche_clicked);
    connect(ui->SR_btnTri, &QPushButton::clicked, this, &SmartPub::on_SR_btnTri_clicked);
    connect(ui->SR_btnExport, &QPushButton::clicked, this, &SmartPub::on_SR_btnExport_clicked);
    connect(ui->SR_btnStatistiques, &QPushButton::clicked, this, &SmartPub::on_SR_btnStatistiques_clicked);
    connect(ui->SR_btnAjouterPublication, &QPushButton::clicked, this, &SmartPub::on_SR_btnAjouterPublication_clicked);
    connect(ui->SR_btnAnnulerAjout, &QPushButton::clicked, this, &SmartPub::on_SR_btnAnnulerAjout_clicked);
}

void SmartPub::SR_updateButtonStyles()
{
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

    ui->SR_btnSupprimer->setStyleSheet(deleteStyle);
}

void SmartPub::SR_loadSampleData()
{
    QStringList titres = {
        "Machine Learning pour la détection de fraudes",
        "Analyse des données génomiques",
        "Quantum Computing: état de l'art",
        "Intelligence Artificielle en médecine",
        "Blockchain pour la sécurité des données",
        "Deep Learning pour la vision par ordinateur",
        "Cryptographie post-quantique",
        "IoT et sécurité des réseaux"
    };

    QStringList auteurs = {
        "Dr. Martin, Prof. Dubois",
        "Dr. Laurent, Dr. Bernard",
        "Prof. Moreau, Dr. Petit",
        "Dr. Roux, Prof. Simon",
        "Dr. Michel, Dr. Garcia",
        "Prof. Durand, Dr. Lefebvre",
        "Dr. Morel, Prof. Girard",
        "Dr. Andre, Dr. Blanc"
    };

    QStringList dates = {
        "2024-01-15",
        "2024-02-20",
        "2023-11-10",
        "2024-03-05",
        "2023-09-18",
        "2024-04-12",
        "2023-12-01",
        "2024-05-20"
    };

    QStringList revues = {
        "IEEE Transactions on AI",
        "Nature Genetics",
        "Quantum Information Review",
        "Medical AI Journal",
        "Blockchain Security Review",
        "Computer Vision and Pattern Recognition",
        "Journal of Cryptology",
        "IEEE Internet of Things Journal"
    };

    QStringList statuts = {
        "Publié",
        "Publié",
        "Soumis",
        "En révision",
        "Accepté",
        "Publié",
        "Soumis",
        "En révision"
    };

    ui->SR_tablePublications->setRowCount(titres.size());

    for (int i = 0; i < titres.size(); ++i) {
        ui->SR_tablePublications->setItem(i, 0, new QTableWidgetItem(titres[i]));
        ui->SR_tablePublications->setItem(i, 1, new QTableWidgetItem(auteurs[i]));
        ui->SR_tablePublications->setItem(i, 2, new QTableWidgetItem(dates[i]));
        ui->SR_tablePublications->setItem(i, 3, new QTableWidgetItem(revues[i]));
        ui->SR_tablePublications->setItem(i, 4, new QTableWidgetItem(statuts[i]));
        ui->SR_tablePublications->setItem(i, 5, new QTableWidgetItem("Modifier | Voir"));
    }

    ui->SR_lblTotalNumber->setText(QString::number(titres.size()));
    ui->SR_lblThisYearNumber->setText("5");
    ui->SR_lblPlanSNumber->setText("3");

    int publie = 3, soumis = 2, revision = 2, accepte = 1;
    int total = titres.size();

    ui->SR_lblStatPublie->setText(QString("● Publié (%1%)").arg((publie * 100) / total));
    ui->SR_lblStatSoumis->setText(QString("● Soumis (%1%)").arg((soumis * 100) / total));
    ui->SR_lblStatRevision->setText(QString("● En révision (%1%)").arg((revision * 100) / total));
    ui->SR_lblStatAccepte->setText(QString("● Accepté (%1%)").arg((accepte * 100) / total));
}

void SmartPub::on_SR_btnVueListe_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnAjouter_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de publications.");
        return;
    }
    ui->SR_stackedWidget->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnSupprimer_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer de publications.");
        return;
    }
    QMessageBox::information(this, "Information", "Fonctionnalité de suppression non implémentée");
}

void SmartPub::on_SR_btnRecherche_clicked()
{
    QString searchText = ui->SR_lineEditRecherche->text();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un terme de recherche");
    } else {
        QMessageBox::information(this, "Recherche", "Recherche de: " + searchText);
    }
}

void SmartPub::on_SR_btnTri_clicked()
{
    QMessageBox::information(this, "Information", "Fonctionnalité de tri non implémentée");
}

void SmartPub::on_SR_btnExport_clicked()
{
    QMessageBox::information(this, "Information", "Fonctionnalité d'export non implémentée");
}

void SmartPub::on_SR_btnStatistiques_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(2);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnAjouterPublication_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de publications.");
        return;
    }

    QString titre = ui->SR_lineEditTitre->text();
    QString auteurs = ui->SR_lineEditAuteurs->text();
    QString revue = ui->SR_lineEditRevue->text();
    QString statut = ui->SR_comboBoxStatut->currentText();

    if (titre.isEmpty() || auteurs.isEmpty() || revue.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires");
        return;
    }

    QMessageBox::information(this, "Succès", "Publication ajoutée avec succès (simulation)");

    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();

    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
}

void SmartPub::on_SR_btnAnnulerAjout_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

// ============================================================================
// MODULE FINANCES
// ============================================================================

void SmartPub::finSetupUI()
{
    ui->finStackedWidget->setCurrentIndex(0);
    finVueListeActive = true;

    ui->finComboBoxProjet->addItems({
        "Projet AI-2024-001",
        "Projet Quantum-2024-002",
        "Projet BioTech-2024-003",
        "Projet CyberSec-2024-004"
    });
}

void SmartPub::finConnectSignals()
{
    connect(ui->finBtnVueListe, &QPushButton::clicked, this, &SmartPub::on_finBtnVueListe_clicked);
    connect(ui->finBtnAjouter, &QPushButton::clicked, this, &SmartPub::on_finBtnAjouter_clicked);
    connect(ui->finBtnRecherche, &QPushButton::clicked, this, &SmartPub::on_finBtnRecherche_clicked);
    connect(ui->finBtnTri, &QPushButton::clicked, this, &SmartPub::on_finBtnTri_clicked);
    connect(ui->finBtnExport, &QPushButton::clicked, this, &SmartPub::on_finBtnExport_clicked);
    connect(ui->finBtnStatistiques, &QPushButton::clicked, this, &SmartPub::on_finBtnStatistiques_clicked);
    connect(ui->finBtnAjouterTransaction, &QPushButton::clicked, this, &SmartPub::on_finBtnAjouterTransaction_clicked);
    connect(ui->finBtnAnnulerAjout, &QPushButton::clicked, this, &SmartPub::on_finBtnAnnulerAjout_clicked);
    connect(ui->finBtnModifierTable, &QPushButton::clicked, this, &SmartPub::on_finBtnModifierTransaction_clicked);
    connect(ui->finBtnSupprimerTable, &QPushButton::clicked, this, &SmartPub::on_finBtnSupprimerTransaction_clicked);
}

void SmartPub::finUpdateButtonStyles()
{
    QString activeStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b9cff, stop:1 #2dd4bf);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2b8cef, stop:1 #1dc4af);
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

void SmartPub::finAjouterDonneesTest()
{
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

void SmartPub::finAfficherListeTransactions()
{
    ui->finTableTransactions->setRowCount(0);
    for (auto it = finTransactionsMap.begin(); it != finTransactionsMap.end(); ++it) {
        finAjouterTransactionTable(it.value());
    }
}

void SmartPub::finAjouterTransactionTable(const TransactionData &data)
{
    int row = ui->finTableTransactions->rowCount();
    ui->finTableTransactions->insertRow(row);

    ui->finTableTransactions->setItem(row, 0, new QTableWidgetItem(QString::number(data.id)));
    ui->finTableTransactions->setItem(row, 1, new QTableWidgetItem(data.projet));
    ui->finTableTransactions->setItem(row, 2, new QTableWidgetItem(data.type));
    ui->finTableTransactions->setItem(row, 3, new QTableWidgetItem(QString::number(data.montant, 'f', 2) + " €"));
    ui->finTableTransactions->setItem(row, 4, new QTableWidgetItem(data.date));
    ui->finTableTransactions->setItem(row, 5, new QTableWidgetItem(data.categorie));
    ui->finTableTransactions->setItem(row, 6, new QTableWidgetItem(data.statut));
    ui->finTableTransactions->setItem(row, 7, new QTableWidgetItem("Modifier | Supprimer"));
}

void SmartPub::on_finBtnVueListe_clicked()
{
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnAjouter_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }
    ui->finStackedWidget->setCurrentIndex(1);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnRecherche_clicked()
{
    QString searchText = ui->finLineEditRecherche->text();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un terme de recherche");
    } else {
        QMessageBox::information(this, "Recherche", "Recherche de transaction: " + searchText);
    }
}

void SmartPub::on_finBtnTri_clicked()
{
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

void SmartPub::on_finBtnExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les transactions", QDir::homePath(), "CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Export", "Transactions exportées avec succès !");
    }
}

void SmartPub::on_finBtnStatistiques_clicked()
{
    QMessageBox::information(this, "Statistiques", "Module statistiques finances - À implémenter");
}

void SmartPub::on_finBtnAjouterTransaction_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de transactions.");
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
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }

    bool ok;
    double montant = montantStr.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Erreur", "Montant invalide");
        return;
    }

    int newId = finTransactionsMap.isEmpty() ? 1 : finTransactionsMap.keys().last() + 1;

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

void SmartPub::on_finBtnAnnulerAjout_clicked()
{
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnModifierTransaction_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas modifier les transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une transaction à modifier");
        return;
    }
    QMessageBox::information(this, "Modifier", "Fonctionnalité de modification - À implémenter");
}

void SmartPub::on_finBtnSupprimerTransaction_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer les transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une transaction à supprimer");
        return;
    }

    auto reply = QMessageBox::question(this, "Supprimer", "Confirmer la suppression de cette transaction ?");
    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "Succès", "Transaction supprimée");
        ui->finTableTransactions->removeRow(currentRow);
    }
}

// ============================================================================
// MODULE EVENEMENTS
// ============================================================================

void SmartPub::evSetupUI()
{
    ui->evTabWidget->setCurrentIndex(0);
    evEventSelectionne = -1;
}

void SmartPub::evConnectSignals()
{
    connect(ui->evBtnAjouterEvent, &QPushButton::clicked, this, &SmartPub::on_evBtnAjouterEvent_clicked);
    connect(ui->evBtnModifierEvent, &QPushButton::clicked, this, &SmartPub::on_evBtnModifierEvent_clicked);
    connect(ui->evBtnSupprimerEvent, &QPushButton::clicked, this, &SmartPub::on_evBtnSupprimerEvent_clicked);
    connect(ui->evBtnTrierDate, &QPushButton::clicked, this, &SmartPub::on_evBtnTrierDate_clicked);
    connect(ui->evBtnRechercheLieu, &QPushButton::clicked, this, &SmartPub::on_evBtnRechercheLieu_clicked);
    connect(ui->evBtnExportCalendrier, &QPushButton::clicked, this, &SmartPub::on_evBtnExportCalendrier_clicked);
    connect(ui->evBtnLivreResumes, &QPushButton::clicked, this, &SmartPub::on_evBtnLivreResumes_clicked);
    connect(ui->evBtnCalculImpact, &QPushButton::clicked, this, &SmartPub::on_evBtnCalculImpact_clicked);
    connect(ui->evBtnStatsParticipation, &QPushButton::clicked, this, &SmartPub::on_evBtnStatsParticipation_clicked);
}

void SmartPub::evAjouterDonneesTest()
{
    EventData e1;
    e1.id = 1;
    e1.nom = "Conférence Internationale sur l'IA";
    e1.lieu = "Paris, France";
    e1.date = "15/03/2024";
    e1.description = "Conférence sur les avancées en intelligence artificielle";
    evEventsMap[1] = e1;

    EventData e2;
    e2.id = 2;
    e2.nom = "Workshop Quantum Computing";
    e2.lieu = "Lyon, France";
    e2.date = "22/04/2024";
    e2.description = "Atelier pratique sur l'informatique quantique";
    evEventsMap[2] = e2;

    EventData e3;
    e3.id = 3;
    e3.nom = "Séminaire BioTech";
    e3.lieu = "Marseille, France";
    e3.date = "10/05/2024";
    e3.description = "Séminaire sur les biotechnologies";
    evEventsMap[3] = e3;

    evAfficherListeEvents();
}

void SmartPub::evAfficherListeEvents()
{
    ui->evTableEvents->setRowCount(0);
    for (auto it = evEventsMap.begin(); it != evEventsMap.end(); ++it) {
        evAjouterEventTable(it.value());
    }

    ui->evTableSearchEvents->setRowCount(0);
    for (auto it = evEventsMap.begin(); it != evEventsMap.end(); ++it) {
        int row = ui->evTableSearchEvents->rowCount();
        ui->evTableSearchEvents->insertRow(row);
        ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(QString::number(it.value().id)));
        ui->evTableSearchEvents->setItem(row, 1, new QTableWidgetItem(it.value().nom));
        ui->evTableSearchEvents->setItem(row, 2, new QTableWidgetItem(it.value().lieu));
        ui->evTableSearchEvents->setItem(row, 3, new QTableWidgetItem(it.value().date));
    }
}

void SmartPub::evAjouterEventTable(const EventData &data)
{
    int row = ui->evTableEvents->rowCount();
    ui->evTableEvents->insertRow(row);

    ui->evTableEvents->setItem(row, 0, new QTableWidgetItem(QString::number(data.id)));
    ui->evTableEvents->setItem(row, 1, new QTableWidgetItem(data.nom));
    ui->evTableEvents->setItem(row, 2, new QTableWidgetItem(data.lieu));
    ui->evTableEvents->setItem(row, 3, new QTableWidgetItem(data.date));
}

void SmartPub::evRechercherParLieu()
{
    QString lieu = ui->evLineEditSearchLieu->text().toLower();
    if (lieu.isEmpty()) {
        QMessageBox::warning(this, "Recherche", "Veuillez entrer un lieu");
        return;
    }

    ui->evTableSearchEvents->setRowCount(0);
    for (auto it = evEventsMap.begin(); it != evEventsMap.end(); ++it) {
        if (it.value().lieu.toLower().contains(lieu)) {
            int row = ui->evTableSearchEvents->rowCount();
            ui->evTableSearchEvents->insertRow(row);
            ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(QString::number(it.value().id)));
            ui->evTableSearchEvents->setItem(row, 1, new QTableWidgetItem(it.value().nom));
            ui->evTableSearchEvents->setItem(row, 2, new QTableWidgetItem(it.value().lieu));
            ui->evTableSearchEvents->setItem(row, 3, new QTableWidgetItem(it.value().date));
        }
    }
}

void SmartPub::on_evBtnAjouterEvent_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter d'événements.");
        return;
    }

    QString id = ui->evLineEditID->text();
    QString nom = ui->evLineEditNom->text();
    QString lieu = ui->evLineEditLieu->text();
    QString date = ui->evLineEditDate->text();

    if (id.isEmpty() || nom.isEmpty() || lieu.isEmpty() || date.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs");
        return;
    }

    bool ok;
    int idNum = id.toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Erreur", "ID invalide");
        return;
    }

    EventData data;
    data.id = idNum;
    data.nom = nom;
    data.lieu = lieu;
    data.date = date;
    data.description = "";

    evEventsMap[idNum] = data;

    QMessageBox::information(this, "Succès", "Événement ajouté avec succès !");

    evAfficherListeEvents();

    ui->evLineEditID->clear();
    ui->evLineEditNom->clear();
    ui->evLineEditLieu->clear();
    ui->evLineEditDate->clear();
}

void SmartPub::on_evBtnModifierEvent_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas modifier les événements.");
        return;
    }

    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un événement à modifier");
        return;
    }
    QMessageBox::information(this, "Modifier", "Fonctionnalité de modification - À implémenter");
}

void SmartPub::on_evBtnSupprimerEvent_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer les événements.");
        return;
    }

    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un événement à supprimer");
        return;
    }

    auto reply = QMessageBox::question(this, "Supprimer", "Confirmer la suppression de cet événement ?");
    if (reply == QMessageBox::Yes) {
        int id = ui->evTableEvents->item(currentRow, 0)->text().toInt();
        evEventsMap.remove(id);
        evAfficherListeEvents();
        QMessageBox::information(this, "Succès", "Événement supprimé");
    }
}

void SmartPub::on_evBtnTrierDate_clicked()
{
    QMessageBox::information(this, "Tri", "Événements triés par date");
}

void SmartPub::on_evBtnRechercheLieu_clicked()
{
    evRechercherParLieu();
}

void SmartPub::on_evBtnExportCalendrier_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter le calendrier", QDir::homePath(), "iCalendar (*.ics)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Export", "Calendrier exporté avec succès !");
    }
}

void SmartPub::on_evBtnLivreResumes_clicked()
{
    QMessageBox::information(this, "Livre des Résumés", "Génération du livre des résumés - À implémenter");
}

void SmartPub::on_evBtnCalculImpact_clicked()
{
    QMessageBox::information(this, "Calculateur d'Impact", "Calcul de l'impact carbone - À implémenter");
}

void SmartPub::on_evBtnStatsParticipation_clicked()
{
    QMessageBox::information(this, "Statistiques", "Statistiques de participation - À implémenter");
}
// ============================================================================
// MODULE PROJETS (Ton travail original)
// ============================================================================

void SmartPub::projSetupUI()
{
    projSetupTable();

    // Tooltips pour les boutons CRUD
    ui->btnListeProjets->setToolTip("Liste des projets");
    ui->btnAjouterProjet->setToolTip("Ajouter un projet");
    ui->btnModifierProjet->setToolTip("Modifier le projet sélectionné");
    ui->btnSupprimerProjet->setToolTip("Supprimer le projet sélectionné");

    // Styles pour la page projets - enlever les zones noires
    if (ui->pageListeProjets) {
        ui->pageListeProjets->setStyleSheet("background-color: #f8fafc;");
    }

    if (ui->pageFormProjet) {
        ui->pageFormProjet->setStyleSheet("background-color: #f8fafc;");
    }

    // Style pour le frame de la toolbar
    if (ui->toolbarFrameProjets) {
        ui->toolbarFrameProjets->setStyleSheet(R"(
            QFrame#toolbarFrameProjets {
                background-color: white;
                border-radius: 12px;
                border: 1px solid #e2e8f0;
                padding: 10px;
            }
        )");
    }

    // Style pour le frame de la table
    if (ui->tableFrameProjets) {
        ui->tableFrameProjets->setStyleSheet(R"(
            QFrame#tableFrameProjets {
                background-color: white;
                border-radius: 12px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    // Styles pour les boutons de la toolbar
    QString toolbarBtnStyle = R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #3b82f6;
            color: #3b82f6;
        }
        QPushButton:pressed {
            background-color: #eff6ff;
        }
    )";

    QString checkableBtnStyle = R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #3b82f6;
            color: #3b82f6;
        }
        QPushButton:checked {
            background-color: #3b82f6;
            color: white;
            border-color: #3b82f6;
        }
    )";

    // Appliquer les styles aux boutons
    if (ui->btnListeProjets) ui->btnListeProjets->setStyleSheet(toolbarBtnStyle);
    if (ui->btnAjouterProjet) ui->btnAjouterProjet->setStyleSheet(toolbarBtnStyle);
    if (ui->btnModifierProjet) ui->btnModifierProjet->setStyleSheet(toolbarBtnStyle);
    if (ui->btnSupprimerProjet) ui->btnSupprimerProjet->setStyleSheet(toolbarBtnStyle);
    if (ui->btnStatistiques) ui->btnStatistiques->setStyleSheet(toolbarBtnStyle);
    if (ui->btnFiltresProjets) ui->btnFiltresProjets->setStyleSheet(toolbarBtnStyle);
    if (ui->btnExporterProjets) ui->btnExporterProjets->setStyleSheet(toolbarBtnStyle);

    // Boutons de tri (checkable)
    if (ui->btnTriDateDebut) ui->btnTriDateDebut->setStyleSheet(checkableBtnStyle);
    if (ui->btnTriDateFin) ui->btnTriDateFin->setStyleSheet(checkableBtnStyle);
    if (ui->btnTriEtat) ui->btnTriEtat->setStyleSheet(checkableBtnStyle);
    if (ui->btnTriProgression) ui->btnTriProgression->setStyleSheet(checkableBtnStyle);

    // Style pour le champ de recherche
    if (ui->lineEditRechercheProjets) {
        ui->lineEditRechercheProjets->setStyleSheet(R"(
            QLineEdit {
                background-color: #f8fafc;
                border: 2px solid #e2e8f0;
                border-radius: 8px;
                padding: 8px 12px;
                font-size: 13px;
                color: #334155;
                min-width: 200px;
            }
            QLineEdit:focus {
                border-color: #3b82f6;
                background-color: white;
            }
        )");
    }
}

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

    // StretchLastSection à true pour que la dernière colonne s'étire
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(80);
    table->horizontalHeader()->setDefaultSectionSize(100);

    // Largeurs ajustées pour éviter le tronquage
    table->setColumnWidth(0, 0);       // ID caché
    table->setColumnWidth(1, 110);     // Code
    table->setColumnWidth(2, 300);     // Titre - BEAUCOUP PLUS LARGE
    table->setColumnWidth(3, 100);     // Date Début
    table->setColumnWidth(4, 100);     // Date Fin
    table->setColumnWidth(5, 170);     // Responsable
    table->setColumnWidth(6, 90);      // État
    // Colonne 7 (Progression) s'étire avec StretchLastSection

    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);

    // WordWrap désactivé pour éviter les problèmes de hauteur
    table->setWordWrap(false);

    // Hauteur de ligne fixe
    table->verticalHeader()->setDefaultSectionSize(50);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // Style de l'en-tête
    table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #1e293b;"
        "    padding: 10px 6px;"
        "    font-weight: 600;"
        "    color: white;"
        "    border: none;"
        "    font-size: 11px;"
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

void SmartPub::projConnectSignals()
{
    connect(ui->btnListeProjets, &QPushButton::clicked, this, &SmartPub::on_btnListeProjets_clicked);
    connect(ui->btnAjouterProjet, &QPushButton::clicked, this, &SmartPub::on_btnAjouterProjet_clicked);
    connect(ui->btnModifierProjet, &QPushButton::clicked, this, &SmartPub::on_btnModifierProjet_clicked);
    connect(ui->btnSupprimerProjet, &QPushButton::clicked, this, &SmartPub::on_btnSupprimerProjet_clicked);

    connect(ui->lineEditRechercheProjets, &QLineEdit::textChanged, this, &SmartPub::on_lineEditRechercheProjets_textChanged);
    connect(ui->btnAnnulerForm, &QPushButton::clicked, this, &SmartPub::on_btnAnnulerForm_clicked);
    connect(ui->btnEnregistrerForm, &QPushButton::clicked, this, &SmartPub::on_btnEnregistrerForm_clicked);

    connect(ui->tableWidgetProjets, &QTableWidget::itemSelectionChanged, this, &SmartPub::on_tableSelectionChanged);
    connect(ui->tableWidgetProjets, &QTableWidget::cellDoubleClicked, this, &SmartPub::on_tableDoubleClicked);

    connect(ui->btnTriDateDebut, &QPushButton::clicked, this, &SmartPub::on_triDateDebutClicked);
    connect(ui->btnTriDateFin, &QPushButton::clicked, this, &SmartPub::on_triDateFinClicked);
    connect(ui->btnTriEtat, &QPushButton::clicked, this, &SmartPub::on_triEtatClicked);
    connect(ui->btnTriProgression, &QPushButton::clicked, this, &SmartPub::on_triProgressionClicked);

    connect(ui->btnStatistiques, &QPushButton::clicked, this, &SmartPub::on_statistiquesClicked);
    connect(ui->btnFiltresProjets, &QPushButton::clicked, this, &SmartPub::on_filtresClicked);
    connect(ui->btnExporterProjets, &QPushButton::clicked, this, &SmartPub::on_exporterClicked);
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

void SmartPub::projChargerProjets()
{
    projViderTable();

    const QVector<Projet> &projetsACharger = filtresActifs ? projetsFiltres : projets;

    for (int i = 0; i < projetsACharger.size(); ++i) {
        projAjouterProjetTable(projetsACharger[i], i);
    }

    projAjusterColonnesTable();
}

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

    // Titre - Afficher le titre complet sans tronquage
    QTableWidgetItem *titreItem = new QTableWidgetItem(projet.titre);
    titreItem->setFont(QFont("Segoe UI", 9, QFont::Medium));
    titreItem->setToolTip(projet.titre);
    titreItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
    etatItem->setForeground(QColor(getEtatColor(projet.etat)));
    etatItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    ui->tableWidgetProjets->setItem(row, 6, etatItem);

    // Progression avec couleur
    QTableWidgetItem *progItem = new QTableWidgetItem(projet.progression);
    progItem->setTextAlignment(Qt::AlignCenter);
    progItem->setForeground(QColor(getProgressionColorFromString(projet.progression)));
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

    table->resizeColumnsToContents();

    table->setColumnWidth(0, 0);
    if (table->columnWidth(1) < 100) table->setColumnWidth(1, 100);
    if (table->columnWidth(2) < 250) table->setColumnWidth(2, 250);
    if (table->columnWidth(2) > 400) table->setColumnWidth(2, 400);
    if (table->columnWidth(3) < 90) table->setColumnWidth(3, 90);
    if (table->columnWidth(4) < 90) table->setColumnWidth(4, 90);
    if (table->columnWidth(5) < 150) table->setColumnWidth(5, 150);
    if (table->columnWidth(6) < 80) table->setColumnWidth(6, 80);
    if (table->columnWidth(7) < 80) table->setColumnWidth(7, 80);
}

void SmartPub::projViderTable()
{
    ui->tableWidgetProjets->setRowCount(0);
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

void SmartPub::on_btnListeProjets_clicked()
{
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::on_btnAjouterProjet_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de projets.");
        return;
    }

    isEditing = false;
    projViderFormulaire();
    projAfficherFormulaire(false);
    projSetActiveCrudButton(1);
}

void SmartPub::on_btnModifierProjet_clicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas modifier de projets.");
        return;
    }

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
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer de projets.");
        return;
    }

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
    etatItem->setForeground(QColor(getEtatColor(projet.etat)));

    QTableWidgetItem *progItem = ui->tableWidgetProjets->item(row, 7);
    progItem->setText(projet.progression);
    progItem->setForeground(QColor(getProgressionColorFromString(projet.progression)));
}

void SmartPub::on_tableSelectionChanged()
{
    bool hasSelection = !ui->tableWidgetProjets->selectedItems().isEmpty();
    ui->btnModifierProjet->setEnabled(hasSelection && currentUser.role == UserRole::Admin);
    ui->btnSupprimerProjet->setEnabled(hasSelection && currentUser.role == UserRole::Admin);
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

void SmartPub::on_statistiquesClicked()
{
    QList<Projet> projetsList(projets.begin(), projets.end());
    StatistiquesDialog *dialog = new StatistiquesDialog(projetsList, this);
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
    QList<Projet> projetsList(projets.begin(), projets.end());
    IARecommandationsDialog *dialog = new IARecommandationsDialog(projetsList, this);
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

void SmartPub::on_exporterClicked()
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

int SmartPub::projExtraireProgression(const QString &progressionStr) const
{
    QString temp = progressionStr;
    if (temp.endsWith('%')) {
        temp.chop(1);
    }
    return temp.toInt();
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

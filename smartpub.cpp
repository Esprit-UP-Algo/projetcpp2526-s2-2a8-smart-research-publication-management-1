#include "smartpub.h"
#include "ui_smartpub.h"


SmartPub::SmartPub(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SmartPub)
    , cherchVueListeActive(true)
    , cherchVueIconesActive(true)
    , cherchChercheurSelectionne(-1)
    , cherchIsLoggedIn(false)
    , cherchSidebarVisible(true)
    , finVueListeActive(true)
    , finTransactionSelectionnee(-1)
    , evEventSelectionne(-1)
{
    ui->setupUi(this);
    setupUI();
    connectSignals();

    // Initialiser le module chercheurs
    cherchSetupUI();
    cherchConnectSignals();
    cherchSetupAnimations();
    cherchApplyModernStyle();
    cherchSetupSidebarToggle();
    cherchAjouterDonneesTest();

    // Initialiser le module publications
    SR_setupUI();
    SR_connectSignals();
    SR_loadSampleData();

    // Initialiser le module finances
    finSetupUI();
    finConnectSignals();
    finAjouterDonneesTest();

    // Initialiser le module evenements
    evSetupUI();
    evConnectSignals();
    evAjouterDonneesTest();

    // Afficher la vue login du module chercheurs par défaut
    cherchShowLoginView();
}

SmartPub::~SmartPub()
{
    delete ui;
}

void SmartPub::setupUI()
{
    setWindowTitle("SmartPub - Gestion des Publications de Recherche");
    showMaximized();

    // Pas de sidebar principale - chaque module a sa propre sidebar
}

void SmartPub::connectSignals()
{
    // Pas de connexions de sidebar principale
    // Les connexions de navigation sont gérées dans chaque module
}

void SmartPub::updateNavButtonStyles(QPushButton *activeBtn)
{
    Q_UNUSED(activeBtn)
    // Cette fonction n'est pas utilisée car chaque module gère sa propre sidebar
}

// === NAVIGATION PRINCIPALE ===

void SmartPub::on_btnPublications_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_btnChercheurs_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(0);
    if (cherchIsLoggedIn) {
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_btnLaboratoires_clicked()
{
    QMessageBox::information(this, "Information", "Module Laboratoires non implémenté");
}

void SmartPub::on_btnProjets_clicked()
{
    QMessageBox::information(this, "Information", "Module Projets non implémenté");
}

void SmartPub::on_btnFinances_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_btnEvenements_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(3);
    evAfficherListeEvents();
}

// ============================================================================
// === MODULE CHERCHEURS (Votre code avec préfixe cherch) ===
// ============================================================================

void SmartPub::cherchSetupUI()
{
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
    ui->horizontalLayoutToolbar->insertWidget(1, cherchBtnToggleVue);
    connect(cherchBtnToggleVue, &QPushButton::clicked, this, &SmartPub::on_cherchBtnToggleVue_clicked);
}

void SmartPub::cherchSetupSidebarToggle()
{
    cherchBtnToggleSidebar = new QPushButton(this);
    cherchBtnToggleSidebar->setObjectName("cherchBtnToggleSidebar");
    cherchBtnToggleSidebar->setFixedSize(40, 40);
    cherchBtnToggleSidebar->setCursor(Qt::PointingHandCursor);
    cherchBtnToggleSidebar->setText("☰");
    cherchBtnToggleSidebar->setToolTip("Masquer/Afficher la sidebar");
    cherchBtnToggleSidebar->setStyleSheet(R"(
        QPushButton {
            background-color: #1e293b;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #334155;
        }
    )");
    ui->horizontalLayoutHeader->insertWidget(0, cherchBtnToggleSidebar);
    connect(cherchBtnToggleSidebar, &QPushButton::clicked, this, &SmartPub::on_cherchBtnToggleSidebar_clicked);
}

void SmartPub::cherchApplyModernStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f8fafc;
        }
        QWidget {
            font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
        }
    )");

    ui->cherchLoginFrame->setStyleSheet(R"(
        QFrame#cherchLoginFrame {
            background-color: white;
            border-radius: 20px;
            border: 1px solid #e2e8f0;
        }
    )");

    ui->cherchLoginHeader->setStyleSheet(R"(
        QFrame#cherchLoginHeader {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            border-top-left-radius: 20px;
            border-top-right-radius: 20px;
        }
    )");

    ui->cherchForgotFrame->setStyleSheet(R"(
        QFrame#cherchForgotFrame {
            background-color: white;
            border-radius: 20px;
            border: 1px solid #e2e8f0;
        }
    )");

    ui->cherchForgotHeader->setStyleSheet(R"(
        QFrame#cherchForgotHeader {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            border-top-left-radius: 20px;
            border-top-right-radius: 20px;
        }
    )");

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

    ui->cherchLineEditLoginEmail->setStyleSheet(cherchLoginInputStyle);
    ui->cherchLineEditLoginPassword->setStyleSheet(cherchLoginInputStyle);
    ui->cherchLineEditLoginPassword->setEchoMode(QLineEdit::Password);
    ui->cherchLineEditForgotEmail->setStyleSheet(cherchLoginInputStyle);

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

    ui->cherchSidebarFrame->setStyleSheet("background-color: #0f172a; border: none;");
    ui->cherchLogoFrame->setStyleSheet("background-color: #020617; border: none;");

    ui->cherchUserProfileFrame->setStyleSheet(R"(
        QFrame#cherchUserProfileFrame {
            background-color: transparent;
            border-radius: 12px;
            border: 1px solid transparent;
        }
        QFrame#cherchUserProfileFrame:hover {
            background-color: rgba(255, 255, 255, 0.1);
            border: 1px solid #334155;
        }
    )");

    ui->cherchUserAvatar->setStyleSheet(R"(
        QLabel#cherchUserAvatar {
            background-color: #10b981;
            border-radius: 20px;
            color: white;
            font-weight: bold;
            font-size: 16px;
        }
    )");

    ui->cherchUserName->setStyleSheet("color: white; font-size: 14px; font-weight: 600;");
    ui->cherchUserRole->setStyleSheet("color: #94a3b8; font-size: 12px;");

    QString cherchNavStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #94a3b8;
            border: none;
            border-radius: 12px;
            padding: 14px 20px;
            font-size: 14px;
            font-weight: 500;
            text-align: left;
            margin: 4px 12px;
        }
        QPushButton:hover {
            background-color: #1e293b;
            color: #e2e8f0;
        }
        QPushButton:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            font-weight: 600;
        }
    )";

    ui->cherchBtnPublications->setStyleSheet(cherchNavStyle);
    ui->cherchBtnChercheurs->setStyleSheet(cherchNavStyle);
    ui->cherchBtnLaboratoires->setStyleSheet(cherchNavStyle);
    ui->cherchBtnProjets->setStyleSheet(cherchNavStyle);
    ui->cherchBtnFinances->setStyleSheet(cherchNavStyle);
    ui->cherchBtnEvenements->setStyleSheet(cherchNavStyle);

    ui->cherchHeaderFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-bottom: 1px solid #e2e8f0;
        }
    )");

    ui->cherchTitleLabel->setStyleSheet("color: #1e293b; font-size: 24px; font-weight: 700; background: transparent; border: none;");
    ui->cherchSubtitleLabel->setStyleSheet("color: #64748b; font-size: 13px; background: transparent; border: none;");

    ui->cherchToolbarFrame->setStyleSheet("background-color: transparent; border: none;");

    ui->cherchTabsFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #e2e8f0;
        }
    )");

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

    ui->cherchBtnVueListe->setStyleSheet(cherchTabActive);
    ui->cherchBtnAjouter->setStyleSheet(cherchTabInactive);

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

    ui->cherchBtnTri->setStyleSheet(cherchSecondaryBtn);
    ui->cherchBtnExport->setStyleSheet(cherchSecondaryBtn);

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

    ui->cherchFormFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 20px;
            border: 1px solid #e2e8f0;
        }
    )");

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

    ui->cherchLineEditNom->setStyleSheet(cherchInputStyle);
    ui->cherchLineEditPrenom->setStyleSheet(cherchInputStyle);
    ui->cherchLineEditCIN->setStyleSheet(cherchInputStyle);
    ui->cherchLineEditEmail->setStyleSheet(cherchInputStyle);

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

    QString cherchLabelStyle = "color: #334155; font-size: 14px; font-weight: 600; background: transparent; border: none;";
    ui->cherchLabelNom->setStyleSheet(cherchLabelStyle);
    ui->cherchLabelPrenom->setStyleSheet(cherchLabelStyle);
    ui->cherchLabelCIN->setStyleSheet(cherchLabelStyle);
    ui->cherchLabelEmail->setStyleSheet(cherchLabelStyle);
    ui->cherchLabelGrade->setStyleSheet(cherchLabelStyle);
    ui->cherchLabelPhoto->setStyleSheet(cherchLabelStyle);

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

    ui->cherchLabelPhotoHint->setStyleSheet("color: #94a3b8; font-size: 12px; background: transparent; border: none;");

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

void SmartPub::cherchConnectSignals()
{
    connect(ui->cherchBtnLogin, &QPushButton::clicked, this, &SmartPub::on_cherchBtnLogin_clicked);
    connect(ui->cherchBtnMotDePasseOublie, &QPushButton::clicked, this, &SmartPub::on_cherchBtnMotDePasseOublie_clicked);
    connect(ui->cherchBtnRetourLogin, &QPushButton::clicked, this, &SmartPub::on_cherchBtnRetourLogin_clicked);
    connect(ui->cherchBtnForgotOk, &QPushButton::clicked, this, &SmartPub::on_cherchBtnForgotOk_clicked);

    ui->cherchUserProfileFrame->setCursor(Qt::PointingHandCursor);
    ui->cherchUserProfileFrame->installEventFilter(this);

    connect(ui->cherchBtnPublications, &QPushButton::clicked, this, &SmartPub::on_cherchBtnPublications_clicked);
    connect(ui->cherchBtnChercheurs, &QPushButton::clicked, this, &SmartPub::on_cherchBtnChercheurs_clicked);
    connect(ui->cherchBtnLaboratoires, &QPushButton::clicked, this, &SmartPub::on_cherchBtnLaboratoires_clicked);
    connect(ui->cherchBtnProjets, &QPushButton::clicked, this, &SmartPub::on_cherchBtnProjets_clicked);
    connect(ui->cherchBtnFinances, &QPushButton::clicked, this, &SmartPub::on_cherchBtnFinances_clicked);
    connect(ui->cherchBtnEvenements, &QPushButton::clicked, this, &SmartPub::on_cherchBtnEvenements_clicked);

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
    ui->cherchStackedWidgetMain->setCurrentIndex(0);
    ui->cherchStackedWidgetLogin->setCurrentIndex(0);
    cherchIsLoggedIn = false;
    ui->cherchLineEditLoginEmail->clear();
    ui->cherchLineEditLoginPassword->clear();
    ui->cherchLineEditLoginEmail->setFocus();
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
    cherchShowLoginView();
}

void SmartPub::on_cherchBtnForgotOk_clicked()
{
    QString email = ui->cherchLineEditForgotEmail->text();
    if (!email.isEmpty()) {
        QMessageBox::information(this, "Email envoyé",
                                 "Un email de récupération a été envoyé à " + email);
        cherchShowLoginView();
    } else {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer une adresse email valide.");
    }
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
    QString email = ui->cherchLineEditLoginEmail->text().trimmed();
    QString password = ui->cherchLineEditLoginPassword->text();

    if (email == "ressourcehumaine@gmail.com" && password == "0000") {
        cherchShowMainView();
    } else {
        QMessageBox::warning(this, "Erreur de connexion",
                             "Email ou mot de passe incorrect.\n\nVeuillez réessayer.");
        ui->cherchLineEditLoginPassword->clear();
    }
}

void SmartPub::on_cherchBtnLogin_clicked()
{
    cherchCheckLogin();
}

void SmartPub::on_cherchUserProfileFrame_clicked()
{
    auto reply = QMessageBox::question(this, "Déconnexion",
                                       "Voulez-vous vraiment vous déconnecter ?",
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        cherchShowLoginView();
    }
}

void SmartPub::on_cherchBtnToggleSidebar_clicked()
{
    cherchToggleSidebar();
}

void SmartPub::cherchToggleSidebar()
{
    cherchSidebarVisible = !cherchSidebarVisible;

    if (cherchSidebarVisible) {
        ui->cherchSidebarFrame->setMaximumWidth(260);
        ui->cherchSidebarFrame->setMinimumWidth(260);
        cherchBtnToggleSidebar->setText("☰");
    } else {
        ui->cherchSidebarFrame->setMaximumWidth(0);
        ui->cherchSidebarFrame->setMinimumWidth(0);
        cherchBtnToggleSidebar->setText("☰");
    }
}

void SmartPub::cherchSetupAnimations() {}

void SmartPub::cherchAnimateCardEntry(QWidget *card, int index)
{
    Q_UNUSED(index)
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

    card->installEventFilter(this);
    card->setMouseTracking(true);

    QGridLayout *grid = qobject_cast<QGridLayout*>(ui->cherchScrollAreaWidgetContents->layout());
    if (grid) {
        int count = grid->count();
        int row = count / 3;
        int col = count % 3;
        grid->addWidget(card, row, col, Qt::AlignTop);
        cherchAnimateCardEntry(card, count);
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

    item->installEventFilter(this);
    item->setMouseTracking(true);

    QVBoxLayout *list = qobject_cast<QVBoxLayout*>(ui->cherchScrollAreaWidgetContents->layout());
    if (list) {
        list->addWidget(item);
        cherchAnimateCardEntry(item, list->count());
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

void SmartPub::on_cherchBtnPublications_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_cherchBtnChercheurs_clicked()
{
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnLaboratoires_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Laboratoires");
}

void SmartPub::on_cherchBtnProjets_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Projets");
}

void SmartPub::on_cherchBtnFinances_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_cherchBtnEvenements_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(3);
    evAfficherListeEvents();
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
        cherchBtnToggleVue->setVisible(true);

        ui->cherchBtnVueListe->setStyleSheet(R"(
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
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
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

    QGridLayout *statsGrid = new QGridLayout();
    statsGrid->setSpacing(20);

    QFrame *cardTotal = new QFrame();
    cardTotal->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
    )");
    cardTotal->setMinimumHeight(140);
    QVBoxLayout *layoutTotal = new QVBoxLayout(cardTotal);
    layoutTotal->setSpacing(8);
    layoutTotal->setContentsMargins(24, 24, 24, 24);

    QLabel *labelTotalTitle = new QLabel("Total Chercheurs", cardTotal);
    labelTotalTitle->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; background: transparent; border: none;");
    QLabel *labelTotalValue = new QLabel(QString::number(cherchChercheursMap.size()), cardTotal);
    labelTotalValue->setStyleSheet("color: #3b82f6; font-size: 48px; font-weight: 700; background: transparent; border: none;");

    layoutTotal->addWidget(labelTotalTitle);
    layoutTotal->addWidget(labelTotalValue);
    layoutTotal->addStretch();
    statsGrid->addWidget(cardTotal, 0, 0);

    int nbProfs = 0;
    for (auto &data : cherchChercheursMap) {
        if (data.grade == "Professeur") nbProfs++;
    }
    QFrame *cardProfs = new QFrame();
    cardProfs->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
    )");
    cardProfs->setMinimumHeight(140);
    QVBoxLayout *layoutProfs = new QVBoxLayout(cardProfs);
    layoutProfs->setSpacing(8);
    layoutProfs->setContentsMargins(24, 24, 24, 24);

    QLabel *labelProfsTitle = new QLabel("Professeurs", cardProfs);
    labelProfsTitle->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; background: transparent; border: none;");
    QLabel *labelProfsValue = new QLabel(QString::number(nbProfs), cardProfs);
    labelProfsValue->setStyleSheet("color: #10b981; font-size: 48px; font-weight: 700; background: transparent; border: none;");

    layoutProfs->addWidget(labelProfsTitle);
    layoutProfs->addWidget(labelProfsValue);
    layoutProfs->addStretch();
    statsGrid->addWidget(cardProfs, 0, 1);

    int nbDocs = 0;
    for (auto &data : cherchChercheursMap) {
        if (data.grade == "Doctorant") nbDocs++;
    }
    QFrame *cardDocs = new QFrame();
    cardDocs->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
    )");
    cardDocs->setMinimumHeight(140);
    QVBoxLayout *layoutDocs = new QVBoxLayout(cardDocs);
    layoutDocs->setSpacing(8);
    layoutDocs->setContentsMargins(24, 24, 24, 24);

    QLabel *labelDocsTitle = new QLabel("Doctorants", cardDocs);
    labelDocsTitle->setStyleSheet("color: #64748b; font-size: 14px; font-weight: 600; background: transparent; border: none;");
    QLabel *labelDocsValue = new QLabel(QString::number(nbDocs), cardDocs);
    labelDocsValue->setStyleSheet("color: #f59e0b; font-size: 48px; font-weight: 700; background: transparent; border: none;");

    layoutDocs->addWidget(labelDocsTitle);
    layoutDocs->addWidget(labelDocsValue);
    layoutDocs->addStretch();
    statsGrid->addWidget(cardDocs, 0, 2);

    contentLayout->addLayout(statsGrid);

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
        gradeLabel->setStyleSheet("font-size: 16px; color: #334155; font-weight: 600;");
        QLabel *countLabel = new QLabel(QString::number(it.value()));
        countLabel->setStyleSheet("font-size: 16px; color: #3b82f6; font-weight: 700;");
        row->addWidget(gradeLabel);
        row->addWidget(countLabel);
        row->addStretch();
        chartLayout->addLayout(row);
    }

    chartLayout->addStretch();
    contentLayout->addWidget(chartFrame);

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
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 48px;
            font-size: 16px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563eb, stop:1 #059669);
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
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 14px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563eb, stop:1 #059669);
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
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
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

    auto createInfoRow = [&](const QString &label, const QString &value, const QString &icon = "") {
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
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 32px;
            font-size: 14px;
            font-weight: 600;
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
// === MODULE PUBLICATIONS (Code de votre ami avec préfixe SR) ===
// ============================================================================

void SmartPub::SR_setupUI()
{
    // Configuration initiale du module publications
    ui->SR_stackedWidget->setCurrentIndex(0);
}

void SmartPub::SR_connectSignals()
{
    // Connexions des boutons de navigation du stacked widget
    connect(ui->SR_btnVueListe, &QPushButton::clicked, this, &SmartPub::on_SR_btnVueListe_clicked);
    connect(ui->SR_btnAjouter, &QPushButton::clicked, this, &SmartPub::on_SR_btnAjouter_clicked);
    connect(ui->SR_btnSupprimer, &QPushButton::clicked, this, &SmartPub::on_SR_btnSupprimer_clicked);
    connect(ui->SR_btnRecherche, &QPushButton::clicked, this, &SmartPub::on_SR_btnRecherche_clicked);
    connect(ui->SR_btnTri, &QPushButton::clicked, this, &SmartPub::on_SR_btnTri_clicked);
    connect(ui->SR_btnExport, &QPushButton::clicked, this, &SmartPub::on_SR_btnExport_clicked);
    connect(ui->SR_btnStatistiques, &QPushButton::clicked, this, &SmartPub::on_SR_btnStatistiques_clicked);
    connect(ui->SR_btnAjouterPublication, &QPushButton::clicked, this, &SmartPub::on_SR_btnAjouterPublication_clicked);
    connect(ui->SR_btnAnnulerAjout, &QPushButton::clicked, this, &SmartPub::on_SR_btnAnnulerAjout_clicked);

    // Connexions des boutons du sidebar pour navigation entre modules
    connect(ui->SR_btnPublications, &QPushButton::clicked, this, &SmartPub::on_SR_btnPublications_clicked);
    connect(ui->SR_btnChercheurs, &QPushButton::clicked, this, &SmartPub::on_SR_btnChercheurs_clicked);
    connect(ui->SR_btnLaboratoires, &QPushButton::clicked, this, &SmartPub::on_SR_btnLaboratoires_clicked);
    connect(ui->SR_btnProjets, &QPushButton::clicked, this, &SmartPub::on_SR_btnProjets_clicked);
    connect(ui->SR_btnFinances, &QPushButton::clicked, this, &SmartPub::on_SR_btnFinances_clicked);
    connect(ui->SR_btnEvenements, &QPushButton::clicked, this, &SmartPub::on_SR_btnEvenements_clicked);
}

void SmartPub::SR_updateButtonStyles()
{
    // Style pour le bouton actif (Liste)
    QString activeStyle = "QPushButton {"
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                          "stop:0 #3b82f6, stop:1 #10b981);"
                          "color: white;"
                          "border: none;"
                          "border-radius: 8px;"
                          "padding: 8px 16px;"
                          "font-size: 13px;"
                          "font-weight: 600;"
                          "}"
                          "QPushButton:hover {"
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                          "stop:0 #2563eb, stop:1 #059669);"
                          "}";

    // Style pour le bouton inactif (Ajouter)
    QString inactiveStyle = "QPushButton {"
                            "background-color: transparent;"
                            "color: #64748b;"
                            "border: none;"
                            "border-radius: 8px;"
                            "padding: 8px 16px;"
                            "font-size: 13px;"
                            "font-weight: 500;"
                            "}"
                            "QPushButton:hover {"
                            "background-color: #f1f5f9;"
                            "color: #334155;"
                            "}";

    // Style pour le bouton Supprimer
    QString deleteStyle = "QPushButton {"
                          "background-color: transparent;"
                          "color: #ef4444;"
                          "border: none;"
                          "border-radius: 8px;"
                          "padding: 8px 16px;"
                          "font-size: 13px;"
                          "font-weight: 500;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: #fef2f2;"
                          "color: #dc2626;"
                          "}";

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
    // Données d'exemple pour la table
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

    // Mise à jour des statistiques
    ui->SR_lblTotalNumber->setText(QString::number(titres.size()));
    ui->SR_lblThisYearNumber->setText("5");
    ui->SR_lblPlanSNumber->setText("3");

    // Calculer et mettre à jour les pourcentages des statuts
    int publie = 3, soumis = 2, revision = 2, accepte = 1;
    int total = titres.size();

    ui->SR_lblStatPublie->setText(QString("● Publié (%1%)").arg((publie * 100) / total));
    ui->SR_lblStatSoumis->setText(QString("● Soumis (%1%)").arg((soumis * 100) / total));
    ui->SR_lblStatRevision->setText(QString("● En révision (%1%)").arg((revision * 100) / total));
    ui->SR_lblStatAccepte->setText(QString("● Accepté (%1%)").arg((accepte * 100) / total));
}

// Slots pour la navigation dans le stacked widget
void SmartPub::on_SR_btnVueListe_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnAjouter_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnSupprimer_clicked()
{
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
    // Récupérer les valeurs (sans traitement réel)
    QString titre = ui->SR_lineEditTitre->text();
    QString auteurs = ui->SR_lineEditAuteurs->text();
    QString revue = ui->SR_lineEditRevue->text();
    QString statut = ui->SR_comboBoxStatut->currentText();

    if (titre.isEmpty() || auteurs.isEmpty() || revue.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires");
        return;
    }

    QMessageBox::information(this, "Succès", "Publication ajoutée avec succès (simulation)");

    // Retour à la liste
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();

    // Vider les champs
    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
}

void SmartPub::on_SR_btnAnnulerAjout_clicked()
{
    // Retour à la liste sans sauvegarder
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

// Slots pour le sidebar - Navigation entre modules
void SmartPub::on_SR_btnPublications_clicked()
{
    // Déjà sur la page publications
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::on_SR_btnChercheurs_clicked()
{
    // Navigation vers le module chercheurs
    ui->stackedWidgetModules->setCurrentIndex(0);
    if (cherchIsLoggedIn) {
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_SR_btnLaboratoires_clicked()
{
    QMessageBox::information(this, "Information", "Module Laboratoires non implémenté");
}

void SmartPub::on_SR_btnProjets_clicked()
{
    QMessageBox::information(this, "Information", "Module Projets non implémenté");
}

void SmartPub::on_SR_btnFinances_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_SR_btnEvenements_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(3);
    evAfficherListeEvents();
}

// ============================================================================
// === MODULE FINANCES (préfixe fin) ===
// ============================================================================

void SmartPub::finSetupUI()
{
    // Configuration initiale du module finances
    ui->finStackedWidget->setCurrentIndex(0);
    finVueListeActive = true;

    // Ajouter des projets au comboBox
    ui->finComboBoxProjet->addItems({
        "Projet AI-2024-001",
        "Projet Quantum-2024-002",
        "Projet BioTech-2024-003",
        "Projet CyberSec-2024-004"
    });
}

void SmartPub::finConnectSignals()
{
    // Connexions des boutons de navigation du stacked widget
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

    // Connexions des boutons du sidebar pour navigation entre modules
    connect(ui->finBtnPublications, &QPushButton::clicked, this, &SmartPub::on_finBtnPublications_clicked);
    connect(ui->finBtnChercheurs, &QPushButton::clicked, this, &SmartPub::on_finBtnChercheurs_clicked);
    connect(ui->finBtnLaboratoires, &QPushButton::clicked, this, &SmartPub::on_finBtnLaboratoires_clicked);
    connect(ui->finBtnProjets, &QPushButton::clicked, this, &SmartPub::on_finBtnProjets_clicked);
    connect(ui->finBtnFinances, &QPushButton::clicked, this, &SmartPub::on_finBtnFinances_clicked);
    connect(ui->finBtnEvenements, &QPushButton::clicked, this, &SmartPub::on_finBtnEvenements_clicked);
}

void SmartPub::finUpdateButtonStyles()
{
    // Style pour le bouton actif (Liste)
    QString activeStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b9cff, stop:1 #2dd4bf);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2b8cef, stop:1 #1dc4af);
        }
    )";

    // Style pour le bouton inactif (Ajouter)
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
    // Données de test pour les transactions
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

// Slots pour la navigation dans le stacked widget
void SmartPub::on_finBtnVueListe_clicked()
{
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnAjouter_clicked()
{
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

    // Retour à la liste
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();

    // Vider les champs
    ui->finLineEditMontant->clear();
    ui->finTextEditDescription->clear();
}

void SmartPub::on_finBtnAnnulerAjout_clicked()
{
    // Retour à la liste sans sauvegarder
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnModifierTransaction_clicked()
{
    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une transaction à modifier");
        return;
    }
    QMessageBox::information(this, "Modifier", "Fonctionnalité de modification - À implémenter");
}

void SmartPub::on_finBtnSupprimerTransaction_clicked()
{
    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner une transaction à supprimer");
        return;
    }

    auto reply = QMessageBox::question(this, "Supprimer", "Confirmer la suppression de cette transaction ?");
    if (reply == QMessageBox::Yes) {
        // Supprimer la transaction (simplifié)
        QMessageBox::information(this, "Succès", "Transaction supprimée");
        ui->finTableTransactions->removeRow(currentRow);
    }
}

// Slots pour le sidebar - Navigation entre modules
void SmartPub::on_finBtnPublications_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_finBtnChercheurs_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(0);
    if (cherchIsLoggedIn) {
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_finBtnLaboratoires_clicked()
{
    QMessageBox::information(this, "Information", "Module Laboratoires non implémenté");
}

void SmartPub::on_finBtnProjets_clicked()
{
    QMessageBox::information(this, "Information", "Module Projets non implémenté");
}

void SmartPub::on_finBtnFinances_clicked()
{
    // Déjà sur la page finances
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnEvenements_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(3);
    evAfficherListeEvents();
}

// ============================================================================
// === MODULE EVENEMENTS (préfixe ev) ===
// ============================================================================

void SmartPub::evSetupUI()
{
    // Configuration initiale du module evenements
    ui->evTabWidget->setCurrentIndex(0);
    evEventSelectionne = -1;
}

void SmartPub::evConnectSignals()
{
    // Connexions des boutons d'actions
    connect(ui->evBtnAjouterEvent, &QPushButton::clicked, this, &SmartPub::on_evBtnAjouterEvent_clicked);
    connect(ui->evBtnModifierEvent, &QPushButton::clicked, this, &SmartPub::on_evBtnModifierEvent_clicked);
    connect(ui->evBtnSupprimerEvent, &QPushButton::clicked, this, &SmartPub::on_evBtnSupprimerEvent_clicked);

    // Connexions des outils de recherche
    connect(ui->evBtnTrierDate, &QPushButton::clicked, this, &SmartPub::on_evBtnTrierDate_clicked);
    connect(ui->evBtnRechercheLieu, &QPushButton::clicked, this, &SmartPub::on_evBtnRechercheLieu_clicked);

    // Connexions des actions avancées
    connect(ui->evBtnExportCalendrier, &QPushButton::clicked, this, &SmartPub::on_evBtnExportCalendrier_clicked);
    connect(ui->evBtnLivreResumes, &QPushButton::clicked, this, &SmartPub::on_evBtnLivreResumes_clicked);
    connect(ui->evBtnCalculImpact, &QPushButton::clicked, this, &SmartPub::on_evBtnCalculImpact_clicked);
    connect(ui->evBtnStatsParticipation, &QPushButton::clicked, this, &SmartPub::on_evBtnStatsParticipation_clicked);

    // Connexions des boutons du sidebar pour navigation entre modules
    connect(ui->evBtnPublications, &QPushButton::clicked, this, &SmartPub::on_evBtnPublications_clicked);
    connect(ui->evBtnChercheurs, &QPushButton::clicked, this, &SmartPub::on_evBtnChercheurs_clicked);
    connect(ui->evBtnLaboratoires, &QPushButton::clicked, this, &SmartPub::on_evBtnLaboratoires_clicked);
    connect(ui->evBtnProjets, &QPushButton::clicked, this, &SmartPub::on_evBtnProjets_clicked);
    connect(ui->evBtnFinances, &QPushButton::clicked, this, &SmartPub::on_evBtnFinances_clicked);
    connect(ui->evBtnEvenements, &QPushButton::clicked, this, &SmartPub::on_evBtnEvenements_clicked);
}

void SmartPub::evAjouterDonneesTest()
{
    // Données de test pour les événements
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

    // Copier dans la table de recherche aussi
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

// Slots pour les actions des événements
void SmartPub::on_evBtnAjouterEvent_clicked()
{
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

    // Vider les champs
    ui->evLineEditID->clear();
    ui->evLineEditNom->clear();
    ui->evLineEditLieu->clear();
    ui->evLineEditDate->clear();
}

void SmartPub::on_evBtnModifierEvent_clicked()
{
    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un événement à modifier");
        return;
    }
    QMessageBox::information(this, "Modifier", "Fonctionnalité de modification - À implémenter");
}

void SmartPub::on_evBtnSupprimerEvent_clicked()
{
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

// Slots pour le sidebar - Navigation entre modules
void SmartPub::on_evBtnPublications_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::on_evBtnChercheurs_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(0);
    if (cherchIsLoggedIn) {
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_evBtnLaboratoires_clicked()
{
    QMessageBox::information(this, "Information", "Module Laboratoires non implémenté");
}

void SmartPub::on_evBtnProjets_clicked()
{
    QMessageBox::information(this, "Information", "Module Projets non implémenté");
}

void SmartPub::on_evBtnFinances_clicked()
{
    ui->stackedWidgetModules->setCurrentIndex(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_evBtnEvenements_clicked()
{
    // Déjà sur la page evenements
    evAfficherListeEvents();
}

// ============================================================================
// === EVENT FILTER ===
// ============================================================================

bool SmartPub::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->cherchUserProfileFrame) {
        if (event->type() == QEvent::MouseButtonRelease) {
            on_cherchUserProfileFrame_clicked();
            return true;
        }
    }

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

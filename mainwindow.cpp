// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , cherchUi(new Ui::MainWindow)
    , cherchVueListeActive(true)
    , cherchVueIconesActive(true)  // Par défaut en mode icônes
    , cherchChercheurSelectionne(-1)
    , cherchIsLoggedIn(false)
    , cherchSidebarVisible(true)
{
    cherchUi->setupUi(this);
    cherchSetupUI();
    cherchConnectSignals();
    cherchSetupAnimations();
    cherchApplyModernStyle();
    cherchSetupSidebarToggle();

    // Initialiser les données de test enrichies
    cherchAjouterDonneesTest();

    // Afficher la vue login au démarrage
    cherchShowLoginView();
}

MainWindow::~MainWindow()
{
    delete cherchUi;
}

void MainWindow::cherchSetupUI()
{
    setWindowTitle("Smart Research - Gestion des Chercheurs");
    showMaximized();
    cherchUi->cherchStackedWidgetMain->setCurrentIndex(0);

    // Configuration du bouton toggle vue
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

    cherchUi->horizontalLayoutToolbar->insertWidget(1, cherchBtnToggleVue);

    connect(cherchBtnToggleVue, &QPushButton::clicked, this, &MainWindow::on_cherchBtnToggleVue_clicked);
}

void MainWindow::cherchSetupSidebarToggle()
{
    // Créer le bouton toggle pour la sidebar
    cherchBtnToggleSidebar = new QPushButton(this);
    cherchBtnToggleSidebar->setObjectName("cherchBtnToggleSidebar");
    cherchBtnToggleSidebar->setFixedSize(40, 40);
    cherchBtnToggleSidebar->setCursor(Qt::PointingHandCursor);
    cherchBtnToggleSidebar->setText("☰");
    cherchBtnToggleSidebar->setToolTip("Masquer/Afficher la sidebar");

    // Style du bouton toggle
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

    // Positionner dans le header
    cherchUi->horizontalLayoutHeader->insertWidget(0, cherchBtnToggleSidebar);

    connect(cherchBtnToggleSidebar, &QPushButton::clicked, this, &MainWindow::on_cherchBtnToggleSidebar_clicked);
}

void MainWindow::cherchApplyModernStyle()
{
    // Style global
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f8fafc;
        }
        QWidget {
            font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
        }
    )");

    // === LOGIN VIEW STYLES ===
    cherchUi->cherchLoginFrame->setStyleSheet(R"(
        QFrame#cherchLoginFrame {
            background-color: white;
            border-radius: 20px;
            border: 1px solid #e2e8f0;
        }
    )");

    cherchUi->cherchLoginHeader->setStyleSheet(R"(
        QFrame#cherchLoginHeader {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            border-top-left-radius: 20px;
            border-top-right-radius: 20px;
        }
    )");

    // Style pour la page mot de passe oublié
    cherchUi->cherchForgotFrame->setStyleSheet(R"(
        QFrame#cherchForgotFrame {
            background-color: white;
            border-radius: 20px;
            border: 1px solid #e2e8f0;
        }
    )");

    cherchUi->cherchForgotHeader->setStyleSheet(R"(
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

    cherchUi->cherchLineEditLoginEmail->setStyleSheet(cherchLoginInputStyle);
    cherchUi->cherchLineEditLoginPassword->setStyleSheet(cherchLoginInputStyle);
    cherchUi->cherchLineEditLoginPassword->setEchoMode(QLineEdit::Password);

    // Style pour le champ email de récupération
    cherchUi->cherchLineEditForgotEmail->setStyleSheet(cherchLoginInputStyle);

    cherchUi->cherchBtnLogin->setStyleSheet(R"(
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

    // Style pour le bouton OK de récupération
    cherchUi->cherchBtnForgotOk->setStyleSheet(R"(
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

    // Style pour le lien mot de passe oublié
    cherchUi->cherchBtnMotDePasseOublie->setStyleSheet(R"(
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

    // Style pour le bouton retour
    cherchUi->cherchBtnRetourLogin->setStyleSheet(R"(
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

    // === SIDEBAR STYLES ===
    cherchUi->cherchSidebarFrame->setStyleSheet("background-color: #0f172a; border: none;");
    cherchUi->cherchLogoFrame->setStyleSheet("background-color: #020617; border: none;");

    // User Profile Frame
    cherchUi->cherchUserProfileFrame->setStyleSheet(R"(
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

    cherchUi->cherchUserAvatar->setStyleSheet(R"(
        QLabel#cherchUserAvatar {
            background-color: #10b981;
            border-radius: 20px;
            color: white;
            font-weight: bold;
            font-size: 16px;
        }
    )");

    cherchUi->cherchUserName->setStyleSheet("color: white; font-size: 14px; font-weight: 600;");
    cherchUi->cherchUserRole->setStyleSheet("color: #94a3b8; font-size: 12px;");

    // Navigation buttons
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

    cherchUi->cherchBtnPublications->setStyleSheet(cherchNavStyle);
    cherchUi->cherchBtnChercheurs->setStyleSheet(cherchNavStyle);
    cherchUi->cherchBtnLaboratoires->setStyleSheet(cherchNavStyle);
    cherchUi->cherchBtnProjets->setStyleSheet(cherchNavStyle);
    cherchUi->cherchBtnFinances->setStyleSheet(cherchNavStyle);
    cherchUi->cherchBtnEvenements->setStyleSheet(cherchNavStyle);

    // Header
    cherchUi->cherchHeaderFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-bottom: 1px solid #e2e8f0;
        }
    )");

    cherchUi->cherchTitleLabel->setStyleSheet("color: #1e293b; font-size: 24px; font-weight: 700; background: transparent; border: none;");
    cherchUi->cherchSubtitleLabel->setStyleSheet("color: #64748b; font-size: 13px; background: transparent; border: none;");

    // Toolbar
    cherchUi->cherchToolbarFrame->setStyleSheet("background-color: transparent; border: none;");

    // Tabs frame
    cherchUi->cherchTabsFrame->setStyleSheet(R"(
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

    cherchUi->cherchBtnVueListe->setStyleSheet(cherchTabActive);
    cherchUi->cherchBtnAjouter->setStyleSheet(cherchTabInactive);

    // Toolbar buttons
    cherchUi->cherchBtnRecherche->setStyleSheet(R"(
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

    cherchUi->cherchBtnTri->setStyleSheet(cherchSecondaryBtn);
    cherchUi->cherchBtnExport->setStyleSheet(cherchSecondaryBtn);

    cherchUi->cherchBtnStatistiques->setStyleSheet(R"(
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

    // Search field
    cherchUi->cherchLineEditRecherche->setStyleSheet(R"(
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

    // Form frame
    cherchUi->cherchFormFrame->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 20px;
            border: 1px solid #e2e8f0;
        }
    )");

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

    cherchUi->cherchLineEditNom->setStyleSheet(cherchInputStyle);
    cherchUi->cherchLineEditPrenom->setStyleSheet(cherchInputStyle);
    cherchUi->cherchLineEditCIN->setStyleSheet(cherchInputStyle);
    cherchUi->cherchLineEditEmail->setStyleSheet(cherchInputStyle);

    // Combo box - CORRECTION DU STYLE POUR VISIBILITÉ
    cherchUi->cherchComboBoxGrade->setStyleSheet(R"(
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

    // Form labels
    QString cherchLabelStyle = "color: #334155; font-size: 14px; font-weight: 600; background: transparent; border: none;";
    cherchUi->cherchLabelNom->setStyleSheet(cherchLabelStyle);
    cherchUi->cherchLabelPrenom->setStyleSheet(cherchLabelStyle);
    cherchUi->cherchLabelCIN->setStyleSheet(cherchLabelStyle);
    cherchUi->cherchLabelEmail->setStyleSheet(cherchLabelStyle);
    cherchUi->cherchLabelGrade->setStyleSheet(cherchLabelStyle);
    cherchUi->cherchLabelPhoto->setStyleSheet(cherchLabelStyle);

    // Form buttons
    cherchUi->cherchBtnAnnulerAjout->setStyleSheet(R"(
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

    cherchUi->cherchBtnAjouterChercheur->setStyleSheet(R"(
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

    // Upload photo button
    cherchUi->cherchBtnUploadPhoto->setStyleSheet(R"(
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

    cherchUi->cherchLabelPhotoHint->setStyleSheet("color: #94a3b8; font-size: 12px; background: transparent; border: none;");

    // ScrollArea
    cherchUi->cherchCardsScrollArea->setStyleSheet(R"(
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

void MainWindow::cherchConnectSignals()
{
    // Login
    connect(cherchUi->cherchBtnLogin, &QPushButton::clicked, this, &MainWindow::on_cherchBtnLogin_clicked);
    connect(cherchUi->cherchBtnMotDePasseOublie, &QPushButton::clicked, this, &MainWindow::on_cherchBtnMotDePasseOublie_clicked);
    connect(cherchUi->cherchBtnRetourLogin, &QPushButton::clicked, this, &MainWindow::on_cherchBtnRetourLogin_clicked);
    connect(cherchUi->cherchBtnForgotOk, &QPushButton::clicked, this, &MainWindow::on_cherchBtnForgotOk_clicked);

    // Profile frame clickable
    cherchUi->cherchUserProfileFrame->setCursor(Qt::PointingHandCursor);
    cherchUi->cherchUserProfileFrame->installEventFilter(this);

    // Navigation
    connect(cherchUi->cherchBtnPublications, &QPushButton::clicked, this, &MainWindow::on_cherchBtnPublications_clicked);
    connect(cherchUi->cherchBtnChercheurs, &QPushButton::clicked, this, &MainWindow::on_cherchBtnChercheurs_clicked);
    connect(cherchUi->cherchBtnLaboratoires, &QPushButton::clicked, this, &MainWindow::on_cherchBtnLaboratoires_clicked);
    connect(cherchUi->cherchBtnProjets, &QPushButton::clicked, this, &MainWindow::on_cherchBtnProjets_clicked);
    connect(cherchUi->cherchBtnFinances, &QPushButton::clicked, this, &MainWindow::on_cherchBtnFinances_clicked);
    connect(cherchUi->cherchBtnEvenements, &QPushButton::clicked, this, &MainWindow::on_cherchBtnEvenements_clicked);

    // Vue switching
    connect(cherchUi->cherchBtnVueListe, &QPushButton::clicked, this, &MainWindow::on_cherchBtnVueListe_clicked);
    connect(cherchUi->cherchBtnAjouter, &QPushButton::clicked, this, &MainWindow::on_cherchBtnAjouter_clicked);

    // Actions
    connect(cherchUi->cherchBtnRecherche, &QPushButton::clicked, this, &MainWindow::on_cherchBtnRecherche_clicked);
    connect(cherchUi->cherchBtnTri, &QPushButton::clicked, this, &MainWindow::on_cherchBtnTri_clicked);
    connect(cherchUi->cherchBtnExport, &QPushButton::clicked, this, &MainWindow::on_cherchBtnExport_clicked);
    connect(cherchUi->cherchBtnStatistiques, &QPushButton::clicked, this, &MainWindow::on_cherchBtnStatistiques_clicked);
    connect(cherchUi->cherchBtnUploadPhoto, &QPushButton::clicked, this, &MainWindow::on_cherchBtnUploadPhoto_clicked);

    // Form
    connect(cherchUi->cherchBtnAjouterChercheur, &QPushButton::clicked, this, &MainWindow::on_cherchBtnAjouterChercheur_clicked);
    connect(cherchUi->cherchBtnAnnulerAjout, &QPushButton::clicked, this, &MainWindow::on_cherchBtnAnnulerAjout_clicked);

    // Search
    connect(cherchUi->cherchLineEditRecherche, &QLineEdit::textChanged, this, &MainWindow::on_cherchLineEditRecherche_textChanged);
}

void MainWindow::cherchAjouterDonneesTest()
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

        // Simuler des projets
        int nbProjets = (i % 4) + 1;
        for (int p = 0; p < nbProjets; ++p) {
            data.projetsIds.append(p + 1);
        }

        data.carriere = cherchDeterminerCarriere(nbProjets, data.grade);
        data.photoPath = ":/avatar.png";

        cherchChercheursMap[i + 1] = data;
    }
}

QString MainWindow::cherchDeterminerCarriere(int projetsCount, const QString &grade)
{
    if (projetsCount >= 4) return "Senior - Expert";
    if (projetsCount >= 2) return "Confirmé";
    if (grade == "Professeur" || grade == "Maitre de Conferences") return "Senior";
    return "Junior";
}

void MainWindow::cherchShowLoginView()
{
    cherchUi->cherchStackedWidgetLogin->setCurrentIndex(0);  // Page login normal
    cherchUi->cherchStackedWidgetMain->setCurrentIndex(0);
    cherchIsLoggedIn = false;
    cherchUi->cherchLineEditLoginEmail->clear();
    cherchUi->cherchLineEditLoginPassword->clear();
    cherchUi->cherchLineEditLoginEmail->setFocus();
}

void MainWindow::cherchShowForgotPasswordView()
{
    cherchUi->cherchStackedWidgetLogin->setCurrentIndex(1);  // Page mot de passe oublié
    cherchUi->cherchLineEditForgotEmail->clear();
    cherchUi->cherchLineEditForgotEmail->setFocus();
}

void MainWindow::on_cherchBtnMotDePasseOublie_clicked()
{
    cherchShowForgotPasswordView();
}

void MainWindow::on_cherchBtnRetourLogin_clicked()
{
    cherchShowLoginView();
}

void MainWindow::on_cherchBtnForgotOk_clicked()
{
    QString email = cherchUi->cherchLineEditForgotEmail->text();
    if (!email.isEmpty()) {
        QMessageBox::information(this, "Email envoyé",
                                 "Un email de récupération a été envoyé à " + email);
        cherchShowLoginView();
    } else {
        QMessageBox::warning(this, "Erreur", "Veuillez entrer une adresse email valide.");
    }
}

void MainWindow::cherchShowMainView()
{
    cherchUi->cherchStackedWidgetMain->setCurrentIndex(1);
    cherchIsLoggedIn = true;
    cherchVueListeActive = true;
    cherchUi->cherchStackedWidget->setCurrentIndex(0);
    cherchAfficherListeChercheurs();
}

void MainWindow::cherchCheckLogin()
{
    QString email = cherchUi->cherchLineEditLoginEmail->text().trimmed();
    QString password = cherchUi->cherchLineEditLoginPassword->text();

    if (email == "ressourcehumaine@gmail.com" && password == "0000") {
        cherchShowMainView();
    } else {
        QMessageBox::warning(this, "Erreur de connexion",
                             "Email ou mot de passe incorrect.\n\nVeuillez réessayer.");
        cherchUi->cherchLineEditLoginPassword->clear();
    }
}

void MainWindow::on_cherchBtnLogin_clicked()
{
    cherchCheckLogin();
}

void MainWindow::on_cherchUserProfileFrame_clicked()
{
    auto reply = QMessageBox::question(this, "Déconnexion",
                                       "Voulez-vous vraiment vous déconnecter ?",
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        cherchShowLoginView();
    }
}

void MainWindow::on_cherchBtnToggleSidebar_clicked()
{
    cherchToggleSidebar();
}

void MainWindow::cherchToggleSidebar()
{
    cherchSidebarVisible = !cherchSidebarVisible;

    if (cherchSidebarVisible) {
        cherchUi->cherchSidebarFrame->setMaximumWidth(260);
        cherchUi->cherchSidebarFrame->setMinimumWidth(260);
        cherchBtnToggleSidebar->setText("☰");
    } else {
        cherchUi->cherchSidebarFrame->setMaximumWidth(0);
        cherchUi->cherchSidebarFrame->setMinimumWidth(0);
        cherchBtnToggleSidebar->setText("☰");
    }
}

void MainWindow::cherchSetupAnimations()
{
    // Animations futures
}

void MainWindow::cherchAnimateCardEntry(QWidget *card, int index)
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

void MainWindow::cherchAfficherListeChercheurs()
{
    cherchClearChercheursList();

    if (cherchVueIconesActive) {
        // Mode icônes (grille)
        QGridLayout *gridLayout = qobject_cast<QGridLayout*>(cherchUi->cherchScrollAreaWidgetContents->layout());
        if (!gridLayout) {
            // Supprimer l'ancien layout s'il existe
            QLayout *oldLayout = cherchUi->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            gridLayout = new QGridLayout(cherchUi->cherchScrollAreaWidgetContents);
            gridLayout->setSpacing(24);
            gridLayout->setContentsMargins(24, 24, 24, 24);
        }

        int count = 0;
        for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
            int id = it.key();
            auto data = it.value();

            QPixmap photo;
            cherchAjouterChercheurCard(id, data.nom, data.prenom,
                                       data.grade, data.email, photo);
            count++;
        }
    } else {
        // Mode liste
        QVBoxLayout *listLayout = qobject_cast<QVBoxLayout*>(cherchUi->cherchScrollAreaWidgetContents->layout());
        if (!listLayout) {
            // Supprimer l'ancien layout s'il existe
            QLayout *oldLayout = cherchUi->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            listLayout = new QVBoxLayout(cherchUi->cherchScrollAreaWidgetContents);
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

    qDebug() << "Nombre de chercheurs affichés:" << cherchChercheursMap.size();
}

void MainWindow::cherchAjouterChercheurCard(int id, const QString &nom, const QString &prenom,
                                            const QString &grade, const QString &email,
                                            const QPixmap &photo)
{
    Q_UNUSED(photo)

    QFrame *card = new QFrame(cherchUi->cherchScrollAreaWidgetContents);
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

    // Avatar avec photo par défaut
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

    // Informations
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

    // Boutons d'action
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

    QGridLayout *grid = qobject_cast<QGridLayout*>(cherchUi->cherchScrollAreaWidgetContents->layout());
    if (grid) {
        int count = grid->count();
        int row = count / 3;
        int col = count % 3;
        grid->addWidget(card, row, col, Qt::AlignTop);
        cherchAnimateCardEntry(card, count);
    }
}

void MainWindow::cherchAjouterChercheurListItem(int id, const QString &nom, const QString &prenom,
                                                const QString &grade, const QString &email)
{
    QFrame *item = new QFrame(cherchUi->cherchScrollAreaWidgetContents);
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

    // Avatar miniature
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

    // Nom et prénom
    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom));
    nameLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #1e293b; background: transparent; border: none;");
    nameLabel->setFixedWidth(200);
    mainLayout->addWidget(nameLabel);

    // Grade
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

    // Email
    QLabel *emailLabel = new QLabel(email);
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: transparent; border: none;");
    mainLayout->addWidget(emailLabel, 1);

    // Boutons d'action
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

    QVBoxLayout *list = qobject_cast<QVBoxLayout*>(cherchUi->cherchScrollAreaWidgetContents->layout());
    if (list) {
        list->addWidget(item);
        cherchAnimateCardEntry(item, list->count());
    }
}

void MainWindow::cherchClearChercheursList()
{
    QLayoutItem *child;
    QLayout *layout = cherchUi->cherchScrollAreaWidgetContents->layout();
    if (!layout) return;

    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

// === SLOTS IMPLEMENTATION ===

void MainWindow::on_cherchBtnPublications_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Publications");
}

void MainWindow::on_cherchBtnChercheurs_clicked()
{
    cherchAfficherListeChercheurs();
}

void MainWindow::on_cherchBtnLaboratoires_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Laboratoires");
}

void MainWindow::on_cherchBtnProjets_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Projets");
}

void MainWindow::on_cherchBtnFinances_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Finances");
}

void MainWindow::on_cherchBtnEvenements_clicked()
{
    QMessageBox::information(this, "Navigation", "Module Événements");
}

void MainWindow::on_cherchBtnVueListe_clicked()
{
    if (!cherchVueListeActive) {
        cherchUi->cherchStackedWidget->setCurrentIndex(0);

        cherchUi->cherchLineEditRecherche->setVisible(true);
        cherchUi->cherchBtnRecherche->setVisible(true);
        cherchUi->cherchBtnTri->setVisible(true);
        cherchUi->cherchBtnExport->setVisible(true);
        cherchUi->cherchBtnStatistiques->setVisible(true);
        cherchBtnToggleVue->setVisible(true);

        cherchUi->cherchBtnVueListe->setStyleSheet(R"(
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
        cherchUi->cherchBtnAjouter->setStyleSheet(R"(
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

        cherchUi->cherchBtnVueListe->setChecked(true);
        cherchUi->cherchBtnAjouter->setChecked(false);
        cherchVueListeActive = true;

        cherchAfficherListeChercheurs();
    }
}

void MainWindow::on_cherchBtnAjouter_clicked()
{
    if (cherchVueListeActive) {
        cherchUi->cherchStackedWidget->setCurrentIndex(1);

        cherchUi->cherchLineEditRecherche->setVisible(false);
        cherchUi->cherchBtnRecherche->setVisible(false);
        cherchUi->cherchBtnTri->setVisible(false);
        cherchUi->cherchBtnExport->setVisible(false);
        cherchUi->cherchBtnStatistiques->setVisible(false);
        cherchBtnToggleVue->setVisible(false);

        cherchUi->cherchBtnVueListe->setStyleSheet(R"(
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
        cherchUi->cherchBtnAjouter->setStyleSheet(R"(
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

        cherchUi->cherchBtnVueListe->setChecked(false);
        cherchUi->cherchBtnAjouter->setChecked(true);
        cherchVueListeActive = false;
    }
}

void MainWindow::on_cherchBtnToggleVue_clicked()
{
    cherchVueIconesActive = !cherchVueIconesActive;

    if (cherchVueIconesActive) {
        cherchBtnToggleVue->setText("⊞");  // Icône grille
        cherchBtnToggleVue->setToolTip("Passer en mode liste");
    } else {
        cherchBtnToggleVue->setText("☰");  // Icône liste
        cherchBtnToggleVue->setToolTip("Passer en mode icônes");
    }

    cherchAfficherListeChercheurs();
}

void MainWindow::on_cherchBtnRecherche_clicked()
{
    QString searchText = cherchUi->cherchLineEditRecherche->text().toLower();

    if (searchText.isEmpty()) {
        cherchAfficherListeChercheurs();
        return;
    }

    cherchClearChercheursList();

    if (cherchVueIconesActive) {
        QGridLayout *gridLayout = qobject_cast<QGridLayout*>(cherchUi->cherchScrollAreaWidgetContents->layout());
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
            QLabel *noResult = new QLabel("Aucun chercheur trouvé", cherchUi->cherchScrollAreaWidgetContents);
            noResult->setAlignment(Qt::AlignCenter);
            noResult->setStyleSheet("color: #94a3b8; font-size: 16px; margin-top: 50px; background: transparent; border: none;");
            gridLayout->addWidget(noResult, 0, 0, 1, 3);
        }
    } else {
        QVBoxLayout *listLayout = qobject_cast<QVBoxLayout*>(cherchUi->cherchScrollAreaWidgetContents->layout());
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

void MainWindow::on_cherchBtnTri_clicked()
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

void MainWindow::cherchTrierParNom(bool croissant)
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

void MainWindow::cherchTrierParGrade()
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

void MainWindow::cherchTrierParDateCreation(bool croissant)
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

void MainWindow::on_cherchBtnExport_clicked()
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

void MainWindow::on_cherchBtnStatistiques_clicked()
{
    cherchAfficherStatistiques();
}

void MainWindow::cherchAfficherStatistiques()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques des Chercheurs");
    dialog->setMinimumSize(1000, 800);
    dialog->setStyleSheet("background-color: #f8fafc;");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(24);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Titre
    QLabel *titleLabel = new QLabel("📊 Tableau de Bord Statistique", dialog);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 700; color: #1e293b; background: transparent; border: none;");
    mainLayout->addWidget(titleLabel);

    // Scroll area pour le contenu
    QScrollArea *scrollArea = new QScrollArea(dialog);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: transparent;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(24);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // Grid pour les cartes de stats
    QGridLayout *statsGrid = new QGridLayout();
    statsGrid->setSpacing(20);

    // Carte 1: Nombre total
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

    // Carte 2: Professeurs
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

    // Carte 3: Doctorants
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

    // Graphique de répartition par grade
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

    // Créer le graphique en barres
    QBarSeries *series = new QBarSeries();

    QMap<QString, int> gradeCount;
    for (auto &data : cherchChercheursMap) {
        gradeCount[data.grade]++;
    }

    QBarSet *set = new QBarSet("Chercheurs");
    set->setColor(QColor("#3b82f6"));
    QStringList categories;

    int maxValue = 0;
    for (auto it = gradeCount.begin(); it != gradeCount.end(); ++it) {
        *set << it.value();
        categories << it.key();
        if (it.value() > maxValue) maxValue = it.value();
    }
    series->append(set);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(Qt::transparent);
    chart->legend()->setVisible(false);
    chart->setMargins(QMargins(20, 20, 20, 20));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setLabelsColor(QColor("#64748b"));
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxValue + 1);
    axisY->setLabelsColor(QColor("#64748b"));
    axisY->setGridLineColor(QColor("#e2e8f0"));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    chartLayout->addWidget(chartView);

    contentLayout->addWidget(chartFrame);

    // Tableau d'indice de surcharge
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

    // En-tête du tableau
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

    // Ligne de séparation
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e2e8f0;");
    line->setFixedHeight(1);
    overloadLayout->addWidget(line);

    // Liste des chercheurs avec barre de progression
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

    // Bouton fermer
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

void MainWindow::on_cherchBtnUploadPhoto_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Photo", QDir::homePath(), "Images (*.png *.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        cherchUi->cherchLabelPhotoHint->setText("Photo sélectionnée ✓");
        cherchUi->cherchLabelPhotoHint->setStyleSheet("color: #10b981; font-size: 12px; background: transparent; border: none;");
    }
}

void MainWindow::on_cherchBtnAjouterChercheur_clicked()
{
    QString nom = cherchUi->cherchLineEditNom->text().trimmed();
    QString prenom = cherchUi->cherchLineEditPrenom->text().trimmed();
    QString cin = cherchUi->cherchLineEditCIN->text().trimmed();

    if (nom.isEmpty() || prenom.isEmpty() || cin.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }

    int newId = cherchChercheursMap.isEmpty() ? 1 : cherchChercheursMap.keys().last() + 1;

    ChercheurData data;
    data.nom = nom;
    data.prenom = prenom;
    data.cin = cin;
    data.email = cherchUi->cherchLineEditEmail->text();
    data.grade = cherchUi->cherchComboBoxGrade->currentText();
    data.dateCreation = QDateTime::currentDateTime();
    data.age = 35;
    data.carriere = "Junior";
    data.photoPath = ":/avatar.png";

    cherchChercheursMap[newId] = data;

    QMessageBox::information(this, "Succès", "Chercheur ajouté !");

    // Reset form
    cherchUi->cherchLineEditNom->clear();
    cherchUi->cherchLineEditPrenom->clear();
    cherchUi->cherchLineEditCIN->clear();
    cherchUi->cherchLineEditEmail->clear();
    cherchUi->cherchComboBoxGrade->setCurrentIndex(0);
    cherchUi->cherchLabelPhotoHint->setText("Cliquez pour ajouter une photo");
    cherchUi->cherchLabelPhotoHint->setStyleSheet("color: #94a3b8; font-size: 12px; background: transparent; border: none;");

    on_cherchBtnVueListe_clicked();
}

void MainWindow::on_cherchBtnAnnulerAjout_clicked()
{
    if (!cherchUi->cherchLineEditNom->text().isEmpty() ||
        !cherchUi->cherchLineEditPrenom->text().isEmpty()) {

        auto reply = QMessageBox::question(this, "Confirmation", "Annuler ?");
        if (reply == QMessageBox::No) return;
    }

    cherchUi->cherchLineEditNom->clear();
    cherchUi->cherchLineEditPrenom->clear();
    cherchUi->cherchLineEditCIN->clear();
    cherchUi->cherchLineEditEmail->clear();

    on_cherchBtnVueListe_clicked();
}

void MainWindow::on_cherchModifierChercheur(int id)
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

void MainWindow::on_cherchSupprimerChercheur(int id)
{
    auto reply = QMessageBox::question(this, "Supprimer", "Confirmer la suppression ?");
    if (reply == QMessageBox::Yes) {
        cherchChercheursMap.remove(id);
        cherchAfficherListeChercheurs();
    }
}

void MainWindow::on_cherchVoirDetailsChercheur(int id)
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

    // Header avec dégradé
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

    // Avatar large avec photo par défaut
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

    // ScrollArea pour le contenu
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: white; border: none;");

    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    // Fonction helper pour créer une ligne d'info
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

    // Détails des projets
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

    // Footer avec boutons
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
    connect(btnExport, &QPushButton::clicked, this, [this, id]() {
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

void MainWindow::on_cherchBtnExportDetails_clicked()
{
    QMessageBox::information(this, "Export", "Fiche exportée avec succès !");
}

void MainWindow::on_cherchLineEditRecherche_textChanged(const QString &text)
{
    if (text.length() >= 2 || text.isEmpty()) {
        QTimer::singleShot(300, this, [this, text]() {
            if (cherchUi->cherchLineEditRecherche->text() == text) {
                on_cherchBtnRecherche_clicked();
            }
        });
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == cherchUi->cherchUserProfileFrame) {
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

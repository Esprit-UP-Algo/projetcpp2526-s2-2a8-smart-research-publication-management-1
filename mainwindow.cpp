#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SmartResearchMainWindow)
{
    ui->setupUi(this);
    setupConnections();
    loadSampleData();

    // Afficher la page liste par défaut
    ui->SR_stackedWidget->setCurrentIndex(0);
    updateButtonStyles();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    // Connexions des boutons de navigation du stacked widget
    connect(ui->SR_btnVueListe, &QPushButton::clicked, this, &MainWindow::on_SR_btnVueListe_clicked);
    connect(ui->SR_btnAjouter, &QPushButton::clicked, this, &MainWindow::on_SR_btnAjouter_clicked);
    connect(ui->SR_btnSupprimer, &QPushButton::clicked, this, &MainWindow::on_SR_btnSupprimer_clicked);
    connect(ui->SR_btnRecherche, &QPushButton::clicked, this, &MainWindow::on_SR_btnRecherche_clicked);
    connect(ui->SR_btnTri, &QPushButton::clicked, this, &MainWindow::on_SR_btnTri_clicked);
    connect(ui->SR_btnExport, &QPushButton::clicked, this, &MainWindow::on_SR_btnExport_clicked);
    connect(ui->SR_btnStatistiques, &QPushButton::clicked, this, &MainWindow::on_SR_btnStatistiques_clicked);
    connect(ui->SR_btnAjouterPublication, &QPushButton::clicked, this, &MainWindow::on_SR_btnAjouterPublication_clicked);
    connect(ui->SR_btnAnnulerAjout, &QPushButton::clicked, this, &MainWindow::on_SR_btnAnnulerAjout_clicked);

    // Connexions des boutons du sidebar
    connect(ui->SR_btnPublications, &QPushButton::clicked, this, &MainWindow::on_SR_btnPublications_clicked);
    connect(ui->SR_btnChercheurs, &QPushButton::clicked, this, &MainWindow::on_SR_btnChercheurs_clicked);
    connect(ui->SR_btnLaboratoires, &QPushButton::clicked, this, &MainWindow::on_SR_btnLaboratoires_clicked);
    connect(ui->SR_btnProjets, &QPushButton::clicked, this, &MainWindow::on_SR_btnProjets_clicked);
    connect(ui->SR_btnFinances, &QPushButton::clicked, this, &MainWindow::on_SR_btnFinances_clicked);
    connect(ui->SR_btnEvenements, &QPushButton::clicked, this, &MainWindow::on_SR_btnEvenements_clicked);
}

void MainWindow::updateButtonStyles()
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

void MainWindow::loadSampleData()
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
void MainWindow::on_SR_btnVueListe_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(0);
    updateButtonStyles();
}

void MainWindow::on_SR_btnAjouter_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(1);
    updateButtonStyles();
}

void MainWindow::on_SR_btnSupprimer_clicked()
{
    QMessageBox::information(this, "Information", "Fonctionnalité de suppression non implémentée");
}

void MainWindow::on_SR_btnRecherche_clicked()
{
    QString searchText = ui->SR_lineEditRecherche->text();
    if (searchText.isEmpty()) {
        QMessageBox::information(this, "Recherche", "Veuillez entrer un terme de recherche");
    } else {
        QMessageBox::information(this, "Recherche", "Recherche de: " + searchText);
    }
}

void MainWindow::on_SR_btnTri_clicked()
{
    QMessageBox::information(this, "Information", "Fonctionnalité de tri non implémentée");
}

void MainWindow::on_SR_btnExport_clicked()
{
    QMessageBox::information(this, "Information", "Fonctionnalité d'export non implémentée");
}

void MainWindow::on_SR_btnStatistiques_clicked()
{
    ui->SR_stackedWidget->setCurrentIndex(2);
    updateButtonStyles();
}

void MainWindow::on_SR_btnAjouterPublication_clicked()
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
    updateButtonStyles();

    // Vider les champs
    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
}

void MainWindow::on_SR_btnAnnulerAjout_clicked()
{
    // Retour à la liste sans sauvegarder
    ui->SR_stackedWidget->setCurrentIndex(0);
    updateButtonStyles();
}

// Slots pour le sidebar
void MainWindow::on_SR_btnPublications_clicked()
{
    // Déjà sur la page publications
    ui->SR_stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_SR_btnChercheurs_clicked()
{
    QMessageBox::information(this, "Information", "Module Chercheurs non implémenté");
}

void MainWindow::on_SR_btnLaboratoires_clicked()
{
    QMessageBox::information(this, "Information", "Module Laboratoires non implémenté");
}

void MainWindow::on_SR_btnProjets_clicked()
{
    QMessageBox::information(this, "Information", "Module Projets non implémenté");
}

void MainWindow::on_SR_btnFinances_clicked()
{
    QMessageBox::information(this, "Information", "Module Finances non implémenté");
}

void MainWindow::on_SR_btnEvenements_clicked()
{
    QMessageBox::information(this, "Information", "Module Evénements non implémenté");
}

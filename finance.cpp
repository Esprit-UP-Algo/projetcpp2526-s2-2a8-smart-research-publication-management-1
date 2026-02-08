#include "finance.h"
#include "ui_finance.h"

finance::finance(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connecter les boutons du sidebar
    connect(ui->btn_dashboard, &QPushButton::toggled, this, &finance::onDashboardClicked);
    connect(ui->btn_evenement, &QPushButton::toggled, this, &finance::onEvenementClicked);
    connect(ui->btn_publication, &QPushButton::toggled, this, &finance::onPublicationClicked);
    connect(ui->btn_chercheurs, &QPushButton::toggled, this, &finance::onChercheursClicked);
    connect(ui->btn_finances, &QPushButton::toggled, this, &finance::onFinancesClicked);
    connect(ui->btn_settings, &QPushButton::toggled, this, &finance::onSettingsClicked);

    // Connecter les boutons de la barre d'outils des finances
    connect(ui->btnListeFin, &QPushButton::toggled, this, &finance::onListeFinClicked);
    connect(ui->btnAjouterFin, &QPushButton::toggled, this, &finance::onAjouterFinClicked);

    // Initialiser l'affichage sur la page des evenements
    ui->stackedWidget->setCurrentIndex(1);
    ui->btn_evenement->setChecked(true);
}

finance::~finance()
{
    delete ui;
}

void finance::onDashboardClicked(bool checked)
{
    if (checked) {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void finance::onEvenementClicked(bool checked)
{
    if (checked) {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void finance::onPublicationClicked(bool checked)
{
    if (checked) {
        QMessageBox::information(this, "Info", "Page Publications en cours de developpement");
    }
}

void finance::onChercheursClicked(bool checked)
{
    if (checked) {
        QMessageBox::information(this, "Info", "Page Chercheurs en cours de developpement");
    }
}

void finance::onFinancesClicked(bool checked)
{
    if (checked) {
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void finance::onSettingsClicked(bool checked)
{
    if (checked) {
        QMessageBox::information(this, "Info", "Page Settings en cours de developpement");
    }
}

void finance::onListeFinClicked(bool checked)
{
    if (checked) {
        ui->tabWidgetFinances->setCurrentIndex(0);
    }
}

void finance::onAjouterFinClicked(bool checked)
{
    if (checked) {
        ui->tabWidgetFinances->setCurrentIndex(1);
    }
}

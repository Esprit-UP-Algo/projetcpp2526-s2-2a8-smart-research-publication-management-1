#ifndef SMARTPUB_H
#define SMARTPUB_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMenu>
#include <QDialog>
#include <QEvent>
#include <QScreen>
#include <QApplication>
#include <QTimer>
#include <QScrollBar>
#include <QInputDialog>
#include <QDir>
#include <QMap>
#include <QDateTime>
#include <QProgressBar>
#include <QTextEdit>
#include <QToolButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QTabWidget>
#include <QGroupBox>

// Forward declarations pour les modules
namespace Ui {
class SmartPub;
}

class SmartPub : public QMainWindow
{
    Q_OBJECT

public:
    explicit SmartPub(QWidget *parent = nullptr);
    ~SmartPub();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // === NAVIGATION ENTRE MODULES (Sidebar principale) ===
    void on_btnPublications_clicked();
    void on_btnChercheurs_clicked();
    void on_btnLaboratoires_clicked();
    void on_btnProjets_clicked();
    void on_btnFinances_clicked();
    void on_btnEvenements_clicked();

    // === SLOTS MODULE CHERCHEURS (préfixe cherch) ===
    void on_cherchBtnPublications_clicked();
    void on_cherchBtnChercheurs_clicked();
    void on_cherchBtnLaboratoires_clicked();
    void on_cherchBtnProjets_clicked();
    void on_cherchBtnFinances_clicked();
    void on_cherchBtnEvenements_clicked();
    void on_cherchBtnVueListe_clicked();
    void on_cherchBtnAjouter_clicked();
    void on_cherchBtnRecherche_clicked();
    void on_cherchBtnTri_clicked();
    void on_cherchBtnExport_clicked();
    void on_cherchBtnStatistiques_clicked();
    void on_cherchBtnUploadPhoto_clicked();
    void on_cherchBtnAjouterChercheur_clicked();
    void on_cherchBtnAnnulerAjout_clicked();
    void on_cherchModifierChercheur(int id);
    void on_cherchSupprimerChercheur(int id);
    void on_cherchVoirDetailsChercheur(int id);
    void on_cherchLineEditRecherche_textChanged(const QString &text);
    void on_cherchUserProfileFrame_clicked();
    void on_cherchBtnLogin_clicked();
    void on_cherchBtnMotDePasseOublie_clicked();
    void on_cherchBtnRetourLogin_clicked();
    void on_cherchBtnForgotOk_clicked();
    void on_cherchBtnToggleSidebar_clicked();
    void on_cherchBtnExportDetails_clicked();
    void on_cherchBtnToggleVue_clicked();

    // === SLOTS MODULE PUBLICATIONS (préfixe SR) ===
    void on_SR_btnPublications_clicked();
    void on_SR_btnChercheurs_clicked();
    void on_SR_btnLaboratoires_clicked();
    void on_SR_btnProjets_clicked();
    void on_SR_btnFinances_clicked();
    void on_SR_btnEvenements_clicked();
    void on_SR_btnVueListe_clicked();
    void on_SR_btnAjouter_clicked();
    void on_SR_btnSupprimer_clicked();
    void on_SR_btnRecherche_clicked();
    void on_SR_btnTri_clicked();
    void on_SR_btnExport_clicked();
    void on_SR_btnStatistiques_clicked();
    void on_SR_btnAjouterPublication_clicked();
    void on_SR_btnAnnulerAjout_clicked();

    // === SLOTS MODULE FINANCES (préfixe fin) ===
    void on_finBtnPublications_clicked();
    void on_finBtnChercheurs_clicked();
    void on_finBtnLaboratoires_clicked();
    void on_finBtnProjets_clicked();
    void on_finBtnFinances_clicked();
    void on_finBtnEvenements_clicked();
    void on_finBtnVueListe_clicked();
    void on_finBtnAjouter_clicked();
    void on_finBtnRecherche_clicked();
    void on_finBtnTri_clicked();
    void on_finBtnExport_clicked();
    void on_finBtnStatistiques_clicked();
    void on_finBtnAjouterTransaction_clicked();
    void on_finBtnAnnulerAjout_clicked();
    void on_finBtnModifierTransaction_clicked();
    void on_finBtnSupprimerTransaction_clicked();

    // === SLOTS MODULE EVENEMENTS (préfixe ev) ===
    void on_evBtnPublications_clicked();
    void on_evBtnChercheurs_clicked();
    void on_evBtnLaboratoires_clicked();
    void on_evBtnProjets_clicked();
    void on_evBtnFinances_clicked();
    void on_evBtnEvenements_clicked();
    void on_evBtnAjouterEvent_clicked();
    void on_evBtnModifierEvent_clicked();
    void on_evBtnSupprimerEvent_clicked();
    void on_evBtnTrierDate_clicked();
    void on_evBtnRechercheLieu_clicked();
    void on_evBtnExportCalendrier_clicked();
    void on_evBtnLivreResumes_clicked();
    void on_evBtnCalculImpact_clicked();
    void on_evBtnStatsParticipation_clicked();

private:
    void setupUI();
    void connectSignals();
    void updateNavButtonStyles(QPushButton *activeBtn);

    // === FONCTIONS MODULE CHERCHEURS ===
    void cherchSetupUI();
    void cherchConnectSignals();
    void cherchSetupAnimations();
    void cherchApplyModernStyle();
    void cherchShowLoginView();
    void cherchShowMainView();
    void cherchCheckLogin();
    void cherchShowForgotPasswordView();
    void cherchAfficherListeChercheurs();
    void cherchAjouterChercheurCard(int id, const QString &nom, const QString &prenom,
                                    const QString &grade, const QString &email,
                                    const QPixmap &photo);
    void cherchAjouterChercheurListItem(int id, const QString &nom, const QString &prenom,
                                        const QString &grade, const QString &email);
    void cherchClearChercheursList();
    void cherchAnimateCardEntry(QWidget *card, int index);
    void cherchTrierParNom(bool croissant = true);
    void cherchTrierParGrade();
    void cherchTrierParDateCreation(bool croissant = true);
    void cherchAfficherStatistiques();
    void cherchToggleSidebar();
    void cherchSetupSidebarToggle();
    void cherchAjouterDonneesTest();
    QString cherchDeterminerCarriere(int projetsCount, const QString &grade);

    // === FONCTIONS MODULE PUBLICATIONS ===
    void SR_setupUI();
    void SR_connectSignals();
    void SR_loadSampleData();
    void SR_updateButtonStyles();

    // === STRUCTURES DE DONNEES (doivent être déclarées avant les fonctions qui les utilisent) ===

    // Structure pour le module Chercheurs
    struct ChercheurData {
        QString nom;
        QString prenom;
        QString grade;
        QString email;
        QString cin;
        QDateTime dateCreation;
        QString carriere;
        QList<int> projetsIds;
        int age;
        QString photoPath;
    };

    // Structure pour le module Finances
    struct TransactionData {
        int id;
        QString projet;
        QString type;
        double montant;
        QString date;
        QString categorie;
        QString statut;
        QString description;
    };

    // Structure pour le module Evenements
    struct EventData {
        int id;
        QString nom;
        QString lieu;
        QString date;
        QString description;
    };

    // === FONCTIONS MODULE FINANCES ===
    void finSetupUI();
    void finConnectSignals();
    void finLoadSampleData();
    void finUpdateButtonStyles();
    void finAjouterDonneesTest();
    void finAfficherListeTransactions();
    void finAjouterTransactionTable(const TransactionData &data);

    // === FONCTIONS MODULE EVENEMENTS ===
    void evSetupUI();
    void evConnectSignals();
    void evLoadSampleData();
    void evAjouterDonneesTest();
    void evAfficherListeEvents();
    void evAjouterEventTable(const EventData &data);
    void evRechercherParLieu();

    Ui::SmartPub *ui;

    // === VARIABLES MODULE CHERCHEURS ===
    bool cherchVueListeActive;
    bool cherchVueIconesActive;
    int cherchChercheurSelectionne;
    bool cherchIsLoggedIn;
    bool cherchSidebarVisible;
    QPushButton *cherchBtnToggleSidebar;
    QPushButton *cherchBtnToggleVue;
    QMap<int, ChercheurData> cherchChercheursMap;

    // === VARIABLES MODULE FINANCES ===
    bool finVueListeActive;
    int finTransactionSelectionnee;
    QMap<int, TransactionData> finTransactionsMap;

    // === VARIABLES MODULE EVENEMENTS ===
    int evEventSelectionne;
    QMap<int, EventData> evEventsMap;
};

#endif // SMARTPUB_H

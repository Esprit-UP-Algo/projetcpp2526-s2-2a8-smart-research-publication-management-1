#ifndef SMARTPUB_H
#define SMARTPUB_H

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QVBoxLayout>
// Qt Charts
#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QValueAxis>
// SQL
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QRegularExpression>
#include <QIntValidator>
#include <QRegularExpressionValidator>
// PDF Export
#include <QPrinter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
// Network (vérification délivrabilité email)
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
// arduino
#include "arduino.h"
#include "scenario1.h"
#include "demi_scenario2.h"

QT_BEGIN_NAMESPACE
class QPieSeries;
class QBarSeries;
class QLineSeries;
namespace Ui {
class SmartPub;
}
QT_END_NAMESPACE

// ============================================================================
// STRUCTURES DE DONNEES
// ============================================================================

// ============================================================================
// HEADERS DES MODULES
// ============================================================================
#include "projet.h"
#include "chercheur.h"
#include "finance.h"
#include "trans_secure.h"
#include "evenement.h"
#include "laboratoire.h"
#include "publication.h"

// ============================================================================
// SYSTÈME D'AUTHENTIFICATION CENTRALISÉ
// ============================================================================
#include "publicationauth.h"

// ============================================================================
// STRUCTURES LOGIN
// ============================================================================

enum class UserRole { Guest, Admin };

// UserAccount : structure interne à SmartPub (post-login).
// Construite à partir de AppUserAccount après authentification réussie.
struct UserAccount {
    QString  email;
    QString  password;
    QString  module;       // "ALL" ou nom du module autorisé
    UserRole role;
    QString  displayName;
    int      moduleIndex = -1; // -1 = accès total, sinon index du module
};

// ============================================================================
// DIALOG LOGIN — utilise AppAuthService pour l'authentification
// ============================================================================

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    UserAccount getLoggedInUser() const { return loggedInUser; }
    bool isLoggedIn() const { return loggedIn; }

private slots:
    void onLoginClicked();
    void onForgotPasswordClicked();
    void onGuestClicked();

private:
    void setupUI();

    AppAuthService  m_authService;  // moteur d'auth centralisé
    QLineEdit      *emailEdit;
    QLineEdit      *passwordEdit;
    QPushButton    *loginBtn;
    QPushButton    *guestBtn;
    QPushButton    *forgotBtn;
    QLabel         *errorLabel;

    UserAccount loggedInUser;
    bool        loggedIn;
};

// ============================================================================
// DIALOGS POUR MODULE PROJETS
// ============================================================================

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    QString getSelectedTheme() const {
        return themeCombo ? themeCombo->currentIndex() == 0
                                ? QStringLiteral("dark")
                                : QStringLiteral("light")
                          : QStringLiteral("light");
    }

private:
    void setupUI();
    QComboBox *themeCombo;
    QComboBox *langCombo;
    QCheckBox *notifCheck;
    QCheckBox *emailCheck;
    QCheckBox *soundCheck;
    QCheckBox *autoSaveCheck;
    QSpinBox *intervalSpin;
};




// ============================================================================
// CLASSE PRINCIPALE
// ============================================================================

class SmartPub : public QMainWindow {
    Q_OBJECT

public:
    explicit SmartPub(QWidget *parent = nullptr);
    ~SmartPub();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // === NAVIGATION PRINCIPALE (Sidebar) — délégué via handle*Navigation() ===
    void on_btnPublications_clicked();
    void on_btnChercheurs_clicked();
    void on_btnLaboratoires_clicked();
    void on_btnProjets_clicked();
    void on_btnFinances_clicked();
    void on_btnEvenements_clicked();
    void expandSidebar();
    void collapseSidebar();
    void onSettingsClicked();

    // === MODULE CHERCHEURS ===
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
    void on_cherchBtnLogin_clicked();
    void on_cherchBtnMotDePasseOublie_clicked();
    void on_cherchBtnRetourLogin_clicked();
    void on_cherchBtnForgotOk_clicked();
    void on_cherchBtnToggleVue_clicked();
    // === Vérification délivrabilité email (AbstractAPI) ===
    void on_cherchEmailVerificationReply(QNetworkReply *reply);

    // === MODULE PUBLICATIONS ===
    void on_SR_btnVueListe_clicked();
    void on_SR_btnAjouter_clicked();

    void on_SR_btnRecherche_clicked();
    void on_SR_btnTri_clicked();
    void on_SR_btnExport_clicked();
    void on_SR_btnStatistiques_clicked();
    void on_SR_btnAjouterPublication_clicked();
    void on_SR_btnAnnulerAjout_clicked();
    void on_btnUploadPDF_clicked();
    void on_SR_modifierPublication_clicked();
    void on_SR_supprimerPublication_clicked();
    void SR_applyFilterListe();
    void SR_reinitFilterListe();

    // === MODULE FINANCES ===
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
    void on_finLineEditRecherche_textChanged(const QString &text);

    // === MODULE EVENEMENTS ===
    void on_evBtnAjouterEvent_clicked();
    void on_evBtnModifierEvent_clicked();
    void on_evBtnSupprimerEvent_clicked();
    void on_evBtnTrierDate_clicked();
    void on_evBtnRechercheLieu_clicked();
    void on_evBtnExportCalendrier_clicked();
    void on_evBtnLivreResumes_clicked();
    void on_evBtnCalculImpact_clicked();
    void on_evBtnStatsParticipation_clicked();

    // === MODULE LABORATOIRES ===
    void on_labBtnAjouter_clicked();
    void on_labBtnModifier_clicked();
    void on_labBtnSupprimer_clicked();
    void on_labBtnConfirmerForm_clicked();
    void on_labBtnAnnulerForm_clicked();
    void on_labBtnStatistiques_clicked();
    void on_labBtnOptimiseur_clicked();
    void on_labBtnPredicteur_clicked();
    void on_labBtnExporter_clicked();
    void on_labBtnTrier_clicked();
    void on_labTableSelectionChanged();
    void on_labSearchChanged(const QString &text);

    // === MODULE PROJETS (Ton travail) ===
    void on_btnListeProjets_clicked();
    void on_btnAjouterProjet_clicked();
    void on_btnModifierProjet_clicked();
    void on_btnSupprimerProjet_clicked();
    void on_lineEditRechercheProjets_textChanged(const QString &text);
    void on_btnAnnulerForm_clicked();
    void on_btnEnregistrerForm_clicked();
    void on_tableSelectionChanged();
    void on_tableDoubleClicked(int row, int column);
    void on_triDateDebutClicked();
    void on_triDateFinClicked();
    void on_triEtatClicked();
    void on_triProgressionClicked();
    void on_statistiquesClicked();
    void on_santeProjetClicked();
    void on_optimiserChargeClicked();
    void on_iaRecommanderClicked();
    void on_filtresClicked();
    void on_exporterClicked();

private:
    // === SETUP ===
    void showLogin();
    void setupUI();
    void setupSidebar();
    void setupConnections();
    void updateNavButtonStyles(QPushButton *activeBtn);
    void setActiveNavigationButton(int index);
    void updateSidebarProfileVisibility();
    void updateProfileName(int moduleIndex);
    void checkPermissions();
    void applyModuleRestrictions(); // remplace applyGuestRestrictions

    // === MODULE CHERCHEURS ===
    void cherchSetupUI();
    void cherchSetupFormValidationWidgets();
    void cherchClearAddFormErrors();
    void cherchShowLineFieldError(QLineEdit *field, QLabel *errLabel, const QString &msg);
    void cherchHideLineFieldError(QLineEdit *field, QLabel *errLabel);
    void cherchShowComboFieldError(QComboBox *cb, QLabel *errLabel, const QString &msg);
    void cherchHideComboFieldError(QComboBox *cb, QLabel *errLabel);
    bool cherchValidateNom(bool forSubmit);
    bool cherchValidatePrenom(bool forSubmit);
    bool cherchValidateCin(bool forSubmit);
    bool cherchValidateEmailFormat(bool forSubmit);
    bool cherchValidateEmailUniqueForAdd(bool forSubmit);
    bool cherchValidateGrade(bool forSubmit);
    void cherchConnectSignals();
    void cherchApplyModernStyle();
    // Vérification délivrabilité email via AbstractAPI
    void cherchVerifyEmailDeliverability(const QString &email);
    void cherchShowLoginView();
    void cherchShowMainView();
    void cherchCheckLogin();
    void cherchShowForgotPasswordView();
    void cherchAfficherListeChercheurs();
    void cherchAjouterChercheurCard(int id, const QString &nom,
                                    const QString &prenom, const QString &grade,
                                    const QString &email, const QString &photoPath);
    void cherchAjouterChercheurListItem(int id, const QString &nom,
                                        const QString &prenom,
                                        const QString &grade,
                                        const QString &email,
                                        const QString &photoPath);
    void cherchClearChercheursList();
    void cherchAnimateCardEntry(QWidget *card, int index);
    void cherchTrierParNom(bool croissant = true);
    void cherchTrierParGrade();
    void cherchTrierParDateCreation(bool croissant = true);
    void cherchEnrichirDonneesDepuisOracle();
    void cherchAfficherStatistiques();
    void cherchAjouterDonneesTest();
    QString cherchDeterminerCarriere(int projetsCount, const QString &grade);
    void handleChercheursNavigation();
    bool   cherchValiderNomPrenom(const QString &nom, const QString &prenom);

    // === MODULE PUBLICATIONS ===
    void SR_setupUI();
    void SR_connectSignals();
    void SR_loadSampleData();
    void SR_updateButtonStyles();
    void SR_addButtonsToRow(int row);
    void SR_refreshStatsForCurrentView();
    void handlePublicationsNavigation();

    // === MODULE FINANCES ===
    void finSetupUI();
    void finConnectSignals();
    void finUpdateButtonStyles();
    void finAjouterDonneesTest();
    void finRemplirComboProjets();
    void finChargerTransactionsDepuisOracle();
    void finAfficherListeTransactions();
    void finAjouterTransactionTable(const TransactionData &data);
    void finViderFormulaire();
    void finRemplirFormulaire(const TransactionData &data);
    QList<TransactionData> finGetTransactionsFiltreesEtTriees() const;
    void handleFinancesNavigation();

    // === MODULE EVENEMENTS ===
    void evSetupUI();
    void evConnectSignals();
    void evAjouterDonneesTest();
    void evAfficherListeEvents();
    void evAjouterEventTable(const EventData &data);
    void evRechercherParLieu();
    void handleEvenementsNavigation();

    // === MODULE LABORATOIRES ===
    void labSetupUI();
    void labConnectSignals();
    void labChargerDonnees();
    void labAfficherListe();
    void labAfficherListe(const QList<LaboratoryData> &labs);
    void labViderFormulaire();
    void labRemplirFormulaire(const LaboratoryData &lab);
    LaboratoryData labGetFormData() const;
    bool labValiderFormulaire() const;
    void labMontrerFormulaire(bool isEdit = false);
    void labCacherFormulaire();
    void labMontrerStatistiques();
    void labOptimiseurCollab();
    void labPredicteurBesoins();
    void labExporter();
    void labTrier();
    void labSetTableRowBackground(QTableWidget *table, int row, const QColor &color);
    void handleLaboratoiresNavigation();

    // === MODULE PROJETS (Ton travail) ===
    void projSetupUI();
    void projSetupConnections();
    void projSetupTable();
    void projSetupComboBoxes();
    void projSetupSampleData();
    void projSetupSidebarProfile();
    void projSetupStatistiquesButton();
    void projSetupAIButton();
    void projChargerProjets();
    void projAjouterProjetTable(const Projet &projet, int rowIndex);
    void projMettreAJourProjetTable(int row, const Projet &projet);
    void projViderTable();
    void projAfficherFormulaire(bool isEdit = false);
    void projCacherFormulaire();
    void projRemplirFormulaire(const Projet &projet);
    void projViderFormulaire();
    Projet projGetProjetFromForm() const;
    bool projValiderFormulaire();
    bool projValidateCode(bool forSubmit);
    bool projValidateTitre(bool forSubmit);
    bool projValidateResponsable(bool forSubmit);
    bool projValidateDates(bool forSubmit);
    void projSetupFormValidationWidgets();
    void projClearProjectFormErrors();
    void projShowLineFieldError(QLineEdit *field, QLabel *errLabel, const QString &msg);
    void projHideLineFieldError(QLineEdit *field, QLabel *errLabel);
    void projShowComboFieldError(QComboBox *cb, QLabel *errLabel, const QString &msg);
    void projHideComboFieldError(QComboBox *cb, QLabel *errLabel);
    void projShowDateOrderError(const QString &msg);
    void projHideDateOrderError();
    void projFiltrerTable(const QString &text);
    void projAppliquerFiltres();
    void projMettreAJourStats();
    void projMettreAJourBadgeFiltres();
    int projGetSelectedRow() const;
    void projSetActiveNavigationButton(int index);
    void projSetActiveCrudButton(int index);
    void projSortProjetsBy(int column, Qt::SortOrder order);
    void projShowProjetDetails(int projetId);
    void projShowProjetDetailsDialog(const Projet &projet);
    QString projCalculerStatutProjet(const Projet &projet) const;
    double projCalculerTauxAvancement(const Projet &projet) const;
    QVector<QString> projGenererAlertes(const Projet &projet) const;
    int projExtraireProgression(const QString &progressionStr) const;
    int projCompterProjetsParEtat(const QString &etat) const;
    double projCalculerProgressionMoyenne() const;
    int projCompterProjetsEnRetard() const;
    void projAjusterColonnesTable();
    void projUpdateSidebarProfileVisibility();
    void handleProjetsNavigation();

    // --- Handlers métier (impl. dans les *.cpp modules ; slots UI dans smartpub.cpp) ---
    void handleCherchBtnMotDePasseOublieClicked();
    void handleCherchBtnRetourLoginClicked();
    void handleCherchBtnForgotOkClicked();
    void handleCherchBtnLoginClicked();
    void handleCherchBtnVueListeClicked();
    void handleCherchBtnAjouterClicked();
    void handleCherchBtnToggleVueClicked();
    void handleCherchBtnRechercheClicked();
    void handleCherchBtnTriClicked();
    void handleCherchBtnExportClicked();
    void handleCherchBtnStatistiquesClicked();
    void handleCherchBtnUploadPhotoClicked();
    void handleCherchBtnAjouterChercheurClicked();
    void handleCherchBtnAnnulerAjoutClicked();
    void handleCherchModifierChercheur(int id);
    void handleCherchSupprimerChercheur(int id);
    void handleCherchVoirDetailsChercheur(int id);
    void handleCherchLineEditRechercheTextChanged(const QString &text);

    void handleSRBtnVueListeClicked();
    void handleSRBtnAjouterClicked();
    void handleSRBtnRechercheClicked();
    void handleSRBtnTriClicked();
    void handleSRBtnExportClicked();
    void handleSRBtnStatistiquesClicked();
    void handleSRBtnAjouterPublicationClicked();
    void handleSRBtnAnnulerAjoutClicked();
    void handleSRModifierPublicationClicked();
    void handleSRSupprimerPublicationClicked();
    void handleSRApplyFilterListe();
    void handleSRReinitFilterListe();

    void handleFinBtnVueListeClicked();
    void handleFinBtnAjouterClicked();
    void handleFinBtnRechercheClicked();
    void handleFinBtnTriClicked();
    void handleFinBtnExportClicked();
    void handleFinBtnStatistiquesClicked();
    void handleFinBtnAjouterTransactionClicked();
    void handleFinBtnAnnulerAjoutClicked();
    void handleFinBtnModifierTransactionClicked();
    void handleFinBtnSupprimerTransactionClicked();
    void handleFinLineEditRechercheTextChanged(const QString &text);

    void handleEvBtnAjouterEventClicked();
    void handleEvBtnModifierEventClicked();
    void handleEvBtnSupprimerEventClicked();
    void handleEvBtnTrierDateClicked();
    void handleEvBtnRechercheLieuClicked();
    void handleEvBtnExportCalendrierClicked();
    void handleEvBtnLivreResumesClicked();
    void handleEvBtnCalculImpactClicked();
    void handleEvBtnStatsParticipationClicked();

    void handleLabBtnAjouterClicked();
    void handleLabBtnModifierClicked();
    void handleLabBtnSupprimerClicked();
    void handleLabBtnConfirmerFormClicked();
    void handleLabBtnAnnulerFormClicked();
    void handleLabBtnStatistiquesClicked();
    void handleLabBtnOptimiseurClicked();
    void handleLabBtnPredicteurClicked();
    void handleLabBtnExporterClicked();
    void handleLabBtnTrierClicked();
    void handleLabTableSelectionChanged();
    void handleLabSearchChanged(const QString &text);

    void handleProjetListeProjets();
    void handleProjetAjouterProjet();
    void handleProjetModifierProjet();
    void handleProjetSupprimerProjet();
    void handleProjetRechercheChanged(const QString &text);
    void handleProjetAnnulerForm();
    void handleProjetEnregistrerForm();
    void handleProjetTableSelectionChanged();
    void handleProjetTableDoubleClicked(int row, int column);
    void handleProjetTriDateDebut();
    void handleProjetTriDateFin();
    void handleProjetTriEtat();
    void handleProjetTriProgression();
    void handleProjetStatistiques();
    void handleProjetSante();
    void handleProjetOptimiserCharge();
    void handleProjetIARecommander();
    void handleProjetFiltres();
    void handleProjetExporter();

    Ui::SmartPub *ui;

    // === MAIN ARCHITECTURE ===
    QStackedWidget *mainStack;
    QWidget *pageApp;
    QWidget *pageLogin;

    // === VARIABLES SIDEBAR ===
    bool sidebarExpanded;
    QTimer *sidebarTimer;
    QWidget *profileWidget;
    QLabel *avatarLabel;
    QLabel *nameLabel;
    QLabel *roleLabel;
    QPushButton *btnSettings;
    QPushButton *btnLogout;

    // === VARIABLES LOGIN ===
    UserAccount currentUser;
    bool isUserLoggedIn;

    // === VARIABLES MODULE CHERCHEURS ===
    bool cherchVueListeActive;
    bool cherchVueIconesActive;
    int cherchChercheurSelectionne;
    bool cherchIsLoggedIn;
    QPushButton *cherchBtnToggleVue;
    QMap<int, ChercheurData> cherchChercheursMap;
    QString cherchOrderByClause;
    QString cherchWhereClause;
    QString cherchCurrentPhotoPath;
    // Réseau — vérification délivrabilité email
    QNetworkAccessManager *cherchNetworkManager = nullptr;
    bool    cherchEmailDeliverabilityOk  = false;
    bool    cherchEmailCheckPending      = false;
    QString cherchLastVerifiedEmail;

    // === VARIABLES MODULE FINANCES ===
    bool finVueListeActive;
    int finTransactionSelectionnee;
    QMap<int, TransactionData> finTransactionsMap;
    int finTriColonne;
    Qt::SortOrder finTriOrdre;

    // === VARIABLES MODULE EVENEMENTS ===
    int evEventSelectionne;
    QMap<QString, EventData> evEventsMap;
    QString evEditingCode;  // code en cours de modification (vide si ajout)

    // === VARIABLES MODULE PROJETS ===
    QVector<Projet> projets;
    QVector<Projet> projetsFiltres;
    int currentProjetId;
    bool isEditing;
    int currentSortColumn;
    Qt::SortOrder currentSortOrder;
    QString filtreEtat;
    QString filtreResponsable;
    QDate filtreDateDebutMin;
    QDate filtreDateDebutMax;
    bool filtresActifs;

    // === WIDGET PROJETS DYNAMIQUE (formulaire ajout chercheur) ===
    QPushButton   *cherchBtnSelectProjets;   // bouton déclencheur toggle
    QListWidget   *cherchProjetsListWidget;  // liste inline des projets
    QLabel        *cherchLabelProjetsSelec;  // label résumé des projets sélectionnés

    // === VARIABLES MODULE PUBLICATIONS ===
    int editingPublicationRow;
    QFrame *SR_filterFrame;
    QLineEdit *SR_filterTitre;
    QComboBox *SR_comboBoxAuteur;
    QLineEdit *SR_filterAuteur;
    QComboBox *SR_filterStatut;
    QPushButton *SR_btnReinitFilter;
    int SR_sortColumn;
    Qt::SortOrder SR_sortOrder;

    // === VARIABLES MODULE LABORATOIRES ===
    QWidget        *labPage;
    QTableWidget   *labTable;
    QLineEdit      *labSearchEdit;
    QLabel         *labTotalLabel;
    QPushButton    *labBtnAjouter;
    QPushButton    *labBtnModifier;
    QPushButton    *labBtnSupprimer;
    QFrame         *labFormFrame;
    QLineEdit      *labFormNom;
    QComboBox      *labFormThematique;
    QLineEdit      *labFormBudget;
    QSpinBox       *labFormCapacite;
    QComboBox      *labFormStatut;
    QLineEdit      *labFormEquipements;
    QComboBox      *labFormDirecteur;
    QMap<int, LaboratoryData> labDataMap;
    int            labNextId;
    int            labEditingId; // -1 = ajout, sinon id en cours d'édition

    // === Validation inline formulaire projets ===
    QLabel *projErrCodeLabel = nullptr;
    QLabel *projErrTitreLabel = nullptr;
    QLabel *projErrDateLabel = nullptr;
    QLabel *projErrResponsableLabel = nullptr;
    bool projTouchedCode = false;
    bool projTouchedTitre = false;
    bool projTouchedResponsable = false;
    bool projTouchedDates = false;
    QTimer *projTitreDebounceTimer = nullptr;
    QTimer *projCodeDebounceTimer = nullptr;

    // === Validation inline formulaire chercheurs (ajout) ===
    QLabel *cherchErrNomLabel = nullptr;
    QLabel *cherchErrPrenomLabel = nullptr;
    QLabel *cherchErrCinLabel = nullptr;
    QLabel *cherchErrEmailLabel = nullptr;
    QLabel *cherchErrGradeLabel = nullptr;
    bool cherchTouchedNom = false;
    bool cherchTouchedPrenom = false;
    bool cherchTouchedCin = false;
    bool cherchTouchedEmail = false;
    bool cherchTouchedGrade = false;
    QTimer *cherchEmailDebounceTimer = nullptr;

    Arduino*    arduino;
    Scenario1*  scenarioAcces;          // Scénario 1 : accès laboratoire (RFID → projet/labo)
    DemiScenario2* scenarioProgramme;   // Scénario 2 : affichage programme LED par module
    QLabel*     labelRfidStatus;        // Affichage résultat RFID dans le module Chercheurs
};

#endif // SMARTPUB_H

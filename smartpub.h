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

// Structure pour le module Projets (ton travail)
struct Projet {
  int id;
  QString code;
  QString titre;
  QDate dateDebut;
  QDate dateFin;
  QString responsable;
  QString etat;
  QString progression;
  QString description;

  Projet() : id(0) {}
  Projet(int id, QString code, QString titre, QDate debut, QDate fin,
         QString resp, QString etat, QString prog, QString desc)
      : id(id), code(code), titre(titre), dateDebut(debut), dateFin(fin),
        responsable(resp), etat(etat), progression(prog), description(desc) {}
};

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

// ============================================================================
// DIALOG LOGIN
// ============================================================================

enum class UserRole { Guest, Admin };

struct UserAccount {
  QString email;
  QString password;
  QString module; // "Chercheurs", "Publications", "Finances", "Evenements",
                  // "Projets"
  UserRole role;
  QString displayName;
};

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
  void setupAccounts();

  QLineEdit *emailEdit;
  QLineEdit *passwordEdit;
  QPushButton *loginBtn;
  QPushButton *guestBtn;
  QPushButton *forgotBtn;
  QLabel *errorLabel;

  QList<UserAccount> accounts;
  UserAccount loggedInUser;
  bool loggedIn;
};

// ============================================================================
// DIALOGS POUR MODULE PROJETS
// ============================================================================

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit SettingsDialog(QWidget *parent = nullptr);

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

class FiltresDialog : public QDialog {
  Q_OBJECT
public:
  explicit FiltresDialog(QWidget *parent = nullptr);
  QString getEtatFiltre() const;
  QString getResponsableFiltre() const;
  QDate getDateDebutMin() const;
  QDate getDateDebutMax() const;
  bool isFiltreActif() const;

private:
  void setupUI();
  QComboBox *comboBoxEtat;
  QComboBox *comboBoxResponsable;
  QDateEdit *dateEditDebutMin;
  QDateEdit *dateEditMax;
  QPushButton *btnAppliquer;
  QPushButton *btnReinitialiser;
  QPushButton *btnAnnuler;
  bool filtreActif;
};

class IARecommandationsDialog : public QDialog {
  Q_OBJECT
public:
  explicit IARecommandationsDialog(const QList<Projet> &projets,
                                   QWidget *parent = nullptr);

private:
  void setupUI();
  void genererRecommandations();
  QList<Projet> m_projets;
  struct Recommandation {
    QString titre;
    QString description;
    double scoreSimilarite;
    QStringList collaborateursSuggeres;
    QString raison;
    QString domaine;
  };
  QVector<Recommandation> m_recommandations;
};

class ProjetDetailsDialog : public QDialog {
  Q_OBJECT
public:
  explicit ProjetDetailsDialog(const Projet &projet, QWidget *parent = nullptr);

private:
  Projet m_projet;
};

class StatistiquesDialog : public QDialog {
  Q_OBJECT
public:
  explicit StatistiquesDialog(const QList<Projet> &projets,
                              QWidget *parent = nullptr);

private:
  void setupUI();
  void calculerStatistiques();
  void creerGraphiques();
  QList<Projet> m_projets;
  QLabel *labelTotalProjets;
  QLabel *labelProjetsActifs;
  QLabel *labelProjetsTermines;
  QLabel *labelProgressionMoyenne;
  QLabel *labelProjetsRetard;
  QLabel *labelProjetsPlanifies;
  QLabel *labelProjetsPause;
  QChartView *chartEtatView;
  QChartView *chartProgressionView;
  QChartView *chartTemporelView;
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
  // === NAVIGATION PRINCIPALE (Sidebar Unique) ===
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
  //     void on_cherchUserProfileFrame_clicked();
  void on_cherchBtnLogin_clicked();
  void on_cherchBtnMotDePasseOublie_clicked();
  void on_cherchBtnRetourLogin_clicked();
  void on_cherchBtnForgotOk_clicked();
  void on_cherchBtnToggleVue_clicked();
  void on_cherchBtnExportDetails_clicked();

  // === MODULE PUBLICATIONS ===
  void on_SR_btnVueListe_clicked();
  void on_SR_btnAjouter_clicked();

  void on_SR_btnRecherche_clicked();
  void on_SR_btnTri_clicked();
  void on_SR_btnExport_clicked();
  void on_SR_btnStatistiques_clicked();
  void on_SR_btnAjouterPublication_clicked();
  void on_SR_btnAnnulerAjout_clicked();

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
  void applyGuestRestrictions();

  // === MODULE CHERCHEURS ===
  void cherchSetupUI();
  void cherchConnectSignals();
  void cherchApplyModernStyle();
  void cherchShowLoginView();
  void cherchShowMainView();
  void cherchCheckLogin();
  void cherchShowForgotPasswordView();
  void cherchAfficherListeChercheurs();
  void cherchAjouterChercheurCard(int id, const QString &nom,
                                  const QString &prenom, const QString &grade,
                                  const QString &email, const QPixmap &photo);
  void cherchAjouterChercheurListItem(int id, const QString &nom,
                                      const QString &prenom,
                                      const QString &grade,
                                      const QString &email);
  void cherchClearChercheursList();
  void cherchAnimateCardEntry(QWidget *card, int index);
  void cherchTrierParNom(bool croissant = true);
  void cherchTrierParGrade();
  void cherchTrierParDateCreation(bool croissant = true);
  void cherchAfficherStatistiques();
  void cherchAjouterDonneesTest();
  QString cherchDeterminerCarriere(int projetsCount, const QString &grade);

  // === MODULE PUBLICATIONS ===
  void SR_setupUI();
  void SR_connectSignals();
  void SR_loadSampleData();
  void SR_updateButtonStyles();
  void SR_addButtonsToRow(int row);

  // === MODULE FINANCES ===
  void finSetupUI();
  void finConnectSignals();
  void finUpdateButtonStyles();
  void finAjouterDonneesTest();
  void finAfficherListeTransactions();
  void finAjouterTransactionTable(const TransactionData &data);

  // === MODULE EVENEMENTS ===
  void evSetupUI();
  void evConnectSignals();
  void evAjouterDonneesTest();
  void evAfficherListeEvents();
  void evAjouterEventTable(const EventData &data);
  void evRechercherParLieu();

  // === MODULE PROJETS (Ton travail) ===
  void projSetupUI();
  void projConnectSignals();
  void projSetupTable();
  void projSetupComboBoxes();
  void projSetupSampleData();
  void projChargerProjets();
  void projAjouterProjetTable(const Projet &projet, int rowIndex);
  void projMettreAJourProjetTable(int row, const Projet &projet);
  void projViderTable();
  void projAfficherFormulaire(bool isEdit = false);
  void projCacherFormulaire();
  void projRemplirFormulaire(const Projet &projet);
  void projViderFormulaire();
  Projet projGetProjetFromForm() const;
  bool projValiderFormulaire() const;
  void projFiltrerTable(const QString &text);
  void projAppliquerFiltres();
  void projMettreAJourBadgeFiltres();
  int projGetSelectedRow() const;
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
  void projSetActiveCrudButton(int index);

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

  // === VARIABLES MODULE FINANCES ===
  bool finVueListeActive;
  int finTransactionSelectionnee;
  QMap<int, TransactionData> finTransactionsMap;

  // === VARIABLES MODULE EVENEMENTS ===
  int evEventSelectionne;
  QMap<int, EventData> evEventsMap;

  // === VARIABLES MODULE PROJETS ===
  QVector<Projet> projets;
  QVector<Projet> projetsFiltres;
  int nextProjetId;
  int currentProjetId;
  bool isEditing;
  int currentSortColumn;
  Qt::SortOrder currentSortOrder;
  QString filtreEtat;
  QString filtreResponsable;
  QDate filtreDateDebutMin;
  QDate filtreDateDebutMax;
  bool filtresActifs;

  // === VARIABLES MODULE PUBLICATIONS ===
  int editingPublicationRow;
};

#endif // SMARTPUB_H

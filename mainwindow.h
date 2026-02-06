// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QBarCategoryAxis>
#include <QProgressBar>
#include <QTextEdit>
#include <QToolButton>
#include <QSplitter>
#include <QStackedWidget>  // Ajouté pour la gestion des vues

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Navigation module chercheur
    void on_cherchBtnPublications_clicked();
    void on_cherchBtnChercheurs_clicked();
    void on_cherchBtnLaboratoires_clicked();
    void on_cherchBtnProjets_clicked();
    void on_cherchBtnFinances_clicked();
    void on_cherchBtnEvenements_clicked();

    // Vue switching
    void on_cherchBtnVueListe_clicked();
    void on_cherchBtnAjouter_clicked();

    // Actions toolbar
    void on_cherchBtnRecherche_clicked();
    void on_cherchBtnTri_clicked();
    void on_cherchBtnExport_clicked();
    void on_cherchBtnStatistiques_clicked();
    void on_cherchBtnUploadPhoto_clicked();

    // Formulaire
    void on_cherchBtnAjouterChercheur_clicked();
    void on_cherchBtnAnnulerAjout_clicked();

    // CRUD
    void on_cherchModifierChercheur(int id);
    void on_cherchSupprimerChercheur(int id);
    void on_cherchVoirDetailsChercheur(int id);

    // Recherche live
    void on_cherchLineEditRecherche_textChanged(const QString &text);

    // Login / Logout
    void on_cherchUserProfileFrame_clicked();
    void on_cherchBtnLogin_clicked();

    // Mot de passe oublié
    void on_cherchBtnMotDePasseOublie_clicked();
    void on_cherchBtnRetourLogin_clicked();
    void on_cherchBtnForgotOk_clicked();  // Nouveau slot pour le bouton OK

    // Sidebar toggle
    void on_cherchBtnToggleSidebar_clicked();

    // Export détails chercheur
    void on_cherchBtnExportDetails_clicked();

    // Toggle vue icônes/liste
    void on_cherchBtnToggleVue_clicked();

private:
    void cherchSetupUI();
    void cherchConnectSignals();
    void cherchSetupAnimations();
    void cherchApplyModernStyle();
    void cherchShowLoginView();
    void cherchShowMainView();
    void cherchCheckLogin();
    void cherchShowForgotPasswordView();  // Nouvelle vue mot de passe oublié

    void cherchAfficherListeChercheurs();
    void cherchAjouterChercheurCard(int id, const QString &nom, const QString &prenom,
                                    const QString &grade, const QString &email,
                                    const QPixmap &photo);
    void cherchAjouterChercheurListItem(int id, const QString &nom, const QString &prenom,
                                        const QString &grade, const QString &email);
    void cherchClearChercheursList();
    void cherchAnimateCardEntry(QWidget *card, int index);

    // Tri
    void cherchTrierParNom(bool croissant = true);
    void cherchTrierParGrade();
    void cherchTrierParDateCreation(bool croissant = true);

    // Nouvelles fonctionnalités
    void cherchAfficherStatistiques();
    void cherchToggleSidebar();
    void cherchSetupSidebarToggle();
    void cherchAjouterDonneesTest();

    void setupForgotPasswordUI();

    int cherchCompterProjetsChercheur(int chercheurId);
    QString cherchDeterminerCarriere(int projetsCount, const QString &grade);
    int cherchCalculerAge(const QString &cin);

    Ui::MainWindow *cherchUi;
    bool cherchVueListeActive;
    bool cherchVueIconesActive;  // Nouveau : mode d'affichage (icônes vs liste)
    int cherchChercheurSelectionne;
    bool cherchIsLoggedIn;
    bool cherchSidebarVisible;

    // Widgets dynamiques
    QPushButton *cherchBtnToggleSidebar;
    QLabel *cherchLogoLabelSidebar;

    // Widget pour toggle vue
    QPushButton *cherchBtnToggleVue;

    // Données enrichies
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

    QMap<int, ChercheurData> cherchChercheursMap;
    QList<int> cherchProjetsCount;
};

#endif // MAINWINDOW_H

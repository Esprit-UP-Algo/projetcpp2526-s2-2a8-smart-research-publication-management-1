#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class SmartResearchMainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_SR_btnVueListe_clicked();
    void on_SR_btnAjouter_clicked();
    void on_SR_btnSupprimer_clicked();
    void on_SR_btnRecherche_clicked();
    void on_SR_btnTri_clicked();
    void on_SR_btnExport_clicked();
    void on_SR_btnStatistiques_clicked();
    void on_SR_btnAjouterPublication_clicked();
    void on_SR_btnAnnulerAjout_clicked();
    void on_SR_btnPublications_clicked();
    void on_SR_btnChercheurs_clicked();
    void on_SR_btnLaboratoires_clicked();
    void on_SR_btnProjets_clicked();
    void on_SR_btnFinances_clicked();
    void on_SR_btnEvenements_clicked();

private:
    Ui::SmartResearchMainWindow *ui;
    void setupConnections();
    void loadSampleData();
    void updateButtonStyles();
};

#endif // MAINWINDOW_H

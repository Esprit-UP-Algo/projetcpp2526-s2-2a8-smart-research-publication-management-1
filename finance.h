#ifndef FINANCE_H
#define FINANCE_H

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class finance : public QMainWindow
{
    Q_OBJECT

public:
    explicit finance(QWidget *parent = nullptr);
    ~finance();

private slots:
    // Slots pour la navigation du sidebar
    void onDashboardClicked(bool checked);
    void onEvenementClicked(bool checked);
    void onPublicationClicked(bool checked);
    void onChercheursClicked(bool checked);
    void onFinancesClicked(bool checked);
    void onSettingsClicked(bool checked);

    // Slots pour les onglets de la section finances
    void onListeFinClicked(bool checked);
    void onAjouterFinClicked(bool checked);

private:
    Ui::MainWindow *ui;
};

#endif // FINANCE_H

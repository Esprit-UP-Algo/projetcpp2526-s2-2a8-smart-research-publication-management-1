#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui {
class laboratoires;
}
QT_END_NAMESPACE

// Classe Laboratory
class Laboratory
{
public:
    Laboratory();
    Laboratory(int id, const QString& name, const QString& thematic,
               double budget, int capacity, const QStringList& equipment,
               const QString& status, const QString& director);

    // Getters
    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getThematic() const { return m_thematic; }
    double getBudget() const { return m_budget; }
    int getCapacity() const { return m_capacity; }
    QStringList getEquipment() const { return m_equipment; }
    QString getStatus() const { return m_status; }
    QString getDirector() const { return m_director; }

    // Setters
    void setId(int id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setThematic(const QString& thematic) { m_thematic = thematic; }
    void setBudget(double budget) { m_budget = budget; }
    void setCapacity(int capacity) { m_capacity = capacity; }
    void setEquipment(const QStringList& equipment) { m_equipment = equipment; }
    void setStatus(const QString& status) { m_status = status; }
    void setDirector(const QString& director) { m_director = director; }

private:
    int m_id;
    QString m_name;
    QString m_thematic;
    double m_budget;
    int m_capacity;
    QStringList m_equipment;
    QString m_status;
    QString m_director;
};

// Classe MainWindow
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Navigation
    void onBtnLaboratoiresClicked();
    
    // Toolbar actions
    void onBtnRechercheClicked();
    void onBtnTrierClicked();
    void onBtnExporterClicked();
    void onBtnStatistiquesClicked();
    
    // Table actions
    void onTableSelectionChanged();
    void onBtnModifierTableClicked();
    void onBtnSupprimerTableClicked();
    void onBtnOptimiseurCollabClicked();
    void onBtnPredicteurBesoinsClicked();
    
    // Form actions
    void onBtnAjouterLaboratoireClicked();
    void onBtnAnnulerClicked();
    
    // Search
    void onSearchTextChanged(const QString& text);

private:
    Ui::laboratoires *ui;
    QList<Laboratory> m_laboratories;
    int m_nextId;
    int m_editingIndex; // Index du laboratoire en cours d'édition (-1 si ajout)
    
    // Helper methods
    void initializeMockData();
    void loadTableData();
    void loadTableData(const QList<Laboratory>& labs);
    void clearForm();
    void fillFormWithLab(const Laboratory& lab);
    Laboratory getLabFromForm();
    void showStatisticsDialog();
    void showCollaborationDialog();
    void showPredictionDialog();
};
#endif // MAINWINDOW_H

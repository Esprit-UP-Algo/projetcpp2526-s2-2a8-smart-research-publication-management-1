#ifndef FINANCE_H
#define FINANCE_H

#include <QString>
#include <QDialog>
#include <QMap>
#include <QLabel>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>

QT_BEGIN_NAMESPACE
class QPieSeries;
class QBarSeries;
class QChartView;
QT_END_NAMESPACE

struct TransactionData {
    int id;
    int idProjet; // CODE_PROJET (FK FINANCE.ID_PROJET), 0 si aucun
    QString projet;
    QString type;
    double montant;
    QString date;
    QString categorie;
    QString statut;
    QString description;

    TransactionData() : id(0), idProjet(0), montant(0.0) {}
};

class FinStatistiquesDialog : public QDialog {
    Q_OBJECT
public:
    explicit FinStatistiquesDialog(const QMap<int, TransactionData> &transactions,
                                   QWidget *parent = nullptr);

private:
    void setupUI();
    void calculerStatistiques();
    void creerGraphiques();
    QMap<int, TransactionData> m_transactions;
    QLabel *labelTotalRecettes;
    QLabel *labelTotalDepenses;
    QLabel *labelSolde;
    QLabel *labelNbTransactions;
    QChartView *chartTypeView;
    QChartView *chartProjetView;
};

#endif // FINANCE_H

#ifndef PROJET_H
#define PROJET_H

#include <QString>
#include <QDate>
#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QLineSeries>
#include <QPieSeries>

QT_BEGIN_NAMESPACE
class QPieSeries;
class QBarSeries;
class QLineSeries;
class QChartView;
QT_END_NAMESPACE

struct Projet {
    int id;
    int responsableId; // ID_CHERCHEUR (FK PROJET.RESPONSABLE)
    QString code;
    QString titre;
    QDate dateDebut;
    QDate dateFin;
    QString responsable;
    QString etat;
    QString progression;
    QString description;

    Projet() : id(0), responsableId(0) {}
    Projet(int id, QString code, QString titre, QDate debut, QDate fin,
           QString resp, QString etat, QString prog, QString desc)
        : id(id),
          responsableId(0),
          code(code),
          titre(titre),
          dateDebut(debut),
          dateFin(fin),
          responsable(resp),
          etat(etat),
          progression(prog),
          description(desc) {}
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
    explicit IARecommandationsDialog(const QVector<Projet> &projets,
                                     QWidget *parent = nullptr);

private:
    void setupUI();
    void genererRecommandations();
    QVector<Projet> m_projets;
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
    explicit StatistiquesDialog(const QVector<Projet> &projets,
                                QWidget *parent = nullptr);

private:
    void setupUI();
    void calculerStatistiques();
    void creerGraphiques();
    QVector<Projet> m_projets;
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

#endif // PROJET_H

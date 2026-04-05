#include "projet.h"
#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include "ai_service.h"
#include "reminder.h"
#include <algorithm>
#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QFrame>
#include <QGroupBox>
#include <QProgressBar>
#include <QListWidget>
#include <QScrollArea>
#include <QTextBrowser>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QLatin1String>
#include <QMap>
#include <QColor>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidgetItem>
#include <QDebug>
#include <QToolTip>
#include <QSignalBlocker>
#include <QTimer>
#include <QLineEdit>
#include <QPixmap>
#include <QSet>
#include <QStringList>

QString projEtatDbToUi(const QString &db)
{
    const QString d = db.trimmed().toLower();
    if (d == QLatin1String("en_cours"))
        return QStringLiteral("En cours");
    if (d == QLatin1String("termine"))
        return QStringLiteral("Terminé");
    if (d == QLatin1String("suspendu"))
        return QStringLiteral("Suspendu");
    if (d == QLatin1String("annule"))
        return QStringLiteral("Annulé");
    return db;
}

QString projEtatColor(const QString &etat)
{
    const QString t = etat.trimmed();
    if (t == QLatin1String("en_cours") || t == QLatin1String("Actif") || t == QLatin1String("En cours"))
        return QStringLiteral("#10b981");
    if (t == QLatin1String("termine") || t == QLatin1String("Terminé"))
        return QStringLiteral("#3b82f6");
    if (t == QLatin1String("suspendu") || t == QLatin1String("En pause") || t == QLatin1String("Suspendu"))
        return QStringLiteral("#f59e0b");
    if (t == QLatin1String("annule") || t == QLatin1String("Planifié") || t == QLatin1String("Annulé"))
        return QStringLiteral("#8b5cf6");
    return QStringLiteral("#64748b");
}

QString projProgressionColor(int valeur)
{
    if (valeur >= 80)
        return QStringLiteral("#10b981");
    if (valeur >= 50)
        return QStringLiteral("#3b82f6");
    if (valeur >= 25)
        return QStringLiteral("#f59e0b");
    return QStringLiteral("#ef4444");
}

QString projProgressionColorFromString(const QString &progression)
{
    QString temp = progression;
    if (temp.endsWith(QLatin1Char('%')))
        temp.chop(1);
    const int val = temp.toInt();
    return projProgressionColor(val);
}

namespace {
static QFrame *createVerticalSeparator()
{
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setStyleSheet(QStringLiteral("color: #e2e8f0;"));
    line->setFixedWidth(1);
    return line;
}
} // namespace

FiltresDialog::FiltresDialog(QWidget *parent)
    : QDialog(parent), filtreActif(false)
{
    setWindowTitle("Filtres de Recherche");
    setMinimumSize(450, 450);
    resize(450, 450);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f8fafc;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #334155;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QComboBox {"
        "    background-color: white;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 10px 15px;"
        "    font-size: 13px;"
        "    color: #334155;"
        "    min-height: 42px;"
        "}"
        "QComboBox:focus { border-color: #3b82f6; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox QAbstractItemView {"
        "    background-color: white;"
        "    border: 1px solid #e2e8f0;"
        "    selection-background-color: #eff6ff;"
        "    color: #334155;"
        "}"
        "QDateEdit {"
        "    background-color: white;"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 10px 15px;"
        "    font-size: 13px;"
        "    color: #334155;"
        "    min-height: 42px;"
        "}"
        "QDateEdit:focus { border-color: #3b82f6; }"
        "QPushButton {"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        );

    setupUI();
}

void FiltresDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Header
    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-bottom: 1px solid #e2e8f0;"
        "}"
        );
    headerFrame->setFixedHeight(80);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(25, 15, 25, 15);

    QLabel *titleLabel = new QLabel("🔍 Filtrer les Projets");
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #1e293b;");

    QLabel *subtitleLabel = new QLabel("Affinez votre recherche avec les critères ci-dessous");
    subtitleLabel->setStyleSheet("font-size: 13px; color: #64748b;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    // Contenu
    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(25, 25, 25, 25);

    // Formulaire avec QFormLayout pour alignement parfait
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setContentsMargins(0, 0, 0, 0);

    // État
    QLabel *labelEtat = new QLabel("État du projet");
    labelEtat->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155;");

    comboBoxEtat = new QComboBox();
    comboBoxEtat->addItem("Tous les états", "");
    comboBoxEtat->addItem("Planifié", "Planifié");
    comboBoxEtat->addItem("Actif", "Actif");
    comboBoxEtat->addItem("En pause", "En pause");
    comboBoxEtat->addItem("Terminé", "Terminé");
    formLayout->addRow(labelEtat, comboBoxEtat);

    // Responsable
    QLabel *labelResp = new QLabel("Responsable");
    labelResp->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155;");

    comboBoxResponsable = new QComboBox();
    comboBoxResponsable->addItem("Tous les responsables", "");
    comboBoxResponsable->addItem("Dr. Ahmed Ben Ali", "Dr. Ahmed Ben Ali");
    comboBoxResponsable->addItem("Pr. Fatima Zohra", "Pr. Fatima Zohra");
    comboBoxResponsable->addItem("Dr. Mohamed Salah", "Dr. Mohamed Salah");
    comboBoxResponsable->addItem("Dr. Sarah Johnson", "Dr. Sarah Johnson");
    comboBoxResponsable->addItem("Pr. Robert Chen", "Pr. Robert Chen");
    formLayout->addRow(labelResp, comboBoxResponsable);

    // Période - Label
    QLabel *labelPeriode = new QLabel("Période de début");
    labelPeriode->setStyleSheet("font-size: 13px; font-weight: 600; color: #334155;");

    // Widget conteneur pour les dates avec layout horizontal
    QWidget *datesWidget = new QWidget();
    QHBoxLayout *datesLayout = new QHBoxLayout(datesWidget);
    datesLayout->setSpacing(15);
    datesLayout->setContentsMargins(0, 0, 0, 0);

    // Date début
    QVBoxLayout *debutLayout = new QVBoxLayout();
    debutLayout->setSpacing(5);
    QLabel *labelDu = new QLabel("Du");
    labelDu->setStyleSheet("font-size: 12px; color: #64748b; font-weight: normal;");
    dateEditDebutMin = new QDateEdit();
    dateEditDebutMin->setCalendarPopup(true);
    dateEditDebutMin->setDate(QDate(2020, 1, 1));
    dateEditDebutMin->setDisplayFormat("dd/MM/yyyy");
    dateEditDebutMin->setMinimumHeight(42);
    debutLayout->addWidget(labelDu);
    debutLayout->addWidget(dateEditDebutMin);

    // Date fin
    QVBoxLayout *finLayout = new QVBoxLayout();
    finLayout->setSpacing(5);
    QLabel *labelAu = new QLabel("Au");
    labelAu->setStyleSheet("font-size: 12px; color: #64748b; font-weight: normal;");
    dateEditMax = new QDateEdit();
    dateEditMax->setCalendarPopup(true);
    dateEditMax->setDate(QDate::currentDate().addYears(5));
    dateEditMax->setDisplayFormat("dd/MM/yyyy");
    dateEditMax->setMinimumHeight(42);
    finLayout->addWidget(labelAu);
    finLayout->addWidget(dateEditMax);

    datesLayout->addLayout(debutLayout, 1);
    datesLayout->addLayout(finLayout, 1);

    formLayout->addRow(labelPeriode, datesWidget);

    contentLayout->addLayout(formLayout);
    contentLayout->addStretch();

    mainLayout->addWidget(contentWidget, 1);

    // Footer avec boutons
    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *buttonLayout = new QHBoxLayout(footerFrame);
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(25, 0, 25, 0);

    btnReinitialiser = new QPushButton("🔄 Réinitialiser");
    btnReinitialiser->setStyleSheet(
        "QPushButton {"
        "    background-color: #e2e8f0;"
        "    color: #475569;"
        "}"
        "QPushButton:hover { background-color: #cbd5e1; }"
        );

    buttonLayout->addWidget(btnReinitialiser);
    buttonLayout->addStretch();

    btnAnnuler = new QPushButton("Annuler");
    btnAnnuler->setStyleSheet(
        "QPushButton {"
        "    background-color: white;"
        "    color: #64748b;"
        "    border: 2px solid #e2e8f0;"
        "}"
        "QPushButton:hover { background-color: #f1f5f9; }"
        );

    btnAppliquer = new QPushButton("Appliquer");
    btnAppliquer->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );

    connect(btnReinitialiser, &QPushButton::clicked, [=]() {
        comboBoxEtat->setCurrentIndex(0);
        comboBoxResponsable->setCurrentIndex(0);
        dateEditDebutMin->setDate(QDate(2020, 1, 1));
        dateEditMax->setDate(QDate::currentDate().addYears(5));
    });

    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnAppliquer, &QPushButton::clicked, [=]() {
        filtreActif = true;
        accept();
    });

    buttonLayout->addWidget(btnAnnuler);
    buttonLayout->addWidget(btnAppliquer);

    mainLayout->addWidget(footerFrame);
}

QString FiltresDialog::getEtatFiltre() const
{
    return comboBoxEtat->currentData().toString();
}

QString FiltresDialog::getResponsableFiltre() const
{
    return comboBoxResponsable->currentData().toString();
}

QDate FiltresDialog::getDateDebutMin() const
{
    return dateEditDebutMin->date();
}

QDate FiltresDialog::getDateDebutMax() const
{
    return dateEditMax->date();
}

bool FiltresDialog::isFiltreActif() const
{
    return filtreActif;
}

// ==================== IA RECOMMANDATIONS DIALOG ====================


// ============================================================================
// MODULE PROJETS - IA RECOMMANDATIONS
// ============================================================================

IARecommandationsDialog::IARecommandationsDialog(const QVector<Projet> &projets, QWidget *parent)
    : QDialog(parent), m_projets(projets)
{
    setWindowTitle("Recommandations Intelligentes");
    setMinimumSize(900, 700);
    resize(1000, 800);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f1f5f9;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #334155;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        "QProgressBar {"
        "    border: 2px solid #e2e8f0;"
        "    border-radius: 10px;"
        "    text-align: center;"
        "    height: 24px;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "    color: #334155;"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 8px;"
        "}"
        "QListWidget {"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    background-color: #f8fafc;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid #e2e8f0;"
        "    color: #475569;"
        "}"
        "QListWidget::item:last {"
        "    border-bottom: none;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #eff6ff;"
        "    color: #1e293b;"
        "    border-radius: 6px;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        );

    setupUI();
}

void IARecommandationsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    border: none;"
        "}"
        );
    headerFrame->setFixedHeight(100);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel *titleLabel = new QLabel("🤖 Recommandations Intelligentes");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *subtitleLabel = new QLabel("Analyse sémantique et génération de projets basée sur l'IA");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background-color: #f1f5f9;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *statsFrame = new QFrame();
    statsFrame->setStyleSheet(
        "QFrame {"
        "    background-color: white;"
        "    border-radius: 12px;"
        "    border: 1px solid #e2e8f0;"
        "}"
        );
    statsFrame->setFixedHeight(120);

    QHBoxLayout *statsLayout = new QHBoxLayout(statsFrame);
    statsLayout->setSpacing(30);
    statsLayout->setContentsMargins(25, 20, 25, 20);

    QVBoxLayout *stat1Layout = new QVBoxLayout();
    QLabel *stat1Value = new QLabel(QString::number(m_projets.size()));
    stat1Value->setStyleSheet("font-size: 32px; font-weight: bold; color: #3b82f6;");
    stat1Value->setAlignment(Qt::AlignCenter);
    QLabel *stat1Label = new QLabel("Projets analysés");
    stat1Label->setStyleSheet("font-size: 13px; color: #64748b;");
    stat1Label->setAlignment(Qt::AlignCenter);
    stat1Layout->addWidget(stat1Value);
    stat1Layout->addWidget(stat1Label);
    statsLayout->addLayout(stat1Layout);

    statsLayout->addWidget(createVerticalSeparator());

    QVBoxLayout *stat2Layout = new QVBoxLayout();
    int actifs = 0;
    for (const auto &p : m_projets)
        if (p.etat == QLatin1String("en_cours")) actifs++;
    QLabel *stat2Value = new QLabel(QString::number(actifs));
    stat2Value->setStyleSheet("font-size: 32px; font-weight: bold; color: #10b981;");
    stat2Value->setAlignment(Qt::AlignCenter);
    QLabel *stat2Label = new QLabel("Projets actifs");
    stat2Label->setStyleSheet("font-size: 13px; color: #64748b;");
    stat2Label->setAlignment(Qt::AlignCenter);
    stat2Layout->addWidget(stat2Value);
    stat2Layout->addWidget(stat2Label);
    statsLayout->addLayout(stat2Layout);

    statsLayout->addWidget(createVerticalSeparator());

    QVBoxLayout *stat3Layout = new QVBoxLayout();
    QLabel *stat3Value = new QLabel("3");
    stat3Value->setStyleSheet("font-size: 32px; font-weight: bold; color: #8b5cf6;");
    stat3Value->setAlignment(Qt::AlignCenter);
    QLabel *stat3Label = new QLabel("Recommandations");
    stat3Label->setStyleSheet("font-size: 13px; color: #64748b;");
    stat3Label->setAlignment(Qt::AlignCenter);
    stat3Layout->addWidget(stat3Value);
    stat3Layout->addWidget(stat3Label);
    statsLayout->addLayout(stat3Layout);

    contentLayout->addWidget(statsFrame);

    genererRecommandations();

    for (int i = 0; i < m_recommandations.size(); ++i) {
        const auto &rec = m_recommandations[i];

        QGroupBox *recGroup = new QGroupBox(QString("Recommandation #%1 - %2").arg(i+1).arg(rec.domaine));

        QVBoxLayout *recLayout = new QVBoxLayout(recGroup);
        recLayout->setSpacing(15);
        recLayout->setContentsMargins(20, 20, 20, 20);

        QHBoxLayout *headerRecLayout = new QHBoxLayout();

        QLabel *titreLabel = new QLabel(rec.titre);
        titreLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1e293b;");
        titreLabel->setWordWrap(true);
        headerRecLayout->addWidget(titreLabel, 1);

        QVBoxLayout *scoreLayout = new QVBoxLayout();
        scoreLayout->setSpacing(2);
        QLabel *scoreLabel = new QLabel(QString("%1%").arg(qRound(rec.scoreSimilarite)));
        scoreLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #10b981;");
        scoreLabel->setAlignment(Qt::AlignRight);
        QLabel *scoreText = new QLabel("Pertinence");
        scoreText->setStyleSheet("font-size: 11px; color: #64748b;");
        scoreText->setAlignment(Qt::AlignRight);
        scoreLayout->addWidget(scoreLabel);
        scoreLayout->addWidget(scoreText);
        headerRecLayout->addLayout(scoreLayout);

        recLayout->addLayout(headerRecLayout);

        QProgressBar *scoreBar = new QProgressBar();
        scoreBar->setValue(qRound(rec.scoreSimilarite));
        scoreBar->setTextVisible(false);
        scoreBar->setFixedHeight(8);

        QString color;
        if (rec.scoreSimilarite >= 85) color = "#10b981";
        else if (rec.scoreSimilarite >= 70) color = "#3b82f6";
        else color = "#f59e0b";

        scoreBar->setStyleSheet(QString(
                                    "QProgressBar { border: none; border-radius: 4px; background-color: #e2e8f0; }"
                                    "QProgressBar::chunk { border-radius: 4px; background-color: %1; }"
                                    ).arg(color));
        recLayout->addWidget(scoreBar);

        QLabel *descLabel = new QLabel(rec.description);
        descLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.6;");
        descLabel->setWordWrap(true);
        recLayout->addWidget(descLabel);

        QLabel *raisonLabel = new QLabel(rec.raison);
        raisonLabel->setStyleSheet(
            "font-size: 13px; color: #3b82f6; "
            "background-color: #eff6ff; padding: 10px; "
            "border-radius: 8px; border-left: 4px solid #3b82f6;"
            );
        raisonLabel->setWordWrap(true);
        recLayout->addWidget(raisonLabel);

        QLabel *collabTitle = new QLabel("Collaborateurs suggérés");
        collabTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #334155; margin-top: 10px;");
        recLayout->addWidget(collabTitle);

        QListWidget *collabList = new QListWidget();
        collabList->setFixedHeight(100);
        for (const QString &c : rec.collaborateursSuggeres) {
            collabList->addItem(c);
        }
        recLayout->addWidget(collabList);

        contentLayout->addWidget(recGroup);
    }

    QGroupBox *insightsGroup = new QGroupBox("Insights & Patterns détectés");
    QVBoxLayout *insightsLayout = new QVBoxLayout(insightsGroup);
    insightsLayout->setContentsMargins(20, 20, 20, 20);

    QTextBrowser *insightsBrowser = new QTextBrowser();
    insightsBrowser->setFixedHeight(150);
    insightsBrowser->setStyleSheet(
        "QTextBrowser {"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "    background-color: #f8fafc;"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "    color: #475569;"
        "}"
        );

    QString insightsHtml = QString(R"(
        <h3 style='color: #1e293b; margin-top: 0;'>Analyse des projets existants</h3>
        <ul style='margin: 10px 0; padding-left: 20px;'>
            <li><b>Domaines dominants :</b> Intelligence Artificielle, Biotechnologie, Énergies Renouvelables</li>
            <li><b>Taux de réussite :</b> 85%% pour les projets interdisciplinaires</li>
            <li><b>Durée optimale :</b> 18-24 mois pour maximiser l'impact</li>
            <li><b>Collaboration :</b> Projets à 3+ chercheurs = +40%% de publications</li>
        </ul>
        <p style='color: #059669; font-weight: 600; margin: 10px 0 0 0;'>
            Recommandation stratégique : Privilégiez les consortiums multi-laboratoires
        </p>
    )");
    insightsBrowser->setHtml(insightsHtml);
    insightsLayout->addWidget(insightsBrowser);
    contentLayout->addWidget(insightsGroup);

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(30, 0, 30, 0);

    footerLayout->addStretch();

    QPushButton *closeButton = new QPushButton("J'ai compris, fermer");
    closeButton->setFixedSize(180, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    footerLayout->addWidget(closeButton);
    mainLayout->addWidget(footerFrame);
}

void IARecommandationsDialog::genererRecommandations()
{
    m_recommandations.clear();

    const QVector<AIService::Recommandation> recs = AIService::genererRecommandations(m_projets);
    m_recommandations.reserve(recs.size());

    for (const auto &r : recs) {
        Recommandation rec;
        rec.titre = r.titre;
        rec.description = r.description;
        rec.scoreSimilarite = r.scoreSimilarite;
        rec.collaborateursSuggeres = r.collaborateursSuggeres;
        rec.raison = r.raison;
        rec.domaine = r.domaine;
        m_recommandations.append(rec);
    }
}

// ==================== PROJET DETAILS DIALOG ====================


// ============================================================================
// MODULE PROJETS - DETAILS
// ============================================================================

ProjetDetailsDialog::ProjetDetailsDialog(const Projet &projet, QWidget *parent)
    : QDialog(parent), m_projet(projet)
{
    setWindowTitle("Détails du Projet - " + projet.titre);
    setMinimumSize(700, 600);
    resize(750, 650);

    setStyleSheet("QDialog { background-color: #1e1e1e; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QVBoxLayout *headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(10);
    headerLayout->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(projet.titre);
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #ffffff;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    QLabel *codeLabel = new QLabel("Code: " + projet.code);
    codeLabel->setStyleSheet("font-size: 14px; color: #a0a0a0;");
    codeLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(codeLabel);
    mainLayout->addLayout(headerLayout);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #3a3a3a; border: none;");
    line->setFixedHeight(2);
    mainLayout->addWidget(line);

    QGroupBox *infoGroup = new QGroupBox("Informations du Projet");
    infoGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border: 2px solid #3a3a3a;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: #2d2d2d;"
        "    color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #4a9eff;"
        "}"
        );

    QGridLayout *infoLayout = new QGridLayout(infoGroup);
    infoLayout->setSpacing(15);
    infoLayout->setContentsMargins(20, 20, 20, 20);
    infoLayout->setColumnStretch(1, 1);
    infoLayout->setColumnStretch(3, 1);

    QString labelStyle = "color: #b0b0b0; font-size: 13px;";
    QString valueStyle = "color: #ffffff; font-size: 13px; font-weight: 600;";

    QLabel *codeLabelInfo = new QLabel("Code:");
    codeLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(codeLabelInfo, 0, 0);

    QLabel *codeValue = new QLabel(projet.code);
    codeValue->setStyleSheet(valueStyle);
    infoLayout->addWidget(codeValue, 0, 1);

    QLabel *respLabelInfo = new QLabel("Responsable:");
    respLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(respLabelInfo, 0, 2);

    QLabel *respValue = new QLabel(projet.responsable);
    respValue->setStyleSheet("color: #4a9eff; font-size: 13px; font-weight: 600;");
    infoLayout->addWidget(respValue, 0, 3);

    QLabel *debutLabel = new QLabel("Début:");
    debutLabel->setStyleSheet(labelStyle);
    infoLayout->addWidget(debutLabel, 1, 0);

    QLabel *debutValue = new QLabel(projet.dateDebut.toString("dd/MM/yyyy"));
    debutValue->setStyleSheet(valueStyle);
    infoLayout->addWidget(debutValue, 1, 1);

    QLabel *finLabel = new QLabel("Fin:");
    finLabel->setStyleSheet(labelStyle);
    infoLayout->addWidget(finLabel, 1, 2);

    QLabel *finValue = new QLabel(projet.dateFin.toString("dd/MM/yyyy"));
    finValue->setStyleSheet(valueStyle);
    infoLayout->addWidget(finValue, 1, 3);

    QLabel *etatLabelInfo = new QLabel("État:");
    etatLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(etatLabelInfo, 2, 0);

    QLabel *etatBadge = new QLabel(projEtatDbToUi(projet.etat));
    QString etatColor = projEtatColor(projEtatDbToUi(projet.etat));
    etatBadge->setStyleSheet(QString(
                                 "background-color: %1;"
                                 "color: white;"
                                 "padding: 6px 16px;"
                                 "border-radius: 6px;"
                                 "font-weight: bold;"
                                 "font-size: 12px;"
                                 ).arg(etatColor));
    etatBadge->setAlignment(Qt::AlignCenter);
    etatBadge->setFixedWidth(100);
    infoLayout->addWidget(etatBadge, 2, 1, Qt::AlignLeft);

    QLabel *progLabelInfo = new QLabel("Progression:");
    progLabelInfo->setStyleSheet(labelStyle);
    infoLayout->addWidget(progLabelInfo, 2, 2);

    QHBoxLayout *progLayout = new QHBoxLayout();
    progLayout->setSpacing(10);

    QProgressBar *progressBar = new QProgressBar();
    QString progStr = projet.progression;
    if (progStr.endsWith('%')) progStr.chop(1);
    int progValue = progStr.toInt();
    progressBar->setValue(progValue);
    progressBar->setTextVisible(false);
    progressBar->setFixedHeight(20);
    progressBar->setStyleSheet(QString(
                                   "QProgressBar {"
                                   "    border: none;"
                                   "    border-radius: 10px;"
                                   "    background-color: #3a3a3a;"
                                   "    text-align: center;"
                                   "}"
                                   "QProgressBar::chunk {"
                                   "    border-radius: 10px;"
                                   "    background-color: %1;"
                                   "}"
                                   ).arg(projProgressionColor(progValue)));

    QLabel *progText = new QLabel(projet.progression);
    progText->setStyleSheet("color: #ffffff; font-weight: bold; font-size: 13px;");
    progText->setFixedWidth(45);

    progLayout->addWidget(progressBar, 1);
    progLayout->addWidget(progText);
    infoLayout->addLayout(progLayout, 2, 3);

    mainLayout->addWidget(infoGroup);

    QGroupBox *descGroup = new QGroupBox("Description");
    descGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    border: 2px solid #3a3a3a;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: #2d2d2d;"
        "    color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #4a9eff;"
        "}"
        );

    QVBoxLayout *descLayout = new QVBoxLayout(descGroup);
    descLayout->setContentsMargins(15, 20, 15, 15);

    QTextBrowser *descBrowser = new QTextBrowser();
    descBrowser->setPlainText(projet.description.isEmpty() ? "Aucune description disponible." : projet.description);
    descBrowser->setStyleSheet(
        "QTextBrowser {"
        "    border: 1px solid #3a3a3a;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "    background-color: #1e1e1e;"
        "    color: #e0e0e0;"
        "    font-size: 14px;"
        "    line-height: 1.6;"
        "}"
        );
    descBrowser->setMinimumHeight(150);
    descBrowser->setReadOnly(true);
    descLayout->addWidget(descBrowser);

    mainLayout->addWidget(descGroup, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setFixedSize(120, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4a9eff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #3a8eef;"
        "}"
        );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}

// ==================== STATISTIQUES DIALOG ====================


// ============================================================================
// MODULE PROJETS - STATISTIQUES
// ============================================================================

StatistiquesDialog::StatistiquesDialog(const QVector<Projet> &projets, QWidget *parent)
    : QDialog(parent), m_projets(projets)
    , labelTotalProjets(nullptr)
    , labelProjetsActifs(nullptr)
    , labelProjetsTermines(nullptr)
    , labelProgressionMoyenne(nullptr)
    , labelProjetsRetard(nullptr)
    , labelProjetsPlanifies(nullptr)
    , labelProjetsPause(nullptr)
    , chartEtatView(nullptr)
    , chartProgressionView(nullptr)
    , chartTemporelView(nullptr)
{
    setWindowTitle("Statistiques Complètes des Projets");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    setStyleSheet(
        "QDialog {"
        "    background-color: #f1f5f9;"
        "    font-family: 'Segoe UI', 'Roboto', sans-serif;"
        "}"
        "QLabel {"
        "    color: #334155;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #e2e8f0;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 15px;"
        "    background-color: white;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 0 10px;"
        "    color: #3b82f6;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        );

    setupUI();
    calculerStatistiques();
    creerGraphiques();
}

void StatistiquesDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    border: none;"
        "}"
        );
    headerFrame->setFixedHeight(100);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setSpacing(5);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel *titleLabel = new QLabel("📊 Tableau de Bord Statistique");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *subtitleLabel = new QLabel("Analyse complète des projets de recherche");
    subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(headerFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background-color: #f1f5f9;");

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(25);
    contentLayout->setContentsMargins(30, 30, 30, 30);

    QFrame *kpiFrame = new QFrame();
    kpiFrame->setStyleSheet("QFrame { background-color: transparent; border: none; }");

    QHBoxLayout *kpiLayout = new QHBoxLayout(kpiFrame);
    kpiLayout->setSpacing(20);
    kpiLayout->setContentsMargins(0, 0, 0, 0);

    auto createKPI = [](const QString &icon, const QString &value, const QString &label,
                        const QString &color, QLabel **valueLabelPtr) -> QFrame* {
        QFrame *kpi = new QFrame();
        kpi->setStyleSheet(
            "QFrame {"
            "    background-color: white;"
            "    border-radius: 12px;"
            "    border: 1px solid #e2e8f0;"
            "}"
            );
        kpi->setFixedHeight(140);
        QVBoxLayout *layout = new QVBoxLayout(kpi);
        layout->setSpacing(5);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 28px;");
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);
        *valueLabelPtr = valueLabel;

        QLabel *textLabel = new QLabel(label);
        textLabel->setStyleSheet("font-size: 13px; color: #64748b;");
        textLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(textLabel);
        return kpi;
    };

    kpiLayout->addWidget(createKPI("📁", "0", "Total Projets", "#3b82f6", &labelTotalProjets));
    kpiLayout->addWidget(createKPI("▶️", "0", "Projets Actifs", "#10b981", &labelProjetsActifs));
    kpiLayout->addWidget(createKPI("✅", "0", "Projets Terminés", "#3b82f6", &labelProjetsTermines));
    kpiLayout->addWidget(createKPI("📈", "0%", "Progression Moyenne", "#8b5cf6", &labelProgressionMoyenne));
    kpiLayout->addWidget(createKPI("⚠️", "0", "Projets en Retard", "#ef4444", &labelProjetsRetard));

    contentLayout->addWidget(kpiFrame);

    QHBoxLayout *chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(20);

    QGroupBox *chartEtatGroup = new QGroupBox("Répartition par État");
    QVBoxLayout *chartEtatLayout = new QVBoxLayout(chartEtatGroup);
    chartEtatView = new QChartView();
    chartEtatView->setMinimumHeight(350);
    chartEtatView->setRenderHint(QPainter::Antialiasing);
    chartEtatLayout->addWidget(chartEtatView);
    chartsLayout->addWidget(chartEtatGroup, 1);

    QGroupBox *chartProgressionGroup = new QGroupBox("Progression des Projets");
    QVBoxLayout *chartProgressionLayout = new QVBoxLayout(chartProgressionGroup);
    chartProgressionView = new QChartView();
    chartProgressionView->setMinimumHeight(350);
    chartProgressionView->setRenderHint(QPainter::Antialiasing);
    chartProgressionLayout->addWidget(chartProgressionView);
    chartsLayout->addWidget(chartProgressionGroup, 1);

    contentLayout->addLayout(chartsLayout);

    QGroupBox *chartTemporelGroup = new QGroupBox("Timeline des Projets");
    QVBoxLayout *chartTemporelLayout = new QVBoxLayout(chartTemporelGroup);
    chartTemporelView = new QChartView();
    chartTemporelView->setMinimumHeight(300);
    chartTemporelView->setRenderHint(QPainter::Antialiasing);
    chartTemporelLayout->addWidget(chartTemporelView);
    contentLayout->addWidget(chartTemporelGroup);

    QGroupBox *detailsGroup = new QGroupBox("Détails par État");
    QHBoxLayout *detailsLayout = new QHBoxLayout(detailsGroup);
    detailsLayout->setSpacing(30);
    detailsLayout->setContentsMargins(20, 20, 20, 20);

    auto createDetailItem = [](const QString &icon, const QString &label,
                               const QString &color, QLabel **valueLabelPtr) -> QVBoxLayout* {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(8);
        layout->setAlignment(Qt::AlignCenter);

        QLabel *iconLabel = new QLabel(icon);
        iconLabel->setStyleSheet("font-size: 24px;");
        iconLabel->setAlignment(Qt::AlignCenter);

        QLabel *valueLabel = new QLabel("0");
        valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);
        *valueLabelPtr = valueLabel;

        QLabel *textLabel = new QLabel(label);
        textLabel->setStyleSheet("font-size: 13px; color: #64748b;");
        textLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(valueLabel);
        layout->addWidget(textLabel);
        return layout;
    };

    detailsLayout->addLayout(createDetailItem("📋", "Planifiés", "#8b5cf6", &labelProjetsPlanifies));
    detailsLayout->addLayout(createDetailItem("⏸️", "En pause", "#f59e0b", &labelProjetsPause));
    detailsLayout->addStretch();

    contentLayout->addWidget(detailsGroup);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);

    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(30, 0, 30, 0);
    footerLayout->addStretch();

    QPushButton *closeButton = new QPushButton("Fermer");
    closeButton->setFixedSize(140, 45);
    closeButton->setCursor(Qt::PointingHandCursor);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    footerLayout->addWidget(closeButton);
    mainLayout->addWidget(footerFrame);
}

void StatistiquesDialog::calculerStatistiques()
{
    int actifs = 0, termines = 0, pause = 0, planifies = 0;
    double progressionTotale = 0;
    int nbProjetsActifsOuPause = 0;
    int projetsEnRetard = 0;

    for (const auto &p : m_projets) {
        if (p.etat == QLatin1String("en_cours")) actifs++;
        else if (p.etat == QLatin1String("termine")) termines++;
        else if (p.etat == QLatin1String("suspendu")) pause++;
        else if (p.etat == QLatin1String("annule")) planifies++;

        if (p.etat != QLatin1String("termine") && p.etat != QLatin1String("annule")) {
            QString progStr = p.progression;
            if (progStr.endsWith('%')) progStr.chop(1);
            progressionTotale += progStr.toInt();
            nbProjetsActifsOuPause++;

            if (QDate::currentDate() > p.dateFin) {
                projetsEnRetard++;
            }
        }
    }

    double progressionMoyenne = nbProjetsActifsOuPause > 0 ?
                                    progressionTotale / nbProjetsActifsOuPause : 0;

    if (labelTotalProjets) labelTotalProjets->setText(QString::number(m_projets.size()));
    if (labelProjetsActifs) labelProjetsActifs->setText(QString::number(actifs));
    if (labelProjetsTermines) labelProjetsTermines->setText(QString::number(termines));
    if (labelProgressionMoyenne) labelProgressionMoyenne->setText(QString::number(qRound(progressionMoyenne)) + "%");
    if (labelProjetsRetard) labelProjetsRetard->setText(QString::number(projetsEnRetard));
    if (labelProjetsPause) labelProjetsPause->setText(QString::number(pause));
    if (labelProjetsPlanifies) labelProjetsPlanifies->setText(QString::number(planifies));
}

void StatistiquesDialog::creerGraphiques()
{
    QPieSeries *seriesEtat = new QPieSeries();

    int actifs = 0, termines = 0, pause = 0, planifies = 0;
    for (const auto &p : m_projets) {
        if (p.etat == QLatin1String("en_cours")) actifs++;
        else if (p.etat == QLatin1String("termine")) termines++;
        else if (p.etat == QLatin1String("suspendu")) pause++;
        else if (p.etat == QLatin1String("annule")) planifies++;
    }

    if (actifs > 0) seriesEtat->append("Actifs", actifs);
    if (termines > 0) seriesEtat->append("Terminés", termines);
    if (pause > 0) seriesEtat->append("En pause", pause);
    if (planifies > 0) seriesEtat->append("Planifiés", planifies);

    QList<QColor> colors = { QColor("#10b981"), QColor("#3b82f6"), QColor("#f59e0b"), QColor("#8b5cf6") };
    for (int i = 0; i < seriesEtat->count() && i < colors.size(); ++i) {
        seriesEtat->slices().at(i)->setColor(colors[i]);
        seriesEtat->slices().at(i)->setLabelVisible(true);
        seriesEtat->slices().at(i)->setLabel(QString("%1 (%2%)")
                                                 .arg(seriesEtat->slices().at(i)->label())
                                                 .arg(qRound(seriesEtat->slices().at(i)->percentage() * 100)));
    }

    QChart *chartEtat = new QChart();
    chartEtat->addSeries(seriesEtat);
    chartEtat->setTitle("Répartition des projets par état");
    chartEtat->setAnimationOptions(QChart::SeriesAnimations);
    chartEtat->legend()->setAlignment(Qt::AlignRight);
    chartEtat->setBackgroundBrush(QBrush(QColor("transparent")));
    chartEtatView->setChart(chartEtat);

    QBarSeries *seriesProgression = new QBarSeries();
    QBarSet *setProgression = new QBarSet("Progression %");

    QStringList categories;
    for (const auto &p : m_projets) {
        QString progStr = p.progression;
        if (progStr.endsWith('%')) progStr.chop(1);
        *setProgression << progStr.toInt();
        categories << p.code;
    }

    seriesProgression->append(setProgression);
    setProgression->setColor(QColor("#3b82f6"));

    QChart *chartProgression = new QChart();
    chartProgression->addSeries(seriesProgression);
    chartProgression->setTitle("Progression par projet");
    chartProgression->setAnimationOptions(QChart::SeriesAnimations);
    chartProgression->setBackgroundBrush(QBrush(QColor("transparent")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartProgression->addAxis(axisX, Qt::AlignBottom);
    seriesProgression->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%d%%");
    chartProgression->addAxis(axisY, Qt::AlignLeft);
    seriesProgression->attachAxis(axisY);

    chartProgression->legend()->setVisible(false);
    chartProgressionView->setChart(chartProgression);

    QLineSeries *seriesTimeline = new QLineSeries();
    seriesTimeline->setName("Projets démarrés");
    seriesTimeline->setColor(QColor("#10b981"));
    seriesTimeline->setPen(QPen(QColor("#10b981"), 3));

    QVector<Projet> projetsSorted = m_projets;
    std::sort(projetsSorted.begin(), projetsSorted.end(),
              [](const Projet &a, const Projet &b) { return a.dateDebut < b.dateDebut; });

    QMap<QString, int> projetsParMois;
    for (const auto &p : projetsSorted) {
        QString mois = p.dateDebut.toString("MMM yyyy");
        projetsParMois[mois]++;
    }

    int i = 0;
    for (auto it = projetsParMois.begin(); it != projetsParMois.end(); ++it, ++i) {
        seriesTimeline->append(i, it.value());
    }

    QChart *chartTimeline = new QChart();
    chartTimeline->addSeries(seriesTimeline);
    chartTimeline->setTitle("Nombre de projets démarrés par mois");
    chartTimeline->setAnimationOptions(QChart::SeriesAnimations);
    chartTimeline->setBackgroundBrush(QBrush(QColor("transparent")));

    QValueAxis *axisXTime = new QValueAxis();
    axisXTime->setRange(0, projetsParMois.size() - 1);
    axisXTime->setLabelFormat("%d");
    chartTimeline->addAxis(axisXTime, Qt::AlignBottom);
    seriesTimeline->attachAxis(axisXTime);

    QValueAxis *axisYTime = new QValueAxis();
    axisYTime->setRange(0, *std::max_element(projetsParMois.begin(), projetsParMois.end()) + 1);
    axisYTime->setLabelFormat("%d");
    chartTimeline->addAxis(axisYTime, Qt::AlignLeft);
    seriesTimeline->attachAxis(axisYTime);

    chartTimeline->legend()->setVisible(false);
    chartTemporelView->setChart(chartTimeline);
}

// ============================================================================
// SmartPub — module Projets (implémentations déplacées depuis smartpub.cpp)
// ============================================================================

namespace {

constexpr int kProjetRowInternalIdRole = Qt::UserRole + 64;

// --- Métadonnées Oracle (USER_TAB_COLUMNS) : pas de noms de colonnes supposés ---
struct ProjetOracleMeta {
    QString table = QStringLiteral("PROJET");
    QString code;
    QString titre;
    QString dateDebut;
    QString dateFin;
    QString responsable;
    QString etat;
    QString progression;
    bool chercheurHasPrenom = true;
    bool fromDictionary = false;
};

static ProjetOracleMeta g_projOra;
static bool g_projOraInit = false;

static QString projOraPickCol(const QSet<QString> &avail, const QStringList &candidates)
{
    for (const QString &c : candidates) {
        if (avail.contains(c))
            return c;
    }
    for (const QString &col : avail) {
        for (const QString &c : candidates) {
            if (col.compare(c, Qt::CaseInsensitive) == 0)
                return col;
        }
    }
    return {};
}

static void projEnsureOracleProjetMeta(QSqlDatabase &db)
{
    if (g_projOraInit)
        return;
    g_projOraInit = true;

    QSqlQuery q(db);
    const QStringList tableCandidates = {QStringLiteral("PROJET"), QStringLiteral("PROJETS")};

    for (const QString &tname : tableCandidates) {
        QSet<QString> cols;
        qDebug() << QStringLiteral("[Oracle SQL] USER_TAB_COLUMNS — table:") << tname;
        q.prepare(QStringLiteral(
            "SELECT column_name FROM user_tab_columns WHERE UPPER(table_name) = :tn ORDER BY column_id"));
        q.bindValue(QStringLiteral(":tn"), tname.toUpper());
        if (!q.exec()) {
            qDebug() << QStringLiteral("[Oracle SQL] Erreur user_tab_columns:") << q.lastError().text();
            continue;
        }
        while (q.next())
            cols.insert(q.value(0).toString().toUpper());

        qDebug() << QStringLiteral("[Oracle SQL] Colonnes trouvées (%1):").arg(tname) << cols.values();

        if (cols.isEmpty())
            continue;

        ProjetOracleMeta m;
        m.table = tname;
        m.code = projOraPickCol(
            cols, {QStringLiteral("CODE"), QStringLiteral("CODE_PROJET"), QStringLiteral("CODPROJ"),
                   QStringLiteral("REF_PROJET"), QStringLiteral("REF_CODE"), QStringLiteral("NUM_CODE")});
        m.titre = projOraPickCol(
            cols, {QStringLiteral("TITRE"), QStringLiteral("TITRE_PROJET"), QStringLiteral("NOM_PROJET"),
                   QStringLiteral("LIBELLE")});
        m.dateDebut = projOraPickCol(
            cols, {QStringLiteral("DATE_DEBUT"), QStringLiteral("DATEDEBUT"), QStringLiteral("DT_DEBUT")});
        m.dateFin = projOraPickCol(
            cols, {QStringLiteral("DATE_FIN"), QStringLiteral("DATEFIN"), QStringLiteral("DT_FIN")});
        m.responsable = projOraPickCol(
            cols, {QStringLiteral("RESPONSABLE"), QStringLiteral("ID_RESPONSABLE"),
                   QStringLiteral("RESPONSABLE_ID"), QStringLiteral("ID_CHERCHEUR_RESP")});
        m.etat = projOraPickCol(
            cols, {QStringLiteral("ETAT"), QStringLiteral("STATUT"), QStringLiteral("ETAT_PROJET")});
        m.progression = projOraPickCol(
            cols, {QStringLiteral("PROGRESSION"), QStringLiteral("AVANCEMENT"),
                   QStringLiteral("PCT_PROGRESSION")});

        if (!m.code.isEmpty() && !m.titre.isEmpty() && !m.dateDebut.isEmpty() && !m.dateFin.isEmpty()
            && !m.responsable.isEmpty()) {
            g_projOra = m;
            g_projOra.fromDictionary = true;
            qDebug() << QStringLiteral("[Oracle SQL] PROJET colonnes retenues — CODE-like:")
                     << g_projOra.code << QStringLiteral("TITRE:") << g_projOra.titre
                     << QStringLiteral("ETAT:") << g_projOra.etat << QStringLiteral("PROGRESSION:")
                     << g_projOra.progression;
            break;
        }
        qDebug() << QStringLiteral("[Oracle SQL] Table %1 ignorée (colonnes obligatoires manquantes).")
                        .arg(tname);
    }

    if (!g_projOra.fromDictionary) {
        qDebug() << QStringLiteral(
            "[Oracle SQL] Dictionnaire indisponible ou table non reconnue — fallback script SmartPub1.sql");
        g_projOra.table = QStringLiteral("PROJET");
        g_projOra.code = QStringLiteral("CODE");
        g_projOra.titre = QStringLiteral("TITRE");
        g_projOra.dateDebut = QStringLiteral("DATE_DEBUT");
        g_projOra.dateFin = QStringLiteral("DATE_FIN");
        g_projOra.responsable = QStringLiteral("RESPONSABLE");
        g_projOra.etat = QStringLiteral("ETAT");
        g_projOra.progression = QStringLiteral("PROGRESSION");
    }

    QSet<QString> chCols;
    q.prepare(QStringLiteral(
        "SELECT column_name FROM user_tab_columns WHERE UPPER(table_name) = 'CHERCHEUR' ORDER BY column_id"));
    if (q.exec()) {
        while (q.next())
            chCols.insert(q.value(0).toString().toUpper());
        qDebug() << QStringLiteral("[Oracle SQL] Colonnes CHERCHEUR:") << chCols.values();
        g_projOra.chercheurHasPrenom = chCols.isEmpty() || chCols.contains(QStringLiteral("PRENOM"));
    } else {
        qDebug() << QStringLiteral("[Oracle SQL] user_tab_columns CHERCHEUR:")
                 << q.lastError().text();
        g_projOra.chercheurHasPrenom = true;
    }
}

static void projLogSql(const char *tag, const QString &sql)
{
    qDebug() << QStringLiteral("[PROJET SQL %1] %2").arg(QLatin1String(tag), sql);
}

static QString projBuildSelectProjetsSql()
{
    const ProjetOracleMeta &m = g_projOra;
    const QString etatExpr = m.etat.isEmpty() ? QStringLiteral("CAST(NULL AS VARCHAR2(30))")
                                              : (QStringLiteral("p.") + m.etat);
    const QString progExpr = m.progression.isEmpty() ? QStringLiteral("CAST(0 AS NUMBER)")
                                                     : (QStringLiteral("p.") + m.progression);
    const QString respNameExpr = m.chercheurHasPrenom
        ? QStringLiteral("TRIM(NVL(c.NOM,'') || ' ' || NVL(c.PRENOM,''))")
        : QStringLiteral("TRIM(NVL(c.NOM,''))");

    return QStringLiteral(
               "SELECT p.ID_PROJET, p.%1, p.%2, p.%3, p.%4, p.%5, %6, %7, %8 "
               "FROM %9 p LEFT JOIN CHERCHEUR c ON c.ID_CHERCHEUR = p.%10 "
               "ORDER BY 2")
        .arg(m.code)
        .arg(m.titre)
        .arg(m.dateDebut)
        .arg(m.dateFin)
        .arg(m.responsable)
        .arg(etatExpr)
        .arg(progExpr)
        .arg(respNameExpr)
        .arg(m.table)
        .arg(m.responsable);
}

static bool projExecInsertProjet(QSqlQuery &query, const Projet &projet, double progVal)
{
    const ProjetOracleMeta &m = g_projOra;
    QStringList cols = {m.code, m.titre, m.dateDebut, m.dateFin, m.responsable};
    QStringList ph = {QStringLiteral(":code"), QStringLiteral(":titre"), QStringLiteral(":debut"),
                      QStringLiteral(":fin"), QStringLiteral(":resp")};
    if (!m.etat.isEmpty()) {
        cols << m.etat;
        ph << QStringLiteral(":etat");
    }
    if (!m.progression.isEmpty()) {
        cols << m.progression;
        ph << QStringLiteral(":prog");
    }
    const QString sql = QStringLiteral("INSERT INTO %1 (%2) VALUES (%3)")
                            .arg(m.table, cols.join(QLatin1Char(',')), ph.join(QLatin1Char(',')));
    projLogSql("INSERT", sql);
    query.prepare(sql);
    query.bindValue(QStringLiteral(":code"), projet.code);
    query.bindValue(QStringLiteral(":titre"), projet.titre);
    query.bindValue(QStringLiteral(":debut"), projet.dateDebut);
    query.bindValue(QStringLiteral(":fin"), projet.dateFin);
    if (projet.responsableId > 0)
        query.bindValue(QStringLiteral(":resp"), projet.responsableId);
    else
        query.bindValue(QStringLiteral(":resp"), QVariant());
    if (!m.etat.isEmpty())
        query.bindValue(QStringLiteral(":etat"), projet.etat);
    if (!m.progression.isEmpty())
        query.bindValue(QStringLiteral(":prog"), progVal);
    const bool ok = query.exec();
    if (!ok)
        qDebug() << QStringLiteral("[PROJET SQL] INSERT lastError:") << query.lastError().text();
    return ok;
}

static QString projProjetTableName(QSqlDatabase &db)
{
    projEnsureOracleProjetMeta(db);
    return g_projOra.table;
}

static bool projExecUpdateProjet(QSqlQuery &query, const Projet &projet, double progVal, int idProjet)
{
    const ProjetOracleMeta &m = g_projOra;
    QStringList sets;
    sets << QStringLiteral("%1 = :code").arg(m.code);
    sets << QStringLiteral("%1 = :titre").arg(m.titre);
    sets << QStringLiteral("%1 = :debut").arg(m.dateDebut);
    sets << QStringLiteral("%1 = :fin").arg(m.dateFin);
    sets << QStringLiteral("%1 = :resp").arg(m.responsable);
    if (!m.etat.isEmpty())
        sets << QStringLiteral("%1 = :etat").arg(m.etat);
    if (!m.progression.isEmpty())
        sets << QStringLiteral("%1 = :prog").arg(m.progression);
    const QString sql = QStringLiteral("UPDATE %1 SET %2 WHERE ID_PROJET = :id")
                            .arg(m.table, sets.join(QLatin1String(", ")));
    projLogSql("UPDATE", sql);
    query.prepare(sql);
    query.bindValue(QStringLiteral(":code"), projet.code);
    query.bindValue(QStringLiteral(":titre"), projet.titre);
    query.bindValue(QStringLiteral(":debut"), projet.dateDebut);
    query.bindValue(QStringLiteral(":fin"), projet.dateFin);
    if (projet.responsableId > 0)
        query.bindValue(QStringLiteral(":resp"), projet.responsableId);
    else
        query.bindValue(QStringLiteral(":resp"), QVariant());
    if (!m.etat.isEmpty())
        query.bindValue(QStringLiteral(":etat"), projet.etat);
    if (!m.progression.isEmpty())
        query.bindValue(QStringLiteral(":prog"), progVal);
    query.bindValue(QStringLiteral(":id"), idProjet);
    const bool ok = query.exec();
    if (!ok)
        qDebug() << QStringLiteral("[PROJET SQL] UPDATE lastError:") << query.lastError().text();
    return ok;
}

static QString projetNormEmail(const QString &s)
{
    return s.trimmed().toLower();
}

static bool projetEmailDomainOk(const QString &domainLower)
{
    if (domainLower.isEmpty())
        return false;
    const int li = domainLower.lastIndexOf(QLatin1Char('.'));
    if (li < 1 || li >= domainLower.size() - 2)
        return false;
    static const QStringList blocked = {
        QStringLiteral("localhost"),
        QStringLiteral("example.com"),
        QStringLiteral("example.org"),
        QStringLiteral("example.net"),
        QStringLiteral("invalid"),
        QStringLiteral("test.com"),
    };
    return !blocked.contains(domainLower);
}

static bool projetChercheurEmailStrictOk(const QString &normalizedEmail, QString *errMsg)
{
    static const QRegularExpression emailRegex(
        QStringLiteral(R"(^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,}$)"));
    if (!emailRegex.match(normalizedEmail).hasMatch()) {
        if (errMsg)
            *errMsg = QStringLiteral("Email invalide.");
        return false;
    }
    const int at = normalizedEmail.lastIndexOf(QLatin1Char('@'));
    const QString dom = normalizedEmail.mid(at + 1);
    if (!projetEmailDomainOk(dom)) {
        if (errMsg)
            *errMsg = QStringLiteral("Domaine email non valide.");
        return false;
    }
    return true;
}

static int projetInternalIdFromTableRow(const QTableWidget *tw, int row)
{
    if (!tw || row < 0)
        return -1;
    QTableWidgetItem *it = tw->item(row, 0);
    if (!it)
        return -1;
    const int id = it->data(kProjetRowInternalIdRole).toInt();
    return id > 0 ? id : -1;
}

static const QRegularExpression &projetCodeExactRx()
{
    static const QRegularExpression re(QStringLiteral("^PRJ-\\d{4}-[A-Z]{2}-\\d{2}$"));
    return re;
}

static bool projetTitreValide(const QString &titreBrut, QString *msg = nullptr)
{
    const QString t = titreBrut.trimmed();
    if (t.isEmpty()) {
        if (msg)
            *msg = QStringLiteral("Le titre du projet est obligatoire.");
        return false;
    }
    if (t.at(0).isDigit()) {
        if (msg)
            *msg = QStringLiteral("Le titre ne doit pas commencer par un chiffre.");
        return false;
    }
    bool onlyDigits = true;
    for (QChar c : t) {
        if (!c.isDigit()) {
            onlyDigits = false;
            break;
        }
    }
    if (onlyDigits) {
        if (msg)
            *msg = QStringLiteral("Le titre ne peut pas être un nombre seul.");
        return false;
    }
    bool hasLetter = false;
    bool hasLower = false;
    for (QChar c : t) {
        if (c.isLetter()) {
            hasLetter = true;
            if (c.isLower())
                hasLower = true;
        }
    }
    if (hasLetter && !hasLower) {
        if (msg)
            *msg = QStringLiteral("Le titre ne doit pas être entièrement en MAJUSCULES.");
        return false;
    }
    return true;
}

} // namespace
static double projProgressionStrToDouble(const QString &s) {
    QString t = s.trimmed();
    if (t.endsWith(QLatin1Char('%')))
        t.chop(1);
    bool ok = false;
    double v = t.toDouble(&ok);
    if (!ok)
        return 0.0;
    return qBound(0.0, v, 100.0);
}
int SmartPub::projExtraireProgression(const QString &progressionStr) const
{
    QString temp = progressionStr;
    if (temp.endsWith('%')) {
        temp.chop(1);
    }
    return temp.toInt();
}

void SmartPub::projSetupStatistiquesButton()
{
    ui->btnStatistiques->setFixedHeight(44);
    ui->btnStatistiques->setMinimumWidth(140);
    ui->btnStatistiques->setCursor(Qt::PointingHandCursor);
    ui->btnStatistiques->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    padding: 0px 20px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );
}

void SmartPub::projSetupAIButton()
{
    // PAS de parent "this" sinon le bouton flotte sur la fenetre principale
    QPushButton *btnAI = new QPushButton("🤖");
    btnAI->setObjectName("btnAIRecommandations");
    btnAI->setFixedSize(44, 44);
    btnAI->setCursor(Qt::PointingHandCursor);
    btnAI->setToolTip("Recommandations IA");

    btnAI->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}"
        );

    connect(btnAI, &QPushButton::clicked, this, &SmartPub::on_iaRecommanderClicked);

    // La toolbar est un QVBoxLayout avec 2 rangees :
    // itemAt(0) = row1 (QHBoxLayout) : CRUD | sep | Statistiques | [AI ici] | spacer | recherche | filtres | exporter
    // itemAt(1) = row2 (QHBoxLayout) : Trier par | Date Debut | Date Fin | Etat | Progression
    QVBoxLayout *vLayout = qobject_cast<QVBoxLayout*>(ui->toolbarFrameProjets->layout());
    if (!vLayout || vLayout->count() == 0) return;

    QHBoxLayout *row1 = qobject_cast<QHBoxLayout*>(vLayout->itemAt(0)->layout());
    if (!row1) return;

    // Chercher btnStatistiques et inserer btnAI juste apres
    int index = -1;
    for (int i = 0; i < row1->count(); ++i) {
        QLayoutItem *item = row1->itemAt(i);
        if (item && item->widget() == ui->btnStatistiques) {
            index = i;
            break;
        }
    }

    if (index >= 0) {
        row1->insertWidget(index + 1, btnAI);
    } else {
        // Fallback : inserer apres le separateur (position 5)
        row1->insertWidget(5, btnAI);
    }
}

void SmartPub::projSetupSidebarProfile()
{
    profileWidget = new QWidget(ui->sidebarFrame);
    profileWidget->setObjectName("sidebarProfile");
    profileWidget->setStyleSheet(
        "QWidget#sidebarProfile {"
        "    background-color: #0f172a;"
        "    border-top: 1px solid #334155;"
        "}"
        );
    profileWidget->setFixedHeight(80);

    QHBoxLayout *profileLayout = new QHBoxLayout(profileWidget);
    profileLayout->setSpacing(15);
    profileLayout->setContentsMargins(20, 10, 20, 10);

    avatarLabel = new QLabel("CP");
    avatarLabel->setFixedSize(45, 45);
    avatarLabel->setStyleSheet(
        "QLabel {"
        "    background-color: #10b981;"
        "    color: white;"
        "    border-radius: 22px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    qproperty-alignment: AlignCenter;"
        "}"
        );
    avatarLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(3);

    nameLabel = new QLabel("Chef de Projet");
    nameLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        );

    roleLabel = new QLabel("Administrateur");
    roleLabel->setStyleSheet(
        "QLabel {"
        "    color: #94a3b8;"
        "    font-size: 12px;"
        "}"
        );

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(roleLabel);

    profileLayout->addWidget(avatarLabel);
    profileLayout->addLayout(infoLayout, 1);

    btnSettings = new QPushButton("⚙");
    btnSettings->setFixedSize(35, 35);
    btnSettings->setCursor(Qt::PointingHandCursor);
    btnSettings->setStyleSheet(
        "QPushButton {"
        "    background-color: transparent;"
        "    color: #94a3b8;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 18px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #334155;"
        "    color: white;"
        "}"
        );
    connect(btnSettings, &QPushButton::clicked, this, &SmartPub::onSettingsClicked);
    profileLayout->addWidget(btnSettings);

    QVBoxLayout *sidebarLayout = qobject_cast<QVBoxLayout*>(ui->sidebarFrame->layout());
    if (sidebarLayout) {
        sidebarLayout->addWidget(profileWidget);
    }

    projUpdateSidebarProfileVisibility();
}

void SmartPub::projUpdateSidebarProfileVisibility()
{
    if (!profileWidget) return;

    if (!sidebarExpanded) {
        profileWidget->setVisible(false);
    } else {
        profileWidget->setVisible(true);
    }
}

// ============================================================================
// MODULE PROJETS
// ============================================================================

void SmartPub::projSetupUI()
{
    projSetupTable();

    ui->btnListeProjets->setToolTip("Liste des projets");
    ui->btnAjouterProjet->setToolTip("Ajouter un projet");
    ui->btnModifierProjet->setToolTip("Modifier le projet sélectionné");
    ui->btnSupprimerProjet->setToolTip("Supprimer le projet sélectionné");

    // Barre de recherche
    ui->lineEditRechercheProjets->setFixedHeight(44);
    ui->lineEditRechercheProjets->setMinimumWidth(220);
    ui->lineEditRechercheProjets->setStyleSheet(
        "QLineEdit {"
        "    background-color: #ffffff;"
        "    border: 1.5px solid #e2e8f0;"
        "    border-radius: 10px;"
        "    padding: 0px 14px;"
        "    font-size: 13px;"
        "    color: #334155;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #3b82f6;"
        "    border-width: 2px;"
        "}");

    // Filtres
    ui->btnFiltresProjets->setFixedHeight(44);
    ui->btnFiltresProjets->setMinimumWidth(90);
    ui->btnFiltresProjets->setCursor(Qt::PointingHandCursor);
    ui->btnFiltresProjets->setStyleSheet(
        "QPushButton {"
        "    background-color: #ffffff;"
        "    color: #475569;"
        "    border: 1.5px solid #e2e8f0;"
        "    border-radius: 10px;"
        "    font-size: 13px;"
        "    font-weight: 500;"
        "    padding: 0px 16px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1f5f9;"
        "    border-color: #94a3b8;"
        "    color: #1e293b;"
        "}");

    // Exporter
    ui->btnExporterProjets->setFixedHeight(44);
    ui->btnExporterProjets->setMinimumWidth(100);
    ui->btnExporterProjets->setCursor(Qt::PointingHandCursor);
    ui->btnExporterProjets->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 10px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    padding: 0px 16px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
        "}");

    // Boutons Trier par
    QString triBtnStyle =
        "QPushButton {"
        "    background-color: white;"
        "    color: #334155;"
        "    border: 1px solid #cbd5e1;"
        "    border-radius: 15px;"
        "    font-size: 12px;"
        "    font-weight: 500;"
        "    padding: 5px 13px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f1f5f9;"
        "    border-color: #94a3b8;"
        "}"
        "QPushButton:checked {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
        "    color: white;"
        "    border: none;"
        "    font-weight: 600;"
        "}";
    ui->btnTriDateDebut->setStyleSheet(triBtnStyle);
    ui->btnTriDateFin->setStyleSheet(triBtnStyle);
    ui->btnTriEtat->setStyleSheet(triBtnStyle);
    ui->btnTriProgression->setStyleSheet(triBtnStyle);

    // Style initial des boutons CRUD
    projSetActiveCrudButton(0);

    if (ui->gridLayoutForm) {
        ui->gridLayoutForm->setColumnStretch(0, 1);
        ui->gridLayoutForm->setColumnStretch(1, 1);
    }
    if (ui->scrollAreaWidgetContents)
        ui->scrollAreaWidgetContents->setMinimumWidth(520);

    if (ui->projHeaderFrame && ui->projHeaderLayoutMain) {
        if (!ui->projHeaderFrame->findChild<QLabel *>(QStringLiteral("projHeaderLogoDeco"))) {
            auto *logoDeco = new QLabel(ui->projHeaderFrame);
            logoDeco->setObjectName(QStringLiteral("projHeaderLogoDeco"));
            QPixmap pm(QStringLiteral(":/logo.png"));
            if (!pm.isNull()) {
                logoDeco->setPixmap(pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                logoDeco->setScaledContents(true);
                logoDeco->setFixedSize(52, 52);
                logoDeco->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
            }
            if (auto *h = qobject_cast<QHBoxLayout *>(ui->projHeaderLayoutMain))
                h->insertWidget(0, logoDeco);
        }
    }

    projSetupFormValidationWidgets();
}

void SmartPub::projSetupFormValidationWidgets()
{
    if (projErrCodeLabel)
        return;

    auto mkErr = [](QVBoxLayout *vly) -> QLabel * {
        auto *l = new QLabel();
        l->hide();
        l->setWordWrap(true);
        l->setStyleSheet(QStringLiteral("color: #dc2626; font-size: 12px;"));
        vly->addWidget(l);
        return l;
    };

    projErrCodeLabel = mkErr(ui->verticalLayoutCodeForm);
    projErrTitreLabel = mkErr(ui->verticalLayoutTitreForm);
    projErrDateLabel = mkErr(ui->verticalLayoutDateDebutForm);
    projErrResponsableLabel = mkErr(ui->verticalLayoutResponsableForm);

    if (!projTitreDebounceTimer) {
        projTitreDebounceTimer = new QTimer(this);
        projTitreDebounceTimer->setSingleShot(true);
        projTitreDebounceTimer->setInterval(400);
        connect(projTitreDebounceTimer, &QTimer::timeout, this, [this]() {
            if (projTouchedTitre)
                projValidateTitre(false);
        });
    }
    if (!projCodeDebounceTimer) {
        projCodeDebounceTimer = new QTimer(this);
        projCodeDebounceTimer->setSingleShot(true);
        projCodeDebounceTimer->setInterval(400);
        connect(projCodeDebounceTimer, &QTimer::timeout, this, [this]() {
            if (projTouchedCode && !isEditing)
                projValidateCode(false);
        });
    }
}

void SmartPub::projShowLineFieldError(QLineEdit *field, QLabel *errLabel, const QString &msg)
{
    if (!field || !errLabel)
        return;
    field->setStyleSheet(QStringLiteral("border: 1px solid #dc2626;"));
    errLabel->setText(msg);
    errLabel->show();
    QToolTip::showText(field->mapToGlobal(QPoint(0, field->height())), msg, field);
}

void SmartPub::projHideLineFieldError(QLineEdit *field, QLabel *errLabel)
{
    if (field)
        field->setStyleSheet(QString());
    if (errLabel) {
        errLabel->clear();
        errLabel->hide();
    }
}

void SmartPub::projShowComboFieldError(QComboBox *cb, QLabel *errLabel, const QString &msg)
{
    if (!cb || !errLabel)
        return;
    cb->setStyleSheet(QStringLiteral("QComboBox { border: 1px solid #dc2626; }"));
    errLabel->setText(msg);
    errLabel->show();
    QToolTip::showText(cb->mapToGlobal(QPoint(0, cb->height())), msg, cb);
}

void SmartPub::projHideComboFieldError(QComboBox *cb, QLabel *errLabel)
{
    if (cb)
        cb->setStyleSheet(QString());
    if (errLabel) {
        errLabel->clear();
        errLabel->hide();
    }
}

void SmartPub::projShowDateOrderError(const QString &msg)
{
    const QString ss = QStringLiteral("QDateEdit { border: 1px solid #dc2626; }");
    ui->dateEditDebutForm->setStyleSheet(ss);
    ui->dateEditFinForm->setStyleSheet(ss);
    if (projErrDateLabel) {
        projErrDateLabel->setText(msg);
        projErrDateLabel->show();
    }
    QToolTip::showText(ui->dateEditDebutForm->mapToGlobal(QPoint(0, ui->dateEditDebutForm->height())),
                       msg, ui->dateEditDebutForm);
}

void SmartPub::projHideDateOrderError()
{
    ui->dateEditDebutForm->setStyleSheet(QString());
    ui->dateEditFinForm->setStyleSheet(QString());
    if (projErrDateLabel) {
        projErrDateLabel->clear();
        projErrDateLabel->hide();
    }
}

void SmartPub::projClearProjectFormErrors()
{
    projHideLineFieldError(ui->lineEditCodeForm, projErrCodeLabel);
    projHideLineFieldError(ui->lineEditTitreForm, projErrTitreLabel);
    projHideComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel);
    projHideDateOrderError();
    projTouchedCode = false;
    projTouchedTitre = false;
    projTouchedResponsable = false;
    projTouchedDates = false;
    if (projTitreDebounceTimer)
        projTitreDebounceTimer->stop();
    if (projCodeDebounceTimer)
        projCodeDebounceTimer->stop();
}

// CORRECTION DU TABLEAU - setupTable amélioré
void SmartPub::projSetupTable()
{
    QTableWidget *table = ui->tableWidgetProjets;

    table->verticalHeader()->setVisible(false);
    table->setSortingEnabled(false);
    table->setAlternatingRowColors(false);

    table->setColumnCount(7);
    QStringList headers;
    headers << "Code" << "Titre" << "Date Début" << "Date Fin"
            << "Responsable" << "État" << "Progression";
    table->setHorizontalHeaderLabels(headers);

    // CORRECTION: StretchLastSection à true pour que la dernière colonne s'étire
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setMinimumSectionSize(80);
    table->horizontalHeader()->setDefaultSectionSize(100);

    // CORRECTION: Largeurs ajustées pour éviter le tronquage
    table->setColumnWidth(0, 130);     // Code
    table->setColumnWidth(1, 300);     // Titre
    table->setColumnWidth(2, 100);     // Date Début
    table->setColumnWidth(3, 100);     // Date Fin
    table->setColumnWidth(4, 170);     // Responsable
    table->setColumnWidth(5, 90);      // État
    // Colonne 6 (Progression) s'étire avec StretchLastSection

    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);

    // CORRECTION: WordWrap désactivé pour éviter les problèmes de hauteur
    table->setWordWrap(false);

    // Hauteur de ligne fixe
    table->verticalHeader()->setDefaultSectionSize(50);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // Style de l'en-tête - CORRECTION: padding réduit pour éviter le tronquage
    table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #1e293b;"
        "    padding: 10px 6px;"        // Padding réduit
        "    font-weight: 600;"
        "    color: white;"
        "    border: none;"
        "    font-size: 11px;"          // Police légèrement plus petite
        "    text-transform: uppercase;"
        "}"
        "QHeaderView::section:hover {"
        "    background-color: #334155;"
        "}"
        );

    // Style du tableau
    table->setStyleSheet(
        "QTableWidget {"
        "    background-color: white;"
        "    border: none;"
        "    font-size: 13px;"
        "    gridline-color: transparent;"
        "    selection-background-color: #eff6ff;"
        "    selection-color: #1e293b;"
        "}"
        "QTableWidget::item {"
        "    padding: 12px 8px;"
        "    border-bottom: 1px solid #f1f5f9;"
        "}"
        "QTableWidget::item:selected {"
        "    background-color: #eff6ff;"
        "    color: #1e293b;"
        "}"
        "QTableWidget::item:hover {"
        "    background-color: #f8fafc;"
        "}"
        "QTableCornerButton::section {"
        "    background-color: #1e293b;"
        "    border: none;"
        "}"
        );
}

void SmartPub::projSetupConnections()
{
    // Note: Les boutons de navigation (btnPublications, btnChercheurs, etc.) sont déjà
    // connectés dans setupConnections(), donc on ne les reconnecte pas ici pour éviter
    // les conflits et les comportements inattendus.
    //
    // btnListeProjets, btnAjouter/Modifier/SupprimerProjet, lineEditRechercheProjets,
    // btnAnnulerForm, btnEnregistrerForm : connectés par QMetaObject::connectSlotsByName (setupUi).

    connect(ui->tableWidgetProjets, &QTableWidget::itemSelectionChanged, this,
            &SmartPub::on_tableSelectionChanged);
    connect(ui->tableWidgetProjets, &QTableWidget::cellDoubleClicked, this,
            &SmartPub::on_tableDoubleClicked);

    connect(ui->btnTriDateDebut, &QPushButton::clicked, this, &SmartPub::on_triDateDebutClicked);
    connect(ui->btnTriDateFin, &QPushButton::clicked, this, &SmartPub::on_triDateFinClicked);
    connect(ui->btnTriEtat, &QPushButton::clicked, this, &SmartPub::on_triEtatClicked);
    connect(ui->btnTriProgression, &QPushButton::clicked, this, &SmartPub::on_triProgressionClicked);

    connect(ui->btnStatistiques, &QPushButton::clicked, this, &SmartPub::on_statistiquesClicked);
    connect(ui->btnFiltresProjets, &QPushButton::clicked, this, &SmartPub::on_filtresClicked);
    connect(ui->btnExporterProjets, &QPushButton::clicked, this, &SmartPub::on_exporterClicked);

    if (!ui->lineEditCodeForm->validator()) {
        ui->lineEditCodeForm->setValidator(
            new QRegularExpressionValidator(projetCodeExactRx(), ui->lineEditCodeForm));
    }
    connect(ui->lineEditCodeForm, &QLineEdit::textChanged, this, [this](const QString &) {
        if (!projErrCodeLabel || isEditing)
            return;
        const QString t = ui->lineEditCodeForm->text().trimmed();
        if (!t.isEmpty() && projetCodeExactRx().match(t).hasMatch()) {
            if (projCodeDebounceTimer)
                projCodeDebounceTimer->stop();
            projHideLineFieldError(ui->lineEditCodeForm, projErrCodeLabel);
            return;
        }
        if (!projTouchedCode)
            return;
        if (projCodeDebounceTimer)
            projCodeDebounceTimer->start();
    });
    connect(ui->lineEditCodeForm, &QLineEdit::editingFinished, this, [this]() {
        if (isEditing)
            return;
        if (projCodeDebounceTimer)
            projCodeDebounceTimer->stop();
        projTouchedCode = true;
        projValidateCode(false);
        const QString t = ui->lineEditCodeForm->text().trimmed();
        if (!t.isEmpty() && !projetCodeExactRx().match(t).hasMatch())
            QTimer::singleShot(0, this, [this]() { ui->lineEditCodeForm->setFocus(); });
    });

    connect(ui->lineEditTitreForm, &QLineEdit::textChanged, this, [this](const QString &) {
        if (!projErrTitreLabel)
            return;
        QString err;
        if (projetTitreValide(ui->lineEditTitreForm->text(), &err)) {
            if (projTitreDebounceTimer)
                projTitreDebounceTimer->stop();
            projHideLineFieldError(ui->lineEditTitreForm, projErrTitreLabel);
            return;
        }
        if (!projTouchedTitre)
            return;
        if (projTitreDebounceTimer)
            projTitreDebounceTimer->start();
    });
    connect(ui->lineEditTitreForm, &QLineEdit::editingFinished, this, [this]() {
        if (projTitreDebounceTimer)
            projTitreDebounceTimer->stop();
        projTouchedTitre = true;
        if (!projValidateTitre(false)) {
            QTimer::singleShot(0, this, [this]() { ui->lineEditTitreForm->setFocus(); });
        }
    });

    connect(ui->comboBoxResponsableForm, QOverload<int>::of(&QComboBox::activated), this,
            [this](int) {
                projTouchedResponsable = true;
                projValidateResponsable(false);
            });

    connect(ui->dateEditDebutForm, &QDateEdit::dateChanged, this, [this](const QDate &) {
        if (ui->dateEditDebutForm->date() <= ui->dateEditFinForm->date()) {
            projHideDateOrderError();
            return;
        }
        if (projTouchedDates)
            projShowDateOrderError(
                QStringLiteral("La date de début doit être antérieure ou égale à la date de fin."));
    });
    connect(ui->dateEditFinForm, &QDateEdit::dateChanged, this, [this](const QDate &) {
        if (ui->dateEditDebutForm->date() <= ui->dateEditFinForm->date()) {
            projHideDateOrderError();
            return;
        }
        if (projTouchedDates)
            projShowDateOrderError(
                QStringLiteral("La date de début doit être antérieure ou égale à la date de fin."));
    });

    ui->lineEditCodeForm->installEventFilter(this);
    ui->lineEditTitreForm->installEventFilter(this);
    ui->dateEditDebutForm->installEventFilter(this);
    ui->dateEditFinForm->installEventFilter(this);
    ui->comboBoxResponsableForm->installEventFilter(this);
}

void SmartPub::projSetupSampleData()
{
    // Les données sont chargées depuis Oracle via projChargerDepuisDB()
    // appelé dans projChargerProjets()
    projets.clear();
}

void SmartPub::projSetupComboBoxes()
{
    ui->comboBoxResponsableForm->clear();
    ui->comboBoxResponsableForm->addItem(QStringLiteral("—"), QVariant());

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        projEnsureOracleProjetMeta(db);
        QSqlQuery q(db);
        const QString sqlCh = QStringLiteral("SELECT ID_CHERCHEUR, NOM, PRENOM FROM CHERCHEUR ORDER BY NOM, PRENOM");
        projLogSql("SELECT CHERCHEUR (combo)", sqlCh);
        if (q.exec(sqlCh)) {
            while (q.next()) {
                const int cid = q.value(0).toInt();
                const QString label = QStringLiteral("%1 %2")
                                          .arg(q.value(1).toString().trimmed(),
                                               q.value(2).toString().trimmed())
                                          .trimmed();
                ui->comboBoxResponsableForm->addItem(label.isEmpty() ? QString::number(cid) : label,
                                                     QVariant(cid));
            }
        } else {
            qDebug() << QStringLiteral("[PROJET SQL] combo CHERCHEUR lastError:") << q.lastError().text();
        }
    }

    ui->comboBoxEtatForm->clear();
    ui->comboBoxEtatForm->addItem(QStringLiteral("En cours"), QStringLiteral("en_cours"));
    ui->comboBoxEtatForm->addItem(QStringLiteral("Terminé"), QStringLiteral("termine"));
    ui->comboBoxEtatForm->addItem(QStringLiteral("Suspendu"), QStringLiteral("suspendu"));
    ui->comboBoxEtatForm->addItem(QStringLiteral("Annulé"), QStringLiteral("annule"));
}


// CORRECTION: ajouterProjetTable amélioré pour éviter le tronquage
void SmartPub::projAjouterProjetTable(const Projet &projet, int rowIndex)
{
    Q_UNUSED(rowIndex);
    int row = ui->tableWidgetProjets->rowCount();
    ui->tableWidgetProjets->insertRow(row);

    // Code (texte visible ; id interne uniquement en UserRole — jamais affiché)
    QTableWidgetItem *codeItem = new QTableWidgetItem(projet.code);
    codeItem->setData(kProjetRowInternalIdRole, projet.id);
    codeItem->setForeground(QColor("#3b82f6"));
    codeItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    codeItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidgetProjets->setItem(row, 0, codeItem);

    // Titre
    QTableWidgetItem *titreItem = new QTableWidgetItem(projet.titre);
    titreItem->setFont(QFont("Segoe UI", 9, QFont::Medium));
    titreItem->setToolTip(projet.titre);
    titreItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titreItem->setForeground(QColor("#1e293b"));
    ui->tableWidgetProjets->setItem(row, 1, titreItem);

    // Date début
    QTableWidgetItem *debutItem = new QTableWidgetItem(projet.dateDebut.toString("dd/MM/yyyy"));
    debutItem->setTextAlignment(Qt::AlignCenter);
    debutItem->setForeground(QColor("#64748b"));
    debutItem->setFont(QFont("Segoe UI", 9));
    ui->tableWidgetProjets->setItem(row, 2, debutItem);

    // Date fin
    QTableWidgetItem *finItem = new QTableWidgetItem(projet.dateFin.toString("dd/MM/yyyy"));
    finItem->setTextAlignment(Qt::AlignCenter);
    finItem->setForeground(QColor("#64748b"));
    finItem->setFont(QFont("Segoe UI", 9));
    ui->tableWidgetProjets->setItem(row, 3, finItem);

    // Responsable
    QTableWidgetItem *respItem = new QTableWidgetItem(projet.responsable);
    respItem->setForeground(QColor("#475569"));
    respItem->setFont(QFont("Segoe UI", 9));
    respItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidgetProjets->setItem(row, 4, respItem);

    // État avec couleur (affichage libellé FR, données = codes Oracle)
    QTableWidgetItem *etatItem = new QTableWidgetItem(projEtatDbToUi(projet.etat));
    etatItem->setTextAlignment(Qt::AlignCenter);
    etatItem->setForeground(QColor(projEtatColor(projEtatDbToUi(projet.etat))));
    etatItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    ui->tableWidgetProjets->setItem(row, 5, etatItem);

    // Progression avec couleur
    QTableWidgetItem *progItem = new QTableWidgetItem(projet.progression);
    progItem->setTextAlignment(Qt::AlignCenter);
    progItem->setForeground(QColor(projProgressionColorFromString(projet.progression)));
    progItem->setFont(QFont("Segoe UI", 9, QFont::Bold));
    ui->tableWidgetProjets->setItem(row, 6, progItem);

    // Couleurs de fond alternées
    QColor bgColor = (row % 2 == 0) ? QColor("#ffffff") : QColor("#f8fafc");
    for (int col = 0; col < 7; ++col) {
        QTableWidgetItem *item = ui->tableWidgetProjets->item(row, col);
        if (item) item->setBackground(bgColor);
    }
}
void SmartPub::projAjusterColonnesTable()
{
    QTableWidget *table = ui->tableWidgetProjets;

    // Recalcule les largeurs optimales
    table->resizeColumnsToContents();

    // Mais avec des minimums et maximums
    if (table->columnWidth(0) < 100) table->setColumnWidth(0, 100);   // Code min
    if (table->columnWidth(1) < 250) table->setColumnWidth(1, 250);   // Titre min
    if (table->columnWidth(1) > 400) table->setColumnWidth(1, 400);  // Titre max
    if (table->columnWidth(2) < 90) table->setColumnWidth(2, 90);    // Date min
    if (table->columnWidth(3) < 90) table->setColumnWidth(3, 90);    // Date min
    if (table->columnWidth(4) < 150) table->setColumnWidth(4, 150);  // Resp min
    if (table->columnWidth(5) < 80) table->setColumnWidth(5, 80);    // État min
    if (table->columnWidth(6) < 80) table->setColumnWidth(6, 80);    // Prog min
}

void SmartPub::projChargerProjets()
{
    projets.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        projEnsureOracleProjetMeta(db);
        const QString sqlSel = projBuildSelectProjetsSql();
        projLogSql("SELECT PROJET", sqlSel);
        QSqlQuery query(db);
        query.prepare(sqlSel);
        if (query.exec()) {
            while (query.next()) {
                Projet p;
                p.id = query.value(0).toInt();
                p.code = query.value(1).toString();
                p.titre = query.value(2).toString();
                p.dateDebut = query.value(3).toDate();
                p.dateFin = query.value(4).toDate();
                QVariant rv = query.value(5);
                p.responsableId = rv.isNull() ? 0 : rv.toInt();
                p.responsable = query.value(8).toString().trimmed();
                if (p.responsable.isEmpty() && p.responsableId != 0)
                    p.responsable = QStringLiteral("—");
                p.etat = query.value(6).toString();
                double prog = query.value(7).toDouble();
                p.progression = QStringLiteral("%1%").arg(qRound(prog));
                p.description.clear();
                projets.append(p);
            }
        } else {
            qDebug() << QStringLiteral("[PROJET SQL] SELECT échec SQL:") << sqlSel
                     << QStringLiteral("lastError:") << query.lastError().text();
        }
    }

    projViderTable();
    if (filtresActifs) {
        projAppliquerFiltres();
        for (int i = 0; i < projetsFiltres.size(); ++i)
            projAjouterProjetTable(projetsFiltres[i], i);
    } else {
        for (int i = 0; i < projets.size(); ++i)
            projAjouterProjetTable(projets[i], i);
    }
    projAjusterColonnesTable();

    Reminder::notifyEndingSoon(projets);
}

void SmartPub::projViderTable()
{
    ui->tableWidgetProjets->setRowCount(0);
}

void SmartPub::projSetActiveNavigationButton(int index)
{
    QVector<QPushButton*> buttons = {
        ui->btnPublications, ui->btnChercheurs, ui->btnLaboratoires,
        ui->btnProjets, ui->btnFinances, ui->btnEvenements
    };

    for (int i = 0; i < buttons.size(); ++i) {
        QString style;
        if (i == index) {
            style = "QPushButton {"
                    "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                    "        stop:0 #3b82f6, stop:1 #10b981);"
                    "    color: white;"
                    "    border: none;"
                    "    border-radius: 12px;"
                    "    padding: 14px 25px;"
                    "    font-size: 14px;"
                    "    font-weight: 600;"
                    "    text-align: left;"
                    "}"
                    "QPushButton:hover {"
                    "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                    "        stop:0 #2563eb, stop:1 #059669);"
                    "}";
        } else {
            style = "QPushButton {"
                    "    background-color: transparent;"
                    "    color: #94a3b8;"
                    "    border: none;"
                    "    border-radius: 12px;"
                    "    padding: 14px 25px;"
                    "    font-size: 14px;"
                    "    font-weight: 500;"
                    "    text-align: left;"
                    "}"
                    "QPushButton:hover {"
                    "    background-color: #334155;"
                    "    color: #e2e8f0;"
                    "}";
        }
        buttons[i]->setStyleSheet(style);
    }
}

void SmartPub::handleProjetListeProjets()
{
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::handleProjetAjouterProjet()
{
    isEditing = false;
    projViderFormulaire();
    projAfficherFormulaire(false);
    projSetActiveCrudButton(1);
}

void SmartPub::handleProjetModifierProjet()
{
    int row = projGetSelectedRow();
    if (row != -1) {
        const int projetId = projetInternalIdFromTableRow(ui->tableWidgetProjets, row);
        for (const Projet &projet : projets) {
            if (projet.id == projetId) {
                isEditing = true;
                currentProjetId = projetId;
                projRemplirFormulaire(projet);
                projAfficherFormulaire(true);
                projSetActiveCrudButton(2);
                break;
            }
        }
    } else {
        const QString msg = QStringLiteral("Sélectionnez un projet dans le tableau.");
        QToolTip::showText(ui->btnModifierProjet->mapToGlobal(QPoint(0, ui->btnModifierProjet->height())),
                           msg, ui->btnModifierProjet);
    }
}

void SmartPub::projSetActiveCrudButton(int index)
{
    for (auto *btn : {ui->btnListeProjets, ui->btnAjouterProjet, ui->btnModifierProjet, ui->btnSupprimerProjet}) {
        btn->setFixedSize(44, 44);
        btn->setCursor(Qt::PointingHandCursor);
    }

    if (index == 0) {
        ui->btnListeProjets->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
            "    color: white; border: none; border-radius: 10px; font-size: 18px;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
            "}");
    } else {
        ui->btnListeProjets->setStyleSheet(
            "QPushButton {"
            "    background-color: #f1f5f9; color: #64748b;"
            "    border: 1.5px solid #e2e8f0; border-radius: 10px; font-size: 18px;"
            "}"
            "QPushButton:hover { background-color: #e2e8f0; color: #1e293b; }");
    }

    ui->btnAjouterProjet->setStyleSheet(
        "QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white; border: none; border-radius: 10px; font-size: 22px; font-weight: 700;"
        "}"
        "QPushButton:hover { background-color: #2563eb; }");

    ui->btnModifierProjet->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1f5f9; color: #64748b;"
        "    border: 1.5px solid #e2e8f0; border-radius: 10px; font-size: 18px;"
        "}"
        "QPushButton:hover { background-color: #e2e8f0; color: #1e293b; }");

    ui->btnSupprimerProjet->setStyleSheet(
        "QPushButton {"
        "    background-color: #f1f5f9; color: #64748b;"
        "    border: 1.5px solid #e2e8f0; border-radius: 10px; font-size: 18px;"
        "}"
        "QPushButton:hover { background-color: #fee2e2; border-color: #fca5a5; color: #dc2626; }");
}

void SmartPub::handleProjetRechercheChanged(const QString &text)
{
    projFiltrerTable(text);
}

void SmartPub::projFiltrerTable(const QString &text)
{
    for (int row = 0; row < ui->tableWidgetProjets->rowCount(); ++row) {
        bool match = false;

        for (int col = 0; col < ui->tableWidgetProjets->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetProjets->item(row, col);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }

        ui->tableWidgetProjets->setRowHidden(row, !match);
    }
}

void SmartPub::handleProjetAnnulerForm()
{
    projClearProjectFormErrors();
    projCacherFormulaire();
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
}

void SmartPub::handleProjetEnregistrerForm()
{
    if (!projValiderFormulaire()) {
        return;
    }

    Projet projet = projGetProjetFromForm();
    QSqlDatabase db = Connection::instance()->getDatabase();

    if (!db.isOpen()) {
        qDebug() << QStringLiteral("PROJET save: base fermée");
        projShowLineFieldError(ui->lineEditCodeForm, projErrCodeLabel,
                               QStringLiteral("Connexion à la base de données perdue."));
        return;
    }

    projEnsureOracleProjetMeta(db);

    QSqlQuery query(db);

    const double progVal = projProgressionStrToDouble(projet.progression);

    if (isEditing) {
        if (projExecUpdateProjet(query, projet, progVal, currentProjetId)) {
            QToolTip::showText(ui->btnEnregistrerForm->mapToGlobal(QPoint(0, ui->btnEnregistrerForm->height())),
                               QStringLiteral("Projet modifié."), ui->btnEnregistrerForm);
        } else {
            QString em = query.lastError().text();
            qDebug() << QStringLiteral("PROJET UPDATE:") << em;
            if (em.size() > 120)
                em = em.left(117) + QLatin1String("...");
            projShowLineFieldError(ui->lineEditCodeForm, projErrCodeLabel,
                                   QStringLiteral("Enregistrement impossible : %1").arg(em));
            return;
        }
    } else {
        if (projExecInsertProjet(query, projet, progVal)) {
            QToolTip::showText(ui->btnEnregistrerForm->mapToGlobal(QPoint(0, ui->btnEnregistrerForm->height())),
                               QStringLiteral("Projet enregistré."), ui->btnEnregistrerForm);
        } else {
            QString em = query.lastError().text();
            qDebug() << QStringLiteral("PROJET INSERT:") << em;
            if (em.size() > 120)
                em = em.left(117) + QLatin1String("...");
            projShowLineFieldError(ui->lineEditCodeForm, projErrCodeLabel,
                                   QStringLiteral("Enregistrement impossible : %1").arg(em));
            return;
        }
    }

    projCacherFormulaire();
    ui->stackedWidgetProjets->setCurrentIndex(0);
    projSetActiveCrudButton(0);
    projChargerProjets();  // Recharger depuis Oracle
}

void SmartPub::projMettreAJourProjetTable(int row, const Projet &projet)
{
    QString titreDisplay = projet.titre;
    if (titreDisplay.length() > 35) {
        titreDisplay = titreDisplay.left(32) + "...";
    }

    QTableWidgetItem *codeItem = ui->tableWidgetProjets->item(row, 0);
    if (codeItem) {
        codeItem->setText(projet.code);
        codeItem->setData(kProjetRowInternalIdRole, projet.id);
    }
    ui->tableWidgetProjets->item(row, 1)->setText(titreDisplay);
    ui->tableWidgetProjets->item(row, 1)->setToolTip(projet.titre);
    ui->tableWidgetProjets->item(row, 1)->setForeground(QColor("#1e293b"));
    ui->tableWidgetProjets->item(row, 2)->setText(projet.dateDebut.toString("dd/MM/yyyy"));
    ui->tableWidgetProjets->item(row, 3)->setText(projet.dateFin.toString("dd/MM/yyyy"));
    ui->tableWidgetProjets->item(row, 4)->setText(projet.responsable);

    QTableWidgetItem *etatItem = ui->tableWidgetProjets->item(row, 5);
    etatItem->setText(projEtatDbToUi(projet.etat));
    etatItem->setForeground(QColor(projEtatColor(projEtatDbToUi(projet.etat))));

    QTableWidgetItem *progItem = ui->tableWidgetProjets->item(row, 6);
    progItem->setText(projet.progression);
    progItem->setForeground(QColor(projProgressionColorFromString(projet.progression)));
}

void SmartPub::handleProjetSupprimerProjet()
{
    int row = projGetSelectedRow();
    if (row == -1) {
        QToolTip::showText(ui->btnSupprimerProjet->mapToGlobal(QPoint(0, ui->btnSupprimerProjet->height())),
                           QStringLiteral("Sélectionnez un projet à supprimer."), ui->btnSupprimerProjet);
        return;
    }

    const int projetId = projetInternalIdFromTableRow(ui->tableWidgetProjets, row);
    if (projetId <= 0) {
        QToolTip::showText(ui->btnSupprimerProjet->mapToGlobal(QPoint(0, ui->btnSupprimerProjet->height())),
                           QStringLiteral("Projet introuvable."), ui->btnSupprimerProjet);
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, QStringLiteral("Confirmer la suppression"),
                                    QStringLiteral("Supprimer ce projet ?"),
                                    QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) {
            qDebug() << QStringLiteral("PROJET DELETE: base fermée");
            QToolTip::showText(ui->btnSupprimerProjet->mapToGlobal(QPoint(0, ui->btnSupprimerProjet->height())),
                               QStringLiteral("Connexion base perdue."), ui->btnSupprimerProjet);
            return;
        }

        QSqlQuery query(db);
        const QString sqlDel =
            QStringLiteral("DELETE FROM %1 WHERE ID_PROJET = :id").arg(projProjetTableName(db));
        projLogSql("DELETE", sqlDel);
        query.prepare(sqlDel);
        query.bindValue(QStringLiteral(":id"), projetId);

        if (query.exec()) {
            QToolTip::showText(ui->btnSupprimerProjet->mapToGlobal(QPoint(0, ui->btnSupprimerProjet->height())),
                               QStringLiteral("Projet supprimé."), ui->btnSupprimerProjet);
            projChargerProjets();
        } else {
            qDebug() << QStringLiteral("PROJET DELETE:") << query.lastError().text();
            QToolTip::showText(ui->btnSupprimerProjet->mapToGlobal(QPoint(0, ui->btnSupprimerProjet->height())),
                               QStringLiteral("Échec suppression (voir journal)."), ui->btnSupprimerProjet);
        }
    }
}

void SmartPub::handleProjetTableSelectionChanged()
{
    bool hasSelection = !ui->tableWidgetProjets->selectedItems().isEmpty();
    ui->btnModifierProjet->setEnabled(hasSelection);
    ui->btnSupprimerProjet->setEnabled(hasSelection);
}

void SmartPub::handleProjetTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    const int projetId = projetInternalIdFromTableRow(ui->tableWidgetProjets, row);
    if (projetId > 0)
        projShowProjetDetails(projetId);
}

void SmartPub::handleProjetTriDateDebut()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateFin->setChecked(false);
    ui->btnTriEtat->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 2) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 2;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📅 Date Début ▲" : "📅 Date Début ▼");
}

void SmartPub::handleProjetTriDateFin()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriEtat->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 3) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 3;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📅 Date Fin ▲" : "📅 Date Fin ▼");
}

void SmartPub::handleProjetTriEtat()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriDateFin->setChecked(false);
    ui->btnTriProgression->setChecked(false);

    if (currentSortColumn == 5) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 5;
        currentSortOrder = Qt::AscendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "🔧 État ▲" : "🔧 État ▼");
}

void SmartPub::handleProjetTriProgression()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    ui->btnTriDateDebut->setChecked(false);
    ui->btnTriDateFin->setChecked(false);
    ui->btnTriEtat->setChecked(false);

    if (currentSortColumn == 6) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortColumn = 6;
        currentSortOrder = Qt::DescendingOrder;
    }

    projSortProjetsBy(currentSortColumn, currentSortOrder);
    btn->setChecked(true);
    btn->setText(currentSortOrder == Qt::AscendingOrder ? "📈 Progression ▲" : "📈 Progression ▼");
}

void SmartPub::projSortProjetsBy(int column, Qt::SortOrder order)
{
    switch (column) {
    case 2:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      return order == Qt::AscendingOrder ? a.dateDebut < b.dateDebut : a.dateDebut > b.dateDebut;
                  });
        break;
    case 3:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      return order == Qt::AscendingOrder ? a.dateFin < b.dateFin : a.dateFin > b.dateFin;
                  });
        break;
    case 5:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      return order == Qt::AscendingOrder ? a.etat < b.etat : a.etat > b.etat;
                  });
        break;
    case 6:
        std::sort(projets.begin(), projets.end(),
                  [order](const Projet &a, const Projet &b) {
                      int progA = a.progression.left(a.progression.indexOf('%')).toInt();
                      int progB = b.progression.left(b.progression.indexOf('%')).toInt();
                      return order == Qt::AscendingOrder ? progA < progB : progA > progB;
                  });
        break;
    }

    projChargerProjets();
}

void SmartPub::handleProjetStatistiques()
{
    StatistiquesDialog *dialog = new StatistiquesDialog(projets, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::handleProjetSante()
{
    QString message = "<b>Santé des Projets</b><br><br>";
    int sains = 0, risque = 0, critiques = 0;

    for (const auto &p : projets) {
        QString statut = projCalculerStatutProjet(p);
        if (statut == "Sain") sains++;
        else if (statut == "À risque") risque++;
        else if (statut == "Critique") critiques++;
    }

    message += QString("<span style='color: #10b981;'>Sains: %1</span><br>").arg(sains);
    message += QString("<span style='color: #f59e0b;'>À risque: %1</span><br>").arg(risque);
    message += QString("<span style='color: #ef4444;'>Critiques: %1</span><br>").arg(critiques);

    QMessageBox::information(this, "Santé des Projets", message);
}

void SmartPub::handleProjetOptimiserCharge()
{
    QMessageBox::information(this, "Optimisation de Charge",
                             "<b>Analyse de la charge de travail</b><br><br>"
                             "Cette fonctionnalité analysera la répartition des projets par responsable "
                             "et suggérera des optimisations.");
}

void SmartPub::handleProjetIARecommander()
{
    IARecommandationsDialog *dialog = new IARecommandationsDialog(projets, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::handleProjetFiltres()
{
    FiltresDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        filtreEtat = dialog.getEtatFiltre();
        filtreResponsable = dialog.getResponsableFiltre();
        filtreDateDebutMin = dialog.getDateDebutMin();
        filtreDateDebutMax = dialog.getDateDebutMax();
        filtresActifs = !filtreEtat.isEmpty() || !filtreResponsable.isEmpty() ||
                        filtreDateDebutMin != QDate(2020, 1, 1) ||
                        filtreDateDebutMax != QDate::currentDate().addYears(5);

        projAppliquerFiltres();
        projMettreAJourBadgeFiltres();
    }
}

void SmartPub::projAppliquerFiltres()
{
    projetsFiltres.clear();

    for (const auto &projet : projets) {
        bool match = true;

        if (!filtreEtat.isEmpty() && projet.etat != filtreEtat) {
            match = false;
        }

        if (!filtreResponsable.isEmpty() && projet.responsable != filtreResponsable) {
            match = false;
        }

        if (projet.dateDebut < filtreDateDebutMin || projet.dateDebut > filtreDateDebutMax) {
            match = false;
        }

        if (match) {
            projetsFiltres.append(projet);
        }
    }

    projChargerProjets();
}

void SmartPub::projMettreAJourBadgeFiltres()
{
    if (filtresActifs) {
        ui->btnFiltresProjets->setText("🎛️ Filtres ✓");
        ui->btnFiltresProjets->setStyleSheet(
            "QPushButton {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);"
            "    color: white;"
            "    border: none;"
            "    border-radius: 10px;"
            "    font-size: 13px;"
            "    font-weight: 600;"
            "}"
            "QPushButton:hover {"
            "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);"
            "}"
            );
    } else {
        ui->btnFiltresProjets->setText("🎛️ Filtres");
        ui->btnFiltresProjets->setStyleSheet(
            "QPushButton {"
            "    background-color: white;"
            "    color: #334155;"
            "    border: 1px solid #e2e8f0;"
            "    border-radius: 10px;"
            "    font-size: 13px;"
            "    font-weight: 500;"
            "}"
            "QPushButton:hover {"
            "    background-color: #f8fafc;"
            "    border-color: #cbd5e1;"
            "}"
            );
    }
}

void SmartPub::handleProjetExporter()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les projets",
                                                    "", "Fichiers CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Exportation", "Projets exportés avec succès dans:\n" + fileName);
    }
}

void SmartPub::projAfficherFormulaire(bool isEdit)
{
    ui->labelFormTitle->setText(isEdit ? "Modifier le Projet" : "Nouveau Projet");
    ui->lineEditCodeForm->setReadOnly(isEdit);
    ui->stackedWidgetProjets->setCurrentIndex(1);
}

void SmartPub::projCacherFormulaire()
{
    ui->stackedWidgetProjets->setCurrentIndex(0);
}

void SmartPub::projRemplirFormulaire(const Projet &projet)
{
    projClearProjectFormErrors();
    {
        const QSignalBlocker b1(ui->lineEditCodeForm);
        const QSignalBlocker b2(ui->lineEditTitreForm);
        const QSignalBlocker b3(ui->dateEditDebutForm);
        const QSignalBlocker b4(ui->dateEditFinForm);
        const QSignalBlocker b5(ui->comboBoxResponsableForm);
        const QSignalBlocker b6(ui->comboBoxEtatForm);
        ui->lineEditCodeForm->setText(projet.code);
        ui->lineEditTitreForm->setText(projet.titre);
        ui->dateEditDebutForm->setDate(projet.dateDebut);
        ui->dateEditFinForm->setDate(projet.dateFin);

        int respIndex = ui->comboBoxResponsableForm->findData(QVariant(projet.responsableId));
        if (respIndex >= 0)
            ui->comboBoxResponsableForm->setCurrentIndex(respIndex);
        else
            ui->comboBoxResponsableForm->setCurrentIndex(0);

        int etatIndex = ui->comboBoxEtatForm->findData(projet.etat);
        if (etatIndex >= 0)
            ui->comboBoxEtatForm->setCurrentIndex(etatIndex);
        else
            ui->comboBoxEtatForm->setCurrentIndex(0);
    }
    ui->textEditDescriptionForm->setText(projet.description);
}

void SmartPub::projViderFormulaire()
{
    projClearProjectFormErrors();
    ui->lineEditCodeForm->setReadOnly(false);
    {
        const QSignalBlocker b1(ui->lineEditCodeForm);
        const QSignalBlocker b2(ui->lineEditTitreForm);
        const QSignalBlocker b3(ui->dateEditDebutForm);
        const QSignalBlocker b4(ui->dateEditFinForm);
        const QSignalBlocker b5(ui->comboBoxResponsableForm);
        const QSignalBlocker b6(ui->comboBoxEtatForm);
        ui->lineEditCodeForm->clear();
        ui->lineEditTitreForm->clear();
        ui->dateEditDebutForm->setDate(QDate::currentDate());
        ui->dateEditFinForm->setDate(QDate::currentDate().addDays(30));
        ui->comboBoxResponsableForm->setCurrentIndex(0);
        ui->comboBoxEtatForm->setCurrentIndex(0);
    }
    ui->textEditDescriptionForm->clear();
}

Projet SmartPub::projGetProjetFromForm() const
{
    Projet projet;
    if (isEditing) {
        projet.id = currentProjetId;
        for (const Projet &p : projets) {
            if (p.id == currentProjetId) {
                projet.progression = p.progression;
                break;
            }
        }
    }
    if (projet.progression.isEmpty())
        projet.progression = QStringLiteral("0%");

    projet.code = ui->lineEditCodeForm->text().trimmed();
    projet.titre = ui->lineEditTitreForm->text().trimmed();
    projet.dateDebut = ui->dateEditDebutForm->date();
    projet.dateFin = ui->dateEditFinForm->date();
    projet.responsableId = ui->comboBoxResponsableForm->currentData().toInt();
    // Libellé affichage uniquement (clé SQL = responsableId via currentData)
    {
        const int ix = ui->comboBoxResponsableForm->currentIndex();
        projet.responsable = (projet.responsableId > 0 && ix >= 0)
                                 ? ui->comboBoxResponsableForm->itemText(ix).trimmed()
                                 : QString();
    }
    projet.etat = ui->comboBoxEtatForm->currentData().toString();
    if (projet.etat.isEmpty())
        projet.etat = QStringLiteral("en_cours");
    projet.description = ui->textEditDescriptionForm->toPlainText();
    return projet;
}

bool SmartPub::projValidateCode(bool forSubmit)
{
    if (isEditing)
        return true;
    if (!forSubmit && !projTouchedCode)
        return true;
    const QString codeTxt = ui->lineEditCodeForm->text().trimmed();
    if (codeTxt.isEmpty()) {
        if (forSubmit) {
            projShowLineFieldError(ui->lineEditCodeForm, projErrCodeLabel,
                                   QStringLiteral("Veuillez renseigner ce champ."));
            return false;
        }
        projHideLineFieldError(ui->lineEditCodeForm, projErrCodeLabel);
        return true;
    }
    if (!projetCodeExactRx().match(codeTxt).hasMatch()) {
        projShowLineFieldError(ui->lineEditCodeForm, projErrCodeLabel,
                               QStringLiteral("Code invalide. Format : PRJ-2024-AI-01"));
        return false;
    }
    projHideLineFieldError(ui->lineEditCodeForm, projErrCodeLabel);
    return true;
}

bool SmartPub::projValidateTitre(bool forSubmit)
{
    if (!forSubmit && !projTouchedTitre)
        return true;
    const QString raw = ui->lineEditTitreForm->text();
    const QString t = raw.trimmed();
    if (t.isEmpty()) {
        if (forSubmit) {
            projShowLineFieldError(ui->lineEditTitreForm, projErrTitreLabel,
                                   QStringLiteral("Veuillez renseigner ce champ."));
            return false;
        }
        projHideLineFieldError(ui->lineEditTitreForm, projErrTitreLabel);
        return true;
    }
    QString errTitre;
    if (!projetTitreValide(raw, &errTitre)) {
        projShowLineFieldError(ui->lineEditTitreForm, projErrTitreLabel, errTitre);
        return false;
    }
    projHideLineFieldError(ui->lineEditTitreForm, projErrTitreLabel);
    return true;
}

bool SmartPub::projValidateResponsable(bool forSubmit)
{
    if (!forSubmit && !projTouchedResponsable)
        return true;
    const int rid = ui->comboBoxResponsableForm->currentData().toInt();
    if (rid <= 0) {
        if (forSubmit) {
            projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel,
                                    QStringLiteral("Veuillez renseigner ce champ."));
            return false;
        }
        projHideComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel);
        return true;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        if (forSubmit) {
            projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel,
                                    QStringLiteral("Connexion à la base de données perdue."));
            return false;
        }
        projHideComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel);
        return true;
    }

    const QString sqlCnt =
        QStringLiteral("SELECT COUNT(*) FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    projLogSql("COUNT CHERCHEUR (responsable)", sqlCnt);
    QSqlQuery cnt(db);
    cnt.prepare(sqlCnt);
    cnt.bindValue(QStringLiteral(":id"), rid);
    if (!cnt.exec()) {
        qDebug() << QStringLiteral("[PROJET SQL] COUNT ID_CHERCHEUR:") << cnt.lastError().text();
        if (forSubmit) {
            projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel,
                                    QStringLiteral("Impossible de vérifier le responsable."));
            return false;
        }
        return true;
    }
    if (!cnt.next() || cnt.value(0).toInt() < 1) {
        projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel,
                                QStringLiteral("Chercheur introuvable."));
        return false;
    }

    const QString sqlEm = QStringLiteral("SELECT EMAIL FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    projLogSql("SELECT EMAIL responsable", sqlEm);
    QSqlQuery q(db);
    q.prepare(sqlEm);
    q.bindValue(QStringLiteral(":id"), rid);
    if (!q.exec()) {
        qDebug() << QStringLiteral("[PROJET SQL] EMAIL responsable:") << q.lastError().text();
        if (forSubmit) {
            projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel,
                                    QStringLiteral("Impossible de vérifier le responsable."));
            return false;
        }
        return true;
    }
    if (!q.next()) {
        projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel,
                                QStringLiteral("Chercheur introuvable."));
        return false;
    }

    const QString emailNorm = projetNormEmail(q.value(0).toString());
    if (emailNorm.isEmpty()) {
        projShowComboFieldError(
            ui->comboBoxResponsableForm, projErrResponsableLabel,
            QStringLiteral("Le responsable doit avoir un e-mail renseigné en base (rappels)."));
        return false;
    }

    QString emErr;
    if (!projetChercheurEmailStrictOk(emailNorm, &emErr)) {
        projShowComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel, emErr);
        return false;
    }

    projHideComboFieldError(ui->comboBoxResponsableForm, projErrResponsableLabel);
    return true;
}

bool SmartPub::projValidateDates(bool forSubmit)
{
    if (!forSubmit && !projTouchedDates)
        return true;
    if (ui->dateEditDebutForm->date() > ui->dateEditFinForm->date()) {
        projShowDateOrderError(
            QStringLiteral("La date de début doit être antérieure ou égale à la date de fin."));
        return false;
    }
    projHideDateOrderError();
    return true;
}

bool SmartPub::projValiderFormulaire()
{
    const bool c = projValidateCode(true);
    const bool t = projValidateTitre(true);
    const bool r = projValidateResponsable(true);
    const bool d = projValidateDates(true);
    if (c && t && r && d)
        return true;
    if (!c)
        ui->lineEditCodeForm->setFocus();
    else if (!t)
        ui->lineEditTitreForm->setFocus();
    else if (!r)
        ui->comboBoxResponsableForm->setFocus();
    else if (!d)
        ui->dateEditDebutForm->setFocus();
    return false;
}

void SmartPub::projMettreAJourStats()
{
}

int SmartPub::projGetSelectedRow() const
{
    QList<QTableWidgetItem*> selected = ui->tableWidgetProjets->selectedItems();
    if (!selected.isEmpty()) {
        return selected.first()->row();
    }
    return -1;
}

void SmartPub::projShowProjetDetails(int projetId)
{
    for (const Projet &projet : projets) {
        if (projet.id == projetId) {
            projShowProjetDetailsDialog(projet);
            break;
        }
    }
}

void SmartPub::projShowProjetDetailsDialog(const Projet &projet)
{
    ProjetDetailsDialog *dialog = new ProjetDetailsDialog(projet, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

QString SmartPub::projCalculerStatutProjet(const Projet &projet) const
{
    int progression = projExtraireProgression(projet.progression);
    int joursRestants = QDate::currentDate().daysTo(projet.dateFin);

    if (projet.etat == QLatin1String("termine")) return QStringLiteral("Sain");

    if (joursRestants < 0) {
        return "Critique";
    } else if (joursRestants < 30) {
        if (progression < 80) return "Critique";
        else if (progression < 90) return "À risque";
        else return "Sain";
    } else {
        return "Sain";
    }
}

double SmartPub::projCalculerTauxAvancement(const Projet &projet) const
{
    int joursTotaux = projet.dateDebut.daysTo(projet.dateFin);
    int joursRestants = QDate::currentDate().daysTo(projet.dateFin);

    if (joursTotaux <= 0) return 100.0;

    double pourcentageTemps = (1.0 - (double)joursRestants / (double)joursTotaux) * 100;
    return qBound(0.0, pourcentageTemps, 100.0);
}

QVector<QString> SmartPub::projGenererAlertes(const Projet &projet) const
{
    QVector<QString> alertes;

    if (projet.etat == QLatin1String("termine")) return alertes;

    int joursRestants = QDate::currentDate().daysTo(projet.dateFin);

    if (joursRestants < 0) {
        alertes.append("Projet en retard ! Date dépassée.");
    }
    else if (joursRestants < 30) {
        alertes.append(QString("Échéance proche (%1 jours)").arg(joursRestants));
    }

    return alertes;
}

int SmartPub::projCompterProjetsParEtat(const QString &etat) const
{
    int count = 0;
    for (const auto &p : projets) {
        if (p.etat == etat) count++;
    }
    return count;
}

double SmartPub::projCalculerProgressionMoyenne() const
{
    double total = 0;
    int count = 0;
    for (const auto &p : projets) {
        if (p.etat != QLatin1String("termine") && p.etat != QLatin1String("annule")) {
            total += projExtraireProgression(p.progression);
            count++;
        }
    }
    return count > 0 ? total / count : 0;
}

int SmartPub::projCompterProjetsEnRetard() const
{
    int count = 0;
    for (const auto &p : projets) {
        if (p.etat != QLatin1String("termine") && QDate::currentDate() > p.dateFin) {
            count++;
        }
    }
    return count;
}

void SmartPub::handleProjetsNavigation() {
    ui->stackedWidgetModules->setCurrentIndex(4);
    setActiveNavigationButton(4);
    updateProfileName(4);
    projChargerProjets();
}

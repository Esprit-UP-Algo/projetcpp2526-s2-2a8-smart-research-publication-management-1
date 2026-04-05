#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include "publicationauth.h"
#include <algorithm>
#include <QTableWidgetItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QSet>
#include <QBrush>
#include <QAbstractBarSeries>


// Publications : libellés UI (combo) <-> valeurs CHECK Oracle (SmartPub1.sql)
static QString SR_statutUiToDb(const QString &ui) {
    const QString t = ui.trimmed();
    if (t.compare(QLatin1String("Publié"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("publie");
    if (t.compare(QLatin1String("Soumis"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("soumis");
    if (t.contains(QLatin1String("révision"), Qt::CaseInsensitive) ||
        t.contains(QLatin1String("revision"), Qt::CaseInsensitive))
        return QStringLiteral("en_revision");
    if (t.compare(QLatin1String("Accepté"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("accepte");
    if (t.compare(QLatin1String("Rejeté"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("rejete");
    return QStringLiteral("soumis");
}

static QString SR_statutDbToUi(const QString &db) {
    const QString d = db.trimmed().toLower();
    if (d == QLatin1String("publie"))
        return QStringLiteral("Publié");
    if (d == QLatin1String("soumis"))
        return QStringLiteral("Soumis");
    if (d == QLatin1String("en_revision"))
        return QStringLiteral("En révision");
    if (d == QLatin1String("accepte"))
        return QStringLiteral("Accepté");
    if (d == QLatin1String("rejete"))
        return QStringLiteral("Rejeté");
    return db;
}

struct PublicationStatData {
    QString titre;
    QString auteur;
    QString date;
    QString revue;
    QString statut;
};

class PubStatistiquesDialog : public QDialog {
public:
    explicit PubStatistiquesDialog(const QList<PublicationStatData> &publications, QWidget *parent = nullptr)
        : QDialog(parent), m_publications(publications), labelTotal(nullptr), labelCetteAnnee(nullptr),
          labelNbRevues(nullptr), labelNbPublications(nullptr), chartStatutView(nullptr), chartRevueView(nullptr) {
        setWindowTitle("Statistiques Publications");
        setMinimumSize(900, 650);
        resize(1000, 700);
        setStyleSheet(
            "QDialog { background-color: #f1f5f9; font-family: 'Segoe UI', sans-serif; }"
            "QLabel { color: #334155; }"
            "QGroupBox { font-weight: bold; border: 1px solid #e2e8f0; border-radius: 12px;"
            " margin-top: 15px; padding-top: 15px; background-color: white; }"
            "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 10px;"
            " color: #3b82f6; font-size: 14px; }"
            "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3b82f6, stop:1 #10b981);"
            " color: white; border: none; border-radius: 10px; padding: 12px 24px; font-size: 14px; font-weight: 600; }"
            "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #2563eb, stop:1 #059669); }");
        setupUI();
        calculerStatistiques();
        creerGraphiques();
    }

private:
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QFrame *headerFrame = new QFrame();
        headerFrame->setStyleSheet(
            "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3b82f6, stop:1 #10b981); border: none; }");
        headerFrame->setFixedHeight(100);
        QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
        headerLayout->setContentsMargins(30, 20, 30, 20);
        QLabel *titleLabel = new QLabel("📊 Statistiques Publications");
        titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
        QLabel *subtitleLabel = new QLabel("Tableau de bord des publications");
        subtitleLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 14px;");
        headerLayout->addWidget(titleLabel);
        headerLayout->addWidget(subtitleLabel);
        mainLayout->addWidget(headerFrame);

        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("background-color: #f1f5f9;");

        QWidget *contentWidget = new QWidget();
        QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setSpacing(25);
        contentLayout->setContentsMargins(30, 30, 30, 30);

        QHBoxLayout *kpiLayout = new QHBoxLayout();
        kpiLayout->setSpacing(20);
        auto createKPI = [](const QString &icon, const QString &value, const QString &label,
                            const QString &color, QLabel **valueLabelPtr) -> QFrame * {
            QFrame *kpi = new QFrame();
            kpi->setStyleSheet("QFrame { background-color: white; border-radius: 12px; border: 1px solid #e2e8f0; }");
            kpi->setFixedHeight(120);
            QVBoxLayout *layout = new QVBoxLayout(kpi);
            layout->setSpacing(5);
            QLabel *iconLabel = new QLabel(icon);
            iconLabel->setStyleSheet("font-size: 24px;");
            iconLabel->setAlignment(Qt::AlignCenter);
            QLabel *valueLabel = new QLabel(value);
            valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(color));
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

        kpiLayout->addWidget(createKPI("📚", "0", "Total publications", "#3b82f6", &labelTotal));
        kpiLayout->addWidget(createKPI("🗓️", "0", "Cette année", "#10b981", &labelCetteAnnee));
        kpiLayout->addWidget(createKPI("🧾", "0", "Revues distinctes", "#8b5cf6", &labelNbRevues));
        kpiLayout->addWidget(createKPI("👤", "0", "Nb. Publications", "#f59e0b", &labelNbPublications));
        contentLayout->addLayout(kpiLayout);

        QHBoxLayout *chartsLayout = new QHBoxLayout();
        chartsLayout->setSpacing(20);

        QGroupBox *chartStatutGroup = new QGroupBox("Répartition par Statut");
        QVBoxLayout *chartStatutLayout = new QVBoxLayout(chartStatutGroup);
        chartStatutView = new QChartView();
        chartStatutView->setMinimumHeight(300);
        chartStatutView->setRenderHint(QPainter::Antialiasing);
        chartStatutLayout->addWidget(chartStatutView);
        chartsLayout->addWidget(chartStatutGroup, 1);

        QGroupBox *chartRevueGroup = new QGroupBox("Publications par Revue");
        QVBoxLayout *chartRevueLayout = new QVBoxLayout(chartRevueGroup);
        chartRevueView = new QChartView();
        chartRevueView->setMinimumHeight(300);
        chartRevueView->setRenderHint(QPainter::Antialiasing);
        chartRevueLayout->addWidget(chartRevueView);
        chartsLayout->addWidget(chartRevueGroup, 1);

        contentLayout->addLayout(chartsLayout);

        QFrame *footerFrame = new QFrame();
        footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
        footerFrame->setFixedHeight(70);
        QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
        footerLayout->addStretch();
        QPushButton *closeButton = new QPushButton("Fermer");
        closeButton->setFixedSize(140, 45);
        closeButton->setCursor(Qt::PointingHandCursor);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
        footerLayout->addWidget(closeButton);
        mainLayout->addWidget(footerFrame);

        scrollArea->setWidget(contentWidget);
        mainLayout->addWidget(scrollArea, 1);
    }

    void calculerStatistiques() {
        const int total = m_publications.size();
        const int currentYear = QDate::currentDate().year();
        int thisYear = 0;
        QSet<QString> revues;
        for (const PublicationStatData &p : m_publications) {
            if (p.date.left(4).toInt() == currentYear)
                thisYear++;
            if (!p.revue.trimmed().isEmpty())
                revues.insert(p.revue.trimmed());
        }

        if (labelTotal)
            labelTotal->setText(QString::number(total));
        if (labelCetteAnnee)
            labelCetteAnnee->setText(QString::number(thisYear));
        if (labelNbRevues)
            labelNbRevues->setText(QString::number(revues.size()));
        if (labelNbPublications)
            labelNbPublications->setText(QString::number(total));
    }

    void creerGraphiques() {
        QMap<QString, int> statutCounts;
        for (const PublicationStatData &p : m_publications)
            statutCounts[p.statut.trimmed().isEmpty() ? QStringLiteral("Inconnu") : p.statut.trimmed()]++;

        QPieSeries *seriesStatut = new QPieSeries();
        if (statutCounts.isEmpty())
            statutCounts.insert(QStringLiteral("Aucune donnée"), 1);
        for (auto it = statutCounts.constBegin(); it != statutCounts.constEnd(); ++it)
            seriesStatut->append(it.key(), it.value());
        for (int i = 0; i < seriesStatut->count(); ++i) {
            seriesStatut->slices().at(i)->setLabelVisible(true);
            const QString name = seriesStatut->slices().at(i)->label();
            seriesStatut->slices().at(i)->setLabel(
                QString("%1 (%2%)").arg(name).arg(seriesStatut->slices().at(i)->percentage() * 100, 0, 'f', 1));
        }
        QChart *chartStatut = new QChart();
        chartStatut->addSeries(seriesStatut);
        chartStatut->setAnimationOptions(QChart::SeriesAnimations);
        chartStatut->setBackgroundBrush(QBrush(QColor("transparent")));
        chartStatut->legend()->setVisible(true);
        chartStatutView->setChart(chartStatut);

        QMap<QString, int> byRevue;
        for (const PublicationStatData &p : m_publications) {
            const QString revue = p.revue.trimmed().isEmpty() ? QStringLiteral("Sans revue") : p.revue.trimmed();
            byRevue[revue]++;
        }
        if (byRevue.isEmpty())
            byRevue.insert(QStringLiteral("Aucune donnée"), 0);

        QVector<QPair<QString, int>> revueData;
        revueData.reserve(byRevue.size());
        for (auto it = byRevue.constBegin(); it != byRevue.constEnd(); ++it)
            revueData.push_back(qMakePair(it.key(), it.value()));
        std::sort(revueData.begin(), revueData.end(),
                  [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                      return a.second > b.second;
                  });

        const int maxBars = 8;
        QBarSet *barSet = new QBarSet("Publications");
        QStringList categories;
        int maxValue = 0;
        for (int i = 0; i < revueData.size() && i < maxBars; ++i) {
            barSet->append(revueData[i].second);
            categories << revueData[i].first;
            maxValue = qMax(maxValue, revueData[i].second);
        }
        barSet->setColor(QColor("#3b82f6"));
        barSet->setLabelColor(QColor("#1e293b"));
        QBarSeries *barSeries = new QBarSeries();
        barSeries->append(barSet);
        barSeries->setLabelsVisible(true);
        barSeries->setLabelsFormat("@value");
        barSeries->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);

        QChart *chartRevue = new QChart();
        chartRevue->addSeries(barSeries);
        chartRevue->setTitle(QStringLiteral("Top revues (%1)").arg(categories.size()));
        chartRevue->setAnimationOptions(QChart::SeriesAnimations);
        chartRevue->setBackgroundBrush(QBrush(QColor("transparent")));
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chartRevue->addAxis(axisX, Qt::AlignBottom);
        barSeries->attachAxis(axisX);
        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0, qMax(1, maxValue + 1));
        axisY->setLabelFormat("%d");
        axisY->setTickCount(qMin(10, qMax(2, maxValue + 2)));
        chartRevue->addAxis(axisY, Qt::AlignLeft);
        barSeries->attachAxis(axisY);
        chartRevue->legend()->setVisible(false);
        chartRevueView->setChart(chartRevue);
    }

    QList<PublicationStatData> m_publications;
    QLabel *labelTotal;
    QLabel *labelCetteAnnee;
    QLabel *labelNbRevues;
    QLabel *labelNbPublications;
    QChartView *chartStatutView;
    QChartView *chartRevueView;
};

// ============================================================================
// MODULE PUBLICATIONS
// ============================================================================

void SmartPub::SR_setupUI() {
    ui->SR_stackedWidget->setCurrentIndex(0);

    SR_filterFrame = new QFrame(ui->SR_tableFrame);
    SR_filterFrame->setStyleSheet("background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 10px; padding: 4px;");
    SR_filterFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout *filterLayout = new QHBoxLayout(SR_filterFrame);
    filterLayout->setSpacing(12);

    QLabel *lblTitre = new QLabel("Titre", SR_filterFrame);
    SR_filterTitre = new QLineEdit(SR_filterFrame);
    SR_filterTitre->setPlaceholderText("Filtrer par titre...");
    SR_filterTitre->setMinimumWidth(140);
    QLabel *lblAuteur = new QLabel("Auteur", SR_filterFrame);
    SR_filterAuteur = new QLineEdit(SR_filterFrame);
    SR_filterAuteur->setPlaceholderText("Filtrer par auteur...");
    SR_filterAuteur->setMinimumWidth(140);
    QLabel *lblStatut = new QLabel("Statut", SR_filterFrame);
    SR_filterStatut = new QComboBox(SR_filterFrame);
    SR_filterStatut->setMinimumWidth(120);
    SR_filterStatut->addItem("Tous");
    SR_filterStatut->addItem("Publié");
    SR_filterStatut->addItem("Soumis");
    SR_filterStatut->addItem("En révision");
    SR_filterStatut->addItem("Accepté");
    SR_filterStatut->addItem("Rejeté");
    SR_btnReinitFilter = new QPushButton("Réinitialiser", SR_filterFrame);

    filterLayout->addWidget(lblTitre);
    filterLayout->addWidget(SR_filterTitre);
    filterLayout->addWidget(lblAuteur);
    filterLayout->addWidget(SR_filterAuteur);
    filterLayout->addWidget(lblStatut);
    filterLayout->addWidget(SR_filterStatut);
    filterLayout->addWidget(SR_btnReinitFilter);
    filterLayout->addStretch();

    QVBoxLayout *tableLayout = qobject_cast<QVBoxLayout *>(ui->SR_tableFrame->layout());
    if (tableLayout)
        tableLayout->insertWidget(1, SR_filterFrame);
}

void SmartPub::SR_connectSignals() {
    connect(ui->SR_btnVueListe, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnVueListe_clicked);
    connect(ui->SR_btnAjouter, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnAjouter_clicked);
    connect(ui->SR_btnRecherche, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnRecherche_clicked);
    connect(ui->SR_lineEditRecherche, &QLineEdit::textChanged, this,
            [this](const QString &) { handleSRApplyFilterListe(); });
    connect(ui->SR_btnTri, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnTri_clicked);
    // Masquer les champs email/password et le bouton guest (User Request)
    // RETABLIR LES CHAMPS VISIBLES (User Request Update)
    ui->cherchLineEditLoginEmail->setPlaceholderText("Email");
    ui->cherchLineEditLoginPassword->setPlaceholderText("Mot de passe");
    ui->cherchLineEditLoginEmail->setVisible(true);
    ui->cherchLineEditLoginPassword->setVisible(true);

    // Masquer le bouton mot de passe oublié si nécessaire (Le laisser visible si
    // champs visibles)
    ui->cherchBtnMotDePasseOublie->setVisible(true);

    // Modern Button Style
    QString buttonStyle = R"(
        QPushButton {
            background-color: #2563eb;
            color: white;
            border-radius: 8px;
            font-weight: 600;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #1d4ed8;
        }
    )";
    ui->cherchBtnLogin->setStyleSheet(buttonStyle);
    ui->cherchBtnLogin->setText("Se Connecter");
    connect(ui->SR_btnExport, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnExport_clicked);
    connect(ui->SR_btnStatistiques, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnStatistiques_clicked);
    connect(ui->SR_btnAjouterPublication, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnAjouterPublication_clicked);
    connect(ui->SR_btnAnnulerAjout, &QPushButton::clicked, this,
            &SmartPub::on_SR_btnAnnulerAjout_clicked);
    connect(SR_filterTitre, &QLineEdit::textChanged, this, [this]() { handleSRApplyFilterListe(); });
    connect(SR_filterAuteur, &QLineEdit::textChanged, this, [this]() { handleSRApplyFilterListe(); });
    connect(SR_filterStatut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { handleSRApplyFilterListe(); });
    connect(SR_btnReinitFilter, &QPushButton::clicked, this, &SmartPub::SR_reinitFilterListe);
    connect(ui->SR_tablePublications, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row < 0)
            return;
        const QString titre = ui->SR_tablePublications->item(row, 0) ? ui->SR_tablePublications->item(row, 0)->text() : QString();
        const QString auteur = ui->SR_tablePublications->item(row, 1) ? ui->SR_tablePublications->item(row, 1)->text() : QString();
        const QString date = ui->SR_tablePublications->item(row, 2) ? ui->SR_tablePublications->item(row, 2)->text() : QString();
        const QString revue = ui->SR_tablePublications->item(row, 3) ? ui->SR_tablePublications->item(row, 3)->text() : QString();
        const QString statut = ui->SR_tablePublications->item(row, 4) ? ui->SR_tablePublications->item(row, 4)->text() : QString();
        const QString doi = ui->SR_tablePublications->item(row, 0) ? ui->SR_tablePublications->item(row, 0)->toolTip() : QString();

        QDialog *dialog = new QDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowTitle(QString("Publication — %1").arg(titre.isEmpty() ? "Détails" : titre));
        dialog->setMinimumSize(720, 620);
        dialog->setMaximumSize(920, 840);
        dialog->setStyleSheet("background-color: #f8fafc;");

        QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QFrame *headerFrame = new QFrame();
        headerFrame->setStyleSheet(R"(
            QFrame {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
            }
        )");
        headerFrame->setFixedHeight(200);
        QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
        headerLayout->setAlignment(Qt::AlignCenter);
        headerLayout->setSpacing(10);
        headerLayout->setContentsMargins(20, 20, 20, 16);

        QLabel *iconLbl = new QLabel("📄");
        iconLbl->setAlignment(Qt::AlignCenter);
        iconLbl->setFixedSize(110, 110);
        iconLbl->setStyleSheet(
            "color: white; font-size: 56px; border: 4px solid white; border-radius: 55px;");
        headerLayout->addWidget(iconLbl, 0, Qt::AlignCenter);

        QLabel *titleHeaderLbl = new QLabel(titre.isEmpty() ? "Publication" : titre);
        titleHeaderLbl->setStyleSheet(
            "color: white; font-size: 22px; font-weight: 700; "
            "background: transparent; border: none;");
        titleHeaderLbl->setAlignment(Qt::AlignCenter);
        titleHeaderLbl->setWordWrap(true);
        headerLayout->addWidget(titleHeaderLbl, 0, Qt::AlignCenter);

        QLabel *statusBadge = new QLabel(statut.isEmpty() ? "—" : statut);
        statusBadge->setStyleSheet(
            "background: rgba(255,255,255,0.22); color: white; "
            "border-radius: 10px; padding: 5px 16px; font-size: 12px; "
            "font-weight: 600; border: none;");
        statusBadge->setAlignment(Qt::AlignCenter);
        headerLayout->addWidget(statusBadge, 0, Qt::AlignCenter);

        mainLayout->addWidget(headerFrame);

        QScrollArea *scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setStyleSheet("background-color: white; border: none;");

        QWidget *contentWidget = new QWidget();
        contentWidget->setStyleSheet("background-color: white;");
        QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setSpacing(10);
        contentLayout->setContentsMargins(28, 24, 28, 24);

        auto createInfoRow = [](const QString &label, const QString &value,
                                const QString &icon = "") -> QFrame * {
            QFrame *row = new QFrame();
            row->setStyleSheet(
                "QFrame { background-color: #f8fafc; border-radius: 10px; border: none; }");
            row->setMaximumHeight(68);
            QHBoxLayout *rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(16, 10, 16, 10);

            QLabel *iconLbl = new QLabel(icon.isEmpty() ? "•" : icon);
            iconLbl->setStyleSheet("font-size: 16px; background: transparent; border: none;");
            iconLbl->setFixedWidth(26);
            rowLayout->addWidget(iconLbl);

            QLabel *labelLbl = new QLabel(label + " :");
            labelLbl->setStyleSheet(
                "color: #64748b; font-size: 13px; font-weight: 600; "
                "min-width: 130px; background: transparent; border: none;");
            rowLayout->addWidget(labelLbl);

            QLabel *valueLbl = new QLabel(value.isEmpty() ? "—" : value);
            valueLbl->setStyleSheet(
                "color: #1e293b; font-size: 14px; font-weight: 500; "
                "background: transparent; border: none;");
            valueLbl->setWordWrap(true);
            rowLayout->addWidget(valueLbl, 1);
            return row;
        };

        contentLayout->addWidget(createInfoRow("Titre", titre, "📝"));
        contentLayout->addWidget(createInfoRow("Auteur(s)", auteur, "👤"));
        contentLayout->addWidget(createInfoRow("Date publication", date, "📅"));
        contentLayout->addWidget(createInfoRow("Revue", revue, "📚"));
        contentLayout->addWidget(createInfoRow("Statut", statut, "🏷️"));
        contentLayout->addWidget(createInfoRow("DOI", doi, "🔗"));
        contentLayout->addStretch();

        scrollArea->setWidget(contentWidget);
        mainLayout->addWidget(scrollArea, 1);

        QFrame *footerFrame = new QFrame();
        footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
        footerFrame->setFixedHeight(70);
        QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
        footerLayout->setContentsMargins(24, 0, 24, 0);
        footerLayout->setSpacing(12);

        QPushButton *btnExport = new QPushButton(QStringLiteral("📄 Exporter en PDF"));
        btnExport->setCursor(Qt::PointingHandCursor);
        btnExport->setStyleSheet(R"(
            QPushButton {
                background-color: white;
                color: #334155;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                padding: 0 22px;
                font-size: 13px;
                font-weight: 600;
                min-height: 42px;
            }
            QPushButton:hover {
                background-color: #eff6ff;
                border-color: #3b82f6;
                color: #1d4ed8;
            }
        )");
        connect(btnExport, &QPushButton::clicked, dialog, [this, titre, auteur, date, revue, statut, doi]() {
            QString safeTitle = titre.trimmed();
            if (safeTitle.isEmpty())
                safeTitle = QStringLiteral("publication");
            safeTitle.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
            const QString fileName = QFileDialog::getSaveFileName(
                this,
                QStringLiteral("Exporter la publication en PDF"),
                QDir::homePath() + "/" + safeTitle + ".pdf",
                QStringLiteral("PDF (*.pdf)"));
            if (fileName.isEmpty())
                return;

            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            printer.setPageSize(QPageSize(QPageSize::A4));
            printer.setPageOrientation(QPageLayout::Portrait);
            printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

            QPainter painter;
            if (!painter.begin(&printer)) {
                QMessageBox::critical(this, QStringLiteral("Erreur"),
                                      QStringLiteral("Impossible de créer le fichier PDF."));
                return;
            }

            painter.setRenderHint(QPainter::Antialiasing);
            int y = 120;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#3b82f6"));
            painter.drawRoundedRect(60, 60, 2200, 180, 14, 14);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
            painter.drawText(100, 175, QStringLiteral("Détails Publication"));

            painter.setPen(QColor("#1e293b"));
            painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
            auto drawField = [&](const QString &label, const QString &value) {
                painter.drawText(80, y, label);
                painter.setFont(QFont("Segoe UI", 12));
                painter.drawText(420, y, value.isEmpty() ? QStringLiteral("—") : value);
                painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
                y += 90;
            };

            y = 330;
            drawField(QStringLiteral("Titre :"), titre);
            drawField(QStringLiteral("Auteur(s) :"), auteur);
            drawField(QStringLiteral("Date publication :"), date);
            drawField(QStringLiteral("Revue :"), revue);
            drawField(QStringLiteral("Statut :"), statut);
            drawField(QStringLiteral("DOI :"), doi);

            painter.setPen(QColor("#94a3b8"));
            painter.setFont(QFont("Segoe UI", 9));
            painter.drawText(80, 3300, QStringLiteral("Exporté depuis SmartPub"));
            painter.end();

            QMessageBox::information(this, QStringLiteral("Export"), QStringLiteral("Export PDF réussi !"));
        });

        footerLayout->addWidget(btnExport);
        footerLayout->addStretch();

        QPushButton *btnClose = new QPushButton(QStringLiteral("Fermer"));
        btnClose->setCursor(Qt::PointingHandCursor);
        btnClose->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 10px;
                padding: 0 30px;
                font-size: 14px;
                font-weight: 600;
                min-height: 42px;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
        connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);
        footerLayout->addWidget(btnClose);
        mainLayout->addWidget(footerFrame);
        dialog->exec();
    });
}

void SmartPub::SR_updateButtonStyles() {
    QString activeStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669);
        }
    )";

    QString inactiveStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #64748b;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #f1f5f9;
            color: #334155;
        }
    )";

    QString deleteStyle = R"(
        QPushButton {
            background-color: transparent;
            color: #ef4444;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #fef2f2;
            color: #dc2626;
        }
    )";

    if (ui->SR_stackedWidget->currentIndex() == 0) {
        ui->SR_btnVueListe->setStyleSheet(activeStyle);
        ui->SR_btnVueListe->setChecked(true);
        ui->SR_btnAjouter->setStyleSheet(inactiveStyle);
        ui->SR_btnAjouter->setChecked(false);
    } else if (ui->SR_stackedWidget->currentIndex() == 1) {
        ui->SR_btnVueListe->setStyleSheet(inactiveStyle);
        ui->SR_btnVueListe->setChecked(false);
        ui->SR_btnAjouter->setStyleSheet(activeStyle);
        ui->SR_btnAjouter->setChecked(true);
    }
}

void SmartPub::SR_loadSampleData() {
    QSqlDatabase db = Connection::instance()->getDatabase();
    ui->SR_tablePublications->setColumnWidth(5, 135);
    ui->SR_tablePublications->setRowCount(0);
    ui->SR_comboBoxStatut->setItemData(0, QStringLiteral("publie"));
    ui->SR_comboBoxStatut->setItemData(1, QStringLiteral("soumis"));
    ui->SR_comboBoxStatut->setItemData(2, QStringLiteral("en_revision"));
    ui->SR_comboBoxStatut->setItemData(3, QStringLiteral("accepte"));
    ui->SR_comboBoxStatut->setItemData(4, QStringLiteral("rejete"));

    auto clearStats = [this]() {
        ui->SR_lblTotalNumber->setText(QStringLiteral("0"));
        ui->SR_lblThisYearNumber->setText(QStringLiteral("0"));
        ui->SR_lblPlanSNumber->setText(QStringLiteral("0"));
        ui->SR_lblStatPublie->setText(QStringLiteral("● Publié (0%)"));
        ui->SR_lblStatSoumis->setText(QStringLiteral("● Soumis (0%)"));
        ui->SR_lblStatRevision->setText(QStringLiteral("● En révision (0%)"));
        ui->SR_lblStatAccepte->setText(QStringLiteral("● Accepté (0%)"));
    };

    if (!db.isOpen()) {
        clearStats();
        return;
    }

    QSqlQuery query(db);
    const QString sql =
        QStringLiteral("SELECT ID_PUBLICATION, DOI, TITRE, AUTEUR, DATE_PUBLICATION, REVUE, STATUT "
                       "FROM PUBLICATION ORDER BY ID_PUBLICATION");
    if (!query.exec(sql)) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Impossible de charger les publications : ") + query.lastError().text());
        clearStats();
        return;
    }

    int row = 0;
    while (query.next()) {
        const int idPub = query.value(QStringLiteral("ID_PUBLICATION")).toInt();
        QString titre = query.value(QStringLiteral("TITRE")).toString();
        QString auteur = query.value(QStringLiteral("AUTEUR")).toString();
        QVariant dateVar = query.value(QStringLiteral("DATE_PUBLICATION"));
        QDate d = dateVar.toDate();
        if (!d.isValid() && dateVar.toDateTime().isValid())
            d = dateVar.toDateTime().date();
        QString dateStr = d.isValid() ? d.toString(QStringLiteral("yyyy-MM-dd")) : dateVar.toString();
        if (dateStr.length() > 10)
            dateStr = dateStr.left(10);
        QString revue = query.value(QStringLiteral("REVUE")).toString();
        QString statut = SR_statutDbToUi(query.value(QStringLiteral("STATUT")).toString());

        ui->SR_tablePublications->insertRow(row);
        QTableWidgetItem *titItem = new QTableWidgetItem(titre);
        titItem->setData(Qt::UserRole, idPub);
        titItem->setToolTip(query.value(QStringLiteral("DOI")).toString());
        ui->SR_tablePublications->setItem(row, 0, titItem);
        ui->SR_tablePublications->setItem(row, 1, new QTableWidgetItem(auteur));
        ui->SR_tablePublications->setItem(row, 2, new QTableWidgetItem(dateStr));
        ui->SR_tablePublications->setItem(row, 3, new QTableWidgetItem(revue));
        ui->SR_tablePublications->setItem(row, 4, new QTableWidgetItem(statut));
        SR_addButtonsToRow(row);
        row++;
    }
    ui->SR_tablePublications->resizeRowsToContents();
    handleSRApplyFilterListe();
    SR_refreshStatsForCurrentView();
}

void SmartPub::handleSRBtnVueListeClicked() {
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::handleSRBtnAjouterClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de publications.");
        return;
    }

    // Reset form state
    editingPublicationRow = -1;
    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
    ui->SR_dateEditPublication->setDate(QDate::currentDate());
    ui->SR_btnAjouterPublication->setText("Ajouter");

    ui->SR_stackedWidget->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::handleSRBtnRechercheClicked() {
    handleSRApplyFilterListe();
}

void SmartPub::handleSRBtnTriClicked() {
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 8px;
            min-width: 220px;
        }
        QMenu::item {
            padding: 12px 20px;
            border-radius: 8px;
            color: #334155;
            font-size: 14px;
            font-weight: 500;
        }
        QMenu::item:selected {
            background-color: #eff6ff;
            color: #3b82f6;
        }
        QMenu::separator {
            height: 1px;
            background-color: #e2e8f0;
            margin: 8px 16px;
        }
    )");

    menu->addAction("Trier par Titre", this, [this]() {
        const int targetColumn = 0; // Titre
        SR_sortOrder = (SR_sortColumn == targetColumn && SR_sortOrder == Qt::AscendingOrder)
                           ? Qt::DescendingOrder
                           : Qt::AscendingOrder;
        SR_sortColumn = targetColumn;
        ui->SR_tablePublications->sortItems(SR_sortColumn, SR_sortOrder);
        handleSRApplyFilterListe();
    });
    menu->addAction("Trier par Date de publication", this, [this]() {
        const int targetColumn = 2; // Date
        SR_sortOrder = (SR_sortColumn == targetColumn && SR_sortOrder == Qt::AscendingOrder)
                           ? Qt::DescendingOrder
                           : Qt::AscendingOrder;
        SR_sortColumn = targetColumn;
        ui->SR_tablePublications->sortItems(SR_sortColumn, SR_sortOrder);
        handleSRApplyFilterListe();
    });

    menu->exec(QCursor::pos());
}

void SmartPub::handleSRBtnExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter", QDir::homePath(), "CSV (*.csv)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "ID,Titre,Auteurs,Date Publication,Revue,Statut,DOI\n";
            for (int r = 0; r < ui->SR_tablePublications->rowCount(); ++r) {
                const QTableWidgetItem *titleItem = ui->SR_tablePublications->item(r, 0);
                const int id = titleItem ? titleItem->data(Qt::UserRole).toInt() : 0;
                const QString titre = titleItem ? titleItem->text() : QString();
                const QString auteur = ui->SR_tablePublications->item(r, 1) ? ui->SR_tablePublications->item(r, 1)->text() : QString();
                const QString date = ui->SR_tablePublications->item(r, 2) ? ui->SR_tablePublications->item(r, 2)->text() : QString();
                const QString revue = ui->SR_tablePublications->item(r, 3) ? ui->SR_tablePublications->item(r, 3)->text() : QString();
                const QString statut = ui->SR_tablePublications->item(r, 4) ? ui->SR_tablePublications->item(r, 4)->text() : QString();
                const QString doi = titleItem ? titleItem->toolTip() : QString();
                stream << id << "," << titre << "," << auteur << ","
                       << date << "," << revue << "," << statut << ","
                       << doi << "\n";
            }
            file.close();
            QMessageBox::information(this, "Export", "Export réussi !");
        }
    }
}

void SmartPub::handleSRBtnStatistiquesClicked() {
    QList<PublicationStatData> publications;
    publications.reserve(ui->SR_tablePublications->rowCount());
    for (int r = 0; r < ui->SR_tablePublications->rowCount(); ++r) {
        if (ui->SR_tablePublications->isRowHidden(r))
            continue;
        PublicationStatData p;
        p.titre = ui->SR_tablePublications->item(r, 0) ? ui->SR_tablePublications->item(r, 0)->text() : QString();
        p.auteur = ui->SR_tablePublications->item(r, 1) ? ui->SR_tablePublications->item(r, 1)->text() : QString();
        p.date = ui->SR_tablePublications->item(r, 2) ? ui->SR_tablePublications->item(r, 2)->text() : QString();
        p.revue = ui->SR_tablePublications->item(r, 3) ? ui->SR_tablePublications->item(r, 3)->text() : QString();
        p.statut = ui->SR_tablePublications->item(r, 4) ? ui->SR_tablePublications->item(r, 4)->text() : QString();
        publications.push_back(p);
    }
    PubStatistiquesDialog *dialog = new PubStatistiquesDialog(publications, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::handleSRBtnAjouterPublicationClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de publications.");
        return;
    }

    QString titre = ui->SR_lineEditTitre->text().trimmed();
    QString auteurs = ui->SR_lineEditAuteurs->text().trimmed();
    QString revue = ui->SR_lineEditRevue->text().trimmed();
    QString statut = ui->SR_comboBoxStatut->currentText();
    QDate datePub = ui->SR_dateEditPublication->date();
    QString dateStr = datePub.toString("yyyy-MM-dd");

    if (titre.isEmpty() || auteurs.isEmpty() || revue.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez remplir tous les champs obligatoires");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    QString statutDb = ui->SR_comboBoxStatut->currentData().toString();
    if (statutDb.isEmpty())
        statutDb = SR_statutUiToDb(statut);

    if (editingPublicationRow != -1) {
        QTableWidgetItem *titItem = ui->SR_tablePublications->item(editingPublicationRow, 0);
        if (!titItem)
            return;
        const int idPublication = titItem->data(Qt::UserRole).toInt();
        QSqlQuery query(db);
        query.prepare(
            QStringLiteral("UPDATE PUBLICATION SET TITRE = :titre, AUTEUR = :auteur, "
                           "DATE_PUBLICATION = TO_DATE(:date_pub, 'YYYY-MM-DD'), REVUE = :revue, STATUT = :statut "
                           "WHERE ID_PUBLICATION = :id"));
        query.bindValue(QStringLiteral(":titre"), titre);
        query.bindValue(QStringLiteral(":auteur"), auteurs);
        query.bindValue(QStringLiteral(":date_pub"), dateStr);
        query.bindValue(QStringLiteral(":revue"), revue);
        query.bindValue(QStringLiteral(":statut"), statutDb);
        query.bindValue(QStringLiteral(":id"), idPublication);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de la modification : ") + query.lastError().text());
            return;
        }
        SR_loadSampleData();
        QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Publication modifiée avec succès"));
        editingPublicationRow = -1;
        ui->SR_btnAjouterPublication->setText(QStringLiteral("Ajouter"));
    } else {
        const QString doi =
            QStringLiteral("10.1000/smartpub/%1").arg(QDateTime::currentMSecsSinceEpoch());
        QSqlQuery query(db);
        query.prepare(
            QStringLiteral("INSERT INTO PUBLICATION (DOI, TITRE, AUTEUR, DATE_PUBLICATION, REVUE, STATUT) "
                           "VALUES (:doi, :titre, :auteur, TO_DATE(:date_pub, 'YYYY-MM-DD'), :revue, :statut)"));
        query.bindValue(QStringLiteral(":doi"), doi);
        query.bindValue(QStringLiteral(":titre"), titre);
        query.bindValue(QStringLiteral(":auteur"), auteurs);
        query.bindValue(QStringLiteral(":date_pub"), dateStr);
        query.bindValue(QStringLiteral(":revue"), revue);
        query.bindValue(QStringLiteral(":statut"), statutDb);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de l'ajout : ") + query.lastError().text());
            return;
        }
        SR_loadSampleData();
        QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Publication ajoutée avec succès"));
    }

    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
    ui->SR_lineEditTitre->clear();
    ui->SR_lineEditAuteurs->clear();
    ui->SR_lineEditRevue->clear();
    ui->SR_dateEditPublication->setDate(QDate::currentDate());
}

void SmartPub::handleSRBtnAnnulerAjoutClicked() {
    editingPublicationRow = -1;
    ui->SR_btnAjouterPublication->setText("Ajouter");
    ui->SR_stackedWidget->setCurrentIndex(0);
    SR_updateButtonStyles();
}

void SmartPub::handleSRApplyFilterListe() {
    const QString searchText = ui->SR_lineEditRecherche->text().trimmed();
    QString titreFilter = SR_filterTitre->text().trimmed();
    QString auteurFilter = SR_filterAuteur->text().trimmed();
    QString statutFilter = SR_filterStatut->currentIndex() <= 0 ? QString() : SR_filterStatut->currentText();

    for (int r = 0; r < ui->SR_tablePublications->rowCount(); r++) {
        bool show = true;
        if (show && !titreFilter.isEmpty()) {
            QTableWidgetItem *it = ui->SR_tablePublications->item(r, 0);
            show = it && it->text().contains(titreFilter, Qt::CaseInsensitive);
        }
        if (show && !auteurFilter.isEmpty()) {
            QTableWidgetItem *it = ui->SR_tablePublications->item(r, 1);
            show = it && it->text().contains(auteurFilter, Qt::CaseInsensitive);
        }
        if (show && !statutFilter.isEmpty()) {
            QTableWidgetItem *it = ui->SR_tablePublications->item(r, 4);
            show = it && it->text().trimmed().compare(statutFilter, Qt::CaseInsensitive) == 0;
        }
        if (show && !searchText.isEmpty()) {
            const QString titre = ui->SR_tablePublications->item(r, 0) ? ui->SR_tablePublications->item(r, 0)->text() : QString();
            const QString auteur = ui->SR_tablePublications->item(r, 1) ? ui->SR_tablePublications->item(r, 1)->text() : QString();
            const QString date = ui->SR_tablePublications->item(r, 2) ? ui->SR_tablePublications->item(r, 2)->text() : QString();
            const QString revue = ui->SR_tablePublications->item(r, 3) ? ui->SR_tablePublications->item(r, 3)->text() : QString();
            const QString statut = ui->SR_tablePublications->item(r, 4) ? ui->SR_tablePublications->item(r, 4)->text() : QString();
            const QString doi = ui->SR_tablePublications->item(r, 0) ? ui->SR_tablePublications->item(r, 0)->toolTip() : QString();
            const QString haystack = QStringLiteral("%1 %2 %3 %4 %5 %6")
                                         .arg(titre, auteur, revue, statut, date, doi);
            show = haystack.contains(searchText, Qt::CaseInsensitive);
        }
        ui->SR_tablePublications->setRowHidden(r, !show);
    }
    SR_refreshStatsForCurrentView();
}

void SmartPub::handleSRReinitFilterListe() {
    SR_filterTitre->clear();
    SR_filterAuteur->clear();
    SR_filterStatut->setCurrentIndex(0);
    ui->SR_lineEditRecherche->clear();
    for (int r = 0; r < ui->SR_tablePublications->rowCount(); r++)
        ui->SR_tablePublications->setRowHidden(r, false);
    SR_refreshStatsForCurrentView();
}

void SmartPub::SR_refreshStatsForCurrentView() {
    const int total = ui->SR_tablePublications->rowCount();
    int visibles = 0;
    int thisYear = QDate::currentDate().year();
    int countThisYear = 0;
    int publie = 0, soumis = 0, revision = 0, accepte = 0;

    for (int r = 0; r < total; ++r) {
        if (ui->SR_tablePublications->isRowHidden(r))
            continue;
        visibles++;
        const QString ds = ui->SR_tablePublications->item(r, 2) ? ui->SR_tablePublications->item(r, 2)->text() : QString();
        if (ds.length() >= 4 && ds.left(4).toInt() == thisYear)
            countThisYear++;

        const QString s = ui->SR_tablePublications->item(r, 4) ? ui->SR_tablePublications->item(r, 4)->text() : QString();
        if (s.contains(QStringLiteral("Publié"), Qt::CaseInsensitive))
            publie++;
        else if (s.contains(QStringLiteral("Soumis"), Qt::CaseInsensitive))
            soumis++;
        else if (s.contains(QStringLiteral("révision"), Qt::CaseInsensitive))
            revision++;
        else if (s.contains(QStringLiteral("Accepté"), Qt::CaseInsensitive))
            accepte++;
    }

    const int base = visibles > 0 ? visibles : 1;
    ui->SR_lblTotalNumber->setText(QString::number(visibles));
    ui->SR_lblThisYearNumber->setText(QString::number(countThisYear));
    ui->SR_lblPlanSNumber->setText(QString::number(total));
    ui->SR_lblStatPublie->setText(QStringLiteral("● Publié (%1%)").arg((publie * 100) / base));
    ui->SR_lblStatSoumis->setText(QStringLiteral("● Soumis (%1%)").arg((soumis * 100) / base));
    ui->SR_lblStatRevision->setText(QStringLiteral("● En révision (%1%)").arg((revision * 100) / base));
    ui->SR_lblStatAccepte->setText(QStringLiteral("● Accepté (%1%)").arg((accepte * 100) / base));
}

static int SR_rowFromActionButton(QTableWidget *table, QObject *sender) {
    QPushButton *btn = qobject_cast<QPushButton *>(sender);
    if (!btn) return -1;
    QWidget *cellWidget = btn->parentWidget();
    if (!cellWidget) return -1;
    for (int r = 0; r < table->rowCount(); r++) {
        if (table->cellWidget(r, 5) == cellWidget)
            return r;
    }
    return -1;
}

void SmartPub::handleSRModifierPublicationClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas modifier les publications.");
        return;
    }
    int row = SR_rowFromActionButton(ui->SR_tablePublications, sender());
    if (row < 0) return;
    QTableWidgetItem *titItem = ui->SR_tablePublications->item(row, 0);
    if (!titItem) return;
    QString titre = titItem->text();
    QString auteurs = ui->SR_tablePublications->item(row, 1) ? ui->SR_tablePublications->item(row, 1)->text() : QString();
    QString dateStr = ui->SR_tablePublications->item(row, 2) ? ui->SR_tablePublications->item(row, 2)->text() : QDate::currentDate().toString("yyyy-MM-dd");
    QString revue = ui->SR_tablePublications->item(row, 3) ? ui->SR_tablePublications->item(row, 3)->text() : QString();
    QString statut = ui->SR_tablePublications->item(row, 4) ? ui->SR_tablePublications->item(row, 4)->text() : QString();
    QDate datePub = QDate::fromString(dateStr.left(10), "yyyy-MM-dd");
    if (!datePub.isValid()) datePub = QDate::currentDate();

    editingPublicationRow = row;
    ui->SR_lineEditTitre->setText(titre);
    ui->SR_lineEditAuteurs->setText(auteurs);
    ui->SR_lineEditRevue->setText(revue);
    ui->SR_dateEditPublication->setDate(datePub);
    int idx = ui->SR_comboBoxStatut->findText(statut);
    if (idx >= 0) ui->SR_comboBoxStatut->setCurrentIndex(idx);
    else ui->SR_comboBoxStatut->setCurrentText(statut);
    ui->SR_btnAjouterPublication->setText("Enregistrer modification");
    ui->SR_stackedWidget->setCurrentIndex(1);
    SR_updateButtonStyles();
}

void SmartPub::handleSRSupprimerPublicationClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas supprimer les publications.");
        return;
    }
    int row = SR_rowFromActionButton(ui->SR_tablePublications, sender());
    if (row < 0) return;
    QTableWidgetItem *titItem = ui->SR_tablePublications->item(row, 0);
    if (!titItem) return;
    QString titre = titItem->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmer la suppression",
        "Êtes-vous sûr de vouloir supprimer la publication \"" + titre + "\" ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"), QStringLiteral("Connexion à la base de données impossible."));
        return;
    }
    const int idPublication = titItem->data(Qt::UserRole).toInt();
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM PUBLICATION WHERE ID_PUBLICATION = :id"));
    query.bindValue(QStringLiteral(":id"), idPublication);
    if (!query.exec()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"),
                              QStringLiteral("Échec de la suppression : ") + query.lastError().text());
        return;
    }
    editingPublicationRow = -1;
    ui->SR_btnAjouterPublication->setText(QStringLiteral("Ajouter"));
    SR_loadSampleData();
    QMessageBox::information(this, QStringLiteral("Succès"), QStringLiteral("Publication supprimée."));
}

void SmartPub::SR_addButtonsToRow(int row)
{
    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(buttonWidget);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    QPushButton *btnEdit = new QPushButton("✏️");
    btnEdit->setMinimumSize(32, 28);
    btnEdit->setMaximumSize(32, 28);
    btnEdit->setCursor(Qt::PointingHandCursor);
    btnEdit->setStyleSheet(
        "QPushButton {"
        "    background-color: #3b82f6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2563eb;"
        "}"
    );

    QPushButton *btnDelete = new QPushButton("🗑️");
    btnDelete->setMinimumSize(32, 28);
    btnDelete->setMaximumSize(32, 28);
    btnDelete->setCursor(Qt::PointingHandCursor);
    btnDelete->setStyleSheet(
        "QPushButton {"
        "    background-color: #ef4444;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #dc2626;"
        "}"
    );

    connect(btnEdit, &QPushButton::clicked, this, &SmartPub::on_SR_modifierPublication_clicked);
    connect(btnDelete, &QPushButton::clicked, this, &SmartPub::on_SR_supprimerPublication_clicked);

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    layout->addStretch();

    ui->SR_tablePublications->setCellWidget(row, 5, buttonWidget);
}

void SmartPub::handlePublicationsNavigation() {
    PublicationLoginDialog authDialog(this);
    if (authDialog.exec() != QDialog::Accepted) {
        QMessageBox::warning(this, QStringLiteral("Accès refusé"),
                             QStringLiteral("Authentification requise pour accéder au module Publications."));
        return;
    }
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    updateProfileName(1);
    SR_updateButtonStyles();
}

#include "calender.h"
#include "connection.h"

#include <QApplication>
#include <QDate>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QToolTip>
#include <QVBoxLayout>
#include <QVariant>

// ============================================================================
// Constructeur
// ============================================================================

CalendarDialog::CalendarDialog(QWidget *parent)
    : QDialog(parent)
    , m_dateActuelle(QDate::currentDate().year(), QDate::currentDate().month(), 1)
    , m_labelMois(nullptr)
    , m_gridJours(nullptr)
    , m_gridWidget(nullptr)
    , m_spinAnnee(nullptr)
{
    setWindowTitle("Calendrier des Événements");
    setMinimumSize(900, 650);
    resize(1000, 700);
    setAttribute(Qt::WA_DeleteOnClose);
    setStyleSheet("QDialog { background-color: #f8fafc; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── En-tête ──────────────────────────────────────────────────────────────
    QFrame *header = new QFrame();
    header->setFixedHeight(72);
    header->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        " stop:0 #1e40af, stop:1 #0ea5e9); border: none; }");

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel *iconLabel = new QLabel("📅");
    iconLabel->setStyleSheet("font-size: 28px;");

    QLabel *titleLabel = new QLabel("Calendrier des Événements");
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold; margin-left: 10px;");

    headerLayout->addWidget(iconLabel);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addWidget(header);

    // ── Barre de navigation mois/année ───────────────────────────────────────
    QFrame *navBar = new QFrame();
    navBar->setFixedHeight(60);
    navBar->setStyleSheet(
        "QFrame { background-color: #ffffff; border-bottom: 1px solid #e2e8f0; }");

    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(24, 8, 24, 8);
    navLayout->setSpacing(12);

    // Bouton "Aujourd'hui"
    QPushButton *btnAujourdhui = new QPushButton("Aujourd'hui");
    btnAujourdhui->setCursor(Qt::PointingHandCursor);
    btnAujourdhui->setStyleSheet(
        "QPushButton { background-color: #f1f5f9; color: #334155; border: 1px solid #cbd5e1;"
        " border-radius: 6px; padding: 6px 14px; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background-color: #e2e8f0; }");
    connect(btnAujourdhui, &QPushButton::clicked, this, [this]() {
        m_dateActuelle = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        m_spinAnnee->blockSignals(true);
        m_spinAnnee->setValue(m_dateActuelle.year());
        m_spinAnnee->blockSignals(false);
        construireCalendrier();
    });

    // Bouton mois précédent
    QPushButton *btnPrev = new QPushButton("‹");
    btnPrev->setFixedSize(36, 36);
    btnPrev->setCursor(Qt::PointingHandCursor);
    btnPrev->setStyleSheet(
        "QPushButton { background-color: #f1f5f9; color: #334155; border: 1px solid #cbd5e1;"
        " border-radius: 18px; font-size: 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #dbeafe; color: #1d4ed8; }");
    connect(btnPrev, &QPushButton::clicked, this, &CalendarDialog::moisPrecedent);

    // Label mois
    m_labelMois = new QLabel();
    m_labelMois->setAlignment(Qt::AlignCenter);
    m_labelMois->setMinimumWidth(180);
    m_labelMois->setStyleSheet("color: #1e293b; font-size: 16px; font-weight: bold;");

    // Bouton mois suivant
    QPushButton *btnNext = new QPushButton("›");
    btnNext->setFixedSize(36, 36);
    btnNext->setCursor(Qt::PointingHandCursor);
    btnNext->setStyleSheet(
        "QPushButton { background-color: #f1f5f9; color: #334155; border: 1px solid #cbd5e1;"
        " border-radius: 18px; font-size: 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #dbeafe; color: #1d4ed8; }");
    connect(btnNext, &QPushButton::clicked, this, &CalendarDialog::moisSuivant);

    // SpinBox année
    m_spinAnnee = new QSpinBox();
    m_spinAnnee->setRange(2000, 2100);
    m_spinAnnee->setValue(m_dateActuelle.year());
    m_spinAnnee->setFixedWidth(80);
    m_spinAnnee->setStyleSheet(
        "QSpinBox { border: 1px solid #cbd5e1; border-radius: 6px; padding: 4px 8px;"
        " font-size: 13px; color: #334155; background: #f8fafc; }"
        "QSpinBox::up-button, QSpinBox::down-button { width: 18px; }");
    connect(m_spinAnnee, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CalendarDialog::anneeChangee);

    navLayout->addWidget(btnAujourdhui);
    navLayout->addSpacing(8);
    navLayout->addWidget(btnPrev);
    navLayout->addWidget(m_labelMois);
    navLayout->addWidget(btnNext);
    navLayout->addStretch();
    navLayout->addWidget(new QLabel("Année :"));
    navLayout->addWidget(m_spinAnnee);
    mainLayout->addWidget(navBar);

    // ── En-têtes des jours de la semaine ─────────────────────────────────────
    QFrame *daysHeader = new QFrame();
    daysHeader->setFixedHeight(40);
    daysHeader->setStyleSheet("QFrame { background-color: #f1f5f9; border-bottom: 1px solid #e2e8f0; }");
    QGridLayout *daysHeaderLayout = new QGridLayout(daysHeader);
    daysHeaderLayout->setContentsMargins(8, 0, 8, 0);
    daysHeaderLayout->setSpacing(2);

    const QStringList joursSemaine = {"Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"};
    for (int i = 0; i < 7; ++i) {
        QLabel *lbl = new QLabel(joursSemaine[i]);
        lbl->setAlignment(Qt::AlignCenter);
        bool weekend = (i >= 5);
        lbl->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: 600;")
                               .arg(weekend ? "#ef4444" : "#64748b"));
        daysHeaderLayout->addWidget(lbl, 0, i);
        daysHeaderLayout->setColumnStretch(i, 1);
    }
    mainLayout->addWidget(daysHeader);

    // ── Zone scrollable pour la grille des jours ─────────────────────────────
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #f8fafc; border: none; }");

    m_gridWidget = new QWidget();
    m_gridWidget->setStyleSheet("background-color: #f8fafc;");
    m_gridJours = new QGridLayout(m_gridWidget);
    m_gridJours->setSpacing(4);
    m_gridJours->setContentsMargins(8, 8, 8, 8);
    for (int i = 0; i < 7; ++i)
        m_gridJours->setColumnStretch(i, 1);

    scrollArea->setWidget(m_gridWidget);
    mainLayout->addWidget(scrollArea, 1);

    // ── Légende + bouton Fermer ───────────────────────────────────────────────
    QFrame *footer = new QFrame();
    footer->setFixedHeight(52);
    footer->setStyleSheet(
        "QFrame { background-color: #ffffff; border-top: 1px solid #e2e8f0; }");
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(20, 8, 20, 8);

    // Légende
    auto makeLegend = [](const QString &color, const QString &text) -> QWidget* {
        QWidget *w = new QWidget();
        QHBoxLayout *l = new QHBoxLayout(w);
        l->setContentsMargins(0,0,0,0);
        l->setSpacing(6);
        QLabel *dot = new QLabel();
        dot->setFixedSize(12, 12);
        dot->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(color));
        QLabel *txt = new QLabel(text);
        txt->setStyleSheet("color: #64748b; font-size: 11px;");
        l->addWidget(dot);
        l->addWidget(txt);
        return w;
    };
    footerLayout->addWidget(makeLegend("#3b82f6", "Événement"));
    footerLayout->addWidget(makeLegend("#10b981", "Aujourd'hui"));
    footerLayout->addWidget(makeLegend("#f59e0b", "Week-end avec événement"));
    footerLayout->addStretch();

    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setFixedSize(100, 36);
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none;"
        " border-radius: 6px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    connect(btnFermer, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addWidget(btnFermer);
    mainLayout->addWidget(footer);

    // ── Chargement initial ────────────────────────────────────────────────────
    chargerEvenements();
    construireCalendrier();
}

// ============================================================================
// Chargement des événements depuis la BD
// ============================================================================

void CalendarDialog::chargerEvenements()
{
    m_evenements.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;

    QSqlQuery q(db);
    if (!q.exec("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT ORDER BY DATE_EVENEMENT"))
        return;

    while (q.next()) {
        EventData ev;
        ev.code = QString::number(q.value(0).toLongLong());
        ev.nom  = q.value(1).toString();
        ev.lieu = q.value(2).toString();

        QVariant dv = q.value(3);
        QDate d = dv.toDate();
        if (!d.isValid() && dv.toDateTime().isValid())
            d = dv.toDateTime().date();
        if (!d.isValid()) {
            QString s = dv.toString();
            if (s.contains("T")) s = s.left(10);
            d = QDate::fromString(s.left(10), "yyyy-MM-dd");
        }
        ev.date = d.isValid() ? d.toString("dd/MM/yyyy") : dv.toString();
        m_evenements.append(ev);
    }
}

// ============================================================================
// Construction de la grille du calendrier
// ============================================================================

void CalendarDialog::construireCalendrier()
{
    // Vider la grille existante
    QLayoutItem *item;
    while ((item = m_gridJours->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    mettreAJourTitre();

    const int annee = m_dateActuelle.year();
    const int mois  = m_dateActuelle.month();
    const QDate premierJour(annee, mois, 1);
    const int nbJours = premierJour.daysInMonth();
    const QDate aujourdhui = QDate::currentDate();

    // Qt::Monday = 1 … Qt::Sunday = 7 → colonne 0..6
    int premierCol = premierJour.dayOfWeek() - 1; // 0=Lun … 6=Dim

    // Construire un index rapide : date → liste d'événements
    QMap<QDate, QList<EventData>> evParDate;
    for (const EventData &ev : m_evenements) {
        QDate d = QDate::fromString(ev.date, "dd/MM/yyyy");
        if (d.isValid())
            evParDate[d].append(ev);
    }

    int col = premierCol;
    int row = 0;

    for (int jour = 1; jour <= nbJours; ++jour) {
        const QDate dateJour(annee, mois, jour);
        const bool estAujourdhui = (dateJour == aujourdhui);
        const bool estWeekend    = (col >= 5);
        const QList<EventData> &evsDuJour = evParDate.value(dateJour);
        const bool aEvenement = !evsDuJour.isEmpty();

        // Cellule du jour
        QFrame *cell = new QFrame();
        cell->setMinimumHeight(90);
        cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QString bgColor = "#ffffff";
        QString borderColor = "#e2e8f0";
        if (estAujourdhui) {
            bgColor = "#eff6ff";
            borderColor = "#3b82f6";
        } else if (estWeekend) {
            bgColor = "#fafafa";
        }

        cell->setStyleSheet(QString(
            "QFrame { background-color: %1; border: 1px solid %2; border-radius: 8px; }")
                                .arg(bgColor, borderColor));

        QVBoxLayout *cellLayout = new QVBoxLayout(cell);
        cellLayout->setContentsMargins(6, 6, 6, 4);
        cellLayout->setSpacing(3);

        // Numéro du jour
        QLabel *numLabel = new QLabel(QString::number(jour));
        numLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
        QString numStyle;
        if (estAujourdhui) {
            numStyle = "background-color: #3b82f6; color: white; border-radius: 12px;"
                       " padding: 2px 7px; font-size: 13px; font-weight: bold;";
        } else if (estWeekend) {
            numStyle = "color: #ef4444; font-size: 13px; font-weight: 600;";
        } else {
            numStyle = "color: #334155; font-size: 13px; font-weight: 600;";
        }
        numLabel->setStyleSheet(numStyle);
        cellLayout->addWidget(numLabel);

        // Événements du jour (max 3 affichés, puis "+N autres")
        int affichés = 0;
        for (const EventData &ev : evsDuJour) {
            if (affichés >= 3) break;

            QLabel *evLabel = new QLabel(ev.nom);
            evLabel->setWordWrap(false);
            evLabel->setMaximumWidth(cell->width() - 12);

            QString evColor = estWeekend ? "#f59e0b" : "#3b82f6";
            evLabel->setStyleSheet(QString(
                "QLabel { background-color: %1; color: white; border-radius: 4px;"
                " padding: 2px 6px; font-size: 10px; font-weight: 500; }")
                                       .arg(evColor));
            evLabel->setToolTip(QString("%1\n📍 %2\n📅 %3").arg(ev.nom, ev.lieu, ev.date));
            evLabel->setCursor(Qt::PointingHandCursor);
            cellLayout->addWidget(evLabel);
            ++affichés;
        }

        if (evsDuJour.size() > 3) {
            QLabel *plusLabel = new QLabel(QString("+%1 autres").arg(evsDuJour.size() - 3));
            plusLabel->setStyleSheet("color: #64748b; font-size: 10px; font-style: italic;");
            cellLayout->addWidget(plusLabel);
        }

        cellLayout->addStretch();
        m_gridJours->addWidget(cell, row, col);

        ++col;
        if (col == 7) {
            col = 0;
            ++row;
        }
    }

    // Remplir les cellules vides de la dernière ligne
    if (col > 0) {
        for (int c = col; c < 7; ++c) {
            QFrame *empty = new QFrame();
            empty->setMinimumHeight(90);
            empty->setStyleSheet(
                "QFrame { background-color: #f8fafc; border: 1px solid #f1f5f9; border-radius: 8px; }");
            m_gridJours->addWidget(empty, row, c);
        }
    }

    // Étirer les lignes uniformément
    for (int r = 0; r <= row; ++r)
        m_gridJours->setRowStretch(r, 1);
}

// ============================================================================
// Navigation
// ============================================================================

void CalendarDialog::moisPrecedent()
{
    m_dateActuelle = m_dateActuelle.addMonths(-1);
    m_spinAnnee->blockSignals(true);
    m_spinAnnee->setValue(m_dateActuelle.year());
    m_spinAnnee->blockSignals(false);
    chargerEvenements();
    construireCalendrier();
}

void CalendarDialog::moisSuivant()
{
    m_dateActuelle = m_dateActuelle.addMonths(1);
    m_spinAnnee->blockSignals(true);
    m_spinAnnee->setValue(m_dateActuelle.year());
    m_spinAnnee->blockSignals(false);
    chargerEvenements();
    construireCalendrier();
}

void CalendarDialog::anneeChangee(int annee)
{
    m_dateActuelle = QDate(annee, m_dateActuelle.month(), 1);
    chargerEvenements();
    construireCalendrier();
}

// ============================================================================
// Mise à jour du titre mois/année
// ============================================================================

void CalendarDialog::mettreAJourTitre()
{
    const QStringList moisNoms = {
        "Janvier","Février","Mars","Avril","Mai","Juin",
        "Juillet","Août","Septembre","Octobre","Novembre","Décembre"
    };
    m_labelMois->setText(
        moisNoms[m_dateActuelle.month() - 1] + "  " +
        QString::number(m_dateActuelle.year()));
}

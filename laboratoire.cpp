#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include <algorithm>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QMenu>
#include <QCursor>

void SmartPub::labSetupUI()
{
    // Créer la page laboratoires et l'ajouter au stackedWidgetModules (index 5)
    labPage = new QWidget();
    labPage->setObjectName("labPage");
    labPage->setStyleSheet("QWidget#labPage { background-color: #f8fafc; }");

    QVBoxLayout *pageLayout = new QVBoxLayout(labPage);
    pageLayout->setSpacing(0);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    // ---- HEADER ----
    QFrame *headerFrame = new QFrame();
    headerFrame->setObjectName("labHeaderFrame");
    headerFrame->setFixedHeight(90);
    headerFrame->setStyleSheet(
        "QFrame#labHeaderFrame {"
        "    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "    border: none;"
        "}");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(30, 0, 30, 0);

    QLabel *titleLabel = new QLabel("🧪  Gestion des Laboratoires");
    titleLabel->setStyleSheet("color:white;font-size:26px;font-weight:bold;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    labTotalLabel = new QLabel("0 laboratoires");
    labTotalLabel->setObjectName("labTotalLabel");
    labTotalLabel->setStyleSheet(
        "color:white;font-size:14px;background:rgba(255,255,255,0.2);"
        "border-radius:12px;padding:6px 16px;");
    headerLayout->addWidget(labTotalLabel);
    pageLayout->addWidget(headerFrame);

    // ---- TOOLBAR ----
    QFrame *toolbarFrame = new QFrame();
    toolbarFrame->setObjectName("labToolbarFrame");
    toolbarFrame->setFixedHeight(70);
    toolbarFrame->setStyleSheet(
        "QFrame#labToolbarFrame {"
        "    background-color:white;"
        "    border-bottom:1px solid #e2e8f0;"
        "}");
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarFrame);
    toolbarLayout->setContentsMargins(20, 0, 20, 0);
    toolbarLayout->setSpacing(10);

    // CRUD buttons
    labBtnAjouter = new QPushButton("➕  Ajouter");
    labBtnAjouter->setObjectName("labBtnAjouter");
    labBtnAjouter->setFixedHeight(44);
    labBtnAjouter->setCursor(Qt::PointingHandCursor);
    labBtnAjouter->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "color:white;border:none;border-radius:10px;padding:0 20px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #059669);}");
    toolbarLayout->addWidget(labBtnAjouter);

    labBtnModifier = new QPushButton("✏️  Modifier");
    labBtnModifier->setObjectName("labBtnModifier");
    labBtnModifier->setFixedHeight(44);
    labBtnModifier->setEnabled(false);
    labBtnModifier->setCursor(Qt::PointingHandCursor);
    labBtnModifier->setStyleSheet(
        "QPushButton{background-color:#f1f5f9;color:#475569;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 16px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#e2e8f0;}"
        "QPushButton:disabled{color:#cbd5e1;}");
    toolbarLayout->addWidget(labBtnModifier);

    labBtnSupprimer = new QPushButton("🗑️  Supprimer");
    labBtnSupprimer->setObjectName("labBtnSupprimer");
    labBtnSupprimer->setFixedHeight(44);
    labBtnSupprimer->setEnabled(false);
    labBtnSupprimer->setCursor(Qt::PointingHandCursor);
    labBtnSupprimer->setStyleSheet(
        "QPushButton{background-color:transparent;color:#ef4444;border:1.5px solid #fca5a5;"
        "border-radius:10px;padding:0 16px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#fef2f2;}"
        "QPushButton:disabled{color:#fca5a5;border-color:#fecaca;}");
    toolbarLayout->addWidget(labBtnSupprimer);

    // Séparateur
    QFrame *sep1 = new QFrame(); sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color:#e2e8f0;"); sep1->setFixedWidth(1);
    toolbarLayout->addWidget(sep1);

    // Boutons IA / Stats / Exporter / Trier
    QPushButton *btnStats = new QPushButton("📊  Statistiques");
    btnStats->setObjectName("labBtnStatistiques");
    btnStats->setFixedHeight(44);
    btnStats->setCursor(Qt::PointingHandCursor);
    btnStats->setStyleSheet(
        "QPushButton{background-color:white;color:#3b82f6;border:1.5px solid #bfdbfe;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#eff6ff;}");
    toolbarLayout->addWidget(btnStats);

    QPushButton *btnOptim = new QPushButton("🤝  Optimiseur");
    btnOptim->setObjectName("labBtnOptimiseur");
    btnOptim->setFixedHeight(44);
    btnOptim->setCursor(Qt::PointingHandCursor);
    btnOptim->setStyleSheet(
        "QPushButton{background-color:white;color:#10b981;border:1.5px solid #a7f3d0;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#ecfdf5;}");
    toolbarLayout->addWidget(btnOptim);

    QPushButton *btnPred = new QPushButton("🔮  Prédicteur");
    btnPred->setObjectName("labBtnPredicteur");
    btnPred->setFixedHeight(44);
    btnPred->setCursor(Qt::PointingHandCursor);
    btnPred->setStyleSheet(
        "QPushButton{background-color:white;color:#8b5cf6;border:1.5px solid #ddd6fe;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#f5f3ff;}");
    toolbarLayout->addWidget(btnPred);

    toolbarLayout->addStretch();

    // Barre de recherche
    labSearchEdit = new QLineEdit();
    labSearchEdit->setObjectName("labSearchEdit");
    labSearchEdit->setPlaceholderText("🔍  Rechercher un laboratoire…");
    labSearchEdit->setFixedHeight(44);
    labSearchEdit->setMinimumWidth(250);
    labSearchEdit->setStyleSheet(
        "QLineEdit{background-color:#f8fafc;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 14px;font-size:13px;color:#334155;}"
        "QLineEdit:focus{border-color:#3b82f6;background-color:white;}");
    toolbarLayout->addWidget(labSearchEdit);

    QPushButton *btnExporter = new QPushButton("📤  Exporter");
    btnExporter->setObjectName("labBtnExporter");
    btnExporter->setFixedHeight(44);
    btnExporter->setCursor(Qt::PointingHandCursor);
    btnExporter->setStyleSheet(
        "QPushButton{background-color:#f8fafc;color:#64748b;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:500;}"
        "QPushButton:hover{background-color:#f1f5f9;}");
    toolbarLayout->addWidget(btnExporter);

    QPushButton *btnTrier = new QPushButton("⇅  Trier");
    btnTrier->setObjectName("labBtnTrier");
    btnTrier->setFixedHeight(44);
    btnTrier->setCursor(Qt::PointingHandCursor);
    btnTrier->setStyleSheet(
        "QPushButton{background-color:#f8fafc;color:#64748b;border:1.5px solid #e2e8f0;"
        "border-radius:10px;padding:0 14px;font-size:13px;font-weight:500;}"
        "QPushButton:hover{background-color:#f1f5f9;}");
    toolbarLayout->addWidget(btnTrier);

    pageLayout->addWidget(toolbarFrame);

    // ---- BODY: Table + Formulaire côte à côte ----
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(20, 16, 20, 16);

    // TABLE
    labTable = new QTableWidget();
    labTable->setObjectName("labTable");
    labTable->setColumnCount(7);
    labTable->setHorizontalHeaderLabels({"ID", "Nom", "Thématique", "Budget (€)", "Capacité", "Statut", "Directeur"});
    labTable->horizontalHeader()->setStretchLastSection(true);
    labTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    labTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    labTable->setSelectionMode(QAbstractItemView::SingleSelection);
    labTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    labTable->setAlternatingRowColors(true);
    labTable->verticalHeader()->setDefaultSectionSize(52);
    labTable->verticalHeader()->setVisible(false);
    labTable->setColumnWidth(0, 55);
    labTable->setColumnWidth(2, 170);
    labTable->setColumnWidth(3, 115);
    labTable->setColumnWidth(4, 90);
    labTable->setColumnWidth(5, 130);
    labTable->setStyleSheet(R"(
        QTableWidget {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            gridline-color: #f1f5f9;
            font-size: 13px;
            color: #334155;
        }
        QTableWidget::item {
            padding: 8px 12px;
        }
        QTableWidget::item:selected {
            background-color: #eff6ff;
            color: #1d4ed8;
        }
        QHeaderView::section {
            background-color: #f8fafc;
            color: #64748b;
            font-weight: 600;
            font-size: 12px;
            padding: 10px 12px;
            border: none;
            border-bottom: 1px solid #e2e8f0;
        }
        QTableWidget::item:alternate {
            background-color: #f8fafc;
        }
    )");
    bodyLayout->addWidget(labTable, 1);

    // FORMULAIRE (caché par défaut)
    labFormFrame = new QFrame();
    labFormFrame->setObjectName("labFormFrame");
    labFormFrame->setFixedWidth(360);
    labFormFrame->setVisible(false);
    labFormFrame->setStyleSheet(
        "QFrame#labFormFrame {"
        "    background-color:white;"
        "    border:1px solid #e2e8f0;"
        "    border-radius:12px;"
        "    margin-left:16px;"
        "}");
    QVBoxLayout *formLayout = new QVBoxLayout(labFormFrame);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(12);

    QLabel *formTitle = new QLabel("📋  Laboratoire");
    formTitle->setObjectName("labFormTitle");
    formTitle->setStyleSheet("font-size:17px;font-weight:bold;color:#1e3a5f;margin-bottom:4px;");
    formLayout->addWidget(formTitle);

    QFrame *formSep = new QFrame(); formSep->setFrameShape(QFrame::HLine);
    formSep->setStyleSheet("color:#e2e8f0;"); formLayout->addWidget(formSep);

    auto makeLabel = [](const QString &txt) {
        QLabel *l = new QLabel(txt);
        l->setStyleSheet("font-size:12px;font-weight:600;color:#64748b;margin-top:4px;");
        return l;
    };
    auto makeInput = []() {
        QLineEdit *e = new QLineEdit();
        e->setFixedHeight(40);
        e->setStyleSheet(
            "QLineEdit{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
            "padding:0 12px;font-size:13px;color:#334155;}"
            "QLineEdit:focus{border-color:#3b82f6;background:white;}");
        return e;
    };

    formLayout->addWidget(makeLabel("Nom du laboratoire *"));
    labFormNom = makeInput();
    labFormNom->setPlaceholderText("Ex: Lab IA Avancée");
    formLayout->addWidget(labFormNom);

    formLayout->addWidget(makeLabel("Thématique *"));
    labFormThematique = new QComboBox();
    labFormThematique->addItems({"Intelligence Artificielle", "Biotechnologie",
        "Nanotechnologie", "Énergies Renouvelables", "Robotique",
        "Chimie", "Physique Quantique", "Sciences de Données", "Autre"});
    labFormThematique->setFixedHeight(40);
    labFormThematique->setStyleSheet(
        "QComboBox{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
        "padding:0 12px;font-size:13px;color:#334155;}"
        "QComboBox:focus{border-color:#3b82f6;background:white;}"
        "QComboBox::drop-down{border:none;width:26px;}");
    formLayout->addWidget(labFormThematique);

    formLayout->addWidget(makeLabel("Budget annuel (€)"));
    labFormBudget = makeInput();
    labFormBudget->setPlaceholderText("Ex: 250000");
    formLayout->addWidget(labFormBudget);

    formLayout->addWidget(makeLabel("Capacité (chercheurs)"));
    labFormCapacite = new QSpinBox();
    labFormCapacite->setRange(1, 500);
    labFormCapacite->setValue(10);
    labFormCapacite->setFixedHeight(40);
    labFormCapacite->setStyleSheet(
        "QSpinBox{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
        "padding:0 12px;font-size:13px;color:#334155;}"
        "QSpinBox:focus{border-color:#3b82f6;background:white;}");
    formLayout->addWidget(labFormCapacite);

    formLayout->addWidget(makeLabel("Statut *"));
    labFormStatut = new QComboBox();
    labFormStatut->addItems({"Actif", "En Construction", "En Rénovation", "Inactif"});
    labFormStatut->setFixedHeight(40);
    labFormStatut->setStyleSheet(
        "QComboBox{background:#f8fafc;border:1.5px solid #e2e8f0;border-radius:8px;"
        "padding:0 12px;font-size:13px;color:#334155;}"
        "QComboBox:focus{border-color:#3b82f6;background:white;}"
        "QComboBox::drop-down{border:none;width:26px;}");
    formLayout->addWidget(labFormStatut);

    formLayout->addWidget(makeLabel("Équipements (séparés par virgule)"));
    labFormEquipements = makeInput();
    labFormEquipements->setPlaceholderText("Ex: Microscope, Spectromètre…");
    formLayout->addWidget(labFormEquipements);

    formLayout->addWidget(makeLabel("Directeur de recherche"));
    labFormDirecteur = makeInput();
    labFormDirecteur->setPlaceholderText("Ex: Dr. Dupont");
    formLayout->addWidget(labFormDirecteur);

    formLayout->addStretch();

    QHBoxLayout *formBtnsLayout = new QHBoxLayout();
    formBtnsLayout->setSpacing(10);
    QPushButton *btnAnnulerForm = new QPushButton("Annuler");
    btnAnnulerForm->setObjectName("labBtnAnnulerForm");
    btnAnnulerForm->setFixedHeight(42);
    btnAnnulerForm->setCursor(Qt::PointingHandCursor);
    btnAnnulerForm->setStyleSheet(
        "QPushButton{background-color:#f1f5f9;color:#475569;border:1.5px solid #e2e8f0;"
        "border-radius:8px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background-color:#e2e8f0;}");
    QPushButton *btnConfirmerForm = new QPushButton("💾  Enregistrer");
    btnConfirmerForm->setObjectName("labBtnConfirmerForm");
    btnConfirmerForm->setFixedHeight(42);
    btnConfirmerForm->setCursor(Qt::PointingHandCursor);
    btnConfirmerForm->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "color:white;border:none;border-radius:8px;font-size:13px;font-weight:600;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #059669);}");
    formBtnsLayout->addWidget(btnAnnulerForm);
    formBtnsLayout->addWidget(btnConfirmerForm);
    formLayout->addLayout(formBtnsLayout);

    bodyLayout->addWidget(labFormFrame);
    pageLayout->addLayout(bodyLayout);

    // Ajouter la page au stackedWidgetModules (index 5)
    ui->stackedWidgetModules->addWidget(labPage);
}

void SmartPub::labConnectSignals()
{
    connect(labBtnAjouter, &QPushButton::clicked, this, &SmartPub::on_labBtnAjouter_clicked);
    connect(labBtnModifier, &QPushButton::clicked, this, &SmartPub::on_labBtnModifier_clicked);
    connect(labBtnSupprimer, &QPushButton::clicked, this, &SmartPub::on_labBtnSupprimer_clicked);
    connect(labSearchEdit, &QLineEdit::textChanged, this, &SmartPub::on_labSearchChanged);
    connect(labTable, &QTableWidget::itemSelectionChanged, this, &SmartPub::on_labTableSelectionChanged);

    QPushButton *btnStats     = labPage->findChild<QPushButton*>("labBtnStatistiques");
    QPushButton *btnOptim     = labPage->findChild<QPushButton*>("labBtnOptimiseur");
    QPushButton *btnPred      = labPage->findChild<QPushButton*>("labBtnPredicteur");
    QPushButton *btnExporter  = labPage->findChild<QPushButton*>("labBtnExporter");
    QPushButton *btnTrier     = labPage->findChild<QPushButton*>("labBtnTrier");
    QPushButton *btnConfirmer = labFormFrame->findChild<QPushButton*>("labBtnConfirmerForm");
    QPushButton *btnAnnuler   = labFormFrame->findChild<QPushButton*>("labBtnAnnulerForm");

    if (btnStats)    connect(btnStats,    &QPushButton::clicked, this, &SmartPub::on_labBtnStatistiques_clicked);
    if (btnOptim)    connect(btnOptim,    &QPushButton::clicked, this, &SmartPub::on_labBtnOptimiseur_clicked);
    if (btnPred)     connect(btnPred,     &QPushButton::clicked, this, &SmartPub::on_labBtnPredicteur_clicked);
    if (btnExporter) connect(btnExporter, &QPushButton::clicked, this, &SmartPub::on_labBtnExporter_clicked);
    if (btnTrier)    connect(btnTrier,    &QPushButton::clicked, this, &SmartPub::on_labBtnTrier_clicked);
    if (btnConfirmer)connect(btnConfirmer,&QPushButton::clicked, this, &SmartPub::on_labBtnConfirmerForm_clicked);
    if (btnAnnuler)  connect(btnAnnuler,  &QPushButton::clicked, this, &SmartPub::on_labBtnAnnulerForm_clicked);
}

// ---------- Chargement des données (DB + fallback mock) ----------
void SmartPub::labChargerDonnees()
{
    labDataMap.clear();
    labNextId = 1;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery q(db);
        if (q.exec("SELECT ID_LABORATOIRE, NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET FROM LABORATOIRE ORDER BY ID_LABORATOIRE")) {
            while (q.next()) {
                LaboratoryData lab;
                lab.id         = q.value("ID_LABORATOIRE").toInt();
                lab.nom        = q.value("NOM").toString();
                lab.thematique = q.value("THEMATIQUE").toString();
                lab.statut     = q.value("DISPONIBILITE").toString() == "disponible" ? "Actif" : "Inactif";
                lab.adresse    = q.value("ADRESSE").toString();
                lab.budget     = 0;
                lab.capacite   = 0;
                lab.equipements.clear();
                lab.directeur.clear();
                labDataMap.insert(lab.id, lab);
                if (lab.id >= labNextId) labNextId = lab.id + 1;
            }
            if (!labDataMap.isEmpty()) {
                labAfficherListe();
                return;
            }
        }
    }

    labAfficherListe();
}

void SmartPub::labAfficherListe()
{
    QList<LaboratoryData> all = labDataMap.values();
    labAfficherListe(all);
}

void SmartPub::labAfficherListe(const QList<LaboratoryData> &labs)
{
    labTable->setRowCount(0);
    for (const LaboratoryData &lab : labs) {
        int row = labTable->rowCount();
        labTable->insertRow(row);
        labTable->setItem(row, 0, new QTableWidgetItem(QString::number(lab.id)));
        labTable->setItem(row, 1, new QTableWidgetItem(lab.nom));
        labTable->setItem(row, 2, new QTableWidgetItem(lab.thematique));
        labTable->setItem(row, 3, new QTableWidgetItem(lab.budget > 0 ? QString("%1 €").arg(lab.budget, 0, 'f', 0) : "N/A"));
        labTable->setItem(row, 4, new QTableWidgetItem(lab.capacite > 0 ? QString::number(lab.capacite) : "N/A"));

        // Badge statut coloré
        QTableWidgetItem *statutItem = new QTableWidgetItem(lab.statut);
        if (lab.statut == "Actif")             statutItem->setForeground(QColor("#10b981"));
        else if (lab.statut == "En Construction") statutItem->setForeground(QColor("#f59e0b"));
        else if (lab.statut == "En Rénovation")   statutItem->setForeground(QColor("#3b82f6"));
        else                                       statutItem->setForeground(QColor("#ef4444"));
        labTable->setItem(row, 5, statutItem);

        labTable->setItem(row, 6, new QTableWidgetItem(lab.directeur.isEmpty() ? "—" : lab.directeur));

        // Stocker l'id dans UserRole
        labTable->item(row, 0)->setData(Qt::UserRole, lab.id);
    }

    if (labTotalLabel)
        labTotalLabel->setText(QString::number(labs.size()) + " laboratoire" + (labs.size() > 1 ? "s" : ""));
}

void SmartPub::labViderFormulaire()
{
    if (labFormNom)        labFormNom->clear();
    if (labFormThematique) labFormThematique->setCurrentIndex(0);
    if (labFormBudget)     labFormBudget->clear();
    if (labFormCapacite)   labFormCapacite->setValue(10);
    if (labFormStatut)     labFormStatut->setCurrentIndex(0);
    if (labFormEquipements)labFormEquipements->clear();
    if (labFormDirecteur)  labFormDirecteur->clear();
}

void SmartPub::labRemplirFormulaire(const LaboratoryData &lab)
{
    if (labFormNom)        labFormNom->setText(lab.nom);
    if (labFormThematique) labFormThematique->setCurrentText(lab.thematique);
    if (labFormBudget)     labFormBudget->setText(lab.budget > 0 ? QString::number(lab.budget, 'f', 2) : "");
    if (labFormCapacite)   labFormCapacite->setValue(lab.capacite > 0 ? lab.capacite : 1);
    if (labFormStatut)     labFormStatut->setCurrentText(lab.statut);
    if (labFormEquipements)labFormEquipements->setText(lab.equipements);
    if (labFormDirecteur)  labFormDirecteur->setText(lab.directeur);
}

LaboratoryData SmartPub::labGetFormData() const
{
    LaboratoryData lab;
    lab.nom        = labFormNom        ? labFormNom->text().trimmed()        : "";
    lab.thematique = labFormThematique ? labFormThematique->currentText()    : "";
    lab.budget     = labFormBudget     ? labFormBudget->text().toDouble()    : 0.0;
    lab.capacite   = labFormCapacite   ? labFormCapacite->value()            : 1;
    lab.statut     = labFormStatut     ? labFormStatut->currentText()        : "Actif";
    lab.equipements= labFormEquipements? labFormEquipements->text().trimmed(): "";
    lab.directeur  = labFormDirecteur  ? labFormDirecteur->text().trimmed()  : "";
    return lab;
}

bool SmartPub::labValiderFormulaire() const
{
    if (!labFormNom || labFormNom->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Validation", "Le nom du laboratoire est obligatoire.");
        return false;
    }
    if (labFormBudget && !labFormBudget->text().isEmpty()) {
        bool ok; double v = labFormBudget->text().toDouble(&ok);
        if (!ok || v < 0) {
            QMessageBox::warning(nullptr, "Validation", "Le budget doit être un nombre positif.");
            return false;
        }
    }
    return true;
}

void SmartPub::labMontrerFormulaire(bool isEdit)
{
    if (!labFormFrame) return;
    QLabel *title = labFormFrame->findChild<QLabel*>("labFormTitle");
    if (title) title->setText(isEdit ? "✏️  Modifier le laboratoire" : "➕  Nouveau laboratoire");
    labFormFrame->setVisible(true);
}

void SmartPub::labCacherFormulaire()
{
    if (labFormFrame) labFormFrame->setVisible(false);
}

void SmartPub::labSetTableRowBackground(QTableWidget *table, int row, const QColor &color)
{
    if (!table) return;
    for (int col = 0; col < table->columnCount(); ++col) {
        QTableWidgetItem *item = table->item(row, col);
        if (item) item->setBackground(color);
    }
}

// ---------- Slots ----------
void SmartPub::handleLabBtnAjouterClicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas ajouter de laboratoires.");
        return;
    }
    labEditingId = -1;
    labViderFormulaire();
    labMontrerFormulaire(false);
}

void SmartPub::handleLabBtnModifierClicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas modifier de laboratoires.");
        return;
    }
    int row = labTable->currentRow();
    if (row < 0) return;
    int id = labTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (!labDataMap.contains(id)) return;
    labEditingId = id;
    labRemplirFormulaire(labDataMap[id]);
    labMontrerFormulaire(true);
}

void SmartPub::handleLabBtnSupprimerClicked()
{
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé", "Les invités ne peuvent pas supprimer de laboratoires.");
        return;
    }
    int row = labTable->currentRow();
    if (row < 0) return;
    int id = labTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (!labDataMap.contains(id)) return;

    QString nom = labDataMap[id].nom;
    auto reply = QMessageBox::question(this, "Confirmation",
        QString("Supprimer le laboratoire\n« %1 » ?").arg(nom),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("DELETE FROM LABORATOIRE WHERE ID_LABORATOIRE = :id");
        q.bindValue(":id", id);
        q.exec();
    }

    labDataMap.remove(id);
    labCacherFormulaire();
    labAfficherListe();
    QMessageBox::information(this, "Succès", "Laboratoire supprimé avec succès !");
}

void SmartPub::handleLabBtnConfirmerFormClicked()
{
    if (!labValiderFormulaire()) return;

    LaboratoryData lab = labGetFormData();

    if (labEditingId == -1) {
        // AJOUT
        lab.id = labNextId;
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery q(db);
            q.prepare("INSERT INTO LABORATOIRE(NOM, THEMATIQUE, DISPONIBILITE, ADRESSE) "
                      "VALUES(:nom, :them, :dispo, :adr)");
            q.bindValue(":nom",  lab.nom);
            q.bindValue(":them", lab.thematique);
            q.bindValue(":dispo", lab.statut == "Actif" ? "disponible" : "indisponible");
            q.bindValue(":adr",  lab.adresse.isEmpty() ? QString() : lab.adresse);
            if (q.exec()) {
                // Récupérer l'id généré par la séquence
                QSqlQuery qid(db);
                if (qid.exec("SELECT SEQ_LABORATOIRE.CURRVAL FROM DUAL")) {
                    qid.next();
                    lab.id = qid.value(0).toInt();
                }
            }
        }
        if (lab.id == labNextId) labNextId++;
        labDataMap.insert(lab.id, lab);
        QMessageBox::information(this, "Succès", "Laboratoire ajouté avec succès !");
    } else {
        // MODIFICATION
        lab.id = labEditingId;
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery q(db);
            q.prepare("UPDATE LABORATOIRE SET NOM=:nom, THEMATIQUE=:them, DISPONIBILITE=:dispo, ADRESSE=:adr WHERE ID_LABORATOIRE=:id");
            q.bindValue(":nom",  lab.nom);
            q.bindValue(":them", lab.thematique);
            q.bindValue(":dispo", lab.statut == "Actif" ? "disponible" : "indisponible");
            q.bindValue(":adr",  lab.adresse.isEmpty() ? QString() : lab.adresse);
            q.bindValue(":id",   lab.id);
            q.exec();
        }
        labDataMap[lab.id] = lab;
        QMessageBox::information(this, "Succès", "Laboratoire modifié avec succès !");
    }

    labEditingId = -1;
    labCacherFormulaire();
    labAfficherListe();
}

void SmartPub::handleLabBtnAnnulerFormClicked()
{
    labEditingId = -1;
    labViderFormulaire();
    labCacherFormulaire();
}

void SmartPub::handleLabTableSelectionChanged()
{
    bool sel = !labTable->selectedItems().isEmpty();
    if (labBtnModifier)  labBtnModifier->setEnabled(sel);
    if (labBtnSupprimer) labBtnSupprimer->setEnabled(sel);
}

void SmartPub::handleLabSearchChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        labAfficherListe();
        return;
    }
    QList<LaboratoryData> filtered;
    for (const LaboratoryData &lab : labDataMap) {
        if (lab.nom.contains(text, Qt::CaseInsensitive)
         || lab.thematique.contains(text, Qt::CaseInsensitive)
         || lab.directeur.contains(text, Qt::CaseInsensitive)) {
            filtered.append(lab);
        }
    }
    labAfficherListe(filtered);
}

void SmartPub::handleLabBtnStatistiquesClicked()
{
    labMontrerStatistiques();
}

void SmartPub::handleLabBtnOptimiseurClicked()
{
    labOptimiseurCollab();
}

void SmartPub::handleLabBtnPredicteurClicked()
{
    labPredicteurBesoins();
}

void SmartPub::handleLabBtnExporterClicked()
{
    labExporter();
}

void SmartPub::handleLabBtnTrierClicked()
{
    labTrier();
}

// ---------- Statistiques ----------
void SmartPub::labMontrerStatistiques()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("📊  Statistiques des Laboratoires");
    dlg->setMinimumSize(900, 680);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog{background-color:#f8fafc;} QLabel{color:#334155;}");

    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 16);

    // Titre
    QLabel *title = new QLabel("📊  Statistiques Globales des Laboratoires");
    title->setStyleSheet("font-size:20px;font-weight:bold;color:#1e3a5f;");
    mainLayout->addWidget(title);

    // Calculs
    double totalBudget = 0; int totalCap = 0;
    QMap<QString, int> byThematic; QMap<QString, double> budgetByThematic;
    for (const LaboratoryData &lab : labDataMap) {
        totalBudget += lab.budget; totalCap += lab.capacite;
        byThematic[lab.thematique]++;
        budgetByThematic[lab.thematique] += lab.budget;
    }
    int count = labDataMap.size();
    double avgBudget = count > 0 ? totalBudget / count : 0;
    double avgCap    = count > 0 ? (double)totalCap / count : 0;

    // Cartes stats
    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(12);
    auto makeCard = [](const QString &title_, const QString &val, const QString &color) {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString("QFrame{background:%1;border-radius:12px;}").arg(color));
        QVBoxLayout *cl = new QVBoxLayout(card); cl->setContentsMargins(16,14,16,14); cl->setSpacing(4);
        QLabel *t = new QLabel(title_); t->setStyleSheet("color:white;font-size:12px;font-weight:600;");
        QLabel *v = new QLabel(val);    v->setStyleSheet("color:white;font-size:22px;font-weight:bold;");
        cl->addWidget(t); cl->addWidget(v); return card;
    };
    cardsLayout->addWidget(makeCard("Laboratoires",   QString::number(count), "#3b82f6"));
    cardsLayout->addWidget(makeCard("Budget Total",   QString("%1 €").arg(totalBudget, 0, 'f', 0), "#10b981"));
    cardsLayout->addWidget(makeCard("Capacité Totale",QString("%1 chercheurs").arg(totalCap), "#f59e0b"));
    cardsLayout->addWidget(makeCard("Budget Moyen",   QString("%1 €").arg(avgBudget, 0, 'f', 0), "#8b5cf6"));
    cardsLayout->addWidget(makeCard("Capacité Moy.",  QString("%1").arg(avgCap, 0, 'f', 1), "#ef4444"));
    mainLayout->addLayout(cardsLayout);

    // Graphiques
    QHBoxLayout *chartsLayout = new QHBoxLayout(); chartsLayout->setSpacing(16);

    // Pie chart thématiques
    QPieSeries *pie = new QPieSeries();
    for (auto it = byThematic.begin(); it != byThematic.end(); ++it) {
        QPieSlice *s = pie->append(it.key(), it.value());
        s->setLabelVisible(true);
        s->setLabel(QString("%1: %2").arg(it.key()).arg(it.value()));
    }
    QChart *pieChart = new QChart(); pieChart->addSeries(pie);
    pieChart->setTitle("Répartition par Thématique");
    pieChart->legend()->setAlignment(Qt::AlignBottom);
    pieChart->setBackgroundBrush(QBrush(Qt::white));
    QChartView *pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    pieView->setMinimumSize(380, 280);
    pieView->setStyleSheet("background:white;border-radius:12px;border:1px solid #e2e8f0;");
    chartsLayout->addWidget(pieView);

    // Bar chart budgets
    QBarSet *bset = new QBarSet("Budget (€)");
    QStringList cats;
    for (auto it = budgetByThematic.begin(); it != budgetByThematic.end(); ++it) {
        *bset << it.value(); cats << it.key();
    }
    QBarSeries *bar = new QBarSeries(); bar->append(bset);
    QChart *barChart = new QChart(); barChart->addSeries(bar);
    barChart->setTitle("Budget par Thématique"); barChart->setAnimationOptions(QChart::SeriesAnimations);
    barChart->setBackgroundBrush(QBrush(Qt::white));
    QBarCategoryAxis *axX = new QBarCategoryAxis(); axX->append(cats);
    barChart->addAxis(axX, Qt::AlignBottom); bar->attachAxis(axX);
    QValueAxis *axY = new QValueAxis(); axY->setTitleText("Budget (€)");
    barChart->addAxis(axY, Qt::AlignLeft); bar->attachAxis(axY);
    barChart->legend()->setVisible(false);
    QChartView *barView = new QChartView(barChart);
    barView->setRenderHint(QPainter::Antialiasing);
    barView->setMinimumSize(380, 280);
    barView->setStyleSheet("background:white;border-radius:12px;border:1px solid #e2e8f0;");
    chartsLayout->addWidget(barView);
    mainLayout->addLayout(chartsLayout);

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(42); closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #3b82f6,stop:1 #10b981);"
        "color:white;border:none;border-radius:8px;font-size:13px;font-weight:600;padding:0 24px;}"
        "QPushButton:hover{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #2563eb,stop:1 #059669);}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout(); btnRow->addStretch(); btnRow->addWidget(closeBtn);
    mainLayout->addLayout(btnRow);

    dlg->exec();
}

// ---------- Optimiseur de Collaboration ----------
void SmartPub::labOptimiseurCollab()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("🤝  Optimiseur de Collaboration");
    dlg->setMinimumSize(900, 600);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog{background-color:white;} QLabel{color:#1f2937;}");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(24, 24, 24, 16); layout->setSpacing(14);

    QLabel *title = new QLabel("🤝  Analyse des Synergies Inter-Laboratoires");
    title->setStyleSheet("font-size:18px;font-weight:bold;color:#1e3a5f;");
    layout->addWidget(title);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Laboratoire 1","Laboratoire 2","Synergie","Score","Économies","Recommandation"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setDefaultSectionSize(52);
    table->setStyleSheet("QTableWidget{color:#1f2937;background:white;gridline-color:#e5e7eb;border:1px solid #e2e8f0;border-radius:10px;}"
                         "QTableWidget::item{padding:8px;color:#1f2937;}"
                         "QHeaderView::section{background:#f3f4f6;color:#374151;padding:10px;border:none;border-bottom:1px solid #e5e7eb;font-weight:600;}");

    QList<LaboratoryData> labs = labDataMap.values();
    int synFound = 0; double totalSav = 0;

    for (int i = 0; i < labs.size(); i++) {
        for (int j = i + 1; j < labs.size(); j++) {
            const LaboratoryData &l1 = labs[i]; const LaboratoryData &l2 = labs[j];
            if (l1.thematique == l2.thematique && (l1.statut == "Actif" || l2.statut == "Actif")) {
                double sav = (l1.budget + l2.budget) * 0.15;
                totalSav += sav; synFound++;
                int row = table->rowCount(); table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(l1.nom));
                table->setItem(row, 1, new QTableWidgetItem(l2.nom));
                table->setItem(row, 2, new QTableWidgetItem("🎯 Thématique commune"));
                table->setItem(row, 3, new QTableWidgetItem("⭐⭐⭐⭐⭐"));
                table->setItem(row, 4, new QTableWidgetItem(QString("%1 €").arg(sav, 0, 'f', 0)));
                table->setItem(row, 5, new QTableWidgetItem("Partage de ressources, projets conjoints"));
                labSetTableRowBackground(table, row, QColor(220, 252, 231));
            }
        }
    }

    if (synFound == 0) {
        QLabel *noSyn = new QLabel("ℹ️  Aucune synergie détectée. Ajoutez plusieurs laboratoires avec des thématiques similaires.");
        noSyn->setWordWrap(true);
        noSyn->setStyleSheet("color:#6b7280;font-size:14px;padding:20px;background:#f9fafb;border-radius:8px;");
        layout->addWidget(noSyn);
    } else {
        layout->addWidget(table);
        QLabel *summary = new QLabel(QString(
            "<span style='color:#1e40af;font-weight:600;'>📊 %1 synergie(s) détectée(s)</span> — "
            "Économies potentielles : <span style='color:#059669;font-weight:600;'>%2 €</span>")
            .arg(synFound).arg(totalSav, 0, 'f', 0));
        summary->setTextFormat(Qt::RichText);
        summary->setStyleSheet("padding:10px;background:#eff6ff;border-radius:8px;font-size:13px;");
        layout->addWidget(summary);
    }

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(40); closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton{background:#6b7280;color:white;border:none;border-radius:8px;padding:0 20px;font-weight:600;}QPushButton:hover{background:#4b5563;}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout(); btnRow->addStretch(); btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
    dlg->exec();
}

// ---------- Prédicteur de Besoins ----------
void SmartPub::labPredicteurBesoins()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("🔮  Prédicteur de Besoins");
    dlg->setMinimumSize(920, 640);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet("QDialog{background-color:white;} QLabel{color:#1f2937;}");

    QVBoxLayout *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(24, 24, 24, 16); layout->setSpacing(14);

    QLabel *title = new QLabel("🔮  Prédicteur de Besoins & Alertes Intelligentes");
    title->setStyleSheet("font-size:18px;font-weight:bold;color:#1e3a5f;");
    layout->addWidget(title);

    QTableWidget *table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Laboratoire","Type d'Alerte","Priorité","Délai","Action Recommandée"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setDefaultSectionSize(56);
    table->setStyleSheet("QTableWidget{color:#1f2937;background:white;gridline-color:#e5e7eb;border:1px solid #e2e8f0;border-radius:10px;}"
                         "QTableWidget::item{padding:10px;color:#1f2937;}"
                         "QHeaderView::section{background:#f3f4f6;color:#374151;padding:10px;border:none;border-bottom:1px solid #e5e7eb;font-weight:600;}");

    int crit = 0, warn = 0;
    for (const LaboratoryData &lab : labDataMap) {
        if (lab.capacite > 22 && lab.statut == "Actif") {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("⚠️ Saturation capacité"));
            table->setItem(row,2,new QTableWidgetItem("🔴 CRITIQUE"));
            table->setItem(row,3,new QTableWidgetItem("2-3 mois"));
            table->setItem(row,4,new QTableWidgetItem("Augmenter la capacité de 20% ou créer une annexe"));
            labSetTableRowBackground(table, row, QColor(254, 226, 226)); crit++;
        } else if (lab.capacite > 18 && lab.statut == "Actif") {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("⚡ Capacité élevée"));
            table->setItem(row,2,new QTableWidgetItem("🟡 ATTENTION"));
            table->setItem(row,3,new QTableWidgetItem("4-6 mois"));
            table->setItem(row,4,new QTableWidgetItem("Planifier extension, optimiser l'espace"));
            labSetTableRowBackground(table, row, QColor(254, 243, 199)); warn++;
        }
        if (lab.budget > 300000) {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("💰 Budget élevé"));
            table->setItem(row,2,new QTableWidgetItem("🔵 INFO"));
            table->setItem(row,3,new QTableWidgetItem("Continu"));
            table->setItem(row,4,new QTableWidgetItem("Audit financier, recherche de synergies budgétaires"));
            labSetTableRowBackground(table, row, QColor(219, 234, 254));
        }
        if (lab.statut == "En Construction" || lab.statut == "En Rénovation") {
            int row = table->rowCount(); table->insertRow(row);
            table->setItem(row,0,new QTableWidgetItem(lab.nom));
            table->setItem(row,1,new QTableWidgetItem("🔧 Laboratoire indisponible"));
            table->setItem(row,2,new QTableWidgetItem("🟠 SUIVI"));
            table->setItem(row,3,new QTableWidgetItem("Variable"));
            table->setItem(row,4,new QTableWidgetItem("Suivre l'avancement, prévoir ouverture"));
            labSetTableRowBackground(table, row, QColor(255, 237, 213));
        }
    }

    if (table->rowCount() == 0) {
        QLabel *ok = new QLabel("✅  Aucune alerte détectée. Tous les laboratoires sont dans des conditions optimales !");
        ok->setWordWrap(true);
        ok->setStyleSheet("color:#059669;font-size:14px;padding:20px;background:#ecfdf5;border-radius:8px;");
        layout->addWidget(ok);
    } else {
        layout->addWidget(table);
        QString summary = QString("🔴 %1 critique(s)  —  🟡 %2 avertissement(s)").arg(crit).arg(warn);
        QLabel *sumLabel = new QLabel(summary);
        sumLabel->setStyleSheet("padding:10px;background:#fef9ec;border-radius:8px;font-size:13px;font-weight:600;color:#92400e;");
        layout->addWidget(sumLabel);
    }

    QPushButton *closeBtn = new QPushButton("Fermer");
    closeBtn->setFixedHeight(40); closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton{background:#6b7280;color:white;border:none;border-radius:8px;padding:0 20px;font-weight:600;}QPushButton:hover{background:#4b5563;}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    QHBoxLayout *btnRow = new QHBoxLayout(); btnRow->addStretch(); btnRow->addWidget(closeBtn);
    layout->addLayout(btnRow);
    dlg->exec();
}

// ---------- Export CSV ----------
void SmartPub::labExporter()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter les Laboratoires", "", "Fichiers CSV (*.csv)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier : " + fileName);
        return;
    }
    QTextStream out(&file);
    out << "ID,Nom,Thématique,Budget(€),Capacité,Statut,Directeur,Équipements\n";
    for (const LaboratoryData &lab : labDataMap) {
        out << lab.id << ","
            << "\"" << lab.nom << "\","
            << "\"" << lab.thematique << "\","
            << lab.budget << ","
            << lab.capacite << ","
            << "\"" << lab.statut << "\","
            << "\"" << lab.directeur << "\","
            << "\"" << lab.equipements << "\"\n";
    }
    file.close();
    QMessageBox::information(this, "Export Réussi", "Données exportées avec succès :\n" + fileName);
}

// ---------- Tri ----------
void SmartPub::labTrier()
{
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu{background:white;border:1px solid #e2e8f0;border-radius:10px;padding:6px;}
        QMenu::item{padding:10px 20px;border-radius:6px;color:#334155;font-size:13px;}
        QMenu::item:selected{background:#eff6ff;color:#1d4ed8;}
    )");
    menu->addAction("⬆️  Nom (A → Z)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.nom < b.nom; });
        labAfficherListe(sorted);
    });
    menu->addAction("⬇️  Nom (Z → A)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.nom > b.nom; });
        labAfficherListe(sorted);
    });
    menu->addAction("💰  Budget (croissant)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.budget < b.budget; });
        labAfficherListe(sorted);
    });
    menu->addAction("💰  Budget (décroissant)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.budget > b.budget; });
        labAfficherListe(sorted);
    });
    menu->addAction("🧪  Thématique (A → Z)", this, [this](){
        QList<LaboratoryData> sorted = labDataMap.values();
        std::sort(sorted.begin(), sorted.end(), [](const LaboratoryData &a, const LaboratoryData &b){ return a.thematique < b.thematique; });
        labAfficherListe(sorted);
    });
    menu->popup(QCursor::pos());
}

void SmartPub::handleLaboratoiresNavigation() {
    ui->stackedWidgetModules->setCurrentIndex(5);
    setActiveNavigationButton(5);
    updateProfileName(5);
    labAfficherListe();
}

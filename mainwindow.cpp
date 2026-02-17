#include "mainwindow.h"
#include "ui_laboratoires.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>

// Implémentation de Laboratory
Laboratory::Laboratory()
    : m_id(0), m_budget(0.0), m_capacity(0)
{
}

Laboratory::Laboratory(int id, const QString& name, const QString& thematic,
                       double budget, int capacity, const QStringList& equipment,
                       const QString& status, const QString& director)
    : m_id(id), m_name(name), m_thematic(thematic),
      m_budget(budget), m_capacity(capacity), m_equipment(equipment),
      m_status(status), m_director(director)
{
}

// Implémentation de MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::laboratoires)
    , m_nextId(1)
    , m_editingIndex(-1)
{
    // Create a central widget and setup UI on it
    QWidget* centralWidget = new QWidget(this);
    ui->setupUi(centralWidget);
    setCentralWidget(centralWidget);
    
    // Set window properties
    setWindowTitle("Smart Research - Gestion des Laboratoires");
    resize(1200, 700);
    
    // Initialize mock data
    initializeMockData();
    
    // Load data into table
    loadTableData();
    
    // Connect signals
    connect(ui->btnLaboratoires, &QPushButton::clicked, this, &MainWindow::onBtnLaboratoiresClicked);
    connect(ui->btnRecherche, &QPushButton::clicked, this, &MainWindow::onBtnRechercheClicked);
    connect(ui->btnTrier, &QPushButton::clicked, this, &MainWindow::onBtnTrierClicked);
    connect(ui->btnExporter, &QPushButton::clicked, this, &MainWindow::onBtnExporterClicked);
    connect(ui->btnStatistiques, &QPushButton::clicked, this, &MainWindow::onBtnStatistiquesClicked);
    
    connect(ui->tableLaboratoires, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(ui->tableLaboratoires, &QTableWidget::cellClicked, this, [this](int row, int col) {
        Q_UNUSED(row);
        Q_UNUSED(col);
        onTableSelectionChanged();
    });
    connect(ui->btnModifierTable, &QPushButton::clicked, this, &MainWindow::onBtnModifierTableClicked);
    connect(ui->btnSupprimerTable, &QPushButton::clicked, this, &MainWindow::onBtnSupprimerTableClicked);
    connect(ui->btnOptimiseurCollab, &QPushButton::clicked, this, &MainWindow::onBtnOptimiseurCollabClicked);
    connect(ui->btnPredicteurBesoins, &QPushButton::clicked, this, &MainWindow::onBtnPredicteurBesoinsClicked);
    
    connect(ui->btnAjouterLaboratoire, &QPushButton::clicked, this, &MainWindow::onBtnAjouterLaboratoireClicked);
    connect(ui->lineEditRecherche, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    // Set initial tab
    ui->tabWidget->setCurrentIndex(0);
    
    // Enable buttons initially for testing (will be controlled by selection)
    ui->btnModifierTable->setEnabled(false);
    ui->btnSupprimerTable->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeMockData()
{
    m_laboratories.clear();
    m_nextId = 1;
    
    // Add sample laboratories
    m_laboratories.append(Laboratory(m_nextId++, "Lab IA Avancée", "Intelligence Artificielle",
                                    250000.0, 25, {"Serveurs GPU", "Clusters de calcul"}, "Actif", "Dr. Martin Dubois"));
    
    m_laboratories.append(Laboratory(m_nextId++, "Lab Biotech Moléculaire", "Biotechnologie",
                                    180000.0, 15, {"Microscope électronique", "Séquenceur ADN"}, "Actif", "Dr. Sophie Laurent"));
    
    m_laboratories.append(Laboratory(m_nextId++, "Lab Nano-Matériaux", "Nanotechnologie",
                                    320000.0, 20, {"Microscope à force atomique", "Spectromètre"}, "Actif", "Dr. Pierre Chen"));
    
    m_laboratories.append(Laboratory(m_nextId++, "Lab Énergies Vertes", "Énergies Renouvelables",
                                    200000.0, 18, {"Panneaux solaires", "Éoliennes test"}, "En Construction", "Dr. Marie Rousseau"));
    
    m_laboratories.append(Laboratory(m_nextId++, "Lab Robotique Autonome", "Robotique",
                                    275000.0, 22, {"Bras robotiques", "Drones", "Capteurs"}, "Actif", "Dr. Ahmed Ben Ali"));
}

void MainWindow::loadTableData()
{
    loadTableData(m_laboratories);
}

void MainWindow::loadTableData(const QList<Laboratory>& labs)
{
    ui->tableLaboratoires->setRowCount(0);
    
    for (const Laboratory& lab : labs) {
        int row = ui->tableLaboratoires->rowCount();
        ui->tableLaboratoires->insertRow(row);
        
        ui->tableLaboratoires->setItem(row, 0, new QTableWidgetItem(QString::number(lab.getId())));
        ui->tableLaboratoires->setItem(row, 1, new QTableWidgetItem(lab.getName()));
        ui->tableLaboratoires->setItem(row, 2, new QTableWidgetItem(lab.getThematic()));
        ui->tableLaboratoires->setItem(row, 3, new QTableWidgetItem(QString::number(lab.getBudget(), 'f', 2) + " €"));
        ui->tableLaboratoires->setItem(row, 4, new QTableWidgetItem(QString::number(lab.getCapacity())));
        ui->tableLaboratoires->setItem(row, 5, new QTableWidgetItem(lab.getEquipment().join(", ")));
        ui->tableLaboratoires->setItem(row, 6, new QTableWidgetItem(lab.getStatus()));
    }
}

void MainWindow::clearForm()
{
    ui->lineEditNom->clear();
    ui->comboBoxThematique->setCurrentIndex(0);
    ui->lineEditBudget->clear();
    ui->spinBoxCapacite->setValue(1);
    ui->comboBoxStatut->setCurrentIndex(0);
    ui->lineEditEquipements->clear();
}

void MainWindow::fillFormWithLab(const Laboratory& lab)
{
    ui->lineEditNom->setText(lab.getName());
    ui->comboBoxThematique->setCurrentText(lab.getThematic());
    ui->lineEditBudget->setText(QString::number(lab.getBudget(), 'f', 2));
    ui->spinBoxCapacite->setValue(lab.getCapacity());
    ui->comboBoxStatut->setCurrentText(lab.getStatus());
    ui->lineEditEquipements->setText(lab.getEquipment().join(", "));
}

Laboratory MainWindow::getLabFromForm()
{
    Laboratory lab;
    lab.setName(ui->lineEditNom->text());
    lab.setThematic(ui->comboBoxThematique->currentText());
    lab.setBudget(ui->lineEditBudget->text().toDouble());
    lab.setCapacity(ui->spinBoxCapacite->value());
    lab.setStatus(ui->comboBoxStatut->currentText());
    
    QString equipStr = ui->lineEditEquipements->text();
    QStringList equipment = equipStr.split(",", Qt::SkipEmptyParts);
    for (QString& eq : equipment) {
        eq = eq.trimmed();
    }
    lab.setEquipment(equipment);
    
    return lab;
}

// Navigation
void MainWindow::onBtnLaboratoiresClicked()
{
    ui->tabWidget->setCurrentIndex(0);
}

// Toolbar actions
void MainWindow::onBtnRechercheClicked()
{
    QString searchText = ui->lineEditRecherche->text();
    onSearchTextChanged(searchText);
}

void MainWindow::onBtnTrierClicked()
{
    // Sort by thematic
    QList<Laboratory> sorted = m_laboratories;
    std::sort(sorted.begin(), sorted.end(), [](const Laboratory& a, const Laboratory& b) {
        return a.getThematic() < b.getThematic();
    });
    
    loadTableData(sorted);
    QMessageBox::information(this, "Tri", "Laboratoires triés par thématique (ordre alphabétique)");
}

void MainWindow::onBtnExporterClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Exporter la liste", "", "CSV Files (*.csv);;PDF Files (*.pdf)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    if (fileName.endsWith(".csv")) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "ID,Nom,Thématique,Budget,Capacité,Équipements,Statut\n";
            
            for (const Laboratory& lab : m_laboratories) {
                out << lab.getId() << ","
                    << lab.getName() << ","
                    << lab.getThematic() << ","
                    << lab.getBudget() << ","
                    << lab.getCapacity() << ","
                    << lab.getEquipment().join(";") << ","
                    << lab.getStatus() << "\n";
            }
            
            file.close();
            QMessageBox::information(this, "Export", "Export CSV réussi !");
        }
    } else {
        QMessageBox::information(this, "Export PDF", "Export PDF non implémenté dans cette version");
    }
}

void MainWindow::onBtnStatistiquesClicked()
{
    showStatisticsDialog();
}

// Table actions
void MainWindow::onTableSelectionChanged()
{
    bool hasSelection = !ui->tableLaboratoires->selectedItems().isEmpty();
    ui->btnModifierTable->setEnabled(hasSelection);
    ui->btnSupprimerTable->setEnabled(hasSelection);
}

void MainWindow::onBtnModifierTableClicked()
{
    int currentRow = ui->tableLaboratoires->currentRow();
    if (currentRow < 0 || currentRow >= m_laboratories.size()) {
        return;
    }
    
    m_editingIndex = currentRow; // Sauvegarder l'index en cours d'édition
    Laboratory& lab = m_laboratories[currentRow];
    fillFormWithLab(lab);
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::onBtnSupprimerTableClicked()
{
    int currentRow = ui->tableLaboratoires->currentRow();
    if (currentRow < 0 || currentRow >= m_laboratories.size()) {
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation",
        "Êtes-vous sûr de vouloir supprimer ce laboratoire ?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_laboratories.removeAt(currentRow);
        loadTableData();
        QMessageBox::information(this, "Suppression", "Laboratoire supprimé avec succès !");
    }
}

void MainWindow::onBtnOptimiseurCollabClicked()
{
    showCollaborationDialog();
}

void MainWindow::onBtnPredicteurBesoinsClicked()
{
    showPredictionDialog();
}

// Form actions
void MainWindow::onBtnAjouterLaboratoireClicked()
{
    Laboratory lab = getLabFromForm();
    
    // Validation simple
    if (lab.getName().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Le nom du laboratoire est obligatoire");
        return;
    }
    
    if (lab.getBudget() < 0) {
        QMessageBox::warning(this, "Erreur", "Le budget ne peut pas être négatif");
        return;
    }
    
    // Check if we're editing or adding
    if (m_editingIndex >= 0 && m_editingIndex < m_laboratories.size()) {
        // Editing existing
        lab.setId(m_laboratories[m_editingIndex].getId());
        m_laboratories[m_editingIndex] = lab;
        QMessageBox::information(this, "Succès", "Laboratoire modifié avec succès !");
        m_editingIndex = -1; // Reset
    } else {
        // Adding new
        lab.setId(m_nextId++);
        m_laboratories.append(lab);
        QMessageBox::information(this, "Succès", "Laboratoire ajouté avec succès !");
    }
    
    loadTableData();
    clearForm();
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::onBtnAnnulerClicked()
{
    m_editingIndex = -1; // Reset
    clearForm();
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        loadTableData();
        return;
    }
    
    QList<Laboratory> filtered;
    for (const Laboratory& lab : m_laboratories) {
        if (lab.getName().contains(text, Qt::CaseInsensitive)) {
            filtered.append(lab);
        }
    }
    
    loadTableData(filtered);
}

void MainWindow::showStatisticsDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques des Laboratoires");
    dialog->setMinimumSize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // Calculate statistics
    double totalBudget = 0;
    int totalCapacity = 0;
    QMap<QString, int> byThematic;
    
    for (const Laboratory& lab : m_laboratories) {
        totalBudget += lab.getBudget();
        totalCapacity += lab.getCapacity();
        byThematic[lab.getThematic()]++;
    }
    
    double avgBudget = m_laboratories.isEmpty() ? 0 : totalBudget / m_laboratories.size();
    double avgCapacity = m_laboratories.isEmpty() ? 0 : (double)totalCapacity / m_laboratories.size();
    
    // Create labels
    QLabel* titleLabel = new QLabel("<h2>📊 Statistiques Globales</h2>");
    layout->addWidget(titleLabel);
    
    QLabel* countLabel = new QLabel(QString("<b>Nombre total de laboratoires:</b> %1").arg(m_laboratories.size()));
    layout->addWidget(countLabel);
    
    QLabel* budgetLabel = new QLabel(QString("<b>Budget total:</b> %1 €").arg(totalBudget, 0, 'f', 2));
    layout->addWidget(budgetLabel);
    
    QLabel* capacityLabel = new QLabel(QString("<b>Capacité totale:</b> %1 chercheurs").arg(totalCapacity));
    layout->addWidget(capacityLabel);
    
    QLabel* avgBudgetLabel = new QLabel(QString("<b>Budget moyen:</b> %1 €").arg(avgBudget, 0, 'f', 2));
    layout->addWidget(avgBudgetLabel);
    
    QLabel* avgCapacityLabel = new QLabel(QString("<b>Capacité moyenne:</b> %1 chercheurs").arg(avgCapacity, 0, 'f', 1));
    layout->addWidget(avgCapacityLabel);
    
    // By thematic
    QLabel* thematicTitle = new QLabel("<h3>Répartition par Thématique</h3>");
    layout->addWidget(thematicTitle);
    
    for (auto it = byThematic.begin(); it != byThematic.end(); ++it) {
        QLabel* thematicLabel = new QLabel(QString("• <b>%1:</b> %2 laboratoire(s)").arg(it.key()).arg(it.value()));
        layout->addWidget(thematicLabel);
    }
    
    layout->addStretch();
    
    QPushButton* closeBtn = new QPushButton("Fermer");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog->exec();
    delete dialog;
}

void MainWindow::showCollaborationDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Optimiseur de Collaboration");
    dialog->setMinimumSize(700, 500);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QLabel* titleLabel = new QLabel("<h2>🤝 Synergies Détectées</h2>");
    layout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel("Analyse des opportunités de collaboration entre laboratoires:");
    layout->addWidget(descLabel);
    
    // Create table for synergies
    QTableWidget* table = new QTableWidget();
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Laboratoire 1", "Laboratoire 2", "Type de Synergie", "Économies", "Potentiel"});
    table->horizontalHeader()->setStretchLastSection(true);
    
    // Detect synergies
    int synergiesFound = 0;
    for (int i = 0; i < m_laboratories.size(); i++) {
        for (int j = i + 1; j < m_laboratories.size(); j++) {
            const Laboratory& lab1 = m_laboratories[i];
            const Laboratory& lab2 = m_laboratories[j];
            
            // Check for common thematic
            if (lab1.getThematic() == lab2.getThematic()) {
                int row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(lab1.getName()));
                table->setItem(row, 1, new QTableWidgetItem(lab2.getName()));
                table->setItem(row, 2, new QTableWidgetItem("Thématique commune"));
                table->setItem(row, 3, new QTableWidgetItem("15%"));
                table->setItem(row, 4, new QTableWidgetItem("⭐⭐⭐⭐"));
                synergiesFound++;
            }
            
            // Check for complementary equipment
            QSet<QString> eq1 = QSet<QString>(lab1.getEquipment().begin(), lab1.getEquipment().end());
            QSet<QString> eq2 = QSet<QString>(lab2.getEquipment().begin(), lab2.getEquipment().end());
            QSet<QString> intersection = eq1;
            intersection.intersect(eq2);
            QSet<QString> unionSet = eq1;
            unionSet.unite(eq2);
            
            if (!unionSet.isEmpty() && (double)intersection.size() / unionSet.size() < 0.3) {
                int row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(lab1.getName()));
                table->setItem(row, 1, new QTableWidgetItem(lab2.getName()));
                table->setItem(row, 2, new QTableWidgetItem("Équipements complémentaires"));
                table->setItem(row, 3, new QTableWidgetItem("10%"));
                table->setItem(row, 4, new QTableWidgetItem("⭐⭐⭐"));
                synergiesFound++;
            }
        }
    }
    
    if (synergiesFound == 0) {
        QLabel* noSynergies = new QLabel("<i>Aucune synergie détectée pour le moment.</i>");
        layout->addWidget(noSynergies);
    } else {
        layout->addWidget(table);
        QLabel* countLabel = new QLabel(QString("<b>Total: %1 synergie(s) détectée(s)</b>").arg(synergiesFound));
        layout->addWidget(countLabel);
    }
    
    layout->addStretch();
    
    QPushButton* closeBtn = new QPushButton("Fermer");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog->exec();
    delete dialog;
}

void MainWindow::showPredictionDialog()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Prédicteur de Besoins");
    dialog->setMinimumSize(700, 500);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    QLabel* titleLabel = new QLabel("<h2>🔮 Prédictions et Alertes</h2>");
    layout->addWidget(titleLabel);
    
    QLabel* descLabel = new QLabel("Analyse prédictive basée sur les tendances historiques:");
    layout->addWidget(descLabel);
    
    // Create table for alerts
    QTableWidget* table = new QTableWidget();
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"Laboratoire", "Type d'Alerte", "Délai", "Recommandation"});
    table->horizontalHeader()->setStretchLastSection(true);
    
    // Generate mock predictions
    int alertsGenerated = 0;
    for (const Laboratory& lab : m_laboratories) {
        // Simulate capacity alert for labs with high capacity
        if (lab.getCapacity() > 20) {
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(lab.getName()));
            table->setItem(row, 1, new QTableWidgetItem("⚠️ Capacité critique"));
            table->setItem(row, 2, new QTableWidgetItem("4 mois"));
            table->setItem(row, 3, new QTableWidgetItem("Augmenter la capacité de 5 unités"));
            alertsGenerated++;
        }
        
        // Simulate budget alert for labs with high budget
        if (lab.getBudget() > 250000) {
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(lab.getName()));
            table->setItem(row, 1, new QTableWidgetItem("💰 Budget critique"));
            table->setItem(row, 2, new QTableWidgetItem("6 mois"));
            table->setItem(row, 3, new QTableWidgetItem("Augmenter le budget de 15%"));
            alertsGenerated++;
        }
    }
    
    if (alertsGenerated == 0) {
        QLabel* noAlerts = new QLabel("<i>Aucune alerte préventive pour le moment. Tous les laboratoires sont dans des conditions optimales.</i>");
        layout->addWidget(noAlerts);
    } else {
        layout->addWidget(table);
        QLabel* countLabel = new QLabel(QString("<b>Total: %1 alerte(s) générée(s)</b>").arg(alertsGenerated));
        layout->addWidget(countLabel);
    }
    
    layout->addStretch();
    
    QPushButton* closeBtn = new QPushButton("Fermer");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dialog->exec();
    delete dialog;
}

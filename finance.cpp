#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"

static QString finTypeDbToUi(const QString &db) {
    const QString d = db.trimmed().toLower();
    if (d == QLatin1String("subvention"))
        return QStringLiteral("Subvention");
    if (d == QLatin1String("achat"))
        return QStringLiteral("Achat");
    if (d == QLatin1String("salaire"))
        return QStringLiteral("Salaire");
    if (d == QLatin1String("remboursement"))
        return QStringLiteral("Remboursement");
    if (d == QLatin1String("autre"))
        return QStringLiteral("Autre");
    // Capitaliser la premiere lettre si valeur inconnue
    if (!db.isEmpty()) {
        QString s = db.trimmed();
        s[0] = s[0].toUpper();
        return s;
    }
    return db;
}

static QString finTypeUiToDb(const QString &ui) {
    const QString u = ui.trimmed();
    if (u.compare(QStringLiteral("Subvention"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("subvention");
    if (u.compare(QStringLiteral("Achat"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("achat");
    if (u.compare(QStringLiteral("Salaire"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("salaire");
    if (u.compare(QStringLiteral("Remboursement"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("remboursement");
    if (u.compare(QStringLiteral("Autre"), Qt::CaseInsensitive) == 0)
        return QStringLiteral("autre");
    if (u.contains(QStringLiteral("Recette"), Qt::CaseInsensitive))
        return QStringLiteral("subvention");
    if (u.contains(QStringLiteral("Dépense"), Qt::CaseInsensitive) || u.contains(QStringLiteral("Depense"), Qt::CaseInsensitive))
        return QStringLiteral("achat");
    return QStringLiteral("autre");
}

// ============================================================================
// MODULE FINANCES
// ============================================================================

void SmartPub::finSetupUI() {
    ui->finStackedWidget->setCurrentIndex(0);
    finVueListeActive = true;

    // === Noms des colonnes du tableau ===
    ui->finTableTransactions->setHorizontalHeaderLabels(
        {"ID", "Projet", "Type de transaction", "Montant", "Date", "Categorie", "Statut", ""});

    // === Types de transactions ===
    ui->finComboBoxType->clear();
    ui->finComboBoxType->addItem(QStringLiteral("Subvention"), QStringLiteral("subvention"));
    ui->finComboBoxType->addItem(QStringLiteral("Achat"), QStringLiteral("achat"));
    ui->finComboBoxType->addItem(QStringLiteral("Salaire"), QStringLiteral("salaire"));
    ui->finComboBoxType->addItem(QStringLiteral("Remboursement"), QStringLiteral("remboursement"));
    ui->finComboBoxType->addItem(QStringLiteral("Autre"), QStringLiteral("autre"));

    finRemplirComboProjets();
}

void SmartPub::finRemplirComboProjets()
{
    ui->finComboBoxProjet->clear();
    ui->finComboBoxProjet->addItem(QStringLiteral("—"), QVariant(0));

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;

    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT CODE_PROJET, TITRE FROM PROJET ORDER BY CODE_PROJET"))) {
        while (q.next()) {
            const QString code = q.value(0).toString();   // CODE_PROJET est une QString
            const QString titre = q.value(1).toString();
            ui->finComboBoxProjet->addItem(QStringLiteral("%1 — %2").arg(code, titre), code);
        }
    }
}

void SmartPub::finChargerTransactionsDepuisOracle()
{
    finTransactionsMap.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        finAfficherListeTransactions();
        return;
    }

    QSqlQuery q(db);
    const QString sql =
        QStringLiteral("SELECT f.ID_TRANSACTION, f.MONTANT, f.TYPE_TRANS, f.CATEGORIE, f.DATE_TRANSACTION, "
                       "f.STATUT, f.DESCRIPTION, f.ID_PROJET, p.TITRE "
                       "FROM FINANCE f LEFT JOIN PROJET p ON p.CODE_PROJET = f.ID_PROJET "
                       "ORDER BY f.ID_TRANSACTION");
    if (!q.exec(sql)) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Impossible de charger les transactions : ") + q.lastError().text());
        finAfficherListeTransactions();
        return;
    }

    while (q.next()) {
        TransactionData t;
        t.id = q.value(0).toInt();
        t.montant = q.value(1).toDouble();
        t.type = finTypeDbToUi(q.value(2).toString());
        t.categorie = q.value(3).toString();
        QVariant dv = q.value(4);
        QDate d = dv.toDate();
        if (!d.isValid() && dv.toDateTime().isValid())
            d = dv.toDateTime().date();
        t.date = d.isValid() ? d.toString(QStringLiteral("dd/MM/yyyy")) : dv.toString();
        t.statut = q.value(5).toString();
        t.description = q.value(6).toString();
        QVariant pv = q.value(7);
        t.idProjet = pv.isNull() ? QString() : pv.toString();
        const QString titre = q.value(8).toString();
        t.projet = titre.isEmpty() && !t.idProjet.isEmpty() ? QStringLiteral("Projet #%1").arg(t.idProjet) : titre;
        finTransactionsMap.insert(t.id, t);
    }
    finAfficherListeTransactions();
}

void SmartPub::finConnectSignals() {
    connect(ui->finBtnVueListe, &QPushButton::clicked, this,
            &SmartPub::on_finBtnVueListe_clicked);
    connect(ui->finBtnAjouter, &QPushButton::clicked, this,
            &SmartPub::on_finBtnAjouter_clicked);
    connect(ui->finBtnRecherche, &QPushButton::clicked, this,
            &SmartPub::on_finBtnRecherche_clicked);
    connect(ui->finBtnTri, &QPushButton::clicked, this,
            &SmartPub::on_finBtnTri_clicked);
    connect(ui->finBtnExport, &QPushButton::clicked, this,
            &SmartPub::on_finBtnExport_clicked);
    connect(ui->finBtnStatistiques, &QPushButton::clicked, this,
            &SmartPub::on_finBtnStatistiques_clicked);
    connect(ui->finBtnAjouterTransaction, &QPushButton::clicked, this,
            &SmartPub::on_finBtnAjouterTransaction_clicked);
    connect(ui->finBtnAnnulerAjout, &QPushButton::clicked, this,
            &SmartPub::on_finBtnAnnulerAjout_clicked);
    connect(ui->finBtnModifierTable, &QPushButton::clicked, this,
            &SmartPub::on_finBtnModifierTransaction_clicked);
    connect(ui->finBtnSupprimerTable, &QPushButton::clicked, this,
            &SmartPub::on_finBtnSupprimerTransaction_clicked);
    connect(ui->finLineEditRecherche, &QLineEdit::textChanged, this,
            &SmartPub::on_finLineEditRecherche_textChanged);
}

// background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b9cff, stop:1
// #2dd4bf); background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2b8cef,
// stop:1 #1dc4af);
void SmartPub::finUpdateButtonStyles() {
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

    if (ui->finStackedWidget->currentIndex() == 0) {
        ui->finBtnVueListe->setStyleSheet(activeStyle);
        ui->finBtnVueListe->setChecked(true);
        ui->finBtnAjouter->setStyleSheet(inactiveStyle);
        ui->finBtnAjouter->setChecked(false);
    } else if (ui->finStackedWidget->currentIndex() == 1) {
        ui->finBtnVueListe->setStyleSheet(inactiveStyle);
        ui->finBtnVueListe->setChecked(false);
        ui->finBtnAjouter->setStyleSheet(activeStyle);
        ui->finBtnAjouter->setChecked(true);
    }
}

void SmartPub::finAjouterDonneesTest() {
    finChargerTransactionsDepuisOracle();
}

void SmartPub::finAfficherListeTransactions() {
    ui->finTableTransactions->setRowCount(0);
    QList<TransactionData> liste = finGetTransactionsFiltreesEtTriees();
    for (const TransactionData &data : liste) {
        finAjouterTransactionTable(data);
    }
}

QList<TransactionData> SmartPub::finGetTransactionsFiltreesEtTriees() const {
    QList<TransactionData> liste;
    QString search = ui->finLineEditRecherche->text().trimmed().toLower();

    for (auto it = finTransactionsMap.constBegin(); it != finTransactionsMap.constEnd(); ++it) {
        const TransactionData &t = it.value();
        if (!search.isEmpty()) {
            if (!t.projet.toLower().contains(search) &&
                !t.type.toLower().contains(search) &&
                !t.categorie.toLower().contains(search) &&
                !t.statut.toLower().contains(search) &&
                !t.description.toLower().contains(search) &&
                !QString::number(t.montant, 'f', 2).contains(search))
                continue;
        }
        liste.append(t);
    }

    std::sort(liste.begin(), liste.end(), [this](const TransactionData &a, const TransactionData &b) {
        bool less = false;
        switch (finTriColonne) {
        case 0: less = a.id < b.id; break;
        case 1: less = a.projet.compare(b.projet, Qt::CaseInsensitive) < 0; break;
        case 2: less = a.type.compare(b.type, Qt::CaseInsensitive) < 0; break;
        case 3: less = a.montant < b.montant; break;
        case 4: {
            QDate da = QDate::fromString(a.date, "dd/MM/yyyy");
            QDate db = QDate::fromString(b.date, "dd/MM/yyyy");
            less = da < db;
            break;
        }
        case 5: less = a.categorie.compare(b.categorie, Qt::CaseInsensitive) < 0; break;
        case 6: less = a.statut.compare(b.statut, Qt::CaseInsensitive) < 0; break;
        default: less = a.id < b.id;
        }
        return finTriOrdre == Qt::AscendingOrder ? less : !less;
    });

    return liste;
}

void SmartPub::finAjouterTransactionTable(const TransactionData &data) {
    int row = ui->finTableTransactions->rowCount();
    ui->finTableTransactions->insertRow(row);

    ui->finTableTransactions->setItem(
        row, 0, new QTableWidgetItem(QString::number(data.id)));
    ui->finTableTransactions->setItem(row, 1, new QTableWidgetItem(data.projet));
    ui->finTableTransactions->setItem(row, 2, new QTableWidgetItem(data.type));
    ui->finTableTransactions->setItem(
        row, 3,
        new QTableWidgetItem(QString::number(data.montant, 'f', 2) + " €"));
    ui->finTableTransactions->setItem(row, 4, new QTableWidgetItem(data.date));
    ui->finTableTransactions->setItem(row, 5,
                                      new QTableWidgetItem(data.categorie));
    // Colonne Statut avec couleur selon valeur
    QTableWidgetItem *statutItem = new QTableWidgetItem(data.statut);
    if (data.statut == "Validée" || data.statut == "validée")
        statutItem->setForeground(QColor("#16a34a"));
    else if (data.statut == "En attente")
        statutItem->setForeground(QColor("#d97706"));
    else if (data.statut == "Rejetée" || data.statut == "rejetée")
        statutItem->setForeground(QColor("#dc2626"));
    statutItem->setFont(QFont("", -1, QFont::Bold));
    ui->finTableTransactions->setItem(row, 6, statutItem);

    // Boutons d'action inline dans la colonne ACTIONS
    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(6, 4, 6, 4);
    actionsLayout->setSpacing(8);

    QPushButton *btnModifier = new QPushButton("✏ Modifier");
    btnModifier->setCursor(Qt::PointingHandCursor);
    btnModifier->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border-radius: 5px;"
        " padding: 4px 10px; font-size: 11px; font-weight: bold; border: none; }"
        "QPushButton:hover { background-color: #2563eb; }"
        "QPushButton:pressed { background-color: #1d4ed8; }");

    int transId = data.id;
    connect(btnModifier, &QPushButton::clicked, this, [this, transId]() {
        finTransactionSelectionnee = transId;
        if (finTransactionsMap.contains(transId)) {
            finRemplirComboProjets();
            finRemplirFormulaire(finTransactionsMap[transId]);
            ui->finFormTitle->setText("Modifier la transaction");
            ui->finFormSubtitle->setText("Modifiez les informations de la transaction");
            ui->finStackedWidget->setCurrentIndex(1);
            ui->finBtnAjouterTransaction->setText("💾 Enregistrer");
            finUpdateButtonStyles();
        }
    });

    QPushButton *btnSupprimer = new QPushButton("✕ Supprimer");
    btnSupprimer->setCursor(Qt::PointingHandCursor);
    btnSupprimer->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; border-radius: 5px;"
        " padding: 4px 10px; font-size: 11px; font-weight: bold; border: none; }"
        "QPushButton:hover { background-color: #dc2626; }"
        "QPushButton:pressed { background-color: #b91c1c; }");
    connect(btnSupprimer, &QPushButton::clicked, this, [this, transId]() {
        int rep = QMessageBox::question(this, "Confirmer la suppression",
            QString("Etes-vous sur de vouloir supprimer la transaction #%1 ?\nCette action est irreversible.").arg(transId),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (rep != QMessageBox::Yes) return;
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) return;
        QSqlQuery q(db);
        q.prepare(QStringLiteral("DELETE FROM FINANCE WHERE ID_TRANSACTION = :id"));
        q.bindValue(":id", transId);
        if (q.exec()) {
            QMessageBox::information(this, "Succès", "Transaction supprimée avec succès !");
            finChargerTransactionsDepuisOracle();
        } else {
            QMessageBox::critical(this, "Erreur", "Échec de la suppression : " + q.lastError().text());
        }
    });

    actionsLayout->addWidget(btnModifier);
    actionsLayout->addWidget(btnSupprimer);
    actionsLayout->addStretch();
    ui->finTableTransactions->setCellWidget(row, 7, actionsWidget);
}

void SmartPub::finViderFormulaire() {
    ui->finComboBoxProjet->setCurrentIndex(0);
    ui->finComboBoxType->setCurrentIndex(0);
    ui->finLineEditMontant->clear();
    ui->finDateEdit->setDate(QDate::currentDate());
    ui->finComboBoxCategorie->setCurrentIndex(0);
    ui->finComboBoxStatut->setCurrentIndex(0);
    ui->finTextEditDescription->clear();
}

void SmartPub::finRemplirFormulaire(const TransactionData &data) {
    int idxProjet = ui->finComboBoxProjet->findData(data.idProjet);
    if (idxProjet >= 0)
        ui->finComboBoxProjet->setCurrentIndex(idxProjet);
    else
        ui->finComboBoxProjet->setCurrentIndex(0);

    int idxType = ui->finComboBoxType->findText(data.type);
    if (idxType >= 0)
        ui->finComboBoxType->setCurrentIndex(idxType);
    else
        ui->finComboBoxType->setCurrentIndex(0);

    ui->finLineEditMontant->setText(QString::number(data.montant, 'f', 2));

    QDate d = QDate::fromString(data.date, "dd/MM/yyyy");
    if (d.isValid()) ui->finDateEdit->setDate(d);
    else ui->finDateEdit->setDate(QDate::currentDate());

    int idxCat = ui->finComboBoxCategorie->findText(data.categorie);
    if (idxCat >= 0) ui->finComboBoxCategorie->setCurrentIndex(idxCat);
    else ui->finComboBoxCategorie->setCurrentText(data.categorie);

    int idxStatut = ui->finComboBoxStatut->findText(data.statut);
    if (idxStatut >= 0) ui->finComboBoxStatut->setCurrentIndex(idxStatut);
    else ui->finComboBoxStatut->setCurrentText(data.statut);

    ui->finTextEditDescription->setPlainText(data.description);
}

void SmartPub::on_finBtnVueListe_clicked() {
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnAjouter_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }
    finTransactionSelectionnee = 0;  // Mode ajout
    finRemplirComboProjets();  // Recharger les projets depuis la BD
    finViderFormulaire();
    ui->finFormTitle->setText("Nouvelle transaction");
    ui->finFormSubtitle->setText("Remplissez les informations pour ajouter une nouvelle transaction");
    ui->finBtnAjouterTransaction->setText("➕ Ajouter");
    ui->finStackedWidget->setCurrentIndex(1);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnRecherche_clicked() {
    finAfficherListeTransactions();
}

void SmartPub::on_finLineEditRecherche_textChanged(const QString &) {
    finAfficherListeTransactions();
}

void SmartPub::on_finBtnTri_clicked() {
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
    )");

    auto appliquerTri = [this](int col, Qt::SortOrder ordre) {
        finTriColonne = col;
        finTriOrdre = ordre;
        finAfficherListeTransactions();
    };

    menu->addAction("Trier par Date (récent → ancien)", this, [appliquerTri]() {
        appliquerTri(4, Qt::DescendingOrder);
    });
    menu->addAction("Trier par Date (ancien → récent)", this, [appliquerTri]() {
        appliquerTri(4, Qt::AscendingOrder);
    });
    menu->addAction("Trier par Montant (croissant)", this, [appliquerTri]() {
        appliquerTri(3, Qt::AscendingOrder);
    });
    menu->addAction("Trier par Montant (décroissant)", this, [appliquerTri]() {
        appliquerTri(3, Qt::DescendingOrder);
    });
    menu->addAction("Trier par Projet (A-Z)", this, [appliquerTri]() {
        appliquerTri(1, Qt::AscendingOrder);
    });
    menu->addAction("Trier par Type", this, [appliquerTri]() {
        appliquerTri(2, Qt::AscendingOrder);
    });

    menu->exec(QCursor::pos());
}

void SmartPub::on_finBtnExport_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les transactions", QDir::homePath(), "CSV (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "ID;Projet;Type;Montant;Date;Catégorie;Statut;Description\n";

    QList<TransactionData> liste = finGetTransactionsFiltreesEtTriees();
    for (const TransactionData &t : liste) {
        QString desc = t.description;
        desc.replace("\"", "\"\"");
        out << t.id << ";\"" << t.projet << "\";\"" << t.type << "\";"
            << QString::number(t.montant, 'f', 2) << ";\"" << t.date << "\";\""
            << t.categorie << "\";\"" << t.statut << "\";\"" << desc << "\"\n";
    }
    file.close();
    QMessageBox::information(this, "Export", "Transactions exportées avec succès !");
}

void SmartPub::on_finBtnStatistiques_clicked() {
    FinStatistiquesDialog *dialog = new FinStatistiquesDialog(finTransactionsMap, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::on_finBtnAjouterTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }

    const QString idProjet = ui->finComboBoxProjet->currentData().toString();
    QString typeDb = ui->finComboBoxType->currentData().toString();
    if (typeDb.isEmpty())
        typeDb = finTypeUiToDb(ui->finComboBoxType->currentText());
    const QString montantStr = ui->finLineEditMontant->text();
    const QString date = ui->finDateEdit->date().toString(QStringLiteral("dd/MM/yyyy"));
    const QString categorie = ui->finComboBoxCategorie->currentText();
    const QString statut = ui->finComboBoxStatut->currentText();
    const QString description = ui->finTextEditDescription->toPlainText();

    // Verification projet obligatoire (NOT NULL base)
    if (idProjet.isEmpty() || idProjet == QStringLiteral("0")) {
        QMessageBox::warning(this, QStringLiteral("Champ obligatoire"),
                             QStringLiteral("Veuillez selectionner un projet existant."));
        return;
    }
    // Verification type obligatoire (NOT NULL base)
    if (typeDb.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Champ obligatoire"),
                             QStringLiteral("Veuillez selectionner un type de transaction."));
        return;
    }
    // Verification date valide (NOT NULL base)
    if (!ui->finDateEdit->date().isValid()) {
        QMessageBox::warning(this, QStringLiteral("Champ obligatoire"),
                             QStringLiteral("Veuillez saisir une date valide."));
        return;
    }
    // Verification montant
    if (montantStr.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Erreur"),
                             QStringLiteral("Veuillez remplir tous les champs obligatoires (*)"));
        return;
    }

    bool ok = false;
    const double montant = montantStr.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Le montant doit etre un nombre valide (ex: 150.50)."));
        return;
    }
    if (montant <= 0.0) {
        QMessageBox::warning(this, QStringLiteral("Erreur"), QStringLiteral("Le montant doit etre superieur a 0."));
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, QStringLiteral("Erreur"),
                              QStringLiteral("Connexion à la base de données impossible."));
        return;
    }

    QSqlQuery query(db);

    if (finTransactionSelectionnee > 0) {
        query.prepare(
            QStringLiteral("UPDATE FINANCE SET MONTANT = :m, TYPE_TRANS = :type, CATEGORIE = :cat, "
                           "DATE_TRANSACTION = TO_DATE(:d, 'DD/MM/YYYY'), STATUT = :statut, "
                           "DESCRIPTION = :desc, ID_PROJET = :idp WHERE ID_TRANSACTION = :id"));
        query.bindValue(QStringLiteral(":m"), montant);
        query.bindValue(QStringLiteral(":type"), typeDb);
        query.bindValue(QStringLiteral(":cat"), categorie);
        query.bindValue(QStringLiteral(":d"), date);
        query.bindValue(QStringLiteral(":statut"), statut);
        query.bindValue(QStringLiteral(":desc"), description);
        if (!idProjet.isEmpty() && idProjet != QStringLiteral("0"))
            query.bindValue(QStringLiteral(":idp"), idProjet);
        else
            query.bindValue(QStringLiteral(":idp"), QVariant());
        query.bindValue(QStringLiteral(":id"), finTransactionSelectionnee);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de la modification : ") + query.lastError().text());
            return;
        }
        QMessageBox::information(this, QStringLiteral("Succès"),
                                 QStringLiteral("Transaction modifiée avec succès !"));
    } else {
        query.prepare(QStringLiteral(
            "INSERT INTO FINANCE (MONTANT, TYPE_TRANS, CATEGORIE, DATE_TRANSACTION, STATUT, DESCRIPTION, "
            "ID_PROJET) VALUES (:m, :type, :cat, TO_DATE(:d, 'DD/MM/YYYY'), :statut, :desc, :idp)"));
        query.bindValue(QStringLiteral(":m"), montant);
        query.bindValue(QStringLiteral(":type"), typeDb);
        query.bindValue(QStringLiteral(":cat"), categorie);
        query.bindValue(QStringLiteral(":d"), date);
        query.bindValue(QStringLiteral(":statut"), statut);
        query.bindValue(QStringLiteral(":desc"), description);
        if (!idProjet.isEmpty() && idProjet != QStringLiteral("0"))
            query.bindValue(QStringLiteral(":idp"), idProjet);
        else
            query.bindValue(QStringLiteral(":idp"), QVariant());
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de l'ajout : ") + query.lastError().text());
            return;
        }
        QMessageBox::information(this, QStringLiteral("Succès"),
                                 QStringLiteral("Transaction ajoutée avec succès !"));
    }

    finTransactionSelectionnee = 0;
    finViderFormulaire();
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finChargerTransactionsDepuisOracle();
}

void SmartPub::on_finBtnAnnulerAjout_clicked() {
    finTransactionSelectionnee = 0;
    finViderFormulaire();
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnModifierTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(
            this, "Accès refusé",
            "Les invités ne peuvent pas modifier les transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner une transaction à modifier");
        return;
    }

    QTableWidgetItem *idItem = ui->finTableTransactions->item(currentRow, 0);
    if (!idItem) return;

    int transactionId = idItem->text().toInt();
    if (!finTransactionsMap.contains(transactionId)) return;

    finTransactionSelectionnee = transactionId;
    finRemplirFormulaire(finTransactionsMap[transactionId]);
    ui->finFormTitle->setText("Modifier la transaction");
    ui->finFormSubtitle->setText("Modifiez les informations de la transaction");
    ui->finBtnAjouterTransaction->setText("💾 Enregistrer");
    ui->finStackedWidget->setCurrentIndex(1);
    finUpdateButtonStyles();
}

void SmartPub::on_finBtnSupprimerTransaction_clicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(
            this, "Accès refusé",
            "Les invités ne peuvent pas supprimer les transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner une transaction à supprimer");
        return;
    }

    QTableWidgetItem *idItem = ui->finTableTransactions->item(currentRow, 0);
    if (!idItem) return;

    int transactionId = idItem->text().toInt();

    auto reply = QMessageBox::question(
        this, "Supprimer", "Confirmer la suppression de cette transaction ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (db.isOpen()) {
            QSqlQuery q(db);
            q.prepare(QStringLiteral("DELETE FROM FINANCE WHERE ID_TRANSACTION = :id"));
            q.bindValue(QStringLiteral(":id"), transactionId);
            if (!q.exec()) {
                QMessageBox::critical(this, QStringLiteral("Erreur"),
                                      QStringLiteral("Échec de la suppression : ") + q.lastError().text());
                return;
            }
        }
        finChargerTransactionsDepuisOracle();
        QMessageBox::information(this, "Succès", "Transaction supprimée");
    }
}


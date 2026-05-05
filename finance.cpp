#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include "trans_secure.h"
#include "osnotification.h"
#include <QStatusBar>
#include <QDoubleValidator>
#include <QLocale>
#include <QPdfWriter>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>

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

    // === Initialisation suppression sécurisée via clavier DG ===
    if (!m_kepadDelete) {
        m_kepadDelete = new FinKepadDelete(arduino, this);
        connect(m_kepadDelete, &FinKepadDelete::suppressionReussie,
                this, [this](int) {
            finChargerTransactionsDepuisOracle();
        });
        connect(m_kepadDelete, &FinKepadDelete::suppressionAnnulee,
                this, [this](int id) {
            statusBar()->showMessage(
                QString("Suppression de la transaction #%1 annulée.").arg(id), 5000);
        });
        connect(m_kepadDelete, &FinKepadDelete::suppressionRefusee,
                this, [this](int id) {
            statusBar()->showMessage(
                QString("❌ Suppression de la transaction #%1 refusée — trop de tentatives.").arg(id), 7000);
        });
        connect(m_kepadDelete, &FinKepadDelete::statutMessage,
                this, [this](const QString &msg) {
            statusBar()->showMessage(msg, 6000);
        });
    }

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

    // === Validation numerique du champ Montant (chiffres et point decimal uniquement) ===
    QDoubleValidator *montantValidator = new QDoubleValidator(0.0, 999999999.99, 2, ui->finLineEditMontant);
    montantValidator->setLocale(QLocale::C);
    montantValidator->setNotation(QDoubleValidator::StandardNotation);
    ui->finLineEditMontant->setValidator(montantValidator);
    ui->finLineEditMontant->setPlaceholderText(QStringLiteral("Ex: 150.00"));

    // === Bouton Journal de Sécurité ===
    // Cherche si un bouton existe déjà (évite les doublons au rechargement)
    QPushButton *btnJournal = ui->finStackedWidget->parentWidget()
                                  ? ui->finStackedWidget->parentWidget()->findChild<QPushButton*>("btnJournalSecurite")
                                  : nullptr;
    if (!btnJournal) {
        // On cherche la barre de boutons du module finance pour y ajouter le bouton
        QPushButton *btnRef = ui->finBtnStatistiques; // bouton de référence existant
        if (btnRef && btnRef->parentWidget()) {
            QHBoxLayout *barLayout = qobject_cast<QHBoxLayout*>(btnRef->parentWidget()->layout());
            btnJournal = new QPushButton("🔒 Journal Sécurité");
            btnJournal->setObjectName("btnJournalSecurite");
            btnJournal->setCursor(Qt::PointingHandCursor);
            btnJournal->setStyleSheet(
                "QPushButton { background-color: #1e293b; color: #e2e8f0; border: none;"
                " border-radius: 8px; padding: 8px 16px; font-size: 13px; font-weight: 600; }"
                "QPushButton:hover { background-color: #334155; }");
            connect(btnJournal, &QPushButton::clicked, this, [this]() {
                TransSecure::afficherJournal(this);
            });
            if (barLayout) {
                barLayout->addWidget(btnJournal);
            } else if (btnRef->parentWidget()->layout()) {
                btnRef->parentWidget()->layout()->addWidget(btnJournal);
            }
        }
    }
}

void SmartPub::finRemplirComboProjets()
{
    ui->finComboBoxProjet->clear();
    ui->finComboBoxProjet->addItem(QStringLiteral("—"), QVariant(0));

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;

    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT ID_PROJET, CODE, TITRE FROM PROJET ORDER BY CODE"))) {
        while (q.next()) {
            const int pid = q.value(0).toInt();
            const QString code = q.value(1).toString();
            const QString titre = q.value(2).toString();
            ui->finComboBoxProjet->addItem(QStringLiteral("%1 — %2").arg(code, titre), pid);
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
                       "FROM FINANCE f LEFT JOIN PROJET p ON p.ID_PROJET = f.ID_PROJET "
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
        t.idProjet = pv.isNull() ? QString() : QString::number(pv.toInt());
        const QString titre = q.value(8).toString();
        t.projet = titre.isEmpty() && !t.idProjet.isEmpty() ? QStringLiteral("Projet #%1").arg(t.idProjet) : titre;
        finTransactionsMap.insert(t.id, t);
    }
    finAfficherListeTransactions();
    finVerifierBudgets();   // surveillance intelligente du budget
}

// ============================================================================
// SURVEILLANCE INTELLIGENTE DU BUDGET
// ============================================================================
//
// Calcule les dépenses totales par projet (types non-recettes).
// Déclenche une notification OS si un projet dépasse son seuil critique.
//
// Seuil par défaut : 10 000 TND (configurable via BUDGET_SEUIL_DEFAUT).
// ============================================================================

void SmartPub::finVerifierBudgets()
{
    // Seuil d'alerte par défaut (TND) — modifiable selon vos données
    static constexpr double BUDGET_SEUIL_DEFAUT = 10000.0;

    // Types considérés comme DÉPENSES (pas des recettes)
    auto estDepense = [](const QString &type) -> bool {
        const QString t = type.trimmed().toLower();
        return t != QLatin1String("subvention") && t != QLatin1String("remboursement");
    };

    // Calculer les dépenses totales par projet
    QMap<QString, double> depensesParProjet;  // nom projet → total dépenses
    for (auto it = finTransactionsMap.constBegin(); it != finTransactionsMap.constEnd(); ++it) {
        const TransactionData &t = it.value();
        if (!estDepense(t.type)) continue;
        const QString nom = t.projet.isEmpty()
                                ? QStringLiteral("Projet #%1").arg(t.idProjet)
                                : t.projet;
        depensesParProjet[nom] += t.montant;
    }

    // Vérifier les dépassements et émettre les notifications OS
    bool alerteEmise = false;
    for (auto it = depensesParProjet.constBegin(); it != depensesParProjet.constEnd(); ++it) {
        const QString &projetNom    = it.key();
        const double   totalDepense = it.value();

        if (totalDepense >= BUDGET_SEUIL_DEFAUT) {
            qDebug().noquote()
                << QStringLiteral("[BUDGET] ALERTE — Projet '%1' : %2 TND >= seuil %3 TND")
                       .arg(projetNom,
                            QString::number(totalDepense, 'f', 2),
                            QString::number(BUDGET_SEUIL_DEFAUT, 'f', 2));

            OsNotification::instance()->alertBudget(projetNom, totalDepense, BUDGET_SEUIL_DEFAUT);
            alerteEmise = true;

        } else if (totalDepense >= BUDGET_SEUIL_DEFAUT * 0.80) {
            // Avertissement à 80 % du seuil
            const QString title = QString::fromUtf8("\U0001f4ca Budget à 80%");
            const QString msg   = QString::fromUtf8(
                "Projet : %1\nDépenses : %2 TND (%3% du seuil de %4 TND)")
                .arg(projetNom,
                     QString::number(totalDepense, 'f', 2),
                     QString::number(totalDepense / BUDGET_SEUIL_DEFAUT * 100.0, 'f', 0),
                     QString::number(BUDGET_SEUIL_DEFAUT, 'f', 0));

            OsNotification::instance()->show(title, msg, QSystemTrayIcon::Warning, 7000);
            alerteEmise = true;
        }
    }

    if (!alerteEmise) {
        qDebug() << "[BUDGET] Tous les projets sont dans les limites budgetaires.";
    }
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
        // Vérification du rôle
        if (currentUser.role != UserRole::Admin) {
            QMessageBox::warning(this, "Accès refusé",
                "Seul l'Administrateur peut supprimer des transactions.");
            return;
        }
        // Si une procédure est déjà en cours
        if (m_kepadDelete && m_kepadDelete->enCours()) {
            QMessageBox::warning(this, "Procédure en cours",
                "Une suppression est déjà en attente de validation\n"
                "sur le clavier du Directeur Général.");
            return;
        }
        // Confirmation initiale
        int rep = QMessageBox::question(this, "Suppression sécurisée",
            QString("Un code de confirmation va être envoyé par email\n"
                    "au Directeur Général.\n\n"
                    "Voulez-vous procéder à la suppression de la\n"
                    "transaction #%1 ?").arg(transId),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (rep != QMessageBox::Yes) return;
        // Démarrer la procédure clavier DG
        if (m_kepadDelete)
            m_kepadDelete->demanderSuppression(transId, m_dgEmail);
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
    int idxProjet = ui->finComboBoxProjet->findData(QVariant(data.idProjet.toInt()));
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

void SmartPub::handleFinBtnVueListeClicked() {
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}

void SmartPub::handleFinBtnAjouterClicked() {
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

void SmartPub::handleFinBtnRechercheClicked() {
    finAfficherListeTransactions();
}

void SmartPub::handleFinLineEditRechercheTextChanged(const QString &) {
    finAfficherListeTransactions();
}

void SmartPub::handleFinBtnTriClicked() {
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

void SmartPub::handleFinBtnExportClicked()
{
    // ── 1. Choix fichier PDF ──────────────────────────────────────────────────
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Exporter le bilan financier"),
        QDir::homePath() + QStringLiteral("/bilan_financier.pdf"),
        QStringLiteral("PDF (*.pdf);;HTML (*.html)"),
        &selectedFilter);
    if (fileName.isEmpty()) return;

    bool exportHtml = selectedFilter.contains("html", Qt::CaseInsensitive)
                   || fileName.endsWith(".html", Qt::CaseInsensitive);

    // Forcer la bonne extension
    if (exportHtml) {
        if (!fileName.endsWith(".html", Qt::CaseInsensitive))
            fileName += QStringLiteral(".html");
    } else {
        if (!fileName.endsWith(".pdf", Qt::CaseInsensitive))
            fileName += QStringLiteral(".pdf");
    }

    // Verifier que le fichier est accessible en ecriture
    {
        QFile testFile(fileName);
        if (!testFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::critical(this, QStringLiteral("Erreur d'acces"),
                QStringLiteral("Impossible d'ecrire dans ce fichier.\n"
                               "S'il est deja ouvert, fermez-le d'abord.\n\n"
                               "Fichier : ") + fileName);
            return;
        }
        testFile.close();
        testFile.remove();
    }

    // ── Export HTML inline (aucune dependance Qt plugin) ────────────────────
    if (exportHtml) {
        // Calculs financiers pour le HTML
        QList<TransactionData> listeHtml = finGetTransactionsFiltreesEtTriees();
        double totRec = 0.0, totDep = 0.0;
        auto isRec = [](const QString &type) -> bool {
            const QString t = type.trimmed().toLower();
            return t == QLatin1String("subvention") || t == QLatin1String("remboursement");
        };
        for (const TransactionData &t : listeHtml) {
            if (isRec(t.type)) totRec += t.montant; else totDep += t.montant;
        }
        double soldeHtml = totRec - totDep;
        QString soldeColor = soldeHtml >= 0 ? "#10b981" : "#ef4444";
        QString soldeGrad2 = soldeHtml >= 0 ? "#059669" : "#b91c1c";
        QString soldeLabel = soldeHtml >= 0 ? "Excedentaire" : "Deficitaire";

        QString rows;
        for (const TransactionData &t : listeHtml) {
            QString sc = "#64748b";
            if      (t.statut.contains("Valid",   Qt::CaseInsensitive)) sc = "#16a34a";
            else if (t.statut.contains("attente", Qt::CaseInsensitive)) sc = "#d97706";
            else if (t.statut.contains("Rejet",   Qt::CaseInsensitive)) sc = "#dc2626";
            rows += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4 TND</td>"
                            "<td>%5</td><td>%6</td>"
                            "<td style='color:%7;font-weight:bold'>%8</td></tr>")
                .arg(t.id).arg(t.projet.toHtmlEscaped()).arg(t.type.toHtmlEscaped())
                .arg(QString::number(t.montant,'f',2)).arg(t.date.toHtmlEscaped())
                .arg(t.categorie.toHtmlEscaped()).arg(sc).arg(t.statut.toHtmlEscaped());
        }

        QString html = QString(
"<!DOCTYPE html><html lang='fr'><head><meta charset='UTF-8'>"
"<title>Bilan Financier - SmartPub</title><style>"
"body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#f8fafc;color:#1e293b}"
".hdr{background:linear-gradient(135deg,#3b82f6,#10b981);color:white;padding:28px;border-radius:12px;margin-bottom:20px}"
".hdr h1{margin:0;font-size:22px}.hdr p{margin:4px 0 0;opacity:.85;font-size:13px}"
".kpis{display:flex;gap:14px;margin-bottom:20px}"
".kpi{background:white;border:1px solid #e2e8f0;border-radius:10px;padding:18px;flex:1;text-align:center}"
".kpi .v{font-size:20px;font-weight:bold;margin-bottom:3px}.kpi .l{font-size:11px;color:#64748b}"
"table{width:100%%;border-collapse:collapse;background:white;border-radius:10px;overflow:hidden;box-shadow:0 1px 3px rgba(0,0,0,.1)}"
"th{background:linear-gradient(90deg,#3b82f6,#10b981);color:white;padding:11px 9px;text-align:left;font-size:12px}"
"td{padding:9px;font-size:12px;border-bottom:1px solid #f1f5f9}"
"tr:nth-child(even) td{background:#f8fafc}"
".solde{background:linear-gradient(135deg,%1,%2);color:white;border-radius:10px;padding:18px;text-align:center;margin:20px 0}"
".solde .v{font-size:26px;font-weight:bold}"
".footer{text-align:center;color:#94a3b8;font-size:10px;margin-top:16px}"
"@media print{body{background:white}}"
"</style></head><body>"
"<div class='hdr'><h1>SmartPub &mdash; Bilan Financier</h1>"
"<p>G&eacute;n&eacute;r&eacute; le %3 &bull; %4 transaction(s)</p></div>"
"<div class='kpis'>"
"<div class='kpi'><div class='v' style='color:#10b981'>%5 TND</div><div class='l'>Total Recettes</div></div>"
"<div class='kpi'><div class='v' style='color:#ef4444'>%6 TND</div><div class='l'>Total D&eacute;penses</div></div>"
"<div class='kpi'><div class='v' style='color:%1'>%7 TND</div><div class='l'>Solde Net</div></div>"
"</div>"
"<table><thead><tr><th>ID</th><th>Projet</th><th>Type</th><th>Montant</th>"
"<th>Date</th><th>Cat&eacute;gorie</th><th>Statut</th></tr></thead>"
"<tbody>%8</tbody></table>"
"<div class='solde'><div>SOLDE NET FINAL</div><div class='v'>%7 TND &mdash; %9</div></div>"
"<div class='footer'>SmartPub &bull; Confidentiel &bull; %3<br>"
"<small>Ouvrez dans un navigateur &rarr; Fichier &rarr; Imprimer &rarr; Enregistrer en PDF</small>"
"</div></body></html>")
            .arg(soldeColor).arg(soldeGrad2)
            .arg(QDate::currentDate().toString("dd/MM/yyyy"))
            .arg(listeHtml.size())
            .arg(QString::number(totRec,'f',2))
            .arg(QString::number(totDep,'f',2))
            .arg(QString::number(soldeHtml,'f',2))
            .arg(rows)
            .arg(soldeLabel);

        QFile hf(fileName);
        if (!hf.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                QStringLiteral("Impossible d'ecrire : ") + fileName);
            return;
        }
        hf.write(html.toUtf8());
        hf.close();

        QMessageBox::information(this, QStringLiteral("Export reussi"),
            QStringLiteral("Bilan exporte en HTML !\n%1\n\n"
                           "Pour obtenir un PDF :\nOuvrez dans Chrome/Firefox → Imprimer → Enregistrer en PDF").arg(fileName));
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        return;
    }

    // ── 2. Calculs financiers ─────────────────────────────────────────────────
    QList<TransactionData> liste = finGetTransactionsFiltreesEtTriees();

    double totalRecettes = 0.0, totalDepenses = 0.0;
    QMap<QString, double> parType;
    QMap<QString, double> parProjet;
    QMap<QString, double> recParMois;
    QMap<QString, double> depParMois;

    auto estRecette = [](const QString &type) -> bool {
        const QString t = type.trimmed().toLower();
        return t == QLatin1String("subvention") || t == QLatin1String("remboursement");
    };

    for (const TransactionData &t : liste) {
        bool rec = estRecette(t.type);
        if (rec) totalRecettes += t.montant;
        else     totalDepenses += t.montant;
        parType[t.type] += t.montant;
        QString proj = t.projet.isEmpty() ? QStringLiteral("—") : t.projet;
        parProjet[proj] += t.montant;
        QDate d = QDate::fromString(t.date, QStringLiteral("dd/MM/yyyy"));
        if (d.isValid()) {
            QString mois = d.toString(QStringLiteral("yyyy-MM"));
            if (rec) recParMois[mois] += t.montant;
            else     depParMois[mois] += t.montant;
        }
    }
    double soldeNet    = totalRecettes - totalDepenses;
    bool excedentaire  = soldeNet >= 0.0;

    // ── 3. Initialisation QPdfWriter ──────────────────────────────────────────
    // QPdfWriter ne dépend d'aucun driver d'impression (contrairement à QPrinter)
    // et fonctionne nativement sur Windows, Linux et macOS.
    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Portrait);
    pdfWriter.setPageMargins(QMarginsF(12, 12, 12, 12), QPageLayout::Millimeter);
    // Résolution 96 DPI : coordonnées identiques à un affichage écran standard
    pdfWriter.setResolution(96);

    // ── 4. Constantes de mise en page ─────────────────────────────────────────
    // Coordonnées logiques A4 à 96 DPI : 794 × 1123 px
    const QRect  logicalPage(0, 0, 794, 1123);

    QPainter p;
    if (!p.begin(&pdfWriter)) {
        // Diagnostic detaille pour aider au debug
        QString errMsg = QStringLiteral(
            "QPdfWriter::begin() a echoue.\n\n"
            "Causes possibles :\n"
            "  - Le fichier est deja ouvert dans un autre programme\n"
            "  - Chemin reseau non supporte\n"
            "  - Plugin Qt 'qpdf' manquant\n\n"
            "Fichier cible : ") + fileName;
        QMessageBox::critical(this, QStringLiteral("Erreur PDF"), errMsg);
        return;
    }
    const int    W        = logicalPage.width();
    const int    margin   = 55;
    const int    colW     = W - 2 * margin;

    // Couleurs
    const QColor cBlue  ("#3b82f6");
    const QColor cGreen ("#10b981");
    const QColor cRed   ("#ef4444");
    const QColor cGray  ("#64748b");
    const QColor cLight ("#f8fafc");
    const QColor cBorder("#e2e8f0");
    const QColor cWhite (Qt::white);
    const QColor cDark  ("#1e293b");
    const QColor cAmber ("#d97706");

    // Polices
    // Utiliser setPixelSize (pas setPointSize) pour eviter la mise a l'echelle DPI
    // Les tailles sont en pixels logiques (espace 794x1123)
    auto font = [](int sz, bool bold = false) {
        QFont f(QStringLiteral("Arial"));
        f.setPixelSize(sz);
        f.setBold(bold);
        return f;
    };

    int y = margin; // curseur vertical

    // ── Helpers ───────────────────────────────────────────────────────────────

    auto fillRRect = [&](int x, int top, int w, int h, int r, const QColor &c) {
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        p.drawRoundedRect(x, top, w, h, r, r);
    };

    auto hLine = [&](int top, const QColor &c = QColor("#e2e8f0"), int lw = 1) {
        p.setPen(QPen(c, lw));
        p.drawLine(margin, top, margin + colW, top);
    };

    // Vérifie si on doit passer à une nouvelle page
    auto checkPage = [&](int needed) {
        if (y + needed > logicalPage.height() - margin - 50) {
            pdfWriter.newPage();
            y = margin;
        }
    };

    // Dessine une ligne de tableau (header ou data)
    auto drawRow = [&](const QStringList &cells, const QList<int> &widths,
                       int rowH, bool isHeader, bool odd = false,
                       const QList<QColor> &cellColors = {}) {
        int x = margin;
        if (isHeader) {
            // Fond dégradé bleu→vert
            QLinearGradient g(margin, y, margin + colW, y);
            g.setColorAt(0, cBlue); g.setColorAt(1, cGreen);
            p.setPen(Qt::NoPen); p.setBrush(g);
            p.drawRect(margin, y, colW, rowH);
        } else if (odd) {
            p.setPen(Qt::NoPen); p.setBrush(cLight);
            p.drawRect(margin, y, colW, rowH);
        }
        for (int i = 0; i < cells.size(); ++i) {
            QColor col = isHeader ? cWhite
                       : (!cellColors.isEmpty() && i < cellColors.size()
                          ? cellColors[i] : cDark);
            p.setFont(font(isHeader ? 12 : 11, isHeader));
            p.setPen(col);
            p.drawText(QRect(x + 6, y, widths[i] - 8, rowH),
                       Qt::AlignVCenter | Qt::AlignLeft, cells[i]);
            x += widths[i];
        }
        p.setPen(QPen(cBorder, 1));
        p.drawLine(margin, y + rowH, margin + colW, y + rowH);
        y += rowH;
    };

    // Titre de section
    auto sectionTitle = [&](const QString &title) {
        checkPage(80);
        p.setFont(font(16, true));
        p.setPen(cDark);
        p.drawText(QRect(margin, y, colW, 36),
                   Qt::AlignVCenter | Qt::AlignLeft, title);
        y += 36;
        hLine(y, cBorder, 2);
        y += 10;
    };

    // ── 5. EN-TÊTE ────────────────────────────────────────────────────────────
    {
        int hH = 155;
        QLinearGradient grad(margin, y, margin + colW, y);
        grad.setColorAt(0, cBlue); grad.setColorAt(1, cGreen);
        p.setPen(Qt::NoPen); p.setBrush(grad);
        p.drawRoundedRect(margin, y, colW, hH, 14, 14);

        p.setFont(font(26, true));
        p.setPen(cWhite);
        p.drawText(QRect(margin + 30, y + 22, colW - 130, 50),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QStringLiteral("SmartPub \u2014 Bilan Financier"));

        p.setFont(font(13));
        p.setPen(QColor(255, 255, 255, 200));
        p.drawText(QRect(margin + 30, y + 76, colW - 130, 28),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QStringLiteral("G\u00e9n\u00e9r\u00e9 le : %1  \u2022  %2 transaction(s)")
                   .arg(QDate::currentDate().toString("dd/MM/yyyy"))
                   .arg(liste.size()));

        // Icône décorative
        p.setFont(font(54));
        p.setPen(QColor(255, 255, 255, 60));
        p.drawText(QRect(margin + colW - 110, y + 10, 100, hH - 20),
                   Qt::AlignCenter, QStringLiteral("\u2630"));

        y += hH + 28;
    }

    // ── 6. CARTES KPI ─────────────────────────────────────────────────────────
    {
        struct Kpi { QString label; double val; QColor color; };
        QList<Kpi> kpis = {
            { QStringLiteral("Total Recettes"),      totalRecettes, cGreen },
            { QStringLiteral("Total D\u00e9penses"), totalDepenses, cRed   },
            { QStringLiteral("Solde Net"),            soldeNet,      excedentaire ? cGreen : cRed }
        };
        int kpiW = (colW - 36) / 3;
        int kpiH = 100;
        int kx   = margin;
        for (const Kpi &k : kpis) {
            p.setPen(QPen(cBorder, 1)); p.setBrush(cWhite);
            p.drawRoundedRect(kx, y, kpiW, kpiH, 10, 10);
            p.setFont(font(22, true)); p.setPen(k.color);
            p.drawText(QRect(kx, y + 16, kpiW, 42), Qt::AlignCenter,
                       QString::number(k.val, 'f', 2) + QStringLiteral(" TND"));
            p.setFont(font(12)); p.setPen(cGray);
            p.drawText(QRect(kx, y + 62, kpiW, 26), Qt::AlignCenter, k.label);
            kx += kpiW + 18;
        }
        y += kpiH + 28;
    }

    // ── 7. DÉTAIL DES TRANSACTIONS ────────────────────────────────────────────
    {
        sectionTitle(QStringLiteral("D\u00e9tail des Transactions"));
        const QStringList hdr = {
            "ID","Projet","Type","Montant (TND)","Date","Cat\u00e9gorie","Statut"
        };
        const QList<int> w = {
            int(colW*.07), int(colW*.19), int(colW*.13),
            int(colW*.14), int(colW*.12), int(colW*.18), int(colW*.17)
        };
        checkPage(34);
        drawRow(hdr, w, 30, true);

        bool odd = false;
        for (const TransactionData &t : liste) {
            checkPage(30);
            QColor sc = cGray;
            if      (t.statut.contains("Valid",   Qt::CaseInsensitive)) sc = cGreen;
            else if (t.statut.contains("attente", Qt::CaseInsensitive)) sc = cAmber;
            else if (t.statut.contains("Rejet",   Qt::CaseInsensitive)) sc = cRed;
            QList<QColor> cc = { cDark,cDark,cDark,cDark,cDark,cDark, sc };
            drawRow({ QString::number(t.id), t.projet, t.type,
                      QString::number(t.montant,'f',2), t.date,
                      t.categorie, t.statut },
                    w, 28, false, odd, cc);
            odd = !odd;
        }
        y += 20;
    }

    // ── 8. RÉPARTITION PAR TYPE ───────────────────────────────────────────────
    {
        sectionTitle(QStringLiteral("R\u00e9partition par Type de Transaction"));
        const QStringList hdr = { "Type","Montant (TND)","Part (%)" };
        const QList<int>  w   = { int(colW*.50), int(colW*.28), int(colW*.22) };
        checkPage(34);
        drawRow(hdr, w, 30, true);

        double grandTotal = totalRecettes + totalDepenses;
        bool odd = false;
        for (auto it = parType.constBegin(); it != parType.constEnd(); ++it) {
            checkPage(28);
            double pct = grandTotal > 0 ? it.value() / grandTotal * 100.0 : 0.0;
            drawRow({ it.key(),
                      QString::number(it.value(),'f',2),
                      QString::number(pct,'f',1) + " %" },
                    w, 28, false, odd);
            odd = !odd;
        }
        y += 20;
    }

    // ── 9. RÉPARTITION PAR PROJET ─────────────────────────────────────────────
    {
        sectionTitle(QStringLiteral("R\u00e9partition par Projet"));
        const QStringList hdr = { "Projet","Montant Total (TND)" };
        const QList<int>  w   = { int(colW*.60), int(colW*.40) };
        checkPage(34);
        drawRow(hdr, w, 30, true);

        bool odd = false;
        for (auto it = parProjet.constBegin(); it != parProjet.constEnd(); ++it) {
            checkPage(28);
            drawRow({ it.key(), QString::number(it.value(),'f',2) },
                    w, 28, false, odd);
            odd = !odd;
        }
        y += 20;
    }

    // ── 10. SYNTHÈSE MENSUELLE ────────────────────────────────────────────────
    {
        QSet<QString> ms;
        for (auto &k : recParMois.keys()) ms.insert(k);
        for (auto &k : depParMois.keys()) ms.insert(k);
        QStringList ml = ms.values();
        std::sort(ml.begin(), ml.end());

        sectionTitle(QStringLiteral("Synth\u00e8se Mensuelle"));
        const QStringList hdr = { "Mois","Recettes (TND)","D\u00e9penses (TND)" };
        const QList<int>  w   = { int(colW*.40), int(colW*.30), int(colW*.30) };
        checkPage(34);
        drawRow(hdr, w, 30, true);

        bool odd = false;
        for (const QString &mois : ml) {
            checkPage(28);
            drawRow({ mois,
                      QString::number(recParMois.value(mois,0.0),'f',2),
                      QString::number(depParMois.value(mois,0.0),'f',2) },
                    w, 28, false, odd);
            odd = !odd;
        }
        y += 20;
    }

    // ── 11. SOLDE NET FINAL ───────────────────────────────────────────────────
    {
        checkPage(100);
        int bH = 86;
        QLinearGradient grad(margin, y, margin + colW, y);
        if (excedentaire) { grad.setColorAt(0, cGreen);  grad.setColorAt(1, QColor("#059669")); }
        else              { grad.setColorAt(0, cRed);    grad.setColorAt(1, QColor("#b91c1c")); }
        p.setPen(Qt::NoPen); p.setBrush(grad);
        p.drawRoundedRect(margin, y, colW, bH, 12, 12);

        p.setFont(font(13, true)); p.setPen(cWhite);
        p.drawText(QRect(margin, y + 10, colW, 26), Qt::AlignCenter,
                   QStringLiteral("SOLDE NET FINAL"));

        p.setFont(font(24, true));
        p.drawText(QRect(margin, y + 38, colW, 36), Qt::AlignCenter,
                   QString::number(soldeNet,'f',2)
                   + QStringLiteral(" TND  ")
                   + (excedentaire ? QStringLiteral("\u2713 Exc\u00e9dentaire")
                                   : QStringLiteral("\u26a0 D\u00e9ficitaire")));
        y += bH + 24;
    }

    // ── 12. PIED DE PAGE ──────────────────────────────────────────────────────
    {
        int footerY = logicalPage.height() - margin - 28;
        p.setPen(QPen(cBorder, 1));
        p.drawLine(margin, footerY, margin + colW, footerY);
        p.setFont(font(11)); p.setPen(cGray);
        p.drawText(QRect(margin, footerY + 6, colW, 22), Qt::AlignCenter,
                   QStringLiteral("SmartPub \u2022 Bilan g\u00e9n\u00e9r\u00e9 automatiquement le %1 \u2022 Confidentiel")
                   .arg(QDate::currentDate().toString("dd/MM/yyyy")));
    }

    p.end();

    QMessageBox::information(this, QStringLiteral("Export r\u00e9ussi"),
                             QStringLiteral("Bilan financier PDF export\u00e9 avec succ\u00e8s !\n%1")
                             .arg(fileName));
}

void SmartPub::handleFinBtnStatistiquesClicked() {
    FinStatistiquesDialog *dialog = new FinStatistiquesDialog(finTransactionsMap, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void SmartPub::handleFinBtnAjouterTransactionClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter de transactions.");
        return;
    }

    const int idProjetNum = ui->finComboBoxProjet->currentData().toInt();
    QString typeDb = ui->finComboBoxType->currentData().toString();
    if (typeDb.isEmpty())
        typeDb = finTypeUiToDb(ui->finComboBoxType->currentText());
    const QString montantStr = ui->finLineEditMontant->text();
    const QString date = ui->finDateEdit->date().toString(QStringLiteral("dd/MM/yyyy"));
    const QString categorie = ui->finComboBoxCategorie->currentText();
    const QString statut = ui->finComboBoxStatut->currentText();
    const QString description = ui->finTextEditDescription->toPlainText();

    // Verification projet obligatoire (NOT NULL base)
    if (idProjetNum <= 0) {
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
        if (idProjetNum > 0)
            query.bindValue(QStringLiteral(":idp"), idProjetNum);
        else
            query.bindValue(QStringLiteral(":idp"), QVariant());
        query.bindValue(QStringLiteral(":id"), finTransactionSelectionnee);
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de la modification : ") + query.lastError().text());
            return;
        }
        TransSecure::logTransaction("MODIFICATION", finTransactionSelectionnee,
                                    finTypeDbToUi(typeDb), montant, date,
                                    categorie, statut,
                                    ui->finComboBoxProjet->currentText(), description);
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
        if (idProjetNum > 0)
            query.bindValue(QStringLiteral(":idp"), idProjetNum);
        else
            query.bindValue(QStringLiteral(":idp"), QVariant());
        if (!query.exec()) {
            QMessageBox::critical(this, QStringLiteral("Erreur"),
                                  QStringLiteral("Échec de l'ajout : ") + query.lastError().text());
            return;
        }
        TransSecure::logTransaction("AJOUT", 0, finTypeDbToUi(typeDb), montant, date,
                                    categorie, statut,
                                    ui->finComboBoxProjet->currentText(), description);
        QMessageBox::information(this, QStringLiteral("Succès"),
                                 QStringLiteral("Transaction ajoutée avec succès !"));
    }

    finTransactionSelectionnee = 0;
    finViderFormulaire();
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
    finChargerTransactionsDepuisOracle();
}

void SmartPub::handleFinBtnAnnulerAjoutClicked() {
    finTransactionSelectionnee = 0;
    finViderFormulaire();
    ui->finStackedWidget->setCurrentIndex(0);
    finUpdateButtonStyles();
}

void SmartPub::handleFinBtnModifierTransactionClicked() {
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

void SmartPub::handleFinBtnSupprimerTransactionClicked() {
    // Vérification du rôle
    if (currentUser.role != UserRole::Admin) {
        QMessageBox::warning(this, "Accès refusé",
            "Seul l'Administrateur peut supprimer des transactions.");
        return;
    }

    int currentRow = ui->finTableTransactions->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner une transaction à supprimer.");
        return;
    }

    QTableWidgetItem *idItem = ui->finTableTransactions->item(currentRow, 0);
    if (!idItem) return;

    int transactionId = idItem->text().toInt();

    // Si une procédure est déjà en cours
    if (m_kepadDelete && m_kepadDelete->enCours()) {
        QMessageBox::warning(this, "Procédure en cours",
            "Une suppression est déjà en attente de validation\n"
            "sur le clavier du Directeur Général.");
        return;
    }

    // Confirmation initiale avant d'envoyer le mail
    int rep = QMessageBox::question(this, "Suppression sécurisée",
        QString("Un code de confirmation va être envoyé par email\n"
                "au Directeur Général.\n\n"
                "Voulez-vous procéder à la suppression de la\n"
                "transaction #%1 ?").arg(transactionId),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (rep != QMessageBox::Yes) return;

    // Démarrer la procédure sécurisée : mail OTP + attente saisie clavier
    if (m_kepadDelete)
        m_kepadDelete->demanderSuppression(transactionId, m_dgEmail);
}

// ============================================================================
// MODULE FINANCES - STATISTIQUES
// ============================================================================

FinStatistiquesDialog::FinStatistiquesDialog(const QMap<int, TransactionData> &transactions,
                                             QWidget *parent)
    : QDialog(parent), m_transactions(transactions)
    , labelTotalRecettes(nullptr), labelTotalDepenses(nullptr)
    , labelSolde(nullptr), labelNbTransactions(nullptr)
    , chartTypeView(nullptr), chartProjetView(nullptr)
{
    setWindowTitle("Statistiques Financières");
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
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #2563eb, stop:1 #059669); }"
        );

    setupUI();
    calculerStatistiques();
    creerGraphiques();
}

void FinStatistiquesDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #3b82f6, stop:1 #10b981); border: none; }");
    headerFrame->setFixedHeight(100);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(30, 20, 30, 20);

    QLabel *titleLabel = new QLabel("📊 Statistiques Financières");
    titleLabel->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");
    QLabel *subtitleLabel = new QLabel("Tableau de bord des transactions");
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
                        const QString &color, QLabel **valueLabelPtr) -> QFrame* {
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

    kpiLayout->addWidget(createKPI("💰", "0 €", "Total Recettes", "#10b981", &labelTotalRecettes));
    kpiLayout->addWidget(createKPI("📤", "0 €", "Total Dépenses", "#ef4444", &labelTotalDepenses));
    kpiLayout->addWidget(createKPI("📊", "0 €", "Solde", "#3b82f6", &labelSolde));
    kpiLayout->addWidget(createKPI("📋", "0", "Nb. Transactions", "#8b5cf6", &labelNbTransactions));
    contentLayout->addLayout(kpiLayout);

    QHBoxLayout *chartsLayout = new QHBoxLayout();
    chartsLayout->setSpacing(20);

    QGroupBox *chartTypeGroup = new QGroupBox("Répartition par Type (Recettes / Dépenses)");
    QVBoxLayout *chartTypeLayout = new QVBoxLayout(chartTypeGroup);
    chartTypeView = new QChartView();
    chartTypeView->setMinimumHeight(300);
    chartTypeView->setRenderHint(QPainter::Antialiasing);
    chartTypeLayout->addWidget(chartTypeView);
    chartsLayout->addWidget(chartTypeGroup, 1);

    QGroupBox *chartProjetGroup = new QGroupBox("Répartition par Projet");
    QVBoxLayout *chartProjetLayout = new QVBoxLayout(chartProjetGroup);
    chartProjetView = new QChartView();
    chartProjetView->setMinimumHeight(300);
    chartProjetView->setRenderHint(QPainter::Antialiasing);
    chartProjetLayout->addWidget(chartProjetView);
    chartsLayout->addWidget(chartProjetGroup, 1);

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

void FinStatistiquesDialog::calculerStatistiques() {
    double totalRecettes = 0, totalDepenses = 0;

    // Même logique que dans handleFinBtnExportClicked :
    // Recette = Subvention ou Remboursement, le reste = Dépense
    auto estRecette = [](const QString &type) -> bool {
        const QString t = type.trimmed().toLower();
        return t == QLatin1String("subvention") || t == QLatin1String("remboursement");
    };

    for (const TransactionData &t : m_transactions) {
        if (estRecette(t.type))
            totalRecettes += t.montant;
        else
            totalDepenses += t.montant;
    }
    double solde = totalRecettes - totalDepenses;

    if (labelTotalRecettes) labelTotalRecettes->setText(QString::number(totalRecettes, 'f', 2) + " TND");
    if (labelTotalDepenses) labelTotalDepenses->setText(QString::number(totalDepenses, 'f', 2) + " TND");
    if (labelSolde) {
        labelSolde->setText(QString::number(solde, 'f', 2) + " TND");
        labelSolde->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;")
            .arg(solde >= 0 ? "#10b981" : "#ef4444"));
    }
    if (labelNbTransactions) labelNbTransactions->setText(QString::number(m_transactions.size()));
}

void FinStatistiquesDialog::creerGraphiques() {
    // Même logique que dans handleFinBtnExportClicked
    auto estRecette = [](const QString &type) -> bool {
        const QString t = type.trimmed().toLower();
        return t == QLatin1String("subvention") || t == QLatin1String("remboursement");
    };

    double totalRecettes = 0, totalDepenses = 0;
    for (const TransactionData &t : m_transactions) {
        if (estRecette(t.type)) totalRecettes += t.montant;
        else totalDepenses += t.montant;
    }

    QPieSeries *seriesType = new QPieSeries();
    if (totalRecettes > 0) seriesType->append("Recettes", totalRecettes);
    if (totalDepenses > 0) seriesType->append("Dépenses", totalDepenses);
    if (seriesType->count() > 0) {
        int idx = 0;
        for (auto *slice : seriesType->slices()) {
            slice->setColor(idx == 0 && totalRecettes > 0
                            ? QColor("#10b981") : QColor("#ef4444"));
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1\n%2%")
                            .arg(slice->label())
                            .arg(slice->percentage() * 100, 0, 'f', 1));
            ++idx;
        }
    }

    QChart *chartType = new QChart();
    chartType->addSeries(seriesType);
    chartType->setTitle(QStringLiteral("Recettes vs Dépenses"));
    chartType->setAnimationOptions(QChart::SeriesAnimations);
    chartType->setBackgroundBrush(QBrush(QColor("transparent")));
    chartType->legend()->setVisible(true);
    chartType->legend()->setAlignment(Qt::AlignBottom);
    chartTypeView->setChart(chartType);

    QMap<QString, double> montantsParProjet;
    for (const TransactionData &t : m_transactions) {
        double sgn = estRecette(t.type) ? 1.0 : -1.0;
        montantsParProjet[t.projet] += sgn * t.montant;
    }

    QBarSet *barSet = new QBarSet("Solde par projet");
    QStringList categories;
    for (auto it = montantsParProjet.constBegin(); it != montantsParProjet.constEnd(); ++it) {
        barSet->append(qAbs(it.value()));
        categories << it.key();
    }
    QBarSeries *barSeries = new QBarSeries();
    barSeries->append(barSet);
    barSet->setColor(QColor("#3b82f6"));

    QChart *chartProjet = new QChart();
    chartProjet->addSeries(barSeries);
    chartProjet->setTitle(QStringLiteral("Solde par Projet (TND)"));
    chartProjet->setAnimationOptions(QChart::SeriesAnimations);
    chartProjet->setBackgroundBrush(QBrush(QColor("transparent")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartProjet->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(QStringLiteral("Montant (TND)"));
    chartProjet->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);

    chartProjet->legend()->setVisible(true);
    chartProjet->legend()->setAlignment(Qt::AlignBottom);
    chartProjetView->setChart(chartProjet);
}

void SmartPub::handleFinancesNavigation() {
    ui->stackedWidgetModules->setCurrentIndex(2);
    setActiveNavigationButton(2);
    updateProfileName(2);
    finUpdateButtonStyles();
    finAfficherListeTransactions();
}
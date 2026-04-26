#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include "calender.h"

#include <algorithm>
#include <QApplication>
#include <QScreen>
#include <QPrinter>
#include <QPainter>
#include <QPageSize>
#include <QPageLayout>
#include <QTextStream>
#include <QDateTime>

// ============================================================================
// MODULE EVENEMENTS
// ============================================================================

void SmartPub::evSetupUI() {
    ui->evTabWidget->setCurrentIndex(0);
    evEventSelectionne = -1;

    // --- Contrôle de saisie ---

    // Code : chiffres uniquement
    ui->evLineEditID->setValidator(new QIntValidator(0, 999999999, this));

    // Nom et Lieu : lettres et espaces uniquement
    QRegularExpression lettersOnly("[A-Za-zÀ-ÿ ]+");
    ui->evLineEditNom->setValidator(new QRegularExpressionValidator(lettersOnly, this));
    ui->evLineEditLieu->setValidator(new QRegularExpressionValidator(lettersOnly, this));

    // Date : validation en temps réel, champ rouge si invalide
    connect(ui->evLineEditDate, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (text.isEmpty()) {
            ui->evLineEditDate->setStyleSheet("");
            return;
        }
        QDate d = QDate::fromString(text.trimmed(), "dd/MM/yyyy");
        if (d.isValid() && d.toString("dd/MM/yyyy") == text.trimmed()) {
            ui->evLineEditDate->setStyleSheet("");
        } else {
            ui->evLineEditDate->setStyleSheet("border: 1px solid red; background-color: #ffe4e4;");
        }
    });

    // Configure evTableEvents to stretch and fill available space
    if (ui->evTableEvents) {
        ui->evTableEvents->horizontalHeader()->setStretchLastSection(true);
        ui->evTableEvents->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        ui->evTableEvents->verticalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
        ui->evTableEvents->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding);
    }

    // Configure evTableSearchEvents to stretch and fill available space
    if (ui->evTableSearchEvents) {
        ui->evTableSearchEvents->horizontalHeader()->setStretchLastSection(true);
        ui->evTableSearchEvents->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);
        ui->evTableSearchEvents->verticalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
        ui->evTableSearchEvents->setSizePolicy(QSizePolicy::Expanding,
                                               QSizePolicy::Expanding);
    }

    // === Bouton Calculateur d'Impact → remplacé par Calendrier ===
    // On réutilise le slot evBtnCalculImpact pour afficher le calendrier
    ui->evBtnCalculImpact->setText("📅  Calendrier");
    ui->evBtnCalculImpact->setCursor(Qt::PointingHandCursor);
    ui->evBtnCalculImpact->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        " stop:0 #1e40af, stop:1 #0ea5e9); color: white; border: none;"
        " border-radius: 8px; padding: 8px 18px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        " stop:0 #1d4ed8, stop:1 #0284c7); }");

    // === Masquer le bouton Livre des Résumés ===
    ui->evBtnLivreResumes->setVisible(false);
}

void SmartPub::evConnectSignals() {
    connect(ui->evBtnAjouterEvent, &QPushButton::clicked, this,
            &SmartPub::on_evBtnAjouterEvent_clicked);
    connect(ui->evBtnModifierEvent, &QPushButton::clicked, this,
            &SmartPub::on_evBtnModifierEvent_clicked);
    connect(ui->evBtnSupprimerEvent, &QPushButton::clicked, this,
            &SmartPub::on_evBtnSupprimerEvent_clicked);
    connect(ui->evBtnTrierDate, &QPushButton::clicked, this,
            &SmartPub::on_evBtnTrierDate_clicked);
    connect(ui->evBtnRechercheLieu, &QPushButton::clicked, this,
            &SmartPub::on_evBtnRechercheLieu_clicked);
    connect(ui->evBtnExportCalendrier, &QPushButton::clicked, this,
            &SmartPub::on_evBtnExportCalendrier_clicked);
    connect(ui->evBtnLivreResumes, &QPushButton::clicked, this,
            &SmartPub::on_evBtnLivreResumes_clicked);
    connect(ui->evBtnCalculImpact, &QPushButton::clicked, this,
            &SmartPub::on_evBtnCalculImpact_clicked);
    connect(ui->evBtnStatsParticipation, &QPushButton::clicked, this,
            &SmartPub::on_evBtnStatsParticipation_clicked);

    // Clic dans la table => remplir les champs du formulaire (édition)
    connect(ui->evTableEvents, &QTableWidget::cellClicked, this,
            [this](int row, int column) {
                Q_UNUSED(column);

                if (row < 0)
                    return;

                ui->evTableEvents->selectRow(row);

                QTableWidgetItem *codeItem = ui->evTableEvents->item(row, 0);
                const QString code = codeItem->text().trimmed();
                if (code.isEmpty())
                    return;

                // Prépare le mode édition; le bouton "Modifier" remplira ensuite
                // les champs du formulaire.
                evEditingCode = code;
            });
}

void SmartPub::evAjouterDonneesTest() {
    evEditingCode.clear();
    evAfficherListeEvents();
}

void SmartPub::evAfficherListeEvents() {
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    QSignalBlocker b1(ui->evTableEvents);
    QSignalBlocker b2(ui->evTableSearchEvents);

    evEventsMap.clear();
    QSqlQuery query(db);
    if (!query.exec("SELECT ID_EVENEMENT, CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT ORDER BY DATE_EVENEMENT")) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les événements : " + query.lastError().text());
        return;
    }

    ui->evTableEvents->setRowCount(0);
    ui->evTableSearchEvents->setRowCount(0);

    while (query.next()) {
        EventData data;
        data.id = QString::number(query.value("ID_EVENEMENT").toLongLong());
        data.code = QString::number(query.value("CODE_EVENEMENT").toLongLong());
        data.nom = query.value("NOM").toString();
        data.lieu = query.value("LIEU").toString();
        QVariant dVal = query.value("DATE_EVENEMENT");
        if (dVal.canConvert<QDate>()) {
            data.date = dVal.toDate().toString("dd/MM/yyyy");
        } else {
            QString dStr = dVal.toString();
            if (dStr.contains("T")) dStr = dStr.left(10);
            if (dStr.contains("-") && dStr.length() >= 10) {
                QDate dt = QDate::fromString(dStr.left(10), "yyyy-MM-dd");
                if (dt.isValid()) data.date = dt.toString("dd/MM/yyyy");
                else data.date = dStr;
            } else {
                data.date = dStr;
            }
        }
        evEventsMap[data.code] = data;
        evAjouterEventTable(data);
        int row = ui->evTableSearchEvents->rowCount();
        ui->evTableSearchEvents->insertRow(row);
        ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(data.code));
        ui->evTableSearchEvents->setItem(row, 1, new QTableWidgetItem(data.nom));
        ui->evTableSearchEvents->setItem(row, 2, new QTableWidgetItem(data.lieu));
        ui->evTableSearchEvents->setItem(row, 3, new QTableWidgetItem(data.date));
    }
}

void SmartPub::evAjouterEventTable(const EventData &data) {
    int row = ui->evTableEvents->rowCount();
    ui->evTableEvents->insertRow(row);
    {
        QTableWidgetItem *codeItem = new QTableWidgetItem(data.code);
        codeItem->setFlags(codeItem->flags() & ~Qt::ItemIsEditable);
        codeItem->setData(Qt::UserRole, data.id);
        ui->evTableEvents->setItem(row, 0, codeItem);
    }
    {
        QTableWidgetItem *nomItem = new QTableWidgetItem(data.nom);
        nomItem->setFlags(nomItem->flags() & ~Qt::ItemIsEditable);
        nomItem->setData(Qt::UserRole, data.nom);
        ui->evTableEvents->setItem(row, 1, nomItem);
    }
    {
        QTableWidgetItem *lieuItem = new QTableWidgetItem(data.lieu);
        lieuItem->setFlags(lieuItem->flags() & ~Qt::ItemIsEditable);
        lieuItem->setData(Qt::UserRole, data.lieu);
        ui->evTableEvents->setItem(row, 2, lieuItem);
    }
    {
        QTableWidgetItem *dateItem = new QTableWidgetItem(data.date);
        dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);
        dateItem->setData(Qt::UserRole, data.date);
        ui->evTableEvents->setItem(row, 3, dateItem);
    }
}

void SmartPub::evRechercherParLieu() {
    QString lieu = ui->evLineEditSearchLieu->text().trimmed();
    if (lieu.isEmpty()) {
        QMessageBox::warning(this, "Recherche", "Veuillez entrer un lieu");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    ui->evTableSearchEvents->setRowCount(0);
    QSqlQuery query(db);
    query.prepare("SELECT ID_EVENEMENT, CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT WHERE UPPER(LIEU) LIKE UPPER(:lieu) ORDER BY DATE_EVENEMENT");
    query.bindValue(":lieu", "%" + lieu + "%");
    if (!query.exec()) {
        QMessageBox::warning(this, "Erreur", "Recherche échouée : " + query.lastError().text());
        return;
    }
    while (query.next()) {
        int row = ui->evTableSearchEvents->rowCount();
        ui->evTableSearchEvents->insertRow(row);
        QVariant dVal = query.value("DATE_EVENEMENT");
        QString dateStr;
        if (dVal.canConvert<QDate>()) {
            dateStr = dVal.toDate().toString("dd/MM/yyyy");
        } else {
            dateStr = dVal.toString();
            if (dateStr.contains("T")) dateStr = dateStr.left(10);
        }
        ui->evTableSearchEvents->setItem(row, 0, new QTableWidgetItem(QString::number(query.value("CODE_EVENEMENT").toLongLong())));
        ui->evTableSearchEvents->setItem(row, 1, new QTableWidgetItem(query.value("NOM").toString()));
        ui->evTableSearchEvents->setItem(row, 2, new QTableWidgetItem(query.value("LIEU").toString()));
        ui->evTableSearchEvents->setItem(row, 3, new QTableWidgetItem(dateStr));
    }
}

void SmartPub::handleEvBtnAjouterEventClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas ajouter d'événements.");
        return;
    }

    QString nom = ui->evLineEditNom->text().trimmed();
    QString lieu = ui->evLineEditLieu->text().trimmed();
    QString date = ui->evLineEditDate->text().trimmed();

    if (nom.isEmpty() || lieu.isEmpty() || date.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir nom, lieu et date");
        return;
    }

    const QDate parsedDate = QDate::fromString(date, "dd/MM/yyyy");
    if (!parsedDate.isValid() || parsedDate.toString("dd/MM/yyyy") != date) {
        QMessageBox::warning(this, "Erreur",
                             "Format de date invalide. Utilisez JJ/MM/AAAA.");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        return;
    }

    auto sqlErrorMessage = [](const QSqlError &error) {
        const QString native = error.nativeErrorCode().trimmed();
        if (native.isEmpty()) {
            return error.text();
        }
        return error.text() + " (Code natif: " + native + ")";
    };

    if (!evEditingCode.isEmpty()) {
        if (!evEventsMap.contains(evEditingCode)) {
            QMessageBox::critical(this, "Erreur",
                                  "Événement introuvable pour la modification.");
            return;
        }
        const QString idEvenement = evEventsMap.value(evEditingCode).id;
        if (idEvenement.isEmpty()) {
            QMessageBox::critical(this, "Erreur",
                                  "Identifiant d'événement invalide pour la modification.");
            return;
        }

        const QString originalDate =
            evEventsMap.contains(evEditingCode) ? evEventsMap.value(evEditingCode).date : QString();
        const bool dateChanged = (originalDate.isEmpty() ? true : (originalDate != date));

        // Workaround QODBC: certaines configs échouent avec requêtes préparées + bind (? / :nom).
        // On exécute un SQL littéral (avec échappement simple) uniquement pour la modification EVENEMENT.
        QString nomSql = nom;
        nomSql.replace('\'', "''");
        QString lieuSql = lieu;
        lieuSql.replace('\'', "''");

        QSqlQuery query(db);
        QString sql;
        if (dateChanged) {
            // Certaines configurations QODBC sont fragiles; on utilise un littéral contrôlé par validation.
            const QString dateLiteral = parsedDate.toString("dd/MM/yyyy");
            sql = "UPDATE EVENEMENT SET NOM = '" + nomSql + "', LIEU = '" + lieuSql + "', "
                  "DATE_EVENEMENT = TO_DATE('" + dateLiteral + "', 'DD/MM/YYYY') "
                  "WHERE ID_EVENEMENT = " + idEvenement;
        } else {
            // Si la date n'a pas changé, on évite d'attaquer DATE_EVENEMENT (ce qui corrige l'erreur pour Nom/Lieu).
            sql = "UPDATE EVENEMENT SET NOM = '" + nomSql + "', LIEU = '" + lieuSql + "' "
                  "WHERE ID_EVENEMENT = " + idEvenement;
        }

        if (!query.exec(sql)) {
            QMessageBox::critical(this, "Erreur",
                                  "Échec de la modification : " +
                                      sqlErrorMessage(query.lastError()) + "\nRequête: " +
                                      query.lastQuery() + "\nDB: " +
                                      query.lastError().databaseText() + "\nDriver: " +
                                      query.lastError().driverText());
            return;
        }
        if (query.numRowsAffected() == 0) {
            QMessageBox::warning(
                this, "Information",
                "Aucun événement modifié. Vérifiez la ligne sélectionnée.");
            return;
        }
        QMessageBox::information(this, "Succès", "Événement modifié avec succès !");
        evEditingCode.clear();
    } else {
        const QString codeStr = ui->evLineEditID->text().trimmed();
        if (codeStr.isEmpty()) {
            QMessageBox::warning(this, "Erreur",
                                 "Veuillez remplir le code de l'événement.");
            return;
        }
        bool codeOk = false;
        const qlonglong codeVal = codeStr.toLongLong(&codeOk);
        if (!codeOk) {
            QMessageBox::warning(this, "Erreur",
                                 "Le code doit être un nombre entier valide.");
            return;
        }

        // Même contournement que pour UPDATE : QODBC + Oracle échoue souvent sur INSERT préparé + bind.
        QString nomSql = nom;
        nomSql.replace('\'', "''");
        QString lieuSql = lieu;
        lieuSql.replace('\'', "''");
        const QString dateLiteral = parsedDate.toString("dd/MM/yyyy");
        const QString sql =
            "INSERT INTO EVENEMENT (CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT) VALUES ("
            + QString::number(codeVal) + ", '" + nomSql + "', '" + lieuSql + "', "
            "TO_DATE('" + dateLiteral + "', 'DD/MM/YYYY'))";

        QSqlQuery query(db);
        if (!query.exec(sql)) {
            QMessageBox::critical(
                this, "Erreur",
                "Échec de l'ajout : " + sqlErrorMessage(query.lastError()) + "\nRequête: " + sql
                    + "\nDB: " + query.lastError().databaseText() + "\nDriver: "
                    + query.lastError().driverText());
            return;
        }
        QMessageBox::information(this, "Succès", "Événement ajouté avec succès !");
    }

    evAfficherListeEvents();
    ui->evLineEditID->clear();
    ui->evLineEditNom->clear();
    ui->evLineEditLieu->clear();
    ui->evLineEditDate->clear();
    ui->evLineEditID->setEnabled(true);
}

void SmartPub::handleEvBtnModifierEventClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas modifier les événements.");
        return;
    }

    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner un événement à modifier");
        return;
    }

    QString code = ui->evTableEvents->item(currentRow, 0)->text();
    ui->evLineEditID->setText(code);
    ui->evLineEditNom->setText(ui->evTableEvents->item(currentRow, 1)->text());
    ui->evLineEditLieu->setText(ui->evTableEvents->item(currentRow, 2)->text());
    ui->evLineEditDate->setText(ui->evTableEvents->item(currentRow, 3)->text());
    ui->evLineEditID->setEnabled(false);
    evEditingCode = code;
}

void SmartPub::handleEvBtnSupprimerEventClicked() {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(
            this, "Accès refusé",
            "Les invités ne peuvent pas supprimer les événements.");
        return;
    }

    int currentRow = ui->evTableEvents->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez sélectionner un événement à supprimer");
        return;
    }

    auto reply = QMessageBox::question(
        this, "Supprimer", "Confirmer la suppression de cet événement ?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QString code = ui->evTableEvents->item(currentRow, 0)->text();
        QTableWidgetItem *codeCell = ui->evTableEvents->item(currentRow, 0);
        QString idStr = codeCell ? codeCell->data(Qt::UserRole).toString().trimmed() : QString();
        if (idStr.isEmpty())
            idStr = evEventsMap.value(code.trimmed()).id;

        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) {
            return;
        }
        bool ok = false;
        const qlonglong idNum = idStr.toLongLong(&ok);
        if (!ok || idStr.isEmpty()) {
            QMessageBox::critical(this, "Erreur", "Identifiant d'événement invalide.");
            return;
        }

        // Workaround QODBC: certaines configs échouent avec requêtes préparées + bind.
        // On supprime d'abord les dépendances éventuelles, puis l'événement.
        QSqlQuery query(db);
        query.exec("DELETE FROM PARTICIPER WHERE ID_EVENEMENT = " + QString::number(idNum));

        if (!query.exec("DELETE FROM EVENEMENT WHERE ID_EVENEMENT = " + QString::number(idNum))) {
            QMessageBox::critical(
                this, "Erreur",
                "Échec de la suppression : " + query.lastError().text() +
                    "\nRequête: " + query.lastQuery() + "\nDB: " +
                    query.lastError().databaseText() + "\nDriver: " +
                    query.lastError().driverText());
            return;
        }
        if (evEditingCode == code) {
            evEditingCode.clear();
            ui->evLineEditID->clear();
            ui->evLineEditNom->clear();
            ui->evLineEditLieu->clear();
            ui->evLineEditDate->clear();
            ui->evLineEditID->setEnabled(true);
        }
        evAfficherListeEvents();
        QMessageBox::information(this, "Succès", "Événement supprimé");
    }
}

void SmartPub::handleEvBtnTrierDateClicked() {
    QMessageBox::information(this, "Tri", "Événements triés par date");
}

void SmartPub::handleEvBtnRechercheLieuClicked() { evRechercherParLieu(); }

void SmartPub::handleEvBtnExportCalendrierClicked() {
    // ── 1. Charger les événements depuis la BD ────────────────────────────────
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Erreur", "Base de données non connectée.");
        return;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT ORDER BY DATE_EVENEMENT")) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les événements : " + query.lastError().text());
        return;
    }

    struct EvExport { QString code, nom, lieu, date; };
    QList<EvExport> tousLesEv;
    while (query.next()) {
        EvExport e;
        e.code = QString::number(query.value(0).toLongLong());
        e.nom  = query.value(1).toString();
        e.lieu = query.value(2).toString();
        QVariant dv = query.value(3);
        QDate d = dv.toDate();
        if (!d.isValid() && dv.toDateTime().isValid()) d = dv.toDateTime().date();
        if (!d.isValid()) {
            QString s = dv.toString();
            if (s.contains("T")) s = s.left(10);
            d = QDate::fromString(s.left(10), "yyyy-MM-dd");
        }
        e.date = d.isValid() ? d.toString("dd/MM/yyyy") : dv.toString();
        tousLesEv.append(e);
    }

    if (tousLesEv.isEmpty()) {
        QMessageBox::information(this, "Export", "Aucun événement à exporter.");
        return;
    }

    // ── 2. Dialog de sélection des événements ─────────────────────────────────
    QDialog selDialog(this);
    selDialog.setWindowTitle("Sélectionner les événements à exporter");
    selDialog.setMinimumSize(500, 420);
    selDialog.resize(560, 460);
    selDialog.setStyleSheet(
        "QDialog { background-color: #f8fafc; }"
        "QLabel  { color: #1e293b; }"
        "QListWidget { background: white; border: 2px solid #e2e8f0; border-radius: 10px;"
        " padding: 6px; font-size: 13px; color: #334155; }"
        "QListWidget::item { padding: 8px 10px; border-radius: 6px; }"
        "QListWidget::item:hover { background-color: #eff6ff; }"
        "QListWidget::item:selected { background-color: #dbeafe; color: #1e40af; font-weight: 600; }");

    QVBoxLayout *selLay = new QVBoxLayout(&selDialog);
    selLay->setContentsMargins(20, 20, 20, 20);
    selLay->setSpacing(12);

    QLabel *selTitle = new QLabel("Cochez les événements à inclure dans l'export :");
    selTitle->setStyleSheet("font-size: 14px; font-weight: 600; color: #1e293b;");
    selLay->addWidget(selTitle);

    // Boutons Tout sélectionner / Tout désélectionner
    QHBoxLayout *selBtnRow = new QHBoxLayout();
    QPushButton *btnAll  = new QPushButton("✔  Tout sélectionner");
    QPushButton *btnNone = new QPushButton("✘  Tout désélectionner");
    for (QPushButton *b : {btnAll, btnNone}) {
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton { background: #f1f5f9; color: #334155; border: 1px solid #cbd5e1;"
            " border-radius: 6px; padding: 6px 14px; font-size: 12px; }"
            "QPushButton:hover { background: #e2e8f0; }");
    }
    selBtnRow->addWidget(btnAll);
    selBtnRow->addWidget(btnNone);
    selBtnRow->addStretch();
    selLay->addLayout(selBtnRow);

    QListWidget *listWidget = new QListWidget();
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    for (const EvExport &e : tousLesEv) {
        QListWidgetItem *item = new QListWidgetItem(
            QString("📅  %1  —  %2  |  📍 %3  |  🗓 %4")
                .arg(e.code, e.nom, e.lieu, e.date));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        listWidget->addItem(item);
    }
    selLay->addWidget(listWidget, 1);

    QObject::connect(btnAll,  &QPushButton::clicked, &selDialog, [&]() {
        for (int i = 0; i < listWidget->count(); ++i)
            listWidget->item(i)->setCheckState(Qt::Checked);
    });
    QObject::connect(btnNone, &QPushButton::clicked, &selDialog, [&]() {
        for (int i = 0; i < listWidget->count(); ++i)
            listWidget->item(i)->setCheckState(Qt::Unchecked);
    });

    QHBoxLayout *selFooter = new QHBoxLayout();
    QPushButton *btnAnnuler  = new QPushButton("Annuler");
    QPushButton *btnExporter = new QPushButton("Exporter →");
    btnAnnuler->setFixedSize(110, 38);
    btnExporter->setFixedSize(130, 38);
    btnAnnuler->setCursor(Qt::PointingHandCursor);
    btnExporter->setCursor(Qt::PointingHandCursor);
    btnAnnuler->setStyleSheet(
        "QPushButton { background: #f1f5f9; color: #334155; border: 1px solid #cbd5e1;"
        " border-radius: 8px; font-size: 13px; }"
        "QPushButton:hover { background: #e2e8f0; }");
    btnExporter->setStyleSheet(
        "QPushButton { background: #3b82f6; color: white; border: none;"
        " border-radius: 8px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #2563eb; }");
    QObject::connect(btnAnnuler,  &QPushButton::clicked, &selDialog, &QDialog::reject);
    QObject::connect(btnExporter, &QPushButton::clicked, &selDialog, &QDialog::accept);
    selFooter->addStretch();
    selFooter->addWidget(btnAnnuler);
    selFooter->addWidget(btnExporter);
    selLay->addLayout(selFooter);

    if (selDialog.exec() != QDialog::Accepted)
        return;

    // Récupérer uniquement les événements cochés
    QList<EvExport> liste;
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->checkState() == Qt::Checked)
            liste.append(tousLesEv[i]);
    }
    if (liste.isEmpty()) {
        QMessageBox::information(this, "Export", "Aucun événement sélectionné.");
        return;
    }

    // ── 3. Choix du fichier ───────────────────────────────────────────────────
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter les événements",
        QDir::homePath() + "/evenements_export",
        "Fichier texte (*.txt);;PDF (*.pdf);;CSV (*.csv)");
    if (fileName.isEmpty())
        return;

    // ── 4. Export TXT — template style poster ────────────────────────────────
    if (fileName.endsWith(".txt", Qt::CaseInsensitive)) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier.");
            return;
        }
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);

        const QString W72  = QString(72, '*');
        const QString W72d = QString(72, '-');
        const QString W72e = QString(72, '=');

        // ╔══ EN-TÊTE GLOBAL ══╗
        out << "\n" << W72 << "\n";
        out << "*" << QString(70, ' ') << "*\n";
        out << "*" << "          ███████╗███╗   ███╗ █████╗ ██████╗ ████████╗██████╗ ██╗   ██╗██████╗ " << "*\n";
        out << "*" << "          ██╔════╝████╗ ████║██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗██║   ██║██╔══██╗" << "*\n";
        out << "*" << "          ███████╗██╔████╔██║███████║██████╔╝   ██║   ██████╔╝██║   ██║██████╔╝" << "*\n";
        out << "*" << "          ╚════██║██║╚██╔╝██║██╔══██║██╔══██╗   ██║   ██╔═══╝ ██║   ██║██╔══██╗" << "*\n";
        out << "*" << "          ███████║██║ ╚═╝ ██║██║  ██║██║  ██║   ██║   ██║     ╚██████╔╝██████╔╝" << "*\n";
        out << "*" << "          ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝      ╚═════╝ ╚═════╝ " << "*\n";
        out << "*" << QString(70, ' ') << "*\n";
        out << "*" << "                    G E S T I O N   D E S   E V E N E M E N T S                  " << "*\n";
        out << "*" << "                           Plateforme SmartPub — ESPRIT                           " << "*\n";
        out << "*" << QString(70, ' ') << "*\n";
        out << W72 << "\n\n";

        out << "  Exporté le  : " << QDateTime::currentDateTime().toString("dddd dd MMMM yyyy  —  HH:mm:ss") << "\n";
        out << "  Événements  : " << liste.size() << " sélectionné(s) sur " << tousLesEv.size() << " au total\n";
        out << "\n" << W72e << "\n\n";

        // ╔══ FICHE PAR ÉVÉNEMENT ══╗
        int num = 1;
        for (const EvExport &e : liste) {
            // Bannière de l'événement
            out << "  " << W72d << "\n";
            out << "  |" << QString(70, ' ') << "|\n";

            // Centrer le nom
            QString nomLine = QString("  E V E N E M E N T   #%1  —  %2").arg(num).arg(e.nom.toUpper());
            int pad = qMax(0, (72 - nomLine.length()) / 2);
            out << QString(pad, ' ') << nomLine << "\n";

            out << "  |" << QString(70, ' ') << "|\n";
            out << "  " << W72d << "\n\n";

            // Détails style badge
            out << "      ┌─────────────────────────────────────────────────────────┐\n";

            auto ligne = [&](const QString &label, const QString &valeur) {
                QString contenu = QString("  %1  :  %2").arg(label, -8).arg(valeur);
                // Tronquer si trop long, puis padder à 55 chars
                if (contenu.length() > 55) contenu = contenu.left(52) + "...";
                int padding = 55 - contenu.length();
                out << "      │" << contenu << QString(padding, ' ') << "│\n";
            };

            ligne("CODE", e.code);
            ligne("NOM",  e.nom);
            ligne("LIEU", e.lieu);
            ligne("DATE", e.date);

            out << "      └─────────────────────────────────────────────────────────┘\n\n";

            ++num;
        }

        // ╔══ PIED DE PAGE ══╗
        out << "\n" << W72 << "\n";
        out << "*" << QString(70, ' ') << "*\n";
        out << "*" << "         Ce document est généré automatiquement par SmartPub.          " << "*\n";
        out << "*" << "              Toute modification manuelle est déconseillée.            " << "*\n";
        out << "*" << QString(70, ' ') << "*\n";
        out << W72 << "\n";

        file.close();
        QMessageBox::information(this, "Export réussi",
                                 QString("%1 événement(s) exporté(s) avec succès !\n\n%2")
                                     .arg(liste.size()).arg(fileName));
        return;
    }

    // ── 5. Export CSV ─────────────────────────────────────────────────────────
    if (fileName.endsWith(".csv", Qt::CaseInsensitive)) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Erreur", "Impossible de créer le fichier.");
            return;
        }
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << "Code;Nom;Lieu;Date\n";
        for (const EvExport &e : liste)
            out << e.code << ";" << e.nom << ";" << e.lieu << ";" << e.date << "\n";
        file.close();
        QMessageBox::information(this, "Export réussi",
                                 QString("%1 événement(s) exporté(s) !\n\n%2")
                                     .arg(liste.size()).arg(fileName));
        return;
    }

    // ── 6. Export PDF — brochure style ───────────────────────────────────────
    if (fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        QPrinter printer(QPrinter::ScreenResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageOrientation(QPageLayout::Portrait);
        printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

        QPainter p;
        if (!p.begin(&printer)) {
            QMessageBox::critical(this, "Erreur", "Impossible d'initialiser le moteur PDF.");
            return;
        }

        const QRect page = printer.pageRect(QPrinter::DevicePixel).toRect();
        const int PW = page.width();
        const int PH = page.height();

        // BROCHURE LAYOUT — all sizes proportional to PW/PH
        const int MX   = PW * 7  / 100;   // marge gauche/droite
        const int FW   = PW - 2 * MX;

        // Font sizes proportional
        const int FS_LOGO    = PW * 45 / 1000;
        const int FS_LOGOSUB = PW * 8  / 1000;
        const int FS_EVNAME  = PW * 32 / 1000;
        const int FS_EVENT   = PW * 55 / 1000;
        const int FS_DATE    = PW * 14 / 1000;
        const int FS_LIEU    = PW * 11 / 1000;
        const int FS_PILL    = PW * 10 / 1000;
        const int FS_CTITLE  = PW * 10 / 1000;
        const int FS_CLABEL  = PW * 8  / 1000;
        const int FS_CVAL    = PW * 9  / 1000;
        const int FS_FOOT    = PW * 8  / 1000;

        // Colours
        const QColor cBg    ("#eef2f7");
        const QColor cCyan  ("#00bcd4");
        const QColor cPink  ("#e91e8c");
        const QColor cPurple("#9c27b0");
        const QColor cNavy  ("#1a237e");
        const QColor cWhite (Qt::white);
        const QColor cGray  ("#607d8b");
        const QColor cDark  ("#263238");

        auto fnt = [](int sz, bool bold=false, bool italic=false) {
            QFont f("Arial", sz); f.setBold(bold); f.setItalic(italic); return f;
        };

        for (int idx = 0; idx < liste.size(); ++idx) {
            const EvExport &e = liste[idx];
            if (idx > 0) printer.newPage();

            // Background light grey-blue
            p.setPen(Qt::NoPen); p.setBrush(cBg);
            p.drawRect(0, 0, PW, PH);

            // Top gradient band cyan->purple->pink
            QLinearGradient topG(0, 0, PW, 0);
            topG.setColorAt(0.0, cCyan);
            topG.setColorAt(0.5, cPurple);
            topG.setColorAt(1.0, cPink);
            int bandH = PH * 13 / 100;
            p.setBrush(topG); p.setPen(Qt::NoPen);
            p.drawRect(0, 0, PW, bandH);

            // Wave under band
            QPainterPath wave;
            wave.moveTo(0, bandH);
            wave.cubicTo(PW/4, bandH + PH*5/100,
                         PW*3/4, bandH - PH*4/100,
                         PW, bandH);
            wave.lineTo(PW, 0); wave.lineTo(0, 0); wave.closeSubpath();
            p.setBrush(topG); p.setPen(Qt::NoPen);
            p.drawPath(wave);

            // Logo "SmartPub" top-left
            int lx = MX, ly = PH*2/100;
            int lsz = PH*8/100;
            p.setBrush(QColor(255,255,255,55)); p.setPen(Qt::NoPen);
            p.drawRoundedRect(lx, ly, lsz, lsz, lsz/5, lsz/5);
            p.setFont(fnt(FS_LOGO, true));
            p.setPen(cWhite);
            p.drawText(QRect(lx, ly, lsz, lsz), Qt::AlignCenter, "S");
            p.setFont(fnt(FS_LOGOSUB+2, true));
            p.drawText(QRect(lx+lsz+PW*2/100, ly, PW*30/100, lsz/2),
                       Qt::AlignLeft|Qt::AlignVCenter, "SmartPub");
            p.setFont(fnt(FS_LOGOSUB));
            p.setPen(QColor(255,255,255,200));
            p.drawText(QRect(lx+lsz+PW*2/100, ly+lsz/2, PW*30/100, lsz/2),
                       Qt::AlignLeft|Qt::AlignVCenter, "ESPRIT");

            // Concentric circles
            int cx = PW/2, cy2 = PH*40/100;
            p.setBrush(Qt::NoBrush);
            for (int r = 5; r >= 1; --r) {
                int rad = PW*(8+r*7)/100;
                QColor rc = (r%2==0)
                    ? QColor(0,188,212, 15+r*10)
                    : QColor(233,30,140, 12+r*8);
                p.setPen(QPen(rc, PW*15/10000));
                p.drawEllipse(cx-rad, cy2-rad, rad*2, rad*2);
            }

            // Central radial gradient circle
            int mainR = PW*20/100;
            QRadialGradient radG(cx, cy2, mainR);
            radG.setColorAt(0.0, QColor("#7b1fa2"));
            radG.setColorAt(0.5, QColor("#0097a7"));
            radG.setColorAt(1.0, QColor("#e91e8c"));
            p.setPen(Qt::NoPen); p.setBrush(radG);
            p.drawEllipse(cx-mainR, cy2-mainR, mainR*2, mainR*2);

            // Text inside circle
            p.setFont(fnt(FS_EVNAME, true));
            p.setPen(cWhite);
            p.drawText(QRect(cx-mainR, cy2-mainR, mainR*2, mainR),
                       Qt::AlignCenter|Qt::AlignBottom, "SmartPub");
            p.setFont(fnt(FS_EVENT, true));
            p.drawText(QRect(cx-mainR, cy2, mainR*2, mainR),
                       Qt::AlignCenter|Qt::AlignTop, "EVENT");

            // Event name below circle
            int iy = cy2 + mainR + PH*3/100;
            p.setFont(fnt(FS_DATE, true));
            p.setPen(cDark);
            p.drawText(QRect(MX, iy, FW, PH*5/100),
                       Qt::AlignCenter, e.nom.toUpper());

            // Date
            iy += PH*5/100;
            p.setFont(fnt(FS_DATE, true));
            p.setPen(cDark);
            p.drawText(QRect(MX, iy, FW, PH*4/100),
                       Qt::AlignCenter, e.date.toUpper());

            // Lieu in cyan
            iy += PH*4/100;
            p.setFont(fnt(FS_LIEU, true));
            p.setPen(cCyan);
            p.drawText(QRect(MX, iy, FW, PH*3/100),
                       Qt::AlignCenter, e.lieu.toUpper());

            // Code pill badge
            iy += PH*4/100;
            int pw2 = FW*30/100, ph2 = PH*4/100;
            int px2 = PW/2 - pw2/2;
            QLinearGradient pillG(px2, 0, px2+pw2, 0);
            pillG.setColorAt(0, cCyan); pillG.setColorAt(1, cPink);
            p.setPen(Qt::NoPen); p.setBrush(pillG);
            p.drawRoundedRect(px2, iy, pw2, ph2, ph2/2, ph2/2);
            p.setFont(fnt(FS_PILL, true));
            p.setPen(cWhite);
            p.drawText(QRect(px2, iy, pw2, ph2),
                       Qt::AlignCenter, "CODE  :  " + e.code);

            // Separator
            iy += PH*6/100;
            QLinearGradient sepG(MX, 0, MX+FW, 0);
            sepG.setColorAt(0, Qt::transparent);
            sepG.setColorAt(0.5, QColor(0,188,212,160));
            sepG.setColorAt(1, Qt::transparent);
            p.setBrush(sepG); p.setPen(Qt::NoPen);
            p.drawRect(MX, iy, FW, PH*2/1000);

            // Two info columns
            iy += PH*3/100;
            int cw = FW*46/100;
            int c1x = MX, c2x = MX+FW-cw;
            int ch = PH*17/100;

            p.setPen(Qt::NoPen); p.setBrush(cWhite);
            p.drawRoundedRect(c1x, iy, cw, ch, PW*1/100, PW*1/100);
            p.drawRoundedRect(c2x, iy, cw, ch, PW*1/100, PW*1/100);

            p.setBrush(cCyan);
            p.drawRoundedRect(c1x, iy, cw, PH*5/1000, PW*1/100, PW*1/100);
            p.setBrush(cPink);
            p.drawRoundedRect(c2x, iy, cw, PH*5/1000, PW*1/100, PW*1/100);

            p.setFont(fnt(FS_CTITLE, true));
            p.setPen(cCyan);
            p.drawText(QRect(c1x+PW*2/100, iy+PH*15/1000, cw, PH*4/100),
                       Qt::AlignLeft|Qt::AlignVCenter, "INFORMATIONS");
            p.setPen(cPink);
            p.drawText(QRect(c2x+PW*2/100, iy+PH*15/1000, cw, PH*4/100),
                       Qt::AlignLeft|Qt::AlignVCenter, "DETAILS");

            p.setPen(QPen(QColor("#e0e0e0"), 1));
            p.drawLine(c1x+PW*2/100, iy+PH*6/100, c1x+cw-PW*2/100, iy+PH*6/100);
            p.drawLine(c2x+PW*2/100, iy+PH*6/100, c2x+cw-PW*2/100, iy+PH*6/100);

            auto colItem = [&](int x, int y, int w,
                               const QString &lbl, const QString &val,
                               const QColor &dc) {
                int dr = PH*10/1000;
                p.setPen(Qt::NoPen); p.setBrush(dc);
                p.drawEllipse(x+PW*2/100, y+dr/4, dr, dr);
                p.setFont(fnt(FS_CLABEL, true));
                p.setPen(cGray);
                p.drawText(QRect(x+PW*5/100, y, w-PW*5/100, PH*3/100),
                           Qt::AlignLeft|Qt::AlignVCenter, lbl);
                p.setFont(fnt(FS_CVAL));
                p.setPen(cDark);
                p.drawText(QRect(x+PW*5/100, y+PH*3/100, w-PW*5/100, PH*3/100),
                           Qt::AlignLeft|Qt::AlignVCenter, val);
            };

            int rit = iy + PH*7/100;
            colItem(c1x, rit,          cw, "Nom",  e.nom,  cCyan);
            colItem(c1x, rit+PH*6/100, cw, "Lieu", e.lieu, cPurple);
            colItem(c2x, rit,          cw, "Date", e.date, cPink);
            colItem(c2x, rit+PH*6/100, cw, "Code", e.code, cNavy);

            // Footer
            QLinearGradient footG(0, 0, PW, 0);
            footG.setColorAt(0, cNavy); footG.setColorAt(1, QColor("#37474f"));
            p.setPen(Qt::NoPen); p.setBrush(footG);
            p.drawRect(0, PH*93/100, PW, PH*7/100);
            p.setFont(fnt(FS_FOOT));
            p.setPen(cWhite);
            p.drawText(QRect(0, PH*93/100, PW, PH*7/100), Qt::AlignCenter,
                       QString("smartpub.esprit.tn   |   Page %1 / %2")
                           .arg(idx+1).arg(liste.size()));
            p.setBrush(topG); p.setPen(Qt::NoPen);
            p.drawRect(0, PH-PH*5/1000, PW, PH*5/1000);
        }

        p.end();
        QMessageBox::information(this, "Export réussi",
                                 QString("%1 événement(s) exporté(s) en PDF !\n\n%2")
                                     .arg(liste.size()).arg(fileName));
    }
}

void SmartPub::handleEvBtnLivreResumesClicked() {
    QMessageBox::information(this, "Livre des Résumés",
                             "Génération du livre des résumés - À implémenter");
}

void SmartPub::handleEvBtnCalculImpactClicked() {
    CalendarDialog *dlg = new CalendarDialog(this);
    dlg->exec();
}

void SmartPub::handleEvBtnStatsParticipationClicked() {
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::warning(this, "Erreur", "Base de données non connectée.");
        return;
    }

    // ── Charger les données depuis la BD ──────────────────────────────────────
    QSqlQuery q(db);
    q.exec("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT ORDER BY DATE_EVENEMENT");

    QMap<QString, int> parLieu;
    QMap<QString, int> parMois;
    QMap<QString, int> parAnnee;
    int total = 0;

    while (q.next()) {
        QString lieu = q.value(2).toString().trimmed();
        if (lieu.isEmpty()) lieu = "(non renseigné)";
        parLieu[lieu]++;

        QVariant dv = q.value(3);
        QDate d = dv.toDate();
        if (!d.isValid() && dv.toDateTime().isValid()) d = dv.toDateTime().date();
        if (!d.isValid()) {
            QString s = dv.toString();
            if (s.contains("T")) s = s.left(10);
            d = QDate::fromString(s.left(10), "yyyy-MM-dd");
        }
        if (d.isValid()) {
            const QStringList moisNoms = {"Jan","Fév","Mar","Avr","Mai","Jun",
                                          "Jul","Aoû","Sep","Oct","Nov","Déc"};
            parMois[moisNoms[d.month()-1] + " " + QString::number(d.year())]++;
            parAnnee[QString::number(d.year())]++;
        }
        total++;
    }

    // ── Créer le dialog ───────────────────────────────────────────────────────
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques — Événements");
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect av = screen->availableGeometry();
        int w = qMax(860, qRound(av.width()  * 0.85));
        int h = qMax(600, qRound(av.height() * 0.85));
        dialog->resize(w, h);
        dialog->move(av.center() - QPoint(w/2, h/2));
    } else {
        dialog->resize(960, 680);
    }
    dialog->setMinimumSize(760, 540);
    dialog->setStyleSheet(R"(
        QDialog { background-color: #f8fafc; }
        QLabel  { color: #1e293b; background: transparent; border: none; }
        QTabWidget::pane {
            border: 2px solid #e2e8f0; border-radius: 12px;
            background-color: white; margin-top: -1px;
        }
        QTabBar::tab {
            padding: 10px 20px; font-weight: 600; color: #64748b;
            background-color: #f1f5f9; border: 1px solid #e2e8f0;
            border-bottom: none; border-radius: 8px 8px 0 0;
            margin-right: 4px; min-width: 140px;
        }
        QTabBar::tab:selected {
            color: #1e40af; background-color: #ffffff;
            border-bottom: 3px solid #3b82f6;
        }
        QTabBar::tab:hover:!selected { background-color: #e2e8f0; color: #334155; }
        QScrollBar:vertical {
            border: none; background: #f8fafc; width: 10px; border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #cbd5e1; min-height: 30px; border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover { background: #94a3b8; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Titre
    QLabel *titleLabel = new QLabel("Tableau de bord — Événements");
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: #1e293b;");
    mainLayout->addWidget(titleLabel);

    // ── Cartes KPI ────────────────────────────────────────────────────────────
    auto createStatCard = [](const QString &title, const QString &value,
                             const QString &color) -> QFrame* {
        QFrame *card = new QFrame();
        card->setStyleSheet("QFrame { background-color: white; border-radius: 16px;"
                            " border: 1px solid #e2e8f0; }");
        card->setMinimumHeight(120);
        QVBoxLayout *l = new QVBoxLayout(card);
        l->setContentsMargins(20,20,20,20); l->setSpacing(8);
        QLabel *t = new QLabel(title);
        t->setStyleSheet("color: #64748b; font-size: 13px; font-weight: 600;");
        QLabel *v = new QLabel(value);
        v->setStyleSheet(QString("color: %1; font-size: 40px; font-weight: 700;").arg(color));
        l->addWidget(t); l->addWidget(v); l->addStretch();
        return card;
    };

    int nbLieux = parLieu.size();
    int nbAnnees = parAnnee.size();

    QGridLayout *kpiGrid = new QGridLayout();
    kpiGrid->setSpacing(16);
    kpiGrid->addWidget(createStatCard("Total événements",  QString::number(total),   "#3b82f6"), 0, 0);
    kpiGrid->addWidget(createStatCard("Lieux distincts",   QString::number(nbLieux), "#10b981"), 0, 1);
    kpiGrid->addWidget(createStatCard("Années couvertes",  QString::number(nbAnnees),"#f59e0b"), 0, 2);

    // ── Helper : créer un bar chart ───────────────────────────────────────────
    auto makeBarChart = [](const QString &title, const QStringList &cats,
                           const QList<double> &vals, const QString &colorHex,
                           const QString &yLabel) -> QChartView* {
        QBarSet *set = new QBarSet("Valeur");
        for (double v : vals) *set << v;
        set->setColor(QColor(colorHex));
        set->setBorderColor(QColor(colorHex).darker(110));

        QBarSeries *series = new QBarSeries();
        series->append(set);
        series->setBarWidth(0.65);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(title);
        chart->setTitleFont(QFont("Segoe UI", 11, QFont::DemiBold));
        chart->setTitleBrush(QBrush(QColor("#1e293b")));
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->setPlotAreaBackgroundBrush(QBrush(Qt::white));
        chart->setPlotAreaBackgroundVisible(true);
        chart->legend()->setVisible(false);
        chart->setMargins(QMargins(8,8,8,8));

        QBarCategoryAxis *axX = new QBarCategoryAxis();
        for (const QString &c : cats) axX->append(c);
        axX->setLabelsAngle(-25);
        axX->setLabelsBrush(QBrush(QColor("#475569")));
        axX->setGridLinePen(QPen(QColor("#f1f5f9")));
        chart->addAxis(axX, Qt::AlignBottom);
        series->attachAxis(axX);

        QValueAxis *axY = new QValueAxis();
        double vmax = 1.0;
        for (double v : vals) vmax = qMax(vmax, v);
        axY->setRange(0, vmax * 1.2 + 0.5);
        axY->setLabelFormat("%.0f");
        axY->setTitleText(yLabel);
        axY->setLabelsBrush(QBrush(QColor("#475569")));
        axY->setGridLinePen(QPen(QColor("#f1f5f9")));
        chart->addAxis(axY, Qt::AlignLeft);
        series->attachAxis(axY);

        QChartView *cv = new QChartView();
        cv->setChart(chart);
        cv->setRenderHint(QPainter::Antialiasing);
        cv->setMinimumHeight(300);
        cv->setStyleSheet("background: transparent; border: none;");
        cv->setBackgroundBrush(QBrush(Qt::transparent));
        return cv;
    };

    // ── Helper : créer un pie chart ───────────────────────────────────────────
    auto makePieChart = [](const QString &title,
                           const QMap<QString,int> &data) -> QChartView* {
        QPieSeries *series = new QPieSeries();
        const QStringList colors = {"#3b82f6","#10b981","#f59e0b","#ef4444",
                                    "#8b5cf6","#06b6d4","#f97316","#84cc16"};
        int ci = 0;
        for (auto it = data.constBegin(); it != data.constEnd(); ++it, ++ci) {
            QPieSlice *slice = series->append(
                QString("%1 (%2)").arg(it.key()).arg(it.value()), it.value());
            slice->setColor(QColor(colors[ci % colors.size()]));
            slice->setLabelVisible(true);
            slice->setLabelColor(QColor("#334155"));
        }
        series->setHoleSize(0.38);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(title);
        chart->setTitleFont(QFont("Segoe UI", 11, QFont::DemiBold));
        chart->setTitleBrush(QBrush(QColor("#1e293b")));
        chart->setAnimationOptions(QChart::SeriesAnimations);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->legend()->setAlignment(Qt::AlignRight);
        chart->setMargins(QMargins(8,8,8,8));

        QChartView *cv = new QChartView();
        cv->setChart(chart);
        cv->setRenderHint(QPainter::Antialiasing);
        cv->setMinimumHeight(300);
        cv->setStyleSheet("background: transparent; border: none;");
        cv->setBackgroundBrush(QBrush(Qt::transparent));
        return cv;
    };

    // ── Onglets ───────────────────────────────────────────────────────────────
    QTabWidget *tabs = new QTabWidget(dialog);
    tabs->setDocumentMode(true);

    // --- Onglet Vue d'ensemble ---
    QWidget *tabOverview = new QWidget();
    tabOverview->setStyleSheet("background-color: #f8fafc;");
    QScrollArea *scrollOv = new QScrollArea();
    scrollOv->setWidgetResizable(true);
    scrollOv->setFrameShape(QFrame::NoFrame);
    QWidget *ovContent = new QWidget();
    ovContent->setStyleSheet("background-color: #f8fafc;");
    QVBoxLayout *ovLay = new QVBoxLayout(ovContent);
    ovLay->setSpacing(20); ovLay->setContentsMargins(0,8,0,8);
    ovLay->addLayout(kpiGrid);

    // Graphique : événements par lieu
    if (!parLieu.isEmpty()) {
        QStringList lieux; QList<double> cntLieux;
        for (auto it = parLieu.constBegin(); it != parLieu.constEnd(); ++it) {
            lieux << it.key(); cntLieux << it.value();
        }
        QFrame *f = new QFrame();
        f->setStyleSheet("QFrame { background: white; border-radius: 16px; border: 1px solid #e2e8f0; }");
        QVBoxLayout *fl = new QVBoxLayout(f); fl->setContentsMargins(16,16,16,16);
        QLabel *ft = new QLabel("Événements par lieu");
        ft->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
        fl->addWidget(ft);
        fl->addWidget(makeBarChart("", lieux, cntLieux, "#3b82f6", "Nombre"));
        ovLay->addWidget(f);
    }

    // Graphique : événements par année
    if (!parAnnee.isEmpty()) {
        QStringList annees; QList<double> cntAnnees;
        QStringList anneesTriees = parAnnee.keys();
        std::sort(anneesTriees.begin(), anneesTriees.end());
        for (const QString &a : anneesTriees) { annees << a; cntAnnees << parAnnee[a]; }
        QFrame *f = new QFrame();
        f->setStyleSheet("QFrame { background: white; border-radius: 16px; border: 1px solid #e2e8f0; }");
        QVBoxLayout *fl = new QVBoxLayout(f); fl->setContentsMargins(16,16,16,16);
        QLabel *ft = new QLabel("Événements par année");
        ft->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
        fl->addWidget(ft);
        fl->addWidget(makeBarChart("", annees, cntAnnees, "#10b981", "Nombre"));
        ovLay->addWidget(f);
    }

    ovLay->addStretch();
    scrollOv->setWidget(ovContent);
    QVBoxLayout *ovTabLay = new QVBoxLayout(tabOverview);
    ovTabLay->setContentsMargins(0,0,0,0);
    ovTabLay->addWidget(scrollOv);
    tabs->addTab(tabOverview, "📊  Vue d'ensemble");

    // --- Onglet Répartition par lieu (pie) ---
    if (!parLieu.isEmpty()) {
        QWidget *tabLieu = new QWidget();
        tabLieu->setStyleSheet("background-color: #f8fafc;");
        QVBoxLayout *lieuLay = new QVBoxLayout(tabLieu);
        lieuLay->setContentsMargins(16,16,16,16);
        QLabel *lt = new QLabel("Répartition des événements par lieu");
        lt->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
        lieuLay->addWidget(lt);
        lieuLay->addWidget(makePieChart("", parLieu));
        tabs->addTab(tabLieu, "📍  Par lieu");
    }

    // --- Onglet Chronologie par mois ---
    if (!parMois.isEmpty()) {
        QWidget *tabMois = new QWidget();
        tabMois->setStyleSheet("background-color: #f8fafc;");
        QVBoxLayout *moisLay = new QVBoxLayout(tabMois);
        moisLay->setContentsMargins(16,16,16,16);
        QLabel *mt = new QLabel("Chronologie mensuelle des événements");
        mt->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
        moisLay->addWidget(mt);

        QStringList moisKeys = parMois.keys();
        QList<double> moisVals;
        for (const QString &k : moisKeys) moisVals << parMois[k];
        moisLay->addWidget(makeBarChart("", moisKeys, moisVals, "#8b5cf6", "Nombre"));
        tabs->addTab(tabMois, "📅  Par mois");
    }

    mainLayout->addWidget(tabs, 1);

    // Bouton Fermer
    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setFixedSize(120, 40);
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none;"
        " border-radius: 8px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    connect(btnFermer, &QPushButton::clicked, dialog, &QDialog::accept);
    QHBoxLayout *footerLay = new QHBoxLayout();
    footerLay->addStretch();
    footerLay->addWidget(btnFermer);
    mainLayout->addLayout(footerLay);

    dialog->exec();
}

void SmartPub::handleEvenementsNavigation() {
    ui->stackedWidgetModules->setCurrentIndex(3);
    setActiveNavigationButton(3);
    updateProfileName(3);
    evAfficherListeEvents();
}


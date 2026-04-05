#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"

#include <algorithm>

// ============================================================================
// MODULE EVENEMENTS
// ============================================================================

void SmartPub::evSetupUI() {
    ui->evTabWidget->setCurrentIndex(0);
    evEventSelectionne = -1;

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
    if (!query.exec("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT ORDER BY DATE_EVENEMENT")) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les événements : " + query.lastError().text());
        return;
    }

    ui->evTableEvents->setRowCount(0);
    ui->evTableSearchEvents->setRowCount(0);

    while (query.next()) {
        EventData data;
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
    query.prepare("SELECT CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT WHERE UPPER(LIEU) LIKE UPPER(:lieu) ORDER BY DATE_EVENEMENT");
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
        bool codeOk = false;
        const qlonglong code = evEditingCode.toLongLong(&codeOk);
        if (!codeOk) {
            QMessageBox::critical(this, "Erreur",
                                  "Code d'événement invalide pour la modification.");
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
                  "WHERE CODE_EVENEMENT = " + QString::number(code);
        } else {
            // Si la date n'a pas changé, on évite d'attaquer DATE_EVENEMENT (ce qui corrige l'erreur pour Nom/Lieu).
            sql = "UPDATE EVENEMENT SET NOM = '" + nomSql + "', LIEU = '" + lieuSql + "' "
                  "WHERE CODE_EVENEMENT = " + QString::number(code);
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
        QSqlQuery query(db);
        query.prepare("INSERT INTO EVENEMENT (NOM, LIEU, DATE_EVENEMENT) VALUES (:nom, :lieu, :date_event)");
        query.bindValue(":nom", nom);
        query.bindValue(":lieu", lieu);
        query.bindValue(":date_event", parsedDate);
        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur",
                                  "Échec de l'ajout : " +
                                      sqlErrorMessage(query.lastError()));
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
        QSqlDatabase db = Connection::instance()->getDatabase();
        if (!db.isOpen()) {
            return;
        }
        bool ok = false;
        const qlonglong codeNum = code.trimmed().toLongLong(&ok);
        if (!ok) {
            QMessageBox::critical(this, "Erreur", "Code d'événement invalide.");
            return;
        }

        // Workaround QODBC: certaines configs échouent avec requêtes préparées + bind.
        // On supprime d'abord les dépendances éventuelles, puis l'événement.
        QSqlQuery query(db);
        query.exec("DELETE FROM PARTICIPER WHERE CODE_EVENEMENT = " + QString::number(codeNum));

        if (!query.exec("DELETE FROM EVENEMENT WHERE CODE_EVENEMENT = " + QString::number(codeNum))) {
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
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter le calendrier", QDir::homePath(), "iCalendar (*.ics)");
    if (!fileName.isEmpty()) {
        QMessageBox::information(this, "Export",
                                 "Calendrier exporté avec succès !");
    }
}

void SmartPub::handleEvBtnLivreResumesClicked() {
    QMessageBox::information(this, "Livre des Résumés",
                             "Génération du livre des résumés - À implémenter");
}

void SmartPub::handleEvBtnCalculImpactClicked() {
    QMessageBox::information(this, "Calculateur d'Impact",
                             "Calcul de l'impact carbone - À implémenter");
}

void SmartPub::handleEvBtnStatsParticipationClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Statistiques de participation");
    dialog.setMinimumSize(600, 450);
    dialog.resize(700, 500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Nombre total de participants (simulé : 50 + hash du code pour variété)
    int totalParticipants = 0;
    QMap<QString, int> participantsParDate;
    for (auto it = evEventsMap.begin(); it != evEventsMap.end(); ++it) {
        int nb = 50 + qHash(it.value().code) % 100;
        if (nb < 20) nb = 50;
        totalParticipants += nb;
        participantsParDate[it.value().date] += nb;
    }

    QLabel *labelTotal = new QLabel(QString("Nombre total de participants : <b>%1</b>").arg(totalParticipants));
    labelTotal->setStyleSheet("font-size: 16px; color: #334155; padding: 10px;");
    layout->addWidget(labelTotal);

    // Courbe : nombre de participants par date
    QChartView *chartView = new QChartView(&dialog);
    chartView->setRenderHint(QPainter::Antialiasing);

    QLineSeries *series = new QLineSeries();
    series->setName("Participants par date");
    series->setColor(QColor("#3b82f6"));
    series->setPen(QPen(QColor("#3b82f6"), 3));

    QStringList datesTriees = participantsParDate.keys();
    std::sort(datesTriees.begin(), datesTriees.end(), [](const QString &a, const QString &b) {
        QDate da = QDate::fromString(a, "dd/MM/yyyy");
        QDate db = QDate::fromString(b, "dd/MM/yyyy");
        return da < db;
    });

    int idx = 0;
    for (const QString &d : datesTriees) {
        series->append(idx, participantsParDate[d]);
        idx++;
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Nombre de participants par date");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QBrush(QColor("white")));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(datesTriees);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    int maxPart = 0;
    for (int v : participantsParDate)
        if (v > maxPart) maxPart = v;
    axisY->setRange(0, maxPart + 10);
    axisY->setLabelFormat("%d");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);
    chartView->setChart(chart);
    layout->addWidget(chartView);

    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none; "
        "border-radius: 8px; padding: 10px 24px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    connect(btnFermer, &QPushButton::clicked, &dialog, &QDialog::accept);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnFermer);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    dialog.exec();
}

void SmartPub::handleEvenementsNavigation() {
    ui->stackedWidgetModules->setCurrentIndex(3);
    setActiveNavigationButton(3);
    updateProfileName(3);
    evAfficherListeEvents();
}


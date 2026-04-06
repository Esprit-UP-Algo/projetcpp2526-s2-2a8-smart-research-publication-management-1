#include "smartpub.h"
#include "ui_smartpub.h"
#include "connection.h"
#include "publicationauth.h"
#include "promotionengine.h"
#include "matchmakingengine.h"
#include "ai_service.h"
#include "reminder.h"
#include <algorithm>
#include <QTableWidgetItem>
#include <QApplication>
#include <QDateTime>
#include <QRegion>
#include <QProcess>
#include <QScreen>
#include <QFile>
#include <QTextStream>
#include <QRegularExpressionValidator>
#include <QDebug>
#include <QToolTip>
#include <QTimer>
#include <QVBoxLayout>
#include <QComboBox>
// Réseau
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

static QPixmap makeCircularPixmap(const QPixmap &src, int size) {
    if (src.isNull() || size <= 0)
        return QPixmap();
    QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setClipRegion(QRegion(0, 0, size, size, QRegion::Ellipse));
    painter.drawPixmap(0, 0, scaled);
    painter.end();
    return result;
}

// ============================================================================
// MODULE CHERCHEURS
// ============================================================================

void SmartPub::cherchSetupUI() {

    // ── Bouton toggle vue (icônes/liste) ──────────────────────────────────────
    cherchBtnToggleVue = new QPushButton(this);
    cherchBtnToggleVue->setObjectName("cherchBtnToggleVue");
    cherchBtnToggleVue->setFixedSize(44, 44);
    cherchBtnToggleVue->setCursor(Qt::PointingHandCursor);
    cherchBtnToggleVue->setText("⊞");
    cherchBtnToggleVue->setToolTip("Changer le mode d'affichage");
    cherchBtnToggleVue->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #3b82f6;
            color: #3b82f6;
        }
    )");
    if (ui->horizontalLayoutToolbar)
        ui->horizontalLayoutToolbar->insertWidget(1, cherchBtnToggleVue);
    connect(cherchBtnToggleVue, &QPushButton::clicked, this,
            &SmartPub::on_cherchBtnToggleVue_clicked);

    // ── Récupérer les widgets Projets déclarés dans le .ui ───────────────────
    // Les widgets cherchBtnSelectProjets, cherchProjetsListWidget,
    // cherchBtnValiderProjets et cherchLabelProjetsSelec sont maintenant
    // définis directement dans smartpub.ui (row=3 de gridLayoutInputs).
    // On se contente ici de récupérer leurs pointeurs et brancher les signaux.

    cherchBtnSelectProjets  = ui->cherchFormFrame->findChild<QPushButton*>(
                                  QStringLiteral("cherchBtnSelectProjets"));
    cherchProjetsListWidget = ui->cherchFormFrame->findChild<QListWidget*>(
                                  QStringLiteral("cherchProjetsListWidget"));
    cherchLabelProjetsSelec = ui->cherchFormFrame->findChild<QLabel*>(
                                  QStringLiteral("cherchLabelProjetsSelec"));
    QPushButton *btnValiderProjets = ui->cherchFormFrame->findChild<QPushButton*>(
                                         QStringLiteral("cherchBtnValiderProjets"));

    // Sécurité : si le .ui n'a pas encore été regénéré, on sort silencieusement
    if (!cherchBtnSelectProjets || !cherchProjetsListWidget ||
        !cherchLabelProjetsSelec || !btnValiderProjets)
        return;

    // ── Connexion : toggle liste de projets ───────────────────────────────────
    connect(cherchBtnSelectProjets, &QPushButton::clicked, this,
            [this, btnValiderProjets]() {
        bool visible = cherchProjetsListWidget->isVisible();
        if (!visible && cherchProjetsListWidget->count() == 0) {
            QSqlDatabase db = Connection::instance()->getDatabase();
            if (db.isOpen()) {
                QSqlQuery q(db);
                if (q.exec(QStringLiteral(
                        "SELECT ID_PROJET, CODE, TITRE FROM PROJET ORDER BY CODE"))) {
                    while (q.next()) {
                        const int pid = q.value(0).toInt();
                        const QString pcode = q.value(1).toString();
                        QListWidgetItem *it = new QListWidgetItem(
                            QString("[%1]  %2")
                                .arg(pcode)
                                .arg(q.value(2).toString()));
                        it->setData(Qt::UserRole, pid);
                        cherchProjetsListWidget->addItem(it);
                    }
                }
            }
        }
        cherchProjetsListWidget->setVisible(!visible);
        btnValiderProjets->setVisible(!visible);
        cherchBtnSelectProjets->setText(
            visible ? QStringLiteral("▼  Sélectionner des projets…")
                    : QStringLiteral("▲  Fermer la liste"));
    });

    // ── Limiter à 5 sélections ────────────────────────────────────────────────
    connect(cherchProjetsListWidget, &QListWidget::itemSelectionChanged,
            this, [this]() {
        QList<QListWidgetItem *> sel = cherchProjetsListWidget->selectedItems();
        if (sel.size() > 5) {
            bool b = cherchProjetsListWidget->blockSignals(true);
            sel.last()->setSelected(false);
            cherchProjetsListWidget->blockSignals(b);
            QToolTip::showText(QCursor::pos(),
                               QStringLiteral("Maximum 5 projets autorisés"),
                               cherchProjetsListWidget);
        }
    });

    // ── Valider la sélection : fermer liste + mettre à jour résumé ───────────
    connect(btnValiderProjets, &QPushButton::clicked, this,
            [this, btnValiderProjets]() {
        QList<QListWidgetItem *> sel = cherchProjetsListWidget->selectedItems();
        cherchProjetsListWidget->setVisible(false);
        btnValiderProjets->setVisible(false);
        cherchBtnSelectProjets->setText(QStringLiteral("▼  Sélectionner des projets…"));
        if (sel.isEmpty()) {
            cherchLabelProjetsSelec->setText(QStringLiteral("Aucun projet sélectionné"));
            cherchLabelProjetsSelec->setStyleSheet(
                QStringLiteral("color: #94a3b8; font-size: 12px; "
                                "background: transparent; border: none;"));
        } else {
            cherchLabelProjetsSelec->setText(
                QString("✔  %1 projet(s) sélectionné(s)").arg(sel.size()));
            cherchLabelProjetsSelec->setStyleSheet(
                QStringLiteral("color: #10b981; font-size: 12px; font-weight: 600; "
                                "background: transparent; border: none;"));
        }
    });
}

void SmartPub::cherchConnectSignals() {
    // NOTE : Les slots on_cherchBtn*_clicked sont connectés automatiquement
    // par ui->setupUi() via le mécanisme de connexion automatique Qt (auto-connect).
    // On ne les reconnecte PAS ici pour éviter le double déclenchement.

    // ── CIN live-validation ───────────────────────────────────────────────────
    connect(ui->cherchLineEditCIN, &QLineEdit::textChanged, this, [this](const QString &) {
        if (!cherchErrCinLabel)
            return;
        static const QRegularExpression cinRx(QStringLiteral(R"(^\d{8}$)"));
        const QString t = ui->cherchLineEditCIN->text().trimmed();
        Q_UNUSED(t)
    });
    connect(ui->cherchLineEditCIN, &QLineEdit::editingFinished, this, [this]() {
        cherchTouchedCin = true;
    });

    connect(ui->cherchComboBoxGrade, QOverload<int>::of(&QComboBox::activated), this,
            [this](int) {
                cherchTouchedGrade = true;
            });

    // ── Écouteurs d'événements sur les champs ───────────────────────────────
    ui->cherchLineEditNom->installEventFilter(this);
    ui->cherchLineEditPrenom->installEventFilter(this);
    ui->cherchLineEditCIN->installEventFilter(this);
    ui->cherchLineEditEmail->installEventFilter(this);
    ui->cherchComboBoxGrade->installEventFilter(this);

    connect(ui->cherchLineEditForgotEmail, &QLineEdit::textChanged, this, [this](const QString &) {
        ui->cherchLineEditForgotEmail->setStyleSheet(QString());
    });

    // ── Vérification délivrabilité email — initialisation du gestionnaire réseau ──
    cherchNetworkManager = new QNetworkAccessManager(this);
    connect(cherchNetworkManager, &QNetworkAccessManager::finished,
            this, &SmartPub::on_cherchEmailVerificationReply);

    // Débounce sur le champ email du formulaire d'ajout (1500 ms)
    cherchEmailDebounceTimer = new QTimer(this);
    cherchEmailDebounceTimer->setSingleShot(true);
    cherchEmailDebounceTimer->setInterval(1500);

    connect(cherchEmailDebounceTimer, &QTimer::timeout, this, [this]() {
        const QString email = ui->cherchLineEditEmail->text().trimmed();
        // Vérifier d'abord le format avant d'appeler l'API
        static const QRegularExpression emailRegex(
            QStringLiteral(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"));
        if (email.isEmpty() || !emailRegex.match(email).hasMatch()) {
            // Format invalide : ne pas lancer la requête API
            cherchEmailDeliverabilityOk = false;
            cherchEmailCheckPending     = false;
            cherchLastVerifiedEmail     = QString();
            if (cherchErrEmailLabel) {
                if (email.isEmpty()) {
                    cherchErrEmailLabel->setText("");
                    cherchErrEmailLabel->setVisible(false);
                } else {
                    cherchErrEmailLabel->setText(
                        "  ⚠️ Format invalide — exemple@domaine.com");
                    cherchErrEmailLabel->setStyleSheet(
                        "color: #f59e0b; font-size: 12px; "
                        "background: #fffbeb; border: 1px solid #fde68a; "
                        "border-radius: 8px; padding: 4px 10px;");
                    cherchErrEmailLabel->setVisible(true);
                }
            }
            return;
        }
        // Format OK — lancer la vérification délivrabilité
        cherchVerifyEmailDeliverability(email);
    });

    // Dès que l'utilisateur tape dans le champ email, démarrer le timer
    connect(ui->cherchLineEditEmail, &QLineEdit::textChanged, this, [this](const QString &text) {
        // Réinitialiser l'état de vérification à chaque frappe
        cherchEmailDeliverabilityOk = false;
        cherchEmailCheckPending     = true;
        cherchLastVerifiedEmail     = QString();
        if (cherchErrEmailLabel) {
            if (text.trimmed().isEmpty()) {
                cherchErrEmailLabel->setText("");
                cherchErrEmailLabel->setVisible(false);
            } else {
                cherchErrEmailLabel->setText(
                    "  🔍 Vérification en cours\u2026");
                cherchErrEmailLabel->setStyleSheet(
                    "color: #3b82f6; font-size: 12px; "
                    "background: #eff6ff; border: 1px solid #bfdbfe; "
                    "border-radius: 8px; padding: 4px 10px;");
                cherchErrEmailLabel->setVisible(true);
            }
        }
        cherchEmailDebounceTimer->start();
    });
}

void SmartPub::cherchApplyModernStyle() {
    // === LOGIN VIEW STYLES ===
    if (ui->cherchLoginFrame) {
        ui->cherchLoginFrame->setStyleSheet(R"(
            QFrame#cherchLoginFrame {
                background-color: white;
                border-radius: 20px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    if (ui->cherchLoginHeader) {
        ui->cherchLoginHeader->setStyleSheet(R"(
            QFrame#cherchLoginHeader {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                border-top-left-radius: 20px;
                border-top-right-radius: 20px;
            }
        )");
    }

    // Style pour la page mot de passe oublié
    if (ui->cherchForgotFrame) {
        ui->cherchForgotFrame->setStyleSheet(R"(
            QFrame#cherchForgotFrame {
                background-color: white;
                border-radius: 20px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    if (ui->cherchForgotHeader) {
        ui->cherchForgotHeader->setStyleSheet(R"(
            QFrame#cherchForgotHeader {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                border-top-left-radius: 20px;
                border-top-right-radius: 20px;
            }
        )");
    }

    QString cherchLoginInputStyle = R"(
        QLineEdit {
            background-color: #f8fafc;
            border: 2px solid #e2e8f0;
            border-radius: 12px;
            padding: 14px 16px;
            font-size: 14px;
            color: #334155;
        }
        QLineEdit:focus {
            border-color: #3b82f6;
            background-color: white;
        }
    )";

    if (ui->cherchLineEditLoginEmail) {
        ui->cherchLineEditLoginEmail->setStyleSheet(cherchLoginInputStyle);
    }

    if (ui->cherchLineEditLoginPassword) {
        ui->cherchLineEditLoginPassword->setStyleSheet(cherchLoginInputStyle);
        ui->cherchLineEditLoginPassword->setEchoMode(QLineEdit::Password);
    }

    // Style pour le champ email de récupération
    if (ui->cherchLineEditForgotEmail) {
        ui->cherchLineEditForgotEmail->setStyleSheet(cherchLoginInputStyle);
    }

    if (ui->cherchBtnLogin) {
        ui->cherchBtnLogin->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 14px 32px;
                font-size: 16px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
    }

    // Style pour le bouton OK de récupération
    if (ui->cherchBtnForgotOk) {
        ui->cherchBtnForgotOk->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 14px 32px;
                font-size: 16px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
    }

    // Style pour le lien mot de passe oublié
    if (ui->cherchBtnMotDePasseOublie) {
        ui->cherchBtnMotDePasseOublie->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: #3b82f6;
                border: none;
                font-size: 13px;
                font-weight: 500;
                text-decoration: underline;
            }
            QPushButton:hover {
                color: #2563eb;
            }
        )");
    }

    // Style pour le bouton retour
    if (ui->cherchBtnRetourLogin) {
        ui->cherchBtnRetourLogin->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.2);
                color: white;
                border: none;
                border-radius: 20px;
                font-size: 20px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.3);
            }
        )");
    }

    // === HEADER STYLES ===
    if (ui->cherchHeaderFrame) {
        ui->cherchHeaderFrame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border-bottom: 1px solid #e2e8f0;
            }
        )");
    }

    if (ui->cherchTitleLabel) {
        ui->cherchTitleLabel->setStyleSheet(

            "color: #1e293b; font-size: 28px; font-weight: 700; background: "
            "transparent; border: none");
    }

    if (ui->cherchSubtitleLabel) {
        ui->cherchSubtitleLabel->setStyleSheet(
            "color: #64748b; font-size: 14px; background: transparent; border: "
            "none");
    }

    // === TOOLBAR STYLES ===
    if (ui->cherchToolbarFrame) {
        ui->cherchToolbarFrame->setStyleSheet(
            "background-color: transparent; border: none;");
    }

    // Tabs frame
    if (ui->cherchTabsFrame) {
        ui->cherchTabsFrame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border-radius: 12px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    QString cherchTabActive = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
    )";

    QString cherchTabInactive = R"(
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

    if (ui->cherchBtnVueListe) {
        ui->cherchBtnVueListe->setStyleSheet(cherchTabActive);
    }

    if (ui->cherchBtnAjouter) {
        ui->cherchBtnAjouter->setStyleSheet(cherchTabInactive);
    }

    // Toolbar buttons
    if (ui->cherchBtnRecherche) {
        ui->cherchBtnRecherche->setStyleSheet(R"(
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 10px;
                padding: 10px 16px;
                font-size: 13px;
                font-weight: 600;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
        )");
    }

    QString cherchSecondaryBtn = R"(
        QPushButton {
            background-color: white;
            color: #334155;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 10px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #f8fafc;
            border-color: #cbd5e1;
        }
    )";

    if (ui->cherchBtnTri)
        ui->cherchBtnTri->setStyleSheet(cherchSecondaryBtn);
    if (ui->cherchBtnExport)
        ui->cherchBtnExport->setStyleSheet(cherchSecondaryBtn);

    if (ui->cherchBtnStatistiques) {
        ui->cherchBtnStatistiques->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 10px;
                padding: 10px 16px;
                font-size: 13px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #059669);
            }
        )");
    }

    // Search field
    if (ui->cherchLineEditRecherche) {
        ui->cherchLineEditRecherche->setStyleSheet(R"(
            QLineEdit {
                background-color: white;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                padding: 10px 14px;
                font-size: 13px;
                color: #334155;
            }
            QLineEdit:focus {
                border-color: #3b82f6;
            }
        )");
    }

    // === FORM STYLES ===
    if (ui->cherchFormFrame) {
        ui->cherchFormFrame->setStyleSheet(R"(
            QFrame {
                background-color: white;
                border-radius: 20px;
                border: 1px solid #e2e8f0;
            }
        )");
    }

    // Form inputs
    QString cherchInputStyle = R"(
        QLineEdit {
            background-color: #f8fafc;
            border: none;
            border-radius: 10px;
            padding: 12px 16px;
            font-size: 14px;
            color: #334155;
        }
        QLineEdit:focus {
            background-color: #eff6ff;
            border: 2px solid #3b82f6;
        }
    )";

    if (ui->cherchLineEditNom)
        ui->cherchLineEditNom->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditPrenom)
        ui->cherchLineEditPrenom->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditCIN)
        ui->cherchLineEditCIN->setStyleSheet(cherchInputStyle);
    if (ui->cherchLineEditEmail)
        ui->cherchLineEditEmail->setStyleSheet(cherchInputStyle);

    // ── Créer le label d'état email (délivrabilité) ─────────────────────────
    // Le label est inséré dynamiquement dans le même QVBoxLayout que le champ email
    if (ui->cherchLineEditEmail && !cherchErrEmailLabel) {
        QVBoxLayout *emailVLayout = nullptr;
        QWidget *parentWidget = ui->cherchLineEditEmail->parentWidget();
        if (parentWidget) {
            // On remonte jusqu'à trouver un layout contenant le champ
            QLayout *layout = parentWidget->layout();
            if (layout && layout->indexOf(ui->cherchLineEditEmail) >= 0) {
                emailVLayout = qobject_cast<QVBoxLayout*>(layout);
            }
        }
        // Fallback : utiliser le layout principal du formulaire
        if (!emailVLayout) {
            emailVLayout = ui->cherchFormFrame->findChild<QVBoxLayout*>("verticalLayoutEmail");
        }
        // Création du label
        cherchErrEmailLabel = new QLabel(ui->cherchFormFrame);
        cherchErrEmailLabel->setObjectName("cherchErrEmailLabel");
        cherchErrEmailLabel->setText(QString());
        cherchErrEmailLabel->setVisible(false);
        cherchErrEmailLabel->setWordWrap(true);
        cherchErrEmailLabel->setStyleSheet(
            "font-size: 12px; background: transparent; border: none; padding: 4px 0;");
        if (emailVLayout) {
            int idx = emailVLayout->indexOf(ui->cherchLineEditEmail);
            if (idx >= 0)
                emailVLayout->insertWidget(idx + 1, cherchErrEmailLabel);
            else
                emailVLayout->addWidget(cherchErrEmailLabel);
        } else {
            // Dernier recours : empiler sous le champ (moins propre)
            QVBoxLayout *fallback = new QVBoxLayout(ui->cherchLineEditEmail->parentWidget());
            fallback->addWidget(ui->cherchLineEditEmail);
            fallback->addWidget(cherchErrEmailLabel);
        }
    }

    // === VALIDATEURS DE SAISIE (contrôle dès la frappe) ===
    // Nom & prénom : uniquement lettres (accentuées autorisées), espaces, apostrophes, tirets
    QRegularExpression nameRegex(QStringLiteral("^[A-Za-zÀ-ÖØ-öø-ÿ\\s'-]*$"));
    auto *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    if (ui->cherchLineEditNom)
        ui->cherchLineEditNom->setValidator(nameValidator);
    if (ui->cherchLineEditPrenom)
        ui->cherchLineEditPrenom->setValidator(nameValidator);

    // CIN : uniquement chiffres, maximum 8 (contrôle direct au clavier)
    QRegularExpression cinRegex(QStringLiteral("^\\d{0,8}$"));
    auto *cinValidator = new QRegularExpressionValidator(cinRegex, this);
    if (ui->cherchLineEditCIN) {
        ui->cherchLineEditCIN->setValidator(cinValidator);
        ui->cherchLineEditCIN->setMaxLength(8);
    }

    // Initialiser la photo courante pour l'ajout de chercheur
    cherchCurrentPhotoPath = QString(":/avatar.png");

    // Combo box
    if (ui->cherchComboBoxGrade) {
        ui->cherchComboBoxGrade->setStyleSheet(R"(
            QComboBox {
                background-color: #f8fafc;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                padding: 12px 16px;
                font-size: 14px;
                color: #334155;
                min-height: 48px;
            }
            QComboBox:hover {
                border-color: #cbd5e1;
            }
            QComboBox:focus {
                border-color: #3b82f6;
                background-color: #eff6ff;
            }
            QComboBox::drop-down {
                border: none;
                width: 40px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 6px solid #64748b;
                width: 0;
                height: 0;
                margin-right: 10px;
            }
            QComboBox QAbstractItemView {
                background-color: white;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                selection-background-color: #eff6ff;
                selection-color: #1e293b;
                outline: none;
                padding: 8px;
                margin-top: 4px;
                min-width: 200px;
            }
            QComboBox QAbstractItemView::item {
                padding: 12px 16px;
                border-radius: 6px;
                color: #334155;
                font-size: 14px;
            }
            QComboBox QAbstractItemView::item:hover {
                background-color: #f1f5f9;
                color: #1e293b;
            }
            QComboBox QAbstractItemView::item:selected {
                background-color: #eff6ff;
                color: #3b82f6;
                font-weight: 600;
            }
        )");
    }

    // Form labels
    QString cherchLabelStyle = "color: #334155; font-size: 14px; font-weight: "
                               "600; background: transparent; border: none;";
    if (ui->cherchLabelNom)
        ui->cherchLabelNom->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelPrenom)
        ui->cherchLabelPrenom->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelCIN)
        ui->cherchLabelCIN->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelEmail)
        ui->cherchLabelEmail->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelGrade)
        ui->cherchLabelGrade->setStyleSheet(cherchLabelStyle);
    if (ui->cherchLabelPhoto)
        ui->cherchLabelPhoto->setStyleSheet(cherchLabelStyle);

    // Form buttons
    if (ui->cherchBtnAnnulerAjout) {
        ui->cherchBtnAnnulerAjout->setStyleSheet(R"(
            QPushButton {
                background-color: white;
                color: #64748b;
                border: 2px solid #e2e8f0;
                border-radius: 12px;
                padding: 12px 32px;
                font-size: 15px;
                font-weight: 600;
            }
            QPushButton:hover {
                background-color: #f1f5f9;
                border-color: #cbd5e1;
            }
        )");
    }

    if (ui->cherchBtnAjouterChercheur) {
        ui->cherchBtnAjouterChercheur->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #10b981, stop:1 #3b82f6);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 12px 32px;
                font-size: 15px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #059669, stop:1 #2563eb);
            }
        )");
    }

    // Upload photo button
    if (ui->cherchBtnUploadPhoto) {
        ui->cherchBtnUploadPhoto->setStyleSheet(R"(
            QPushButton {
                background-color: #f8fafc;
                border: 2px dashed #cbd5e1;
                border-radius: 16px;
                color: #94a3b8;
                font-size: 48px;
                font-weight: 300;
            }
            QPushButton:hover {
                border-color: #3b82f6;
                background-color: #eff6ff;
                color: #3b82f6;
            }
        )");
    }

    if (ui->cherchLabelPhotoHint) {
        ui->cherchLabelPhotoHint->setStyleSheet(
            "color: #94a3b8; font-size: 12px; background: transparent; border: "
            "none;");
    }

    // === SCROLL AREA STYLES ===
    if (ui->cherchCardsScrollArea) {
        ui->cherchCardsScrollArea->setStyleSheet(R"(
            QScrollArea {
                background-color: transparent;
                border: none;
            }
            QScrollArea > QWidget > QWidget {
                background-color: transparent;
            }
            QScrollBar:vertical {
                background-color: #f1f5f9;
                width: 12px;
                border-radius: 6px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background-color: #cbd5e1;
                border-radius: 6px;
                min-height: 40px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #94a3b8;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0;
            }
        )");
    }

    if (ui->cherchScrollAreaWidgetContents) {
        ui->cherchScrollAreaWidgetContents->setStyleSheet(
            "background-color: #f8fafc;");
    }
}



QString SmartPub::cherchDeterminerCarriere(int projetsCount,
                                           const QString &grade) {
    // Basé sur le grade académique ET le nombre de contributions dans CONTRIBUER
    if (grade == "Professeur") {
        return projetsCount >= 3 ? "Professeur Senior — Expert" : "Professeur";
    }
    if (grade == "Maitre de Conferences") {
        return projetsCount >= 3 ? "MdC — Confirmé Expert" : "Maître de Conférences";
    }
    if (grade == "Docteur") {
        return projetsCount >= 2 ? "Docteur — Chercheur Actif" : "Docteur";
    }
    if (grade == "Ingenieur de Recherche") {
        return projetsCount >= 2 ? "Ingénieur Recherche Senior" : "Ingénieur de Recherche";
    }
    if (grade == "Post-doctorant") {
        return "Post-Doctorant";
    }
    if (grade == "Doctorant") {
        return "Doctorant — En Formation";
    }
    // Fallback générique basé sur le nombre de projets
    if (projetsCount >= 4) return "Senior — Expert";
    if (projetsCount >= 2) return "Confirmé";
    return "Junior";
}

bool SmartPub::cherchValiderNomPrenom(const QString &nom,
                                      const QString &prenom) {
    // Autoriser uniquement les lettres (y compris accentuées), espaces,
    // apostrophes et tirets
    static const QRegularExpression nameRegex(
        QStringLiteral(R"(^[A-Za-zÀ-ÖØ-öø-ÿ\s'-]+$)"));

    if (nom.isEmpty() || prenom.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Le nom et le prénom sont obligatoires.");
        return false;
    }

    if (!nameRegex.match(nom).hasMatch() || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Erreur",
                             "Le nom et le prénom ne doivent contenir que des lettres.");
        return false;
    }

    return true;
}

void SmartPub::cherchShowLoginView() {
    // Retour au login global
    mainStack->setCurrentIndex(0);
    isUserLoggedIn = false;
    ui->cherchLineEditLoginPassword->clear();
}

void SmartPub::cherchShowMainView() {
    // Transition vers l'application principale
    mainStack->setCurrentIndex(1);
}

void SmartPub::cherchCheckLogin() {
    // === BYPASS AUTHENTICATION (User Request) ===
    currentUser =
        UserAccount{"admin", "", "Tous", UserRole::Admin, "Administrateur"};
    isUserLoggedIn = true;

    // Configuration post-login
    updateSidebarProfileVisibility();

    // Mettre à jour les infos avatar/nom
    if (nameLabel)
        nameLabel->setText(currentUser.displayName);
    if (roleLabel)
        roleLabel->setText("Administrateur");

    // FIX: Ouvrir Publications en premier lieu après login (index 1)
    ui->stackedWidgetModules->setCurrentIndex(1);
    setActiveNavigationButton(1);
    updateProfileName(1);
    SR_updateButtonStyles();

    // S'assurer que le module Chercheur sera en vue liste lors d'une navigation future
    ui->cherchStackedWidget->setCurrentIndex(0);
    cherchVueListeActive = true;

    // Afficher l'application principale
    mainStack->setCurrentIndex(1);

    // Vider les champs de login
    ui->cherchLineEditLoginEmail->clear();
    ui->cherchLineEditLoginPassword->clear();
}

void SmartPub::cherchShowForgotPasswordView() {
    ui->cherchStackedWidgetLogin->setCurrentIndex(1);
    ui->cherchLineEditForgotEmail->clear();
    ui->cherchLineEditForgotEmail->setFocus();
}

void SmartPub::on_cherchBtnMotDePasseOublie_clicked() {
    ui->cherchStackedWidgetLogin->setCurrentIndex(1);
    ui->cherchLineEditForgotEmail
        ->clear(); // Clear the field when navigating to forgot password view
    ui->cherchLineEditForgotEmail->setFocus();
}

void SmartPub::on_cherchBtnRetourLogin_clicked() {
    ui->cherchStackedWidgetLogin->setCurrentIndex(0);
}

void SmartPub::on_cherchBtnForgotOk_clicked() {
    QString email = ui->cherchLineEditForgotEmail->text().trimmed();
    if (!email.isEmpty() && email.contains("@")) {
        QMessageBox::information(this, "Email Envoyé",
                                 "Un lien de réinitialisation a été envoyé à " +
                                     email);
        ui->cherchStackedWidgetLogin->setCurrentIndex(0);
    } else {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez entrer une adresse email valide.");
    }
}

void SmartPub::on_cherchBtnLogin_clicked() { cherchCheckLogin(); }

// void SmartPub::on_cherchUserProfileFrame_clicked()
// {
//     // Déconnexion
//     auto reply = QMessageBox::question(this, "Déconnexion",
//                                        "Voulez-vous vraiment vous déconnecter
//                                        ?", QMessageBox::Yes |
//                                        QMessageBox::No, QMessageBox::No);
//
//     if (reply == QMessageBox::Yes) {
//         // Redémarrer l'application pour retourner au login
//         qApp->quit();
//         QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
//     }
// }


void SmartPub::cherchEnrichirDonneesDepuisOracle()
{
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;
    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        it->projetsIds.clear();
    }
    QSqlQuery q(db);
    if (q.exec(QStringLiteral("SELECT ID_CHERCHEUR, CODE_PROJET FROM CONTRIBUER"))) {
        while (q.next()) {
            const int cid = q.value(0).toInt();
            const int pid = q.value(1).toInt();
            if (!cherchChercheursMap.contains(cid))
                continue;
            auto &lst = cherchChercheursMap[cid].projetsIds;
            if (!lst.contains(pid))
                lst.append(pid);
        }
    }
    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        const int n = it->projetsIds.size();
        it->carriere = cherchDeterminerCarriere(n, it->grade);
    }
}

void SmartPub::cherchAfficherListeChercheurs() {
    cherchClearChercheursList();
    cherchChercheursMap.clear();

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        if (cherchVueIconesActive) {
            QGridLayout *gridLayout = qobject_cast<QGridLayout *>(
                ui->cherchScrollAreaWidgetContents->layout());
            if (!gridLayout) {
                QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
                if (oldLayout) {
                    QLayoutItem *child;
                    while ((child = oldLayout->takeAt(0)) != nullptr) {
                        if (child->widget()) delete child->widget();
                        delete child;
                    }
                    delete oldLayout;
                }
                gridLayout = new QGridLayout(ui->cherchScrollAreaWidgetContents);
                gridLayout->setSpacing(24);
                gridLayout->setContentsMargins(24, 24, 24, 24);
            }
            QLabel *noDb = new QLabel("Connexion base de données indisponible.",
                                      ui->cherchScrollAreaWidgetContents);
            noDb->setAlignment(Qt::AlignCenter);
            noDb->setStyleSheet("color: #94a3b8; font-size: 16px; background: transparent; border: none;");
            gridLayout->addWidget(noDb, 0, 0, 1, 3);
        }
        return;
    }

    QString sql = "SELECT ID_CHERCHEUR, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM CHERCHEUR";
    if (!cherchWhereClause.isEmpty())
        sql += " WHERE " + cherchWhereClause;
    if (!cherchOrderByClause.isEmpty())
        sql += " ORDER BY " + cherchOrderByClause;

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        QMessageBox::warning(this, "Erreur", "Impossible de charger les chercheurs : " + query.lastError().text());
        return;
    }

    // --- MODIFICATION : stocker l'ordre des ID récupérés ---
    QList<int> orderedIds;

    while (query.next()) {
        int id = query.value("ID_CHERCHEUR").toInt();
        ChercheurData data;
        data.nom = query.value("NOM").toString();
        data.prenom = query.value("PRENOM").toString();
        data.email = query.value("EMAIL").toString();
        data.grade = query.value("GRADE").toString();
        data.cin = query.value("CIN").toString();
        data.photoPath = query.value("PHOTO_PROFIL").toString();
        if (data.photoPath.isEmpty())
            data.photoPath = ":/avatar.png";
        data.dateCreation = QDateTime();
        data.carriere = "";
        data.age = 0;
        cherchChercheursMap[id] = data;
        orderedIds.append(id);   // <--- ordre SQL préservé
    }

    cherchEnrichirDonneesDepuisOracle();

    if (cherchVueIconesActive) {
        QGridLayout *gridLayout = qobject_cast<QGridLayout *>(
            ui->cherchScrollAreaWidgetContents->layout());
        if (!gridLayout) {
            QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget())
                        delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            gridLayout = new QGridLayout(ui->cherchScrollAreaWidgetContents);
            gridLayout->setSpacing(24);
            gridLayout->setContentsMargins(24, 24, 24, 24);
        }

        // --- MODIFICATION : itérer sur orderedIds au lieu de la map ---
        for (int id : orderedIds) {
            auto data = cherchChercheursMap.value(id);
            cherchAjouterChercheurCard(id, data.nom, data.prenom, data.grade,
                                       data.email, data.photoPath);
        }
    } else {
        QVBoxLayout *listLayout = qobject_cast<QVBoxLayout *>(
            ui->cherchScrollAreaWidgetContents->layout());
        if (!listLayout) {
            QLayout *oldLayout = ui->cherchScrollAreaWidgetContents->layout();
            if (oldLayout) {
                QLayoutItem *child;
                while ((child = oldLayout->takeAt(0)) != nullptr) {
                    if (child->widget())
                        delete child->widget();
                    delete child;
                }
                delete oldLayout;
            }
            listLayout = new QVBoxLayout(ui->cherchScrollAreaWidgetContents);
            listLayout->setSpacing(12);
            listLayout->setContentsMargins(24, 24, 24, 24);
            listLayout->setAlignment(Qt::AlignTop);
        }

        // --- MODIFICATION : itérer sur orderedIds au lieu de la map ---
        for (int id : orderedIds) {
            auto data = cherchChercheursMap.value(id);
            cherchAjouterChercheurListItem(id, data.nom, data.prenom, data.grade,
                                           data.email, data.photoPath);
        }
    }
}

void SmartPub::cherchAjouterChercheurCard(int id, const QString &nom,
                                          const QString &prenom,
                                          const QString &grade,
                                          const QString &email,
                                          const QString &photoPath) {
    QFrame *card = new QFrame(ui->cherchScrollAreaWidgetContents);
    card->setObjectName(QString("cherchCard_%1").arg(id));
    card->setFixedSize(340, 200);
    card->setProperty("cherchChercheurId", id);
    card->setCursor(Qt::PointingHandCursor);

    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 16px;
            border: 1px solid #e2e8f0;
        }
        QFrame:hover {
            border: 2px solid #3b82f6;
            background-color: #f8fafc;
        }
    )");

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 25));
    shadow->setOffset(0, 4);
    card->setGraphicsEffect(shadow);

    QHBoxLayout *mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QLabel *avatarLabel = new QLabel(card);
    avatarLabel->setFixedSize(80, 80);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet("border: 3px solid white; background: transparent;");
    avatarLabel->setScaledContents(false);
    avatarLabel->setMask(QRegion(0, 0, 80, 80, QRegion::Ellipse));
    QPixmap avatarPix;
    QString path = photoPath.isEmpty() ? QString(":/avatar.png") : photoPath;
    if (!avatarPix.load(path))
        avatarPix.load(":/avatar.png");
    if (!avatarPix.isNull())
        avatarLabel->setPixmap(makeCircularPixmap(avatarPix, 80));
    mainLayout->addWidget(avatarLabel);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);
    infoLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom), card);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #1e293b; "
                             "background: transparent; border: none;");
    infoLayout->addWidget(nameLabel);

    QLabel *gradeLabel = new QLabel(grade, card);
    gradeLabel->setStyleSheet(R"(
        font-size: 12px;
        font-weight: 600;
        color: #059669;
        background-color: #d1fae5;
        padding: 6px 12px;
        border-radius: 20px;
    )");
    gradeLabel->setMaximumWidth(150);
    infoLayout->addWidget(gradeLabel);

    QLabel *emailLabel = new QLabel(email, card);
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: "
                              "transparent; border: none;");
    infoLayout->addWidget(emailLabel);

    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout, 1);

    // Boutons d'action (masqués pour les guests)
    if (currentUser.role == UserRole::Admin) {
        QVBoxLayout *btnLayout = new QVBoxLayout();
        btnLayout->setSpacing(10);
        btnLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QPushButton *btnEdit = new QPushButton("✎", card);
        btnEdit->setFixedSize(40, 40);
        btnEdit->setToolTip("Modifier le chercheur");
        btnEdit->setStyleSheet(R"(
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 10px;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
        )");
        connect(btnEdit, &QPushButton::clicked, this,
                [this, id]() { on_cherchModifierChercheur(id); });

        QPushButton *btnDelete = new QPushButton("🗑", card);
        btnDelete->setFixedSize(40, 40);
        btnDelete->setToolTip("Supprimer le chercheur");
        btnDelete->setStyleSheet(R"(
            QPushButton {
                background-color: #ef4444;
                color: white;
                border: none;
                border-radius: 10px;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: #dc2626;
            }
        )");
        connect(btnDelete, &QPushButton::clicked, this,
                [this, id]() { on_cherchSupprimerChercheur(id); });

        btnLayout->addWidget(btnEdit);
        btnLayout->addWidget(btnDelete);
        btnLayout->addStretch();
        mainLayout->addLayout(btnLayout);
    }

    card->installEventFilter(this);
    card->setMouseTracking(true);

    QGridLayout *grid =
        qobject_cast<QGridLayout *>(ui->cherchScrollAreaWidgetContents->layout());
    if (grid) {
        int count = grid->count();
        int row = count / 3;
        int col = count % 3;
        grid->addWidget(card, row, col, Qt::AlignTop);

        // Animation
        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(card);
        opacityEffect->setOpacity(0.0);
        card->setGraphicsEffect(opacityEffect);
        QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
        anim->setDuration(400);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void SmartPub::cherchAjouterChercheurListItem(int id, const QString &nom,
                                              const QString &prenom,
                                              const QString &grade,
                                              const QString &email,
                                              const QString &photoPath) {
    QFrame *item = new QFrame(ui->cherchScrollAreaWidgetContents);
    item->setObjectName(QString("cherchListItem_%1").arg(id));
    item->setFixedHeight(80);
    item->setProperty("cherchChercheurId", id);
    item->setCursor(Qt::PointingHandCursor);

    item->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 12px;
            border: 1px solid #e2e8f0;
        }
        QFrame:hover {
            border: 2px solid #3b82f6;
            background-color: #f8fafc;
        }
    )");

    QHBoxLayout *mainLayout = new QHBoxLayout(item);
    mainLayout->setContentsMargins(20, 10, 20, 10);
    mainLayout->setSpacing(20);

    QLabel *avatarLabel = new QLabel(item);
    avatarLabel->setFixedSize(50, 50);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet("background: transparent;");
    avatarLabel->setScaledContents(false);
    avatarLabel->setMask(QRegion(0, 0, 50, 50, QRegion::Ellipse));
    QPixmap listPix;
    QString path = photoPath.isEmpty() ? QString(":/avatar.png") : photoPath;
    if (!listPix.load(path))
        listPix.load(":/avatar.png");
    if (!listPix.isNull())
        avatarLabel->setPixmap(makeCircularPixmap(listPix, 50));
    mainLayout->addWidget(avatarLabel);

    QLabel *nameLabel = new QLabel(QString("%1 %2").arg(prenom).arg(nom));
    nameLabel->setStyleSheet("font-size: 16px; font-weight: 600; color: #1e293b; "
                             "background: transparent; border: none;");
    nameLabel->setFixedWidth(200);
    mainLayout->addWidget(nameLabel);

    QLabel *gradeLabel = new QLabel(grade);
    gradeLabel->setStyleSheet(R"(
        font-size: 12px;
        font-weight: 600;
        color: #059669;
        background-color: #d1fae5;
        padding: 4px 12px;
        border-radius: 12px;
    )");
    gradeLabel->setFixedWidth(150);
    mainLayout->addWidget(gradeLabel);

    QLabel *emailLabel = new QLabel(email);
    emailLabel->setStyleSheet("font-size: 13px; color: #64748b; background: "
                              "transparent; border: none;");
    mainLayout->addWidget(emailLabel, 1);

    if (currentUser.role == UserRole::Admin) {
        QPushButton *btnEdit = new QPushButton("✎");
        btnEdit->setFixedSize(36, 36);
        btnEdit->setToolTip("Modifier");
        btnEdit->setStyleSheet(R"(
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 8px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
        )");
        connect(btnEdit, &QPushButton::clicked, this,
                [this, id]() { on_cherchModifierChercheur(id); });

        QPushButton *btnDelete = new QPushButton("🗑");
        btnDelete->setFixedSize(36, 36);
        btnDelete->setToolTip("Supprimer");
        btnDelete->setStyleSheet(R"(
            QPushButton {
                background-color: #ef4444;
                color: white;
                border: none;
                border-radius: 8px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #dc2626;
            }
        )");
        connect(btnDelete, &QPushButton::clicked, this,
                [this, id]() { on_cherchSupprimerChercheur(id); });

        mainLayout->addWidget(btnEdit);
        mainLayout->addWidget(btnDelete);
    }

    item->installEventFilter(this);
    item->setMouseTracking(true);

    QVBoxLayout *list =
        qobject_cast<QVBoxLayout *>(ui->cherchScrollAreaWidgetContents->layout());
    if (list) {
        list->addWidget(item);

        // Animation
        QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(item);
        opacityEffect->setOpacity(0.0);
        item->setGraphicsEffect(opacityEffect);
        QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
        anim->setDuration(400);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void SmartPub::cherchClearChercheursList() {
    QLayoutItem *child;
    QLayout *layout = ui->cherchScrollAreaWidgetContents->layout();
    if (!layout)
        return;

    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
}

void SmartPub::on_cherchBtnVueListe_clicked() {
    if (!cherchVueListeActive) {
        ui->cherchStackedWidget->setCurrentIndex(0);
        ui->cherchLineEditRecherche->setVisible(true);
        ui->cherchBtnRecherche->setVisible(true);
        ui->cherchBtnTri->setVisible(true);
        ui->cherchBtnExport->setVisible(true);
        ui->cherchBtnStatistiques->setVisible(true);
        if (cherchBtnToggleVue)
            cherchBtnToggleVue->setVisible(true);

        ui->cherchBtnVueListe->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 13px;
                font-weight: 600;
            }
        )");
        ui->cherchBtnAjouter->setStyleSheet(R"(
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
        )");

        ui->cherchBtnVueListe->setChecked(true);
        ui->cherchBtnAjouter->setChecked(false);
        cherchVueListeActive = true;
        cherchAfficherListeChercheurs();
    }
}

void SmartPub::on_cherchBtnAjouter_clicked() {
    if (cherchVueListeActive) {
        ui->cherchStackedWidget->setCurrentIndex(1);
        ui->cherchLineEditRecherche->setVisible(false);
        ui->cherchBtnRecherche->setVisible(false);
        ui->cherchBtnTri->setVisible(false);
        ui->cherchBtnExport->setVisible(false);
        ui->cherchBtnStatistiques->setVisible(false);
        if (cherchBtnToggleVue)
            cherchBtnToggleVue->setVisible(false);

        ui->cherchBtnVueListe->setStyleSheet(R"(
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
        )");
        ui->cherchBtnAjouter->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-size: 13px;
                font-weight: 600;
            }
        )");

        ui->cherchBtnVueListe->setChecked(false);
        ui->cherchBtnAjouter->setChecked(true);
        cherchVueListeActive = false;
    }
}

void SmartPub::on_cherchBtnToggleVue_clicked() {
    cherchVueIconesActive = !cherchVueIconesActive;
    if (cherchVueIconesActive) {
        cherchBtnToggleVue->setText("⊞");
        cherchBtnToggleVue->setToolTip("Passer en mode liste");
    } else {
        cherchBtnToggleVue->setText("☰");
        cherchBtnToggleVue->setToolTip("Passer en mode icônes");
    }
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnRecherche_clicked() {
    QString searchText = ui->cherchLineEditRecherche->text().trimmed();
    if (searchText.isEmpty()) {
        cherchWhereClause.clear();
        cherchAfficherListeChercheurs();
        return;
    }
    QString escaped = searchText;
    escaped.replace("'", "''");
    QString likeVal = escaped.toLower();
    cherchWhereClause = QString("(LOWER(NOM) LIKE '%%1%' OR LOWER(PRENOM) LIKE '%%1%' OR LOWER(CIN) LIKE '%%1%')")
                            .arg(likeVal);
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnTri_clicked() {
    QMenu *menu = new QMenu(this);
    menu->setStyleSheet(R"(
        QMenu {
            background-color: white;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            padding: 8px;
            min-width: 250px;
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

    menu->addAction("⬆️  Nom (A → Z)", this,
                    [this]() { cherchTrierParNom(true); });
    menu->addAction("⬇️  Nom (Z → A)", this,
                    [this]() { cherchTrierParNom(false); });
    menu->addSeparator();
    menu->addAction("🎓  Grade (Hiérarchie académique)", this,
                    [this]() { cherchTrierParGrade(); });
    menu->addSeparator();
    menu->addAction("🕐  Date d'ajout (Plus récent)", this,
                    [this]() { cherchTrierParDateCreation(true); });
    menu->addAction("🕓  Date d'ajout (Plus ancien)", this,
                    [this]() { cherchTrierParDateCreation(false); });

    menu->exec(QCursor::pos());
}

void SmartPub::cherchTrierParNom(bool croissant) {
    cherchOrderByClause = croissant ? "NOM ASC, PRENOM ASC" : "NOM DESC, PRENOM DESC";
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParGrade() {
    // CASE WHEN portable Oracle/standard — tri hiérarchique académique
    cherchOrderByClause =
        "CASE GRADE "
        "WHEN 'Professeur' THEN 1 "
        "WHEN 'Maitre de Conferences' THEN 2 "
        "WHEN 'Docteur' THEN 3 "
        "WHEN 'Ingenieur de Recherche' THEN 4 "
        "WHEN 'Post-doctorant' THEN 5 "
        "WHEN 'Doctorant' THEN 6 "
        "ELSE 99 END, NOM ASC, PRENOM ASC";
    cherchAfficherListeChercheurs();
}

void SmartPub::cherchTrierParDateCreation(bool croissant) {
    // ID_CHERCHEUR auto-incrémenté via séquence Oracle = proxy fiable de la date d'insertion
    cherchOrderByClause = croissant ? "ID_CHERCHEUR DESC" : "ID_CHERCHEUR ASC";
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchBtnExport_clicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Exporter la liste des chercheurs",
        QDir::homePath() + "/Chercheurs_SmartPub.csv",
        "Fichier CSV (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Erreur",
                              "Impossible de créer le fichier :\n" + fileName);
        return;
    }

    // UTF-8 avec BOM pour que Excel reconnaisse l'encodage correctement
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "\xEF\xBB\xBF"; // BOM UTF-8

    // ── Helper : encapsuler une valeur dans des guillemets CSV ────────────────
    // Règle CSV : si la valeur contient ",", "\n" ou '"', on entoure de guillemets
    // et on double les guillemets internes.
    auto csvCell = [](const QString &val) -> QString {
        QString v = val;
        v.replace(QLatin1Char('"'), QStringLiteral("\"\""));  // doubler les guillemets
        // Toujours encapsuler pour garantir la lisibilité dans Excel/LibreOffice
        return QStringLiteral("\"") + v + QStringLiteral("\"");
    };

    // ── En-tête ───────────────────────────────────────────────────────────────
    QStringList headers = {
        "ID",
        "Nom",
        "Prénom",
        "Grade",
        "Email",
        "CIN",
        "Carrière",
        "Nb Projets",
        "Codes Projets",
        "Titres des Projets"
    };
    stream << headers.join(";") << "\n";   // séparateur ";" — standard FR pour Excel

    // ── Charger les projets depuis la BD pour chaque chercheur ────────────────
    QSqlDatabase db = Connection::instance()->getDatabase();

    // Précharger TOUS les projets par chercheur en une seule requête
    // Structure : idChercheur → liste de "CODE — TITRE"
    QHash<int, QStringList> projetsParChercheur;
    QHash<int, QStringList> codesParChercheur;
    if (db.isOpen()) {
        QSqlQuery qProj(db);
        if (qProj.exec(
                QStringLiteral(
                    "SELECT c.ID_CHERCHEUR, p.CODE, p.TITRE "
                    "FROM CONTRIBUER c "
                    "INNER JOIN PROJET p ON p.ID_PROJET = c.ID_PROJET "
                    "ORDER BY c.ID_CHERCHEUR, p.CODE"))) {
            while (qProj.next()) {
                int    cid   = qProj.value(0).toInt();
                QString code = qProj.value(1).toString();
                QString titre = qProj.value(2).toString();
                codesParChercheur[cid].append(code);
                projetsParChercheur[cid].append(
                    QString("[%1] %2").arg(code, titre));
            }
        }
    }

    // ── Lignes de données ─────────────────────────────────────────────────────
    // Trier par ID pour un export ordonné
    QList<int> ids = cherchChercheursMap.keys();
    std::sort(ids.begin(), ids.end());

    int nbExportes = 0;
    for (int id : ids) {
        const ChercheurData &d = cherchChercheursMap.value(id);

        // Projets : codes séparés par " | " et titres séparés par " | "
        QStringList codes  = codesParChercheur.value(id);
        QStringList titres = projetsParChercheur.value(id);

        QString codesStr  = codes.isEmpty()  ? "—" : codes.join(" | ");
        QString titresStr = titres.isEmpty() ? "Aucun projet" : titres.join(" | ");

        // Carrière calculée depuis nb projets + grade (déjà dans le cache)
        QString carriere = d.carriere.isEmpty()
                               ? cherchDeterminerCarriere(d.projetsIds.size(), d.grade)
                               : d.carriere;

        QStringList row = {
            csvCell(QString::number(id)),
            csvCell(d.nom),
            csvCell(d.prenom),
            csvCell(d.grade.isEmpty()    ? "—" : d.grade),
            csvCell(d.email.isEmpty()    ? "—" : d.email),
            csvCell(d.cin.isEmpty()      ? "—" : d.cin),
            csvCell(carriere.isEmpty()   ? "—" : carriere),
            csvCell(QString::number(codes.size())),
            csvCell(codesStr),
            csvCell(titresStr)
        };

        stream << row.join(";") << "\n";
        ++nbExportes;
    }

    file.close();

    QMessageBox::information(
        this,
        "Export réussi",
        QString("✅  %1 chercheur(s) exporté(s) avec succès.\n\n"
                "Fichier :\n%2\n\n"
                "💡 Conseil : ouvrez le fichier avec Excel ou LibreOffice Calc.\n"
                "    Si les colonnes ne se séparent pas, utilisez\n"
                "    Données → Convertir → Délimiteur : point-virgule.")
            .arg(nbExportes)
            .arg(fileName));
}

void SmartPub::on_cherchBtnStatistiques_clicked() {
    cherchAfficherStatistiques();
}


void SmartPub::cherchAfficherStatistiques() {
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistiques et métiers innovants — Chercheurs");

    // Dimensionner à 85 % de l'écran disponible, avec un minimum raisonnable
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect available = screen->availableGeometry();
        int w = qMax(860, qRound(available.width()  * 0.85));
        int h = qMax(600, qRound(available.height() * 0.85));
        dialog->resize(w, h);
        // Centrer sur l'écran
        dialog->move(available.center() - QPoint(w / 2, h / 2));
    } else {
        dialog->resize(960, 680);
    }
    dialog->setMinimumSize(760, 540);
    dialog->setStyleSheet(R"(
            QDialog { background-color: #f8fafc; }
            QLabel { color: #1e293b; background: transparent; border: none; }
            QTabWidget::pane {
                border: 1px solid #e2e8f0;
                border-radius: 12px;
                background-color: white;
            }
            QTabBar::tab {
                padding: 10px 20px;
                font-weight: 600;
                color: #64748b;
                background: #f1f5f9;
                border: none;
                border-radius: 6px;
                margin-right: 4px;
            }
            QTabBar::tab:selected {
                color: #1e40af;
                background: white;
                border-bottom: 3px solid #3b82f6;
            }
QComboBox, QSpinBox {
                background-color: #ffffff;
                border: 2px solid #e2e8f0;
                border-radius: 8px;
                padding: 6px 12px;
                color: #1e293b;
                font-size: 14px;
            }
            QComboBox:focus, QSpinBox:focus {
                border: 2px solid #3b82f6;
                background-color: #ffffff;
            }
            QComboBox::drop-down {
                border: none;
                width: 24px;
            }
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                border: 2px solid #e2e8f0;
                border-radius: 8px;
                selection-background-color: #eff6ff;
                selection-color: #1e40af;
                color: #1e293b;
                outline: none;
                padding: 4px;
            }
            QComboBox QAbstractItemView::item {
                padding: 8px 12px;
                border-radius: 4px;
                color: #1e293b;
                min-height: 28px;
            }
            QComboBox QAbstractItemView::item:hover {
                background-color: #f1f5f9;
                color: #1e293b;
            }
            QComboBox QAbstractItemView::item:selected {
                background-color: #eff6ff;
                color: #1e40af;
                font-weight: 600;
            }
            QListWidget {
                background-color: #ffffff;
                border: 2px solid #e2e8f0;
                border-radius: 10px;
                padding: 8px;
                color: #1e293b;
                font-size: 14px;
            }
            QListWidget::item {
                padding: 10px 12px;
                border-radius: 6px;
                color: #1e293b;
                background-color: transparent;
                border-bottom: 1px solid #f1f5f9;
            }
            QListWidget::item:hover {
                background-color: #f8fafc;
                color: #1e293b;
            }
            QListWidget::item:selected {
                background-color: #eff6ff;
                color: #1e40af;
                font-weight: 600;
            }
            /* --- Style pour le Tableau Matchmaking --- */
            QTableWidget {
                background-color: #ffffff;
                border: 1px solid #e2e8f0;
                border-radius: 8px;
                gridline-color: #f1f5f9;
                color: #334155;
                selection-background-color: #eff6ff; /* Bleu très clair au clic */
                selection-color: #1e40af; /* Texte bleu foncé au clic */
                outline: none;
            }
            QTableWidget::item {
                padding: 8px;
                border-bottom: 1px solid #f8fafc;
            }
            QHeaderView::section {
                background-color: #f8fafc;
                color: #475569;
                font-weight: 700;
                padding: 10px;
                border: none;
                border-bottom: 2px solid #e2e8f0;
                border-right: 1px solid #f1f5f9;
            }
            QHeaderView {
                background-color: transparent;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
            }

            /* --- Style pour les Scrollbars (Vue d'ensemble et Listes) --- */
            QScrollBar:vertical {
                border: none;
                background: #f8fafc;
                width: 10px;
                margin: 0px 0px 0px 0px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical {
                background: #cbd5e1;
                min-height: 30px;
                border-radius: 5px;
            }
            QScrollBar::handle:vertical:hover {
                background: #94a3b8; /* Devient plus foncé au survol */
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px; /* Cache les petites flèches de défilement (design moderne) */
            }
            QScrollBar:horizontal {
                border: none;
                background: #f8fafc;
                height: 10px;
                border-radius: 5px;
            }
            QScrollBar::handle:horizontal {
                background: #cbd5e1;
                min-width: 30px;
                border-radius: 5px;
            }
            QScrollBar::handle:horizontal:hover {
                background: #94a3b8;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                width: 0px;
            }
        )");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    QLabel *titleLabel = new QLabel("Tableau de bord — Chercheurs", dialog);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: 700; color: "
                              "#1e293b; background: transparent; border: none;");
    mainLayout->addWidget(titleLabel);

    auto createStatCard = [](const QString &title, const QString &value,
                             const QString &color) -> QFrame * {
        QFrame *card = new QFrame();
        card->setStyleSheet(QString(R"(
            QFrame {
                background-color: white;
                border-radius: 16px;
                border: 1px solid #e2e8f0;
            }
        )"));
        card->setMinimumHeight(120);
        QVBoxLayout *layout = new QVBoxLayout(card);
        layout->setSpacing(8);
        layout->setContentsMargins(20, 20, 20, 20);

        QLabel *t = new QLabel(title);
        t->setStyleSheet("color: #64748b; font-size: 13px; font-weight: "
                         "600; background: transparent; border: none;");

        QLabel *v = new QLabel(value);
        v->setStyleSheet(QString("color: %1; font-size: 40px; font-weight: 700; background: "
                                 "transparent; border: none;")
                             .arg(color));

        layout->addWidget(t);
        layout->addWidget(v);
        layout->addStretch();
        return card;
    };

    int nbProfs = 0, nbDocs = 0;
    for (auto &data : cherchChercheursMap) {
        if (data.grade == QLatin1String("Professeur"))
            nbProfs++;
        if (data.grade == QLatin1String("Doctorant"))
            nbDocs++;
    }

    QGridLayout *statsGrid = new QGridLayout();
    statsGrid->setSpacing(16);
    statsGrid->addWidget(
        createStatCard("Total chercheurs", QString::number(cherchChercheursMap.size()), "#3b82f6"),
        0, 0);
    statsGrid->addWidget(createStatCard("Professeurs", QString::number(nbProfs), "#10b981"), 0, 1);
    statsGrid->addWidget(createStatCard("Doctorants", QString::number(nbDocs), "#f59e0b"), 0, 2);

    QTabWidget *tabs = new QTabWidget(dialog);
    tabs->setDocumentMode(true);
    tabs->setStyleSheet(R"(
        QTabWidget::pane {
            border: 2px solid #e2e8f0;
            border-radius: 12px;
            background-color: #ffffff;
            margin-top: -1px;
        }
        QTabBar::tab {
            padding: 10px 20px;
            font-weight: 600;
            color: #64748b;
            background-color: #f1f5f9;
            border: 1px solid #e2e8f0;
            border-bottom: none;
            border-radius: 8px 8px 0 0;
            margin-right: 4px;
            min-width: 140px;
        }
        QTabBar::tab:selected {
            color: #1e40af;
            background-color: #ffffff;
            border-bottom: 3px solid #3b82f6;
        }
        QTabBar::tab:hover:!selected {
            background-color: #e2e8f0;
            color: #334155;
        }
    )");

    QWidget *tabOverview = new QWidget();
    tabOverview->setStyleSheet("background-color: #f8fafc;");
    QVBoxLayout *ovMain = new QVBoxLayout(tabOverview);
    ovMain->setSpacing(16);
    ovMain->setContentsMargins(16, 16, 16, 16);
    ovMain->addLayout(statsGrid);

    QScrollArea *scrollOverview = new QScrollArea();
    scrollOverview->setWidgetResizable(true);
    scrollOverview->setFrameShape(QFrame::NoFrame);
    scrollOverview->setStyleSheet(
        "QScrollArea { background-color: #f8fafc; border: none; }"
        "QScrollArea > QWidget > QWidget { background-color: #f8fafc; }");
    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: #f8fafc;");
    QVBoxLayout *ovScrollLay = new QVBoxLayout(scrollContent);
    ovScrollLay->setSpacing(20);
    ovScrollLay->setContentsMargins(0, 8, 0, 8);

    QHBoxLayout *chartsRow = new QHBoxLayout();
    chartsRow->setSpacing(16);

    QSqlDatabase db = Connection::instance()->getDatabase();
    QStringList labNames;
    QList<double> labCounts;
    QStringList labNamesSat;
    QList<double> labSurcharge;

    if (db.isOpen()) {
        QSqlQuery qLab(db);
        const QString sqlEffectifs =
            QStringLiteral("SELECT L.NOM, COUNT(DISTINCT C.ID_CHERCHEUR) AS NB "
                           "FROM LABORATOIRE L "
                           "INNER JOIN CONTRIBUER C ON C.CODE_PROJET = L.CODE_PROJET "
                           "GROUP BY L.ID_LABORATOIRE, L.NOM ORDER BY L.NOM");
        if (qLab.exec(sqlEffectifs)) {
            while (qLab.next()) {
                labNames << qLab.value(0).toString();
                labCounts << qLab.value(1).toDouble();
            }
        }
        QSqlQuery qSat(db);
        const QString sqlSat =
            QStringLiteral("SELECT L.NOM, AVG(LEAST(cnt * 25, 100)) AS SAT "
                           "FROM ( "
                           "  SELECT L2.ID_LABORATOIRE, C.ID_CHERCHEUR, COUNT(DISTINCT C.CODE_PROJET) AS cnt "
                           "  FROM LABORATOIRE L2 "
                           "  INNER JOIN CONTRIBUER C ON C.CODE_PROJET = L2.CODE_PROJET "
                           "  GROUP BY L2.ID_LABORATOIRE, C.ID_CHERCHEUR "
                           ") X "
                           "JOIN LABORATOIRE L ON L.ID_LABORATOIRE = X.ID_LABORATOIRE "
                           "GROUP BY L.ID_LABORATOIRE, L.NOM ORDER BY L.NOM");
        if (qSat.exec(sqlSat)) {
            while (qSat.next()) {
                labNamesSat << qSat.value(0).toString();
                labSurcharge << qSat.value(1).toDouble();
            }
        }
    }

    auto makeBarChartView = [](const QString &title, const QStringList &categories,
                               const QList<double> &values, const QString &colorHex,
                               const QString &yLabel) -> QChartView * {
        QChartView *cv = new QChartView();
        cv->setRenderHint(QPainter::Antialiasing);
        cv->setMinimumHeight(320);

        QBarSet *set = new QBarSet("Valeur");
        for (double v : values)
            *set << v;
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
        chart->setBackgroundRoundness(0);
        chart->setBackgroundBrush(QBrush(Qt::transparent));
        chart->setPlotAreaBackgroundBrush(QBrush(QColor("#ffffff")));
        chart->setPlotAreaBackgroundVisible(true);
        chart->legend()->setVisible(false);
        chart->setMargins(QMargins(8, 8, 8, 8));

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        for (const QString &c : categories)
            axisX->append(c);
        axisX->setLabelsAngle(-25);
        axisX->setLabelsBrush(QBrush(QColor("#475569")));
        axisX->setLinePenColor(QColor("#e2e8f0"));
        axisX->setGridLinePen(QPen(QColor("#f1f5f9")));
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        double vmax = 1.0;
        for (double v : values)
            vmax = qMax(vmax, v);
        axisY->setRange(0, vmax * 1.15 + 0.5);
        axisY->setLabelFormat("%.0f");
        axisY->setTitleText(yLabel);
        axisY->setLabelsBrush(QBrush(QColor("#475569")));
        axisY->setTitleBrush(QBrush(QColor("#64748b")));
        axisY->setLinePenColor(QColor("#e2e8f0"));
        axisY->setGridLinePen(QPen(QColor("#f1f5f9")));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        cv->setChart(chart);
        cv->setStyleSheet("background: transparent; border: none;");
        cv->setBackgroundBrush(QBrush(Qt::transparent));
        return cv;
    };

    if (!labNames.isEmpty() && labCounts.size() == labNames.size()) {
        chartsRow->addWidget(makeBarChartView(
            "Effectifs par laboratoire (chercheurs distincts via projets)", labNames, labCounts,
            "#3b82f6", "Nombre de chercheurs"));
    } else {
        QFrame *emptyFrame = new QFrame();
        emptyFrame->setStyleSheet(
            "QFrame { background-color: #fff7ed; border: 1px solid #fed7aa; "
            "border-radius: 12px; }");
        QHBoxLayout *emptyLay = new QHBoxLayout(emptyFrame);
        emptyLay->setContentsMargins(16, 14, 16, 14);
        QLabel *empty = new QLabel(
            "⚠️  Aucune donnée laboratoire — vérifiez les tables LABORATOIRE, CONTRIBUER et les clés CODE_PROJET.");
        empty->setWordWrap(true);
        empty->setStyleSheet(
            "color: #92400e; font-size: 13px; font-weight: 500; "
            "background: transparent; border: none;");
        emptyLay->addWidget(empty);
        chartsRow->addWidget(emptyFrame);
    }

    if (!labNamesSat.isEmpty() && labSurcharge.size() == labNamesSat.size()) {
        chartsRow->addWidget(makeBarChartView(
            "Taux de charge moyen par laboratoire (min(n×25, 100) par chercheur)", labNamesSat,
            labSurcharge, "#8b5cf6", "Score sur 100"));
    }

    QFrame *chartsFrame = new QFrame();
    chartsFrame->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 16px; border: 1px solid #e2e8f0; }");
    QVBoxLayout *chartsFrameLay = new QVBoxLayout(chartsFrame);
    chartsFrameLay->setContentsMargins(16, 16, 16, 16);
    QLabel *chartsTitle = new QLabel("Statistiques par laboratoire");
    chartsTitle->setStyleSheet(
        "font-size: 17px; font-weight: 600; color: #1e293b; "
        "background: transparent; border: none;");
    chartsFrameLay->addWidget(chartsTitle);
    chartsFrameLay->addLayout(chartsRow);
    ovScrollLay->addWidget(chartsFrame);

    QFrame *gradeFrame = new QFrame();
    gradeFrame->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 16px; "
        "border: 1px solid #e2e8f0; } "
        "QFrame QLabel { background: transparent; border: none; } "
        "QFrame QProgressBar { border: none; }");
    QVBoxLayout *gradeLay = new QVBoxLayout(gradeFrame);
    gradeLay->setContentsMargins(20, 20, 20, 20);
    QLabel *gradeTitle = new QLabel("Répartition par grade");
    gradeTitle->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
    gradeLay->addWidget(gradeTitle);

    QMap<QString, int> gradeCount;
    for (auto &data : cherchChercheursMap)
        gradeCount[data.grade]++;

    int totalC = qMax(1, cherchChercheursMap.size());
    for (auto it = gradeCount.begin(); it != gradeCount.end(); ++it) {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *gradeLabel = new QLabel(it.key().isEmpty() ? QStringLiteral("(non renseigné)") : it.key());
        gradeLabel->setStyleSheet("font-size: 14px; color: #334155; font-weight: 600;");
        gradeLabel->setFixedWidth(200);
        QProgressBar *progress = new QProgressBar();
        progress->setRange(0, totalC);
        progress->setValue(it.value());
        progress->setTextVisible(true);
        progress->setFormat(QString("%1 chercheur(s)").arg(it.value()));
        progress->setStyleSheet(R"(
            QProgressBar { border: none; border-radius: 8px; background-color: #e2e8f0; text-align: center; height: 22px; }
            QProgressBar::chunk { background-color: #0ea5e9; border-radius: 8px; }
        )");
        row->addWidget(gradeLabel);
        row->addWidget(progress, 1);
        gradeLay->addLayout(row);
    }
    ovScrollLay->addWidget(gradeFrame);

    QFrame *overloadFrame = new QFrame();
    overloadFrame->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 16px; "
        "border: 1px solid #e2e8f0; } "
        "QFrame QLabel { background: transparent; border: none; } "
        "QFrame QProgressBar { border: none; }");
    QVBoxLayout *overloadLayout = new QVBoxLayout(overloadFrame);
    overloadLayout->setContentsMargins(20, 20, 20, 20);
    QLabel *overloadTitle = new QLabel("Indice de surcharge par chercheur (projets affectés)");
    overloadTitle->setStyleSheet("font-size: 17px; font-weight: 600; color: #1e293b;");
    overloadLayout->addWidget(overloadTitle);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *headerName = new QLabel("Chercheur");
    headerName->setStyleSheet("color: #64748b; font-weight: 600; font-size: 12px;");
    headerName->setFixedWidth(200);
    QLabel *headerProgress = new QLabel("Charge (n projets)");
    headerProgress->setStyleSheet("color: #64748b; font-weight: 600; font-size: 12px;");
    QLabel *headerStatus = new QLabel("Statut");
    headerStatus->setStyleSheet("color: #64748b; font-weight: 600; font-size: 12px;");
    headerStatus->setFixedWidth(90);
    headerRow->addWidget(headerName);
    headerRow->addWidget(headerProgress, 1);
    headerRow->addWidget(headerStatus);
    overloadLayout->addLayout(headerRow);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e2e8f0; max-height: 1px;");
    overloadLayout->addWidget(line);

    for (auto it = cherchChercheursMap.begin(); it != cherchChercheursMap.end(); ++it) {
        const ChercheurData &data = it.value();
        int nbProjets = data.projetsIds.size();
        int surcharge = qMin(nbProjets * 25, 100);

        QHBoxLayout *rowLayout = new QHBoxLayout();
        QLabel *nameLabel =
            new QLabel(QString("%1 %2").arg(data.prenom, data.nom));
        nameLabel->setFixedWidth(200);
        nameLabel->setStyleSheet("font-weight: 600; color: #334155;");

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(surcharge);
        progressBar->setTextVisible(true);
        progressBar->setFormat(QString("%1 projet(s)").arg(nbProjets));
        progressBar->setFixedHeight(26);

        QString color;
        if (surcharge < 50)
            color = "#10b981";
        else if (surcharge < 75)
            color = "#f59e0b";
        else
            color = "#ef4444";

        progressBar->setStyleSheet(QString(R"(
            QProgressBar { border: none; border-radius: 13px; background-color: #e2e8f0; text-align: center; color: white; font-weight: 600; font-size: 11px; }
            QProgressBar::chunk { background-color: %1; border-radius: 13px; }
        )")
                                       .arg(color));

        QLabel *statusLabel = new QLabel();
        if (surcharge < 50)
            statusLabel->setText("Normal");
        else if (surcharge < 75)
            statusLabel->setText("Occupé");
        else
            statusLabel->setText("Surchargé");
        statusLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(color));
        statusLabel->setFixedWidth(90);

        rowLayout->addWidget(nameLabel);
        rowLayout->addWidget(progressBar, 1);
        rowLayout->addWidget(statusLabel);
        overloadLayout->addLayout(rowLayout);
    }

    ovScrollLay->addWidget(overloadFrame);
    ovScrollLay->addStretch();
    scrollOverview->setWidget(scrollContent);
    ovMain->addWidget(scrollOverview, 1);
    tabs->addTab(tabOverview, "Vue d'ensemble");

    QWidget *tabPromo = new QWidget();
    tabPromo->setStyleSheet("background-color: #f8fafc;");
    QVBoxLayout *tabPromoOuterLay = new QVBoxLayout(tabPromo);
    tabPromoOuterLay->setContentsMargins(0, 0, 0, 0);
    tabPromoOuterLay->setSpacing(0);

    // ScrollArea pour éviter la compression quand le dialog est redimensionné
    QScrollArea *promoScroll = new QScrollArea();
    promoScroll->setWidgetResizable(true);
    promoScroll->setFrameShape(QFrame::NoFrame);
    promoScroll->setStyleSheet(
        "QScrollArea { background-color: #f8fafc; border: none; }"
        "QScrollArea > QWidget > QWidget { background-color: #f8fafc; }");

    QWidget *promoScrollContent = new QWidget();
    promoScrollContent->setStyleSheet("background-color: #f8fafc;");
    QVBoxLayout *promoLay = new QVBoxLayout(promoScrollContent);
    promoLay->setContentsMargins(16, 16, 16, 16);
    promoLay->setSpacing(12);

    // ── Carte : Sélection du chercheur + critères ─────────────────────────────
    QFrame *promoFormCard = new QFrame();
    promoFormCard->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 14px; border: 1px solid #e2e8f0; }"
        "QLabel { background: transparent; border: none; color: #334155; font-size: 13px; font-weight: 600; }"
        "QComboBox, QSpinBox { background-color: #ffffff; border: 2px solid #e2e8f0; border-radius: 8px; "
        "                      padding: 6px 10px; color: #1e293b; font-size: 13px; }"
        "QComboBox:focus, QSpinBox:focus { border-color: #3b82f6; }"
        "QComboBox QAbstractItemView { background: #ffffff; color: #1e293b; border: 2px solid #e2e8f0; "
        "                              selection-background-color: #eff6ff; selection-color: #1e40af; }"
        "QComboBox QAbstractItemView::item { color: #1e293b; padding: 6px 10px; }"
        );
    QVBoxLayout *promoFormCardLay = new QVBoxLayout(promoFormCard);
    promoFormCardLay->setContentsMargins(20, 16, 20, 16);
    promoFormCardLay->setSpacing(10);

    QLabel *promoFormTitle = new QLabel("⚙️  Paramètres d'évaluation");
    promoFormTitle->setStyleSheet(
        "font-size: 15px; font-weight: 700; color: #1e293b; "
        "background: transparent; border: none;");
    promoFormCardLay->addWidget(promoFormTitle);

    QFormLayout *promoForm = new QFormLayout();
    promoForm->setSpacing(8);
    promoForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QList<int> idsSorted = cherchChercheursMap.keys();
    std::sort(idsSorted.begin(), idsSorted.end());

    QComboBox *comboPromo = new QComboBox();
    for (int id : idsSorted) {
        const ChercheurData &d = cherchChercheursMap[id];
        comboPromo->addItem(
            QString("%1 %2").arg(d.prenom, d.nom), id);
    }

    PromotionCriteria critDefaults;
    QSpinBox *spinAns  = new QSpinBox(); spinAns->setRange(0, 50);
    QSpinBox *spinPub  = new QSpinBox(); spinPub->setRange(0, 500);
    QSpinBox *spinProj = new QSpinBox(); spinProj->setRange(0, 100);
    spinAns->setValue(critDefaults.minAnneesAnciennete);
    spinPub->setValue(critDefaults.minPublications);
    spinProj->setValue(critDefaults.minProjetsGeres);

    promoForm->addRow("Chercheur :", comboPromo);
    promoForm->addRow("Seuil ancienneté (années) :", spinAns);
    promoForm->addRow("Seuil publications :", spinPub);
    promoForm->addRow("Seuil projets (contributions) :", spinProj);
    promoFormCardLay->addLayout(promoForm);
    promoLay->addWidget(promoFormCard);

    // ── Carte : Score visuel ──────────────────────────────────────────────────
    QFrame *scoreCard = new QFrame();
    scoreCard->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 14px; border: 1px solid #e2e8f0; }"
        "QLabel { background: transparent; border: none; }");
    QHBoxLayout *scoreLay = new QHBoxLayout(scoreCard);
    scoreLay->setContentsMargins(20, 16, 20, 16);
    scoreLay->setSpacing(20);

    // Cercle de score (label grand)
    QLabel *scoreCircle = new QLabel("—");
    scoreCircle->setFixedSize(90, 90);
    scoreCircle->setAlignment(Qt::AlignCenter);
    scoreCircle->setStyleSheet(
        "font-size: 26px; font-weight: 800; color: #64748b; "
        "border: 4px solid #e2e8f0; border-radius: 45px; background: #f8fafc;");

    QVBoxLayout *scoreRightLay = new QVBoxLayout();
    scoreRightLay->setSpacing(6);

    QLabel *promoVerdict = new QLabel("Sélectionnez un chercheur pour évaluer son éligibilité.");
    promoVerdict->setWordWrap(true);
    promoVerdict->setStyleSheet(
        "font-size: 15px; color: #475569; background: transparent; border: none;");

    QProgressBar *scoreBar = new QProgressBar();
    scoreBar->setRange(0, 100);
    scoreBar->setValue(0);
    scoreBar->setTextVisible(false);
    scoreBar->setFixedHeight(10);
    scoreBar->setStyleSheet(
        "QProgressBar { border: none; border-radius: 5px; background: #e2e8f0; }"
        "QProgressBar::chunk { border-radius: 5px; background: #64748b; }");

    QLabel *scoreLabel = new QLabel("Score : — / 10");
    scoreLabel->setStyleSheet(
        "font-size: 12px; color: #64748b; background: transparent; border: none;");

    scoreRightLay->addWidget(promoVerdict);
    scoreRightLay->addWidget(scoreBar);
    scoreRightLay->addWidget(scoreLabel);

    scoreLay->addWidget(scoreCircle);
    scoreLay->addLayout(scoreRightLay, 1);
    promoLay->addWidget(scoreCard);

    // ── Carte : Détails des critères ──────────────────────────────────────────
    QFrame *detailsCard = new QFrame();
    detailsCard->setStyleSheet(
        "QFrame { background-color: #ffffff; border-radius: 14px; border: 1px solid #e2e8f0; }"
        "QLabel { background: transparent; border: none; }");
    QVBoxLayout *detailsCardLay = new QVBoxLayout(detailsCard);
    detailsCardLay->setContentsMargins(20, 14, 20, 14);
    detailsCardLay->setSpacing(8);

    QLabel *detailsTitle = new QLabel("📋  Détail des critères");
    detailsTitle->setStyleSheet(
        "font-size: 14px; font-weight: 700; color: #1e293b; "
        "background: transparent; border: none;");
    detailsCardLay->addWidget(detailsTitle);

    QListWidget *promoDetails = new QListWidget();
    promoDetails->setMinimumHeight(130);
    promoDetails->setStyleSheet(R"(
        QListWidget {
            background-color: #f8fafc;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            padding: 6px;
            color: #1e293b;
            font-size: 13px;
        }
        QListWidget::item {
            padding: 7px 10px;
            border-radius: 6px;
            color: #1e293b;
            border-bottom: 1px solid #f1f5f9;
        }
        QListWidget::item:hover { background-color: #f1f5f9; }
        QListWidget::item:selected {
            background-color: #eff6ff;
            color: #1e40af;
        }
    )");
    detailsCardLay->addWidget(promoDetails);
    promoLay->addWidget(detailsCard);

    // ── Boutons en bas : Évaluer (gauche) + Promouvoir (droite) ──────────────
    QPushButton *btnEvalPromo = new QPushButton("🔍  Évaluer l'éligibilité");
    btnEvalPromo->setCursor(Qt::PointingHandCursor);
    btnEvalPromo->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none; "
        "border-radius: 10px; padding: 10px 22px; font-weight: 600; font-size: 13px; }"
        "QPushButton:hover { background-color: #2563eb; }");

    QPushButton *btnPromote = new QPushButton("⬆️  Promouvoir ce chercheur");
    btnPromote->setCursor(Qt::PointingHandCursor);
    btnPromote->setEnabled(false); // Désactivé tant que non éligible
    btnPromote->setStyleSheet(
        "QPushButton { background-color: #d1fae5; color: #065f46; border: 2px solid #a7f3d0; "
        "border-radius: 10px; padding: 10px 22px; font-weight: 600; font-size: 13px; }"
        "QPushButton:enabled { background-color: #10b981; color: white; border: 2px solid #059669; }"
        "QPushButton:enabled:hover { background-color: #059669; }"
        "QPushButton:disabled { background-color: #f1f5f9; color: #94a3b8; border: 2px solid #e2e8f0; }");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addWidget(btnEvalPromo, 0, Qt::AlignLeft);
    btnRow->addStretch();
    btnRow->addWidget(btnPromote, 0, Qt::AlignRight);
    promoLay->addLayout(btnRow);
    promoLay->addStretch();

    // ── Logique runPromotion ──────────────────────────────────────────────────
    // On stocke le profil courant pour le bouton "Promouvoir"
    struct PromoState { ChercheurPromotionProfile profile; bool eligible = false; };
    auto *state = new PromoState();

    auto runPromotion = [=]() {
        if (!db.isOpen() || comboPromo->count() == 0) {
            promoVerdict->setText("Base indisponible ou aucun chercheur chargé.");
            promoDetails->clear();
            btnPromote->setEnabled(false);
            return;
        }
        PromotionCriteria c;
        c.minAnneesAnciennete = spinAns->value();
        c.minPublications     = spinPub->value();
        c.minProjetsGeres     = spinProj->value();

        int cid = comboPromo->currentData().toInt();
        QSqlDatabase dbConn(db);
        ChercheurPromotionProfile p = PromotionEngine::loadProfile(dbConn, cid);
        state->profile  = p;
        state->eligible = PromotionEngine::isEligible(p, c);
        double score    = PromotionEngine::eligibilityScore(p, c);

        // Score en % pour la barre (score est sur 10)
        int scorePct = qRound(score * 10.0); // sur 100
        scoreBar->setValue(scorePct);

        // Texte du cercle
        scoreCircle->setText(QString("%1").arg(score, 0, 'f', 1));
        scoreLabel->setText(QString("Score de complétude : %1 / 10").arg(score, 0, 'f', 1));

        QString next = PromotionEngine::nextGrade(p.grade);

        if (state->eligible) {
            // Couleur verte
            scoreCircle->setStyleSheet(
                "font-size: 22px; font-weight: 800; color: #059669; "
                "border: 4px solid #10b981; border-radius: 45px; background: #d1fae5;");
            scoreBar->setStyleSheet(
                "QProgressBar { border: none; border-radius: 5px; background: #d1fae5; }"
                "QProgressBar::chunk { border-radius: 5px; background: #10b981; }");
            scoreLabel->setStyleSheet(
                "font-size: 12px; color: #059669; background: transparent; border: none;");

            QString verdictHtml = QString(
                "<span style='color:#059669;font-weight:700;font-size:16px'>✅ Éligible</span>"
                " à la promotion");
            if (!next.isEmpty())
                verdictHtml += QString(" vers <b>%1</b>").arg(next);
            promoVerdict->setText(verdictHtml);
            btnPromote->setEnabled(!next.isEmpty());
            if (!next.isEmpty())
                btnPromote->setText(QString("⬆️  Promouvoir → %1").arg(next));
        } else {
            // Couleur rouge
            scoreCircle->setStyleSheet(
                "font-size: 22px; font-weight: 800; color: #b91c1c; "
                "border: 4px solid #ef4444; border-radius: 45px; background: #fee2e2;");
            scoreBar->setStyleSheet(
                "QProgressBar { border: none; border-radius: 5px; background: #fee2e2; }"
                "QProgressBar::chunk { border-radius: 5px; background: #ef4444; }");
            scoreLabel->setStyleSheet(
                "font-size: 12px; color: #b91c1c; background: transparent; border: none;");
            promoVerdict->setText(
                "<span style='color:#b91c1c;font-weight:700;font-size:16px'>❌ Non éligible</span>"
                " — critères non atteints");
            btnPromote->setEnabled(false);
            btnPromote->setText("⬆️  Promouvoir ce chercheur");
        }

        promoDetails->clear();
        for (const QString &line : PromotionEngine::detailChecks(p, c)) {
            QListWidgetItem *item = new QListWidgetItem(line);
            item->setForeground(QColor("#1e293b"));
            promoDetails->addItem(item);
        }
    };

    // ── Connexions évaluation ─────────────────────────────────────────────────
    connect(btnEvalPromo, &QPushButton::clicked, dialog, [runPromotion]() { runPromotion(); });
    connect(comboPromo,   QOverload<int>::of(&QComboBox::currentIndexChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    connect(spinAns,  QOverload<int>::of(&QSpinBox::valueChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    connect(spinPub,  QOverload<int>::of(&QSpinBox::valueChanged), dialog,
            [runPromotion](int) { runPromotion(); });
    connect(spinProj, QOverload<int>::of(&QSpinBox::valueChanged), dialog,
            [runPromotion](int) { runPromotion(); });

    // ── Connexion bouton Promouvoir ───────────────────────────────────────────
    connect(btnPromote, &QPushButton::clicked, dialog, [=]() {
        if (!state->eligible) return;
        const QString nextG = PromotionEngine::nextGrade(state->profile.grade);
        if (nextG.isEmpty()) return;

        const QString nom = QString("%1 %2")
                                .arg(state->profile.prenom, state->profile.nom).trimmed();
        auto reply = QMessageBox::question(
            dialog,
            "Confirmer la promotion",
            QString("Voulez-vous promouvoir\n\n"
                    "  %1\n\n"
                    "du grade  « %2 »\n"
                    "vers le grade  « %3 » ?\n\n"
                    "Cette action modifie la base de données.")
                .arg(nom, state->profile.grade, nextG),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (reply != QMessageBox::Yes) return;

        QSqlDatabase dbConn(db);
        if (PromotionEngine::promoteInDatabase(dbConn, state->profile.id, nextG)) {
            QMessageBox::information(
                dialog,
                "Promotion effectuée",
                QString("✅  %1 a été promu(e) au grade\n« %2 ».")
                    .arg(nom, nextG));

            // Mettre à jour le cache local pour cohérence immédiate
            if (cherchChercheursMap.contains(state->profile.id))
                cherchChercheursMap[state->profile.id].grade = nextG;

            // Relancer l'évaluation pour refléter le nouveau grade
            runPromotion();
        } else {
            QMessageBox::critical(
                dialog,
                "Erreur",
                "❌  La mise à jour de la base de données a échoué.\n"
                "Vérifiez votre connexion et vos droits.");
        }
    });

    promoScroll->setWidget(promoScrollContent);
    tabPromoOuterLay->addWidget(promoScroll);
    tabs->addTab(tabPromo, "Prédicteur de promotion");
    runPromotion();

    QWidget *tabMatch = new QWidget();
    QVBoxLayout *matchLay = new QVBoxLayout(tabMatch);
    matchLay->setContentsMargins(16, 16, 16, 16);
    matchLay->setSpacing(12);

    QLabel *matchInfo = new QLabel(
        "Suggestions de collègues ayant des mots-clés proches (titres de publications), "
        "sans collaboration passée sur un même projet (CONTRIBUER). Classement par indice de Jaccard.");
    matchInfo->setWordWrap(true);
    matchInfo->setStyleSheet("color: #64748b; font-size: 13px;");
    matchLay->addWidget(matchInfo);

    QHBoxLayout *matchRow = new QHBoxLayout();
    QComboBox *comboMatch = new QComboBox();
    for (int id : idsSorted) {
        const ChercheurData &d = cherchChercheursMap[id];
        comboMatch->addItem(QString("%1 %2 — id %3").arg(d.prenom, d.nom).arg(id), id);
    }
    QPushButton *btnMatch = new QPushButton("Actualiser les suggestions");
    btnMatch->setCursor(Qt::PointingHandCursor);
    btnMatch->setStyleSheet(
        "QPushButton { background-color: #10b981; color: white; border: none; border-radius: 10px; "
        "padding: 10px 20px; font-weight: 600; }"
        "QPushButton:hover { background-color: #059669; }");
    matchRow->addWidget(new QLabel("Chercheur de référence:"));
    matchRow->addWidget(comboMatch, 1);
    matchRow->addWidget(btnMatch);
    matchLay->addLayout(matchRow);

    QTableWidget *tableMatch = new QTableWidget(0, 4);
    tableMatch->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableMatch->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableMatch->setSelectionMode(QAbstractItemView::SingleSelection);
    tableMatch->setShowGrid(false);
    tableMatch->setAlternatingRowColors(true);
    tableMatch->setSortingEnabled(false); // Désactive le tri automatique qui perturbe les colonnes
    tableMatch->verticalHeader()->setVisible(false);
    tableMatch->horizontalHeader()->setSectionsMovable(false); // Empêche le déplacement des colonnes
    tableMatch->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed); // Largeurs fixes
    tableMatch->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Nom complet" << "Indice Jaccard" << "Mots-clés communs");
    // Largeurs fixes pour chaque colonne — stable à chaque actualisation
    tableMatch->setColumnWidth(0, 60);   // ID
    tableMatch->setColumnWidth(1, 280);  // Nom
    tableMatch->setColumnWidth(2, 130);  // Score Jaccard
    tableMatch->setColumnWidth(3, 140);  // Mots communs
    tableMatch->horizontalHeader()->setStretchLastSection(false);
    // Étirer la colonne "Nom" pour occuper l'espace restant
    tableMatch->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableMatch->setStyleSheet(R"(
        QTableWidget {
            gridline-color: #f1f5f9;
            background-color: #ffffff;
            border: none;
            color: #1e293b;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 10px 8px;
            color: #1e293b;
            border-bottom: 1px solid #f1f5f9;
        }
        QTableWidget::item:selected {
            background-color: #eff6ff;
            color: #1e40af;
        }
        QTableWidget::item:alternate {
            background-color: #f8fafc;
        }
        QHeaderView::section {
            background-color: #f1f5f9;
            color: #475569;
            font-weight: 700;
            font-size: 12px;
            padding: 10px 8px;
            border: none;
            border-bottom: 2px solid #e2e8f0;
        }
    )");
    matchLay->addWidget(tableMatch);

    auto runMatch = [=]() {
        tableMatch->setRowCount(0);
        if (!db.isOpen() || comboMatch->count() == 0)
            return;
        int cid = comboMatch->currentData().toInt();
        QSqlDatabase dbConn(db);
        QVector<MatchCandidate> vec = MatchmakingEngine::suggestColleagues(dbConn, cid, 20);
        tableMatch->setRowCount(vec.size());
        for (int i = 0; i < vec.size(); ++i) {
            const MatchCandidate &m = vec[i];
            tableMatch->setItem(i, 0, new QTableWidgetItem(QString::number(m.idChercheur)));
            tableMatch->setItem(i, 1, new QTableWidgetItem(m.nomComplet));
            tableMatch->setItem(i, 2,
                                new QTableWidgetItem(QString::number(m.scoreJaccard, 'f', 3)));
            tableMatch->setItem(i, 3, new QTableWidgetItem(QString::number(m.nbMotsCommuns)));
        }
    };
    connect(btnMatch, &QPushButton::clicked, dialog, [runMatch]() { runMatch(); });
    connect(comboMatch, QOverload<int>::of(&QComboBox::currentIndexChanged), dialog,
            [runMatch](int) { runMatch(); });
    tabs->addTab(tabMatch, "Smart matchmaking");
    runMatch();

    mainLayout->addWidget(tabs, 1);

    QPushButton *btnClose = new QPushButton("Fermer", dialog);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:1 #10b981); "
        "color: white; border: none; border-radius: 12px; padding: 12px 40px; font-size: 15px; font-weight: 600; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563eb, stop:1 #059669); }");
    connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);
    mainLayout->addWidget(btnClose, 0, Qt::AlignCenter);

    dialog->exec();
}

void SmartPub::on_cherchBtnUploadPhoto_clicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Photo", QDir::homePath(), "Images (*.png *.jpg *.jpeg)");
    if (fileName.isEmpty()) {
        if (cherchCurrentPhotoPath.isEmpty())
            cherchCurrentPhotoPath = QString(":/avatar.png");
        return;
    }

    cherchCurrentPhotoPath = fileName;

    // ── Afficher l'aperçu de la photo dans le bouton upload ──────────────────
    QPixmap pix(fileName);
    if (!pix.isNull() && ui->cherchBtnUploadPhoto) {
        // Redimensionner le pixmap à la taille du bouton et l'afficher comme icône
        QSize btnSize = ui->cherchBtnUploadPhoto->size();
        int sz = qMin(btnSize.width(), btnSize.height()) - 8;
        QPixmap circ = makeCircularPixmap(pix, sz);
        ui->cherchBtnUploadPhoto->setIcon(QIcon(circ));
        ui->cherchBtnUploadPhoto->setIconSize(QSize(sz, sz));
        ui->cherchBtnUploadPhoto->setText(""); // Retirer le texte "+" quand photo présente
    }

    // ── Mettre à jour le label hint ───────────────────────────────────────────
    if (ui->cherchLabelPhotoHint) {
        ui->cherchLabelPhotoHint->setText("✓  Photo sélectionnée");
        ui->cherchLabelPhotoHint->setStyleSheet(
            "color: #10b981; font-size: 12px; font-weight: 600; "
            "background: transparent; border: none;");
    }
}

void SmartPub::on_cherchBtnAjouterChercheur_clicked() {
    QString nom = ui->cherchLineEditNom->text().trimmed();
    QString prenom = ui->cherchLineEditPrenom->text().trimmed();
    QString cin = ui->cherchLineEditCIN->text().trimmed();
    QString email = ui->cherchLineEditEmail->text().trimmed();
    QString grade = ui->cherchComboBoxGrade->currentText().trimmed();

    if (nom.isEmpty() || prenom.isEmpty() || cin.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "Veuillez remplir tous les champs obligatoires (*)");
        return;
    }

    // Validation nom et prénom (uniquement caractères alphabétiques)
    static const QRegularExpression nameRegex(
        QStringLiteral(R"(^[A-Za-zÀ-ÖØ-öø-ÿ\s'-]+$)"));
    if (!nameRegex.match(nom).hasMatch() || !nameRegex.match(prenom).hasMatch()) {
        QMessageBox::warning(this, "Erreur",
                             "Le nom et le prénom ne doivent contenir que des lettres.");
        return;
    }
    if (email.isEmpty() || grade.isEmpty()) {
        QMessageBox::warning(this, "Erreur",
                             "L'email et le grade sont obligatoires.");
        return;
    }

    // Validation format email
    static const QRegularExpression emailRegex(
        QStringLiteral(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"));
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Erreur",
                             "L'adresse email n'est pas valide.\n"
                             "Format attendu : exemple@domaine.com");
        ui->cherchLineEditEmail->setFocus();
        return;
    }

    // ── Vérification délivrabilité (AbstractAPI) ──────────────────────────────
    // Si le timer est encore actif (l'utilisateur vient de finir de taper)
    if (cherchEmailDebounceTimer && cherchEmailDebounceTimer->isActive()) {
        cherchEmailDebounceTimer->stop();
        cherchVerifyEmailDeliverability(email); // On force le lancement immédiat
    }

    // Si la réponse de l'API n'est pas encore arrivée
    if (cherchEmailCheckPending) {
        QMessageBox::warning(this, "Patience",
                             "La vérification de l'adresse email est en cours auprès du serveur.\n"
                             "Veuillez attendre le message de confirmation (✅) sous le champ email.");
        return;
    }

    // Si l'API a répondu mais que l'email est mauvais
    if (!cherchEmailDeliverabilityOk) {
        QMessageBox::critical(this, "Email Invalide",
                              "L'adresse email fournie n'est pas délivrable. Veuillez la corriger.");
        return;
    }

    // Validation format CIN : exactement 8 chiffres
    static const QRegularExpression cinRegex(QStringLiteral(R"(^\d{8}$)"));
    if (!cinRegex.match(cin).hasMatch()) {
        QMessageBox::warning(this, "Erreur",
                             "Le CIN doit être composé exactement de 8 chiffres.");
        ui->cherchLineEditCIN->setFocus();
        return;
    }

    QString photoPath = cherchCurrentPhotoPath.isEmpty()
                            ? QString(":/avatar.png")
                            : cherchCurrentPhotoPath;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    // Vérification unicité email avant INSERT
    {
        QSqlQuery chkQuery(db);
        chkQuery.prepare("SELECT COUNT(*) FROM CHERCHEUR WHERE LOWER(EMAIL) = LOWER(:email)");
        chkQuery.bindValue(":email", email);
        if (chkQuery.exec() && chkQuery.next() && chkQuery.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Erreur",
                                 "Un chercheur avec cet email existe déjà.");
            ui->cherchLineEditEmail->setFocus();
            return;
        }
    }

    // Vérification unicité CIN avant INSERT
    {
        QSqlQuery chkQuery(db);
        chkQuery.prepare("SELECT COUNT(*) FROM CHERCHEUR WHERE CIN = :cin");
        chkQuery.bindValue(":cin", cin);
        if (chkQuery.exec() && chkQuery.next() && chkQuery.value(0).toInt() > 0) {
            QMessageBox::warning(this, "Erreur",
                                 "Un chercheur avec ce CIN existe déjà.");
            ui->cherchLineEditCIN->setFocus();
            return;
        }
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO CHERCHEUR (NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL) "
                  "VALUES (:nom, :prenom, :email, :grade, :cin, :photo_profil)");
    query.bindValue(":nom", nom);
    query.bindValue(":prenom", prenom);
    query.bindValue(":email", email);
    query.bindValue(":grade", grade);
    query.bindValue(":cin", cin);
    query.bindValue(":photo_profil", photoPath);

    if (!query.exec()) {
        QString err = query.lastError().text();
        if (err.contains("unique") || err.contains("UK_CHERCHEUR"))
            QMessageBox::warning(this, "Erreur", "Un chercheur avec cet email ou ce CIN existe déjà.");
        else
            QMessageBox::critical(this, "Erreur", "Échec de l'ajout : " + err);
        return;
    }

    QMessageBox::information(this, "Succès", "Chercheur ajouté !");

    // Récupérer l'ID du nouveau chercheur via la séquence Oracle
    int newId = -1;
    {
        QSqlQuery qId(db);
        if (qId.exec("SELECT SEQ_CHERCHEUR.CURRVAL FROM DUAL") && qId.next())
            newId = qId.value(0).toInt();
    }

    // Insérer les contributions dans CONTRIBUER (colonne ID_PROJET, clé numérique)
    if (newId > 0 && cherchProjetsListWidget) {
        QList<QListWidgetItem*> selItems = cherchProjetsListWidget->selectedItems();
        for (QListWidgetItem *item : selItems) {
            int idProjet = item->data(Qt::UserRole).toInt(); // ID_PROJET stocké dans UserRole
            if (idProjet <= 0) continue;
            QSqlQuery qContrib(db);
            qContrib.prepare(
                "INSERT INTO CONTRIBUER (ID_CHERCHEUR, ID_PROJET) "
                "VALUES (:idc, :idp)");
            qContrib.bindValue(":idc", newId);
            qContrib.bindValue(":idp", idProjet);
            if (!qContrib.exec())
                qWarning() << "[CONTRIBUER] INSERT échoué :" << qContrib.lastError().text();
        }
    }

    ui->cherchLineEditNom->clear();
    ui->cherchLineEditPrenom->clear();
    ui->cherchLineEditCIN->clear();
    ui->cherchLineEditEmail->clear();
    ui->cherchComboBoxGrade->setCurrentIndex(0);
    ui->cherchLabelPhotoHint->setText("Cliquez pour ajouter une photo");
    ui->cherchLabelPhotoHint->setStyleSheet(
        "color: #94a3b8; font-size: 12px; background: transparent; border: "
        "none;");
    // Réinitialiser l'aperçu photo du bouton upload
    if (ui->cherchBtnUploadPhoto) {
        ui->cherchBtnUploadPhoto->setIcon(QIcon());
        ui->cherchBtnUploadPhoto->setText("+");
    }
    cherchCurrentPhotoPath.clear();
    // Réinitialiser le widget projets
    if (cherchProjetsListWidget) cherchProjetsListWidget->clearSelection();
    if (cherchBtnSelectProjets)  cherchBtnSelectProjets->setText("Sélectionner des projets…");
    if (cherchLabelProjetsSelec) {
        cherchLabelProjetsSelec->setText("Aucun projet sélectionné");
        cherchLabelProjetsSelec->setStyleSheet(
            "color: #94a3b8; font-size: 12px; background: transparent; border: none;");
    }

    on_cherchBtnVueListe_clicked();
}

void SmartPub::on_cherchBtnAnnulerAjout_clicked() {
    if (!ui->cherchLineEditNom->text().isEmpty() ||
        !ui->cherchLineEditPrenom->text().isEmpty()) {

        auto reply = QMessageBox::question(this, "Confirmation", "Annuler ?");
        if (reply == QMessageBox::No)
            return;
    }

    ui->cherchLineEditNom->clear();
    ui->cherchLineEditPrenom->clear();
    ui->cherchLineEditCIN->clear();
    ui->cherchLineEditEmail->clear();

    // Réinitialiser l'aperçu photo
    if (ui->cherchBtnUploadPhoto) {
        ui->cherchBtnUploadPhoto->setIcon(QIcon());
        ui->cherchBtnUploadPhoto->setText("+");
    }
    cherchCurrentPhotoPath.clear();
    if (ui->cherchLabelPhotoHint) {
        ui->cherchLabelPhotoHint->setText("Cliquez pour ajouter une photo");
        ui->cherchLabelPhotoHint->setStyleSheet(
            "color: #94a3b8; font-size: 12px; background: transparent; border: none;");
    }

    on_cherchBtnVueListe_clicked();
}

void SmartPub::on_cherchModifierChercheur(int id) {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas modifier les données.");
        return;
    }

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT ID_CHERCHEUR, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Erreur", "Chercheur introuvable.");
        return;
    }

    ChercheurData data;
    data.nom = query.value("NOM").toString();
    data.prenom = query.value("PRENOM").toString();
    data.email = query.value("EMAIL").toString();
    data.grade = query.value("GRADE").toString();
    data.cin = query.value("CIN").toString();
    data.photoPath = query.value("PHOTO_PROFIL").toString();
    if (data.photoPath.isEmpty()) data.photoPath = ":/avatar.png";

    QDialog dialog(this);
    dialog.setWindowTitle(QString("Modifier - %1 %2").arg(data.prenom).arg(data.nom));
    dialog.setMinimumSize(520, 620);
    dialog.setMaximumSize(600, 780);
    dialog.resize(520, 700);
    dialog.setStyleSheet("background-color: #f8fafc;");

    const QString labelStyle = "color: #334155; font-size: 13px; font-weight: 600; background: transparent; border: none;";
    const QString inputStyle = "padding: 10px 12px; border-radius: 10px; border: 2px solid #e2e8f0; font-size: 13px; color: #1e293b; background-color: white;";

    QVBoxLayout *dialogMainLayout = new QVBoxLayout(&dialog);
    dialogMainLayout->setSpacing(0);
    dialogMainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *titleFrame = new QFrame();
    titleFrame->setStyleSheet("background-color: white; border-bottom: 1px solid #e2e8f0;");
    titleFrame->setFixedHeight(60);
    QHBoxLayout *titleHL = new QHBoxLayout(titleFrame);
    titleHL->setContentsMargins(24, 0, 24, 0);
    QLabel *titleLbl = new QLabel("Modifier le chercheur");
    titleLbl->setStyleSheet("font-size: 18px; font-weight: 700; color: #1e293b; background: transparent; border: none;");
    titleHL->addWidget(titleLbl);
    dialogMainLayout->addWidget(titleFrame);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: #f8fafc; border: none;");
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: #f8fafc;");
    QVBoxLayout *layout = new QVBoxLayout(scrollContent);
    layout->setSpacing(10);
    layout->setContentsMargins(24, 16, 24, 16);

    // Photo
    QString newPhotoPath = data.photoPath;
    QHBoxLayout *photoRow = new QHBoxLayout();
    photoRow->setSpacing(16);
    QLabel *photoLabel = new QLabel(scrollContent);
    photoLabel->setFixedSize(70, 70);
    photoLabel->setAlignment(Qt::AlignCenter);
    photoLabel->setStyleSheet("border: 2px solid #e2e8f0; background: transparent;");
    photoLabel->setScaledContents(false);
    photoLabel->setMask(QRegion(0, 0, 70, 70, QRegion::Ellipse));
    QPixmap photoPix;
    if (!newPhotoPath.isEmpty()) photoPix.load(newPhotoPath);
    if (photoPix.isNull()) photoPix.load(":/avatar.png");
    if (!photoPix.isNull()) photoLabel->setPixmap(makeCircularPixmap(photoPix, 70));
    photoRow->addWidget(photoLabel);
    QVBoxLayout *photoCol = new QVBoxLayout();
    QLabel *photoTitle = new QLabel("Photo de profil");
    photoTitle->setStyleSheet(labelStyle);
    photoCol->addWidget(photoTitle);
    QPushButton *btnChangerPhoto = new QPushButton("Changer la photo");
    btnChangerPhoto->setStyleSheet(R"(
        QPushButton { background-color: #e2e8f0; color: #334155; border: none; border-radius: 8px; padding: 7px 14px; font-size: 12px; font-weight: 500; }
        QPushButton:hover { background-color: #cbd5e1; }
    )");
    connect(btnChangerPhoto, &QPushButton::clicked, &dialog, [&dialog, photoLabel, &newPhotoPath]() {
        QString path = QFileDialog::getOpenFileName(&dialog, "Choisir une photo", QDir::homePath(), "Images (*.png *.jpg *.jpeg)");
        if (path.isEmpty()) return;
        newPhotoPath = path;
        QPixmap pm(path);
        if (!pm.isNull()) photoLabel->setPixmap(makeCircularPixmap(pm, 70));
    });
    photoCol->addWidget(btnChangerPhoto);
    photoRow->addLayout(photoCol);
    layout->addLayout(photoRow);

    // Nom + Prénom
    QHBoxLayout *nomPrenomRow = new QHBoxLayout();
    nomPrenomRow->setSpacing(12);
    QVBoxLayout *colNom = new QVBoxLayout();
    QLabel *lblNom = new QLabel("Nom :");
    lblNom->setStyleSheet(labelStyle);
    colNom->addWidget(lblNom);
    QLineEdit *editNom = new QLineEdit(data.nom);
    editNom->setStyleSheet(inputStyle);
    QRegularExpression dlgNameRegex(QStringLiteral("^[A-Za-zÀ-ÖØ-öø-ÿ\\s'-]*$"));
    auto *dlgNameValidator = new QRegularExpressionValidator(dlgNameRegex, &dialog);
    editNom->setValidator(dlgNameValidator);
    colNom->addWidget(editNom);
    nomPrenomRow->addLayout(colNom, 1);
    QVBoxLayout *colPrenom = new QVBoxLayout();
    QLabel *lblPrenom = new QLabel("Prénom :");
    lblPrenom->setStyleSheet(labelStyle);
    colPrenom->addWidget(lblPrenom);
    QLineEdit *editPrenom = new QLineEdit(data.prenom);
    editPrenom->setStyleSheet(inputStyle);
    editPrenom->setValidator(dlgNameValidator);
    colPrenom->addWidget(editPrenom);
    nomPrenomRow->addLayout(colPrenom, 1);
    layout->addLayout(nomPrenomRow);

    // Email + CIN
    QHBoxLayout *emailCinRow = new QHBoxLayout();
    emailCinRow->setSpacing(12);
    QVBoxLayout *colEmail = new QVBoxLayout();
    QLabel *lblEmail = new QLabel("Email :");
    lblEmail->setStyleSheet(labelStyle);
    colEmail->addWidget(lblEmail);
    QLineEdit *editEmail = new QLineEdit(data.email);
    editEmail->setStyleSheet(inputStyle);
    colEmail->addWidget(editEmail);
    // Label d'erreur email
    QLabel *errEmailLabelModif = new QLabel();
    errEmailLabelModif->setObjectName("cherchErrEmailLabelModif");
    errEmailLabelModif->setText(QString());
    errEmailLabelModif->setVisible(false);
    errEmailLabelModif->setWordWrap(true);
    errEmailLabelModif->setStyleSheet(
        "font-size: 12px; background: transparent; border: none; padding: 4px 0;");
    colEmail->addWidget(errEmailLabelModif);
    emailCinRow->addLayout(colEmail, 3);

    QVBoxLayout *colCIN = new QVBoxLayout();
    QLabel *lblCIN = new QLabel("CIN :");
    lblCIN->setStyleSheet(labelStyle);
    colCIN->addWidget(lblCIN);
    QLineEdit *editCIN = new QLineEdit(data.cin);
    editCIN->setStyleSheet(inputStyle);
    editCIN->setPlaceholderText("8 chiffres");
    editCIN->setMaxLength(8);
    QRegularExpression dlgCinRegex(QStringLiteral("^\\d{0,8}$"));
    auto *dlgCinValidator = new QRegularExpressionValidator(dlgCinRegex, &dialog);
    editCIN->setValidator(dlgCinValidator);
    colCIN->addWidget(editCIN);
    emailCinRow->addLayout(colCIN, 2);
    layout->addLayout(emailCinRow);

    // Grade
    QLabel *lblGrade = new QLabel("Grade :");
    lblGrade->setStyleSheet(labelStyle);
    layout->addWidget(lblGrade);
    QComboBox *comboGrade = new QComboBox();
    comboGrade->setStyleSheet(R"(
        QComboBox {
            padding: 10px 12px;
            border-radius: 10px;
            border: 2px solid #e2e8f0;
            font-size: 13px;
            min-height: 40px;
            color: #1e293b;
            background-color: white;
        }
        QComboBox QAbstractItemView { color: #1e293b; background-color: white; }
    )");
    comboGrade->addItems({"Professeur", "Maitre de Conferences", "Docteur",
                          "Ingenieur de Recherche", "Post-doctorant", "Doctorant"});
    comboGrade->setCurrentText(data.grade);
    layout->addWidget(comboGrade);

    // Projets déjà affectés au chercheur (via CONTRIBUER.ID_PROJET)
    QList<int> projetsAffectes;
    {
        QSqlQuery qProj(db);
        qProj.prepare(
            "SELECT ID_PROJET FROM CONTRIBUER WHERE ID_CHERCHEUR = :id");
        qProj.bindValue(":id", id);
        if (qProj.exec())
            while (qProj.next())
                projetsAffectes.append(qProj.value(0).toInt());
    }
    // Tous les projets disponibles (ID_PROJET = clé PK, CODE = code métier, TITRE)
    QList<QPair<int,QString>> tousLesProjets; // first = ID_PROJET, second = affichage
    {
        QSqlQuery qAll(db);
        if (qAll.exec(
                "SELECT ID_PROJET, CODE, TITRE FROM PROJET ORDER BY CODE"))
            while (qAll.next()) {
                int    idProjet = qAll.value(0).toInt();
                QString code    = qAll.value(1).toString();
                QString titre   = qAll.value(2).toString();
                tousLesProjets.append({idProjet,
                                       QString("[%1]  %2").arg(code, titre)});
            }
    }

    QLabel *lblProjets = new QLabel("Projets (max 5) :");
    lblProjets->setStyleSheet(labelStyle);
    layout->addWidget(lblProjets);

    QPushButton *btnProjetsModif = new QPushButton(scrollContent);
    btnProjetsModif->setCursor(Qt::PointingHandCursor);
    btnProjetsModif->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 10px 12px;
            font-size: 13px;
            color: #334155;
            min-height: 40px;
            text-align: left;
        }
        QPushButton:hover { border-color: #cbd5e1; }
        QPushButton:pressed { border-color: #3b82f6; background-color: #eff6ff; }
    )");
    btnProjetsModif->setText(projetsAffectes.isEmpty()
                                 ? "▼  Sélectionner des projets…"
                                 : QString("▼  %1 projet(s) affecté(s)").arg(projetsAffectes.size()));
    layout->addWidget(btnProjetsModif);

    QListWidget *listProjetsModif = new QListWidget(scrollContent);
    listProjetsModif->setSelectionMode(QAbstractItemView::MultiSelection);
    listProjetsModif->setFixedHeight(160);
    listProjetsModif->setVisible(false);
    listProjetsModif->setStyleSheet(R"(
        QListWidget {
            border: 2px solid #3b82f6;
            border-radius: 10px;
            background-color: white;
            font-size: 13px;
            color: #334155;
            outline: none;
        }
        QListWidget::item {
            padding: 8px 12px;
            border-bottom: 1px solid #f1f5f9;
        }
        QListWidget::item:last { border-bottom: none; }
        QListWidget::item:selected {
            background-color: #eff6ff;
            color: #1d4ed8;
            font-weight: 600;
        }
        QListWidget::item:hover { background-color: #f8fafc; }
    )");
    // Remplir la liste — UserRole contient ID_PROJET (clé numérique pour CONTRIBUER)
    for (auto &p : tousLesProjets) {
        QListWidgetItem *item = new QListWidgetItem(p.second); // texte = "[CODE]  TITRE"
        item->setData(Qt::UserRole, p.first);                  // UserRole = ID_PROJET
        listProjetsModif->addItem(item);
        if (projetsAffectes.contains(p.first))
            item->setSelected(true);
    }
    layout->addWidget(listProjetsModif);

    connect(listProjetsModif, &QListWidget::itemSelectionChanged,
            &dialog, [listProjetsModif]() {
                QList<QListWidgetItem*> sel = listProjetsModif->selectedItems();
                if (sel.size() > 5) {
                    bool b = listProjetsModif->blockSignals(true);
                    sel.last()->setSelected(false);
                    listProjetsModif->blockSignals(b);
                    QToolTip::showText(QCursor::pos(), "Maximum 5 projets autorisés", listProjetsModif);
                }
            });

    QPushButton *btnValiderModif = new QPushButton("✔  Valider la sélection");
    btnValiderModif->setCursor(Qt::PointingHandCursor);
    btnValiderModif->setVisible(false);
    btnValiderModif->setFixedHeight(38);
    btnValiderModif->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white; border: none; border-radius: 8px;
            font-size: 13px; font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #2563eb, stop:1 #059669);
        }
    )");
    layout->addWidget(btnValiderModif);

    QLabel *lblProjetsSelec = new QLabel(scrollContent);
    lblProjetsSelec->setWordWrap(true);
    if (projetsAffectes.isEmpty()) {
        lblProjetsSelec->setText("Aucun projet sélectionné");
        lblProjetsSelec->setStyleSheet(
            "color: #94a3b8; font-size: 12px; background: transparent; border: none;");
    } else {
        QStringList titresCourants;
        for (int pid : projetsAffectes)
            for (auto &p : tousLesProjets)
                if (p.first == pid) { titresCourants << QString("[%1] %2").arg(p.first).arg(p.second); break; }
        lblProjetsSelec->setText("✔  " + titresCourants.join("  |  "));
        lblProjetsSelec->setStyleSheet(
            "color: #10b981; font-size: 12px; font-weight: 600; background: transparent; border: none;");
    }
    layout->addWidget(lblProjetsSelec);

    connect(btnProjetsModif, &QPushButton::clicked, &dialog,
            [listProjetsModif, btnValiderModif, btnProjetsModif, scrollArea]() {
                bool visible = listProjetsModif->isVisible();
                listProjetsModif->setVisible(!visible);
                btnValiderModif->setVisible(!visible);
                btnProjetsModif->setText(visible ? "▼  Sélectionner des projets…" : "▲  Fermer la liste");
                if (!visible)
                    QTimer::singleShot(50, scrollArea, [scrollArea](){ scrollArea->verticalScrollBar()->setValue(
                                                                            scrollArea->verticalScrollBar()->maximum()); });
            });

    connect(btnValiderModif, &QPushButton::clicked, &dialog,
            [listProjetsModif, btnValiderModif, btnProjetsModif, lblProjetsSelec]() {
                listProjetsModif->setVisible(false);
                btnValiderModif->setVisible(false);
                QList<QListWidgetItem*> sel = listProjetsModif->selectedItems();
                if (sel.isEmpty()) {
                    btnProjetsModif->setText("▼  Sélectionner des projets…");
                    lblProjetsSelec->setText("Aucun projet sélectionné");
                    lblProjetsSelec->setStyleSheet(
                        "color: #94a3b8; font-size: 12px; background: transparent; border: none;");
                } else {
                    btnProjetsModif->setText(QString("▼  %1 projet(s) sélectionné(s)").arg(sel.size()));
                    QStringList t;
                    for (auto *it : sel) t << it->text();
                    lblProjetsSelec->setText("✔  " + t.join("  |  "));
                    lblProjetsSelec->setStyleSheet(
                        "color: #10b981; font-size: 12px; font-weight: 600; background: transparent; border: none;");
                }
            });

    scrollArea->setWidget(scrollContent);
    dialogMainLayout->addWidget(scrollArea, 1);

    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet("background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(62);
    QHBoxLayout *footerHL = new QHBoxLayout(footerFrame);
    footerHL->setContentsMargins(24, 0, 24, 0);
    footerHL->addStretch();
    QPushButton *btnSave = new QPushButton("💾  Sauvegarder");
    btnSave->setFixedHeight(42);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white; border: none; border-radius: 10px;
            padding: 0 32px; font-size: 14px; font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #2563eb, stop:1 #059669);
        }
    )");
    footerHL->addWidget(btnSave);
    dialogMainLayout->addWidget(footerFrame);

    // ========== VÉRIFICATION EMAIL API (ADAPTÉE REPUTATION) ==========
    bool emailDeliverableModif = true; // Par défaut true si l'email n'est pas changé
    bool emailCheckPendingModif = false;
    QString lastVerifiedEmailModif = data.email; // L'email actuel est considéré comme vérifié

    QTimer *emailDebounceTimerModif = new QTimer(&dialog);
    emailDebounceTimerModif->setSingleShot(true);
    emailDebounceTimerModif->setInterval(1000);



    auto verifyEmailModif = [&](const QString &email) {
        if (!cherchNetworkManager) return;

        // Si l'email est le même que celui d'origine, on ne re-vérifie pas
        if (email == data.email) {
            emailDeliverableModif = true;
            emailCheckPendingModif = false;
            errEmailLabelModif->setVisible(false);
            return;
        }

        emailCheckPendingModif = true;
        emailDeliverableModif = false;
        lastVerifiedEmailModif = email;

        errEmailLabelModif->setText("  🔍 Analyse de réputation en cours…");
        errEmailLabelModif->setStyleSheet(
            "color: #3b82f6; font-size: 12px; background: #eff6ff; border: 1px solid #bfdbfe; "
            "border-radius: 8px; padding: 4px 10px;");
        errEmailLabelModif->setVisible(true);

        // NOUVELLE URL (Reputation API)
        QString apiKey = "2b9bff7449d243b28938b55d44c905ed";
        QUrl url("https://emailreputation.abstractapi.com/v1/");
        QUrlQuery query;
        query.addQueryItem("api_key", apiKey);
        query.addQueryItem("email", email);
        url.setQuery(query);

        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, email);
        QNetworkReply *reply = cherchNetworkManager->get(request);

        connect(reply, &QNetworkReply::finished, &dialog, [&, reply]() {
            reply->deleteLater();
            const QString reqEmail = reply->request().attribute(QNetworkRequest::User).toString();
            if (reqEmail != editEmail->text().trimmed()) return;

            emailCheckPendingModif = false;

            if (reply->error() != QNetworkReply::NoError) {
                emailDeliverableModif = true; // On laisse passer en cas d'erreur serveur
                errEmailLabelModif->setText("  ⚠️ Service indisponible (vérification ignorée)");
                return;
            }

            QByteArray raw = reply->readAll();
            QJsonObject obj = QJsonDocument::fromJson(raw).object();

            // Extraction du score et du statut (Format Reputation API)
            QJsonObject deliverabilityObj = obj["email_deliverability"].toObject();
            QString status = deliverabilityObj["status"].toString();

            QJsonObject qualityObj = obj["email_quality"].toObject();
            int score = qualityObj["score"].toDouble() * 100;

            if (status == "deliverable" || score > 60) {
                emailDeliverableModif = true;
                errEmailLabelModif->setText(QString("  ✅ Email valide (Confiance: %1%)").arg(score));
                errEmailLabelModif->setStyleSheet(
                    "color: #059669; font-size: 12px; background: #d1fae5; border: 1px solid #6ee7b7; "
                    "border-radius: 8px; padding: 4px 10px;");
            } else {
                emailDeliverableModif = false;
                errEmailLabelModif->setText(QString("  ❌ Email suspect ou inexistant (Score: %1%)").arg(score));
                errEmailLabelModif->setStyleSheet(
                    "color: #dc2626; font-size: 12px; background: #fee2e2; border: 1px solid #fca5a5; "
                    "border-radius: 8px; padding: 4px 10px;");
            }
        });
    };


    // 1. Connexion du changement de texte au timer
    connect(editEmail, &QLineEdit::textChanged, &dialog, [&](const QString &text) { // Le & ici capture tout par référence
        emailDeliverableModif = false;
        emailCheckPendingModif = true;

        if (text.trimmed().isEmpty()) {
            errEmailLabelModif->setVisible(false);
            emailDebounceTimerModif->stop();
            return;
        }
        emailDebounceTimerModif->start();
    });

    // 2. Connexion du Timer à la fonction de vérification
    // C'EST ICI QUE L'ERREUR SE PRODUISAIT : Ajoutez "verifyEmailModif" ou "&" dans les crochets
    connect(emailDebounceTimerModif, &QTimer::timeout, &dialog, [&]() {
        verifyEmailModif(editEmail->text().trimmed());
    });


    // Gestion du bouton Sauvegarder avec forçage de la vérification
    btnSave->disconnect();
    connect(btnSave, &QPushButton::clicked, &dialog, [&]() {
        QString currentEmail = editEmail->text().trimmed();

        // 1. Si l'utilisateur clique pendant que le timer attend
        if (emailDebounceTimerModif->isActive()) {
            emailDebounceTimerModif->stop();
            verifyEmailModif(currentEmail);
        }

        // 2. Si on attend encore la réponse de l'API
        if (emailCheckPendingModif) {
            QMessageBox::information(this, "Vérification en cours",
                                     "Veuillez patienter pendant la validation de l'adresse email...");
            return;
        }

        // 3. Si l'email a été refusé par l'API
        if (!emailDeliverableModif && currentEmail != data.email) {
            QMessageBox::warning(this, "Email Invalide",
                                 "L'adresse email saisie n'est pas valide. Veuillez la corriger.");
            return;
        }

        dialog.accept();
    });

    if (dialog.exec() == QDialog::Accepted) {
        QString newNom    = editNom->text().trimmed();
        QString newPrenom = editPrenom->text().trimmed();
        QString newEmail  = editEmail->text().trimmed();
        QString newCIN    = editCIN->text().trimmed();
        QString newGrade  = comboGrade->currentText().trimmed();

        if (newNom.isEmpty() || newPrenom.isEmpty() || newEmail.isEmpty()
            || newCIN.isEmpty() || newGrade.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Tous les champs sont obligatoires.");
            return;
        }

        static const QRegularExpression nameRegex(
            QStringLiteral(R"(^[A-Za-zÀ-ÖØ-öø-ÿ\s'-]+$)"));
        if (!nameRegex.match(newNom).hasMatch() ||
            !nameRegex.match(newPrenom).hasMatch()) {
            QMessageBox::warning(this, "Erreur",
                                 "Le nom et le prénom ne doivent contenir que des lettres.");
            return;
        }

        static const QRegularExpression emailRegex(
            QStringLiteral(R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"));
        if (!emailRegex.match(newEmail).hasMatch()) {
            QMessageBox::warning(this, "Erreur",
                                 "L'adresse email n'est pas valide.\n"
                                 "Format attendu : exemple@domaine.com");
            return;
        }

        static const QRegularExpression cinRegex(QStringLiteral(R"(^\d{8}$)"));
        if (!cinRegex.match(newCIN).hasMatch()) {
            QMessageBox::warning(this, "Erreur",
                                 "Le CIN doit être composé exactement de 8 chiffres.");
            return;
        }

        {
            QSqlQuery chkQuery(db);
            chkQuery.prepare("SELECT COUNT(*) FROM CHERCHEUR "
                             "WHERE LOWER(EMAIL) = LOWER(:email) AND ID_CHERCHEUR <> :id");
            chkQuery.bindValue(":email", newEmail);
            chkQuery.bindValue(":id", id);
            if (chkQuery.exec() && chkQuery.next() && chkQuery.value(0).toInt() > 0) {
                QMessageBox::warning(this, "Erreur",
                                     "Un autre chercheur possède déjà cet email.");
                return;
            }
        }

        {
            QSqlQuery chkQuery(db);
            chkQuery.prepare("SELECT COUNT(*) FROM CHERCHEUR "
                             "WHERE CIN = :cin AND ID_CHERCHEUR <> :id");
            chkQuery.bindValue(":cin", newCIN);
            chkQuery.bindValue(":id", id);
            if (chkQuery.exec() && chkQuery.next() && chkQuery.value(0).toInt() > 0) {
                QMessageBox::warning(this, "Erreur",
                                     "Un autre chercheur possède déjà ce CIN.");
                return;
            }
        }

        QSqlQuery updateQuery(db);
        updateQuery.prepare(
            "UPDATE CHERCHEUR SET NOM = :nom, PRENOM = :prenom, EMAIL = :email, "
            "CIN = :cin, GRADE = :grade, PHOTO_PROFIL = :photo_profil "
            "WHERE ID_CHERCHEUR = :id");
        updateQuery.bindValue(":nom",         newNom);
        updateQuery.bindValue(":prenom",      newPrenom);
        updateQuery.bindValue(":email",       newEmail);
        updateQuery.bindValue(":cin",         newCIN);
        updateQuery.bindValue(":grade",       newGrade);
        updateQuery.bindValue(":photo_profil",
                              newPhotoPath.isEmpty() ? QString(":/avatar.png") : newPhotoPath);
        updateQuery.bindValue(":id", id);

        if (!updateQuery.exec()) {
            QString err = updateQuery.lastError().text();
            if (err.contains("unique") || err.contains("UK_CHERCHEUR"))
                QMessageBox::warning(this, "Erreur",
                                     "Un chercheur avec cet email ou ce CIN existe déjà.");
            else
                QMessageBox::critical(this, "Erreur",
                                      "Échec de la modification : " + err);
            return;
        }

        // Supprimer toutes les contributions existantes puis réinsérer
        QSqlQuery delContrib(db);
        delContrib.prepare(
            "DELETE FROM CONTRIBUER WHERE ID_CHERCHEUR = :id");
        delContrib.bindValue(":id", id);
        if (!delContrib.exec())
            qWarning() << "[CONTRIBUER] DELETE échoué :"
                       << delContrib.lastError().text();

        QList<QListWidgetItem*> selItems = listProjetsModif->selectedItems();
        for (QListWidgetItem *item : selItems) {
            int idProjet = item->data(Qt::UserRole).toInt(); // ID_PROJET
            if (idProjet <= 0) continue;
            QSqlQuery insContrib(db);
            insContrib.prepare(
                "INSERT INTO CONTRIBUER (ID_CHERCHEUR, ID_PROJET) "
                "VALUES (:idc, :idp)");
            insContrib.bindValue(":idc", id);
            insContrib.bindValue(":idp", idProjet);
            if (!insContrib.exec())
                qWarning() << "[CONTRIBUER] INSERT échoué :"
                           << insContrib.lastError().text();
        }

        cherchAfficherListeChercheurs();
        QMessageBox::information(this, "Succès", "Chercheur modifié avec succès !");
    }
}

void SmartPub::on_cherchSupprimerChercheur(int id) {
    if (currentUser.role == UserRole::Guest) {
        QMessageBox::warning(this, "Accès refusé",
                             "Les invités ne peuvent pas supprimer les données.");
        return;
    }

    auto reply =
        QMessageBox::question(this, "Supprimer", "Confirmer la suppression ?");
    if (reply != QMessageBox::Yes)
        return;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur", "Échec de la suppression : " + query.lastError().text());
        return;
    }
    cherchAfficherListeChercheurs();
}

void SmartPub::on_cherchVoirDetailsChercheur(int id) {
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Erreur", "Connexion à la base de données impossible.");
        return;
    }

    // ── Charger les données du chercheur ──────────────────────────────────────
    QSqlQuery query(db);
    query.prepare("SELECT ID_CHERCHEUR, NOM, PRENOM, EMAIL, GRADE, CIN, PHOTO_PROFIL "
                  "FROM CHERCHEUR WHERE ID_CHERCHEUR = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Erreur", "Chercheur introuvable.");
        return;
    }

    ChercheurData data;
    data.nom       = query.value("NOM").toString();
    data.prenom    = query.value("PRENOM").toString();
    data.email     = query.value("EMAIL").toString();
    data.grade     = query.value("GRADE").toString();
    data.cin       = query.value("CIN").toString();
    data.photoPath = query.value("PHOTO_PROFIL").toString();
    if (data.photoPath.isEmpty()) data.photoPath = ":/avatar.png";
    data.age = 0;

    // ── Charger les projets via CONTRIBUER → PROJET (jointure sur ID_PROJET) ──
    // projetsTitres : liste d'affichage  "[CODE]  TITRE"
    // data.projetsIds : liste des ID_PROJET pour le calcul de carrière
    QList<QString> projetsTitres;   // pour l'affichage dans le dialog et le PDF
    QList<QString> projetsCodes;    // codes métier (CODE) pour référence
    {
        QSqlQuery qProj(db);
        qProj.prepare(
            "SELECT p.ID_PROJET, p.CODE, p.TITRE "
            "FROM CONTRIBUER c "
            "INNER JOIN PROJET p ON p.ID_PROJET = c.ID_PROJET "
            "WHERE c.ID_CHERCHEUR = :id "
            "ORDER BY p.CODE");
        qProj.bindValue(":id", id);
        if (qProj.exec()) {
            while (qProj.next()) {
                data.projetsIds.append(qProj.value(0).toInt());  // ID_PROJET
                projetsCodes.append(qProj.value(1).toString());  // CODE métier
                projetsTitres.append(
                    QString("[%1]  %2")
                        .arg(qProj.value(1).toString(),           // CODE
                             qProj.value(2).toString()));         // TITRE
            }
        }
    }

    // ── Calculer la carrière depuis le nb de projets et le grade ─────────────
    data.carriere = cherchDeterminerCarriere(data.projetsIds.size(), data.grade);

    // ═══════════════════════════════════════════════════════════════════════════
    // DIALOGUE DE DÉTAILS
    // ═══════════════════════════════════════════════════════════════════════════
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(QString("Profil — %1 %2").arg(data.prenom, data.nom));
    dialog->setMinimumSize(720, 640);
    dialog->setMaximumSize(920, 840);
    dialog->setStyleSheet("background-color: #f8fafc;");
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Header gradient ───────────────────────────────────────────────────────
    QFrame *headerFrame = new QFrame();
    headerFrame->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
        }
    )");
    headerFrame->setFixedHeight(210);

    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setAlignment(Qt::AlignCenter);
    headerLayout->setSpacing(10);
    headerLayout->setContentsMargins(20, 20, 20, 16);

    // Avatar circulaire
    QLabel *avatarLbl = new QLabel();
    avatarLbl->setFixedSize(110, 110);
    avatarLbl->setAlignment(Qt::AlignCenter);
    avatarLbl->setStyleSheet("border: 4px solid white; background: transparent;");
    avatarLbl->setScaledContents(false);
    avatarLbl->setMask(QRegion(0, 0, 110, 110, QRegion::Ellipse));
    QPixmap profilePix;
    if (!profilePix.load(data.photoPath)) profilePix.load(":/avatar.png");
    if (!profilePix.isNull())
        avatarLbl->setPixmap(makeCircularPixmap(profilePix, 110));
    headerLayout->addWidget(avatarLbl, 0, Qt::AlignCenter);

    QLabel *nameHeaderLbl = new QLabel(QString("%1 %2").arg(data.prenom, data.nom));
    nameHeaderLbl->setStyleSheet(
        "color: white; font-size: 22px; font-weight: 700; "
        "background: transparent; border: none;");
    nameHeaderLbl->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(nameHeaderLbl, 0, Qt::AlignCenter);

    // Badge carrière dans le header
    QLabel *careerBadge = new QLabel(data.carriere.isEmpty() ? "—" : data.carriere);
    careerBadge->setStyleSheet(
        "background: rgba(255,255,255,0.22); color: white; "
        "border-radius: 10px; padding: 5px 16px; font-size: 12px; "
        "font-weight: 600; border: none;");
    careerBadge->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(careerBadge, 0, Qt::AlignCenter);

    mainLayout->addWidget(headerFrame);

    // ── Scroll area avec les infos ────────────────────────────────────────────
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background-color: white; border: none;");

    QWidget *contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(28, 24, 28, 24);

    // Fabrique de ligne info
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

    contentLayout->addWidget(createInfoRow("Grade",    data.grade,                    "🎓"));
    contentLayout->addWidget(createInfoRow("Email",    data.email,                    "✉️"));
    contentLayout->addWidget(createInfoRow("CIN",      data.cin,                      "🆔"));
    contentLayout->addWidget(createInfoRow("Carrière", data.carriere,                 "⭐"));
    contentLayout->addWidget(createInfoRow(
        "Projets",
        data.projetsIds.isEmpty()
            ? "Aucune contribution"
            : QString("%1 contribution(s)").arg(data.projetsIds.size()),
        "📁"));

    // ── Liste des projets associés ────────────────────────────────────────────
    if (!projetsTitres.isEmpty()) {
        QLabel *projTitle = new QLabel("Projets associés");
        projTitle->setStyleSheet(
            "color: #1e293b; font-size: 15px; font-weight: 700; "
            "margin-top: 6px; background: transparent; border: none;");
        contentLayout->addWidget(projTitle);

        for (const QString &titre : projetsTitres) {
            QFrame *projFrame = new QFrame();
            projFrame->setStyleSheet(
                "QFrame { background-color: #eff6ff; border-left: 4px solid #3b82f6; "
                "border-radius: 8px; border-top: none; border-right: none; border-bottom: none; }");
            QHBoxLayout *pLayout = new QHBoxLayout(projFrame);
            pLayout->setContentsMargins(14, 10, 14, 10);
            QLabel *pLbl = new QLabel(titre);
            pLbl->setStyleSheet(
                "color: #1e40af; font-weight: 600; background: transparent; border: none;");
            pLayout->addWidget(pLbl, 1);
            contentLayout->addWidget(projFrame);
        }
    }

    contentLayout->addStretch();
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1);

    // ── Footer : Export PDF + Fermer ──────────────────────────────────────────
    QFrame *footerFrame = new QFrame();
    footerFrame->setStyleSheet(
        "background-color: white; border-top: 1px solid #e2e8f0;");
    footerFrame->setFixedHeight(70);
    QHBoxLayout *footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(24, 0, 24, 0);
    footerLayout->setSpacing(12);

    QPushButton *btnExport = new QPushButton("📄  Exporter en PDF");
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

    // ── Lambda export PDF capturant id, data et projetsTitres ─────────────────
    connect(btnExport, &QPushButton::clicked, dialog,
            [this, data, projetsTitres]() {

                QString safeNom = data.nom;
                safeNom.replace(" ", "_");
                QString safePrenom = data.prenom;
                safePrenom.replace(" ", "_");

                QString fileName = QFileDialog::getSaveFileName(
                    this,
                    "Exporter Fiche Chercheur — PDF",
                    QDir::homePath() + "/Fiche_" + safeNom + "_" + safePrenom + ".pdf",
                    "Fichiers PDF (*.pdf)");
                if (fileName.isEmpty()) return;

                QPrinter printer(QPrinter::HighResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName(fileName);
                printer.setPageSize(QPageSize(QPageSize::A4));
                printer.setPageOrientation(QPageLayout::Portrait);

                QPainter painter;
                if (!painter.begin(&printer)) {
                    QMessageBox::critical(this, "Erreur PDF",
                                          "Impossible d'initialiser le fichier PDF :\n" + fileName);
                    return;
                }

                const double res = printer.resolution();  // points / inch (ex: 1200)
                const double cm  = res / 2.54;            // 1 cm en points
                int x = (int)(1.8 * cm);
                int y = (int)(1.5 * cm);

                // ── EN-TÊTE : rectangle gradient simulé ────────────────────────────
                QLinearGradient headerGrad(x, y, x + (int)(17.4 * cm), y);
                headerGrad.setColorAt(0.0, QColor("#3b82f6"));
                headerGrad.setColorAt(1.0, QColor("#10b981"));
                painter.setPen(Qt::NoPen);
                painter.setBrush(headerGrad);
                painter.drawRoundedRect(x, y, (int)(17.4 * cm), (int)(4.0 * cm), 14, 14);

                // Photo de profil (rendue circulaire)
                QString photoPath = data.photoPath.isEmpty() ? ":/avatar.png" : data.photoPath;
                QPixmap pix;
                if (!pix.load(photoPath)) pix.load(":/avatar.png");
                if (!pix.isNull()) {
                    int sz = (int)(3.2 * cm);
                    QPixmap circ = makeCircularPixmap(pix, sz);
                    // Dessin du cercle blanc derrière la photo
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(Qt::white);
                    painter.drawEllipse(x + (int)(0.25 * cm), y + (int)(0.35 * cm), sz + 8, sz + 8);
                    painter.drawPixmap(x + (int)(0.29 * cm) + 4,
                                       y + (int)(0.39 * cm) + 4, sz, sz, circ);
                }

                // Nom + Grade
                int textX = x + (int)(4.2 * cm);
                painter.setPen(Qt::white);
                painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
                painter.drawText(textX, y + (int)(1.3 * cm),
                                 QString("%1 %2").arg(data.prenom, data.nom));

                painter.setFont(QFont("Segoe UI", 13));
                painter.drawText(textX, y + (int)(2.1 * cm), data.grade);

                // Badge carrière
                if (!data.carriere.isEmpty()) {
                    painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
                    int bx = textX, by = y + (int)(2.55 * cm);
                    int bw = (int)(5.5 * cm), bh = (int)(0.6 * cm);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor(255, 255, 255, 55));
                    painter.drawRoundedRect(bx, by, bw, bh, 10, 10);
                    painter.setPen(Qt::white);
                    painter.drawText(QRect(bx, by, bw, bh), Qt::AlignCenter, data.carriere);
                }

                y += (int)(4.8 * cm);

                // ── Helpers locaux ──────────────────────────────────────────────────
                auto drawSectionHeader = [&](const QString &titre) {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor("#f1f5f9"));
                    painter.drawRoundedRect(x, y - (int)(0.05 * cm),
                                            (int)(17.4 * cm), (int)(0.75 * cm), 6, 6);
                    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
                    painter.setPen(QColor("#1e293b"));
                    painter.drawText(x + (int)(0.5 * cm), y + (int)(0.52 * cm), titre);
                    y += (int)(1.1 * cm);
                };

                auto drawField = [&](const QString &label, const QString &value) {
                    painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
                    painter.setPen(QColor("#64748b"));
                    painter.drawText(x + (int)(0.4 * cm), y, label);
                    painter.setFont(QFont("Segoe UI", 11));
                    painter.setPen(QColor("#1e293b"));
                    painter.drawText(x + (int)(5.2 * cm), y, value.isEmpty() ? "—" : value);
                    y += (int)(0.75 * cm);
                };

                // ── Informations personnelles ───────────────────────────────────────
                drawSectionHeader("Informations personnelles");
                drawField("Email :",     data.email);
                drawField("CIN :",       data.cin);
                drawField("Grade :",     data.grade);
                drawField("Carrière :",  data.carriere);

                y += (int)(0.5 * cm);

                // ── Projets de recherche ────────────────────────────────────────────
                drawSectionHeader(
                    QString("Projets de recherche  (%1 contribution(s))")
                        .arg(projetsTitres.size()));

                if (projetsTitres.isEmpty()) {
                    painter.setFont(QFont("Segoe UI", 11));
                    painter.setPen(QColor("#94a3b8"));
                    painter.drawText(x + (int)(0.5 * cm), y,
                                     "Aucun projet associé.");
                    y += (int)(0.8 * cm);
                } else {
                    for (int pi = 0; pi < projetsTitres.size(); ++pi) {
                        const QString &ligne = projetsTitres.at(pi);

                        // Fond alterné léger
                        painter.setPen(Qt::NoPen);
                        painter.setBrush(pi % 2 == 0
                                             ? QColor("#f8fafc")
                                             : QColor("#eff6ff"));
                        painter.drawRoundedRect(
                            x, y - (int)(0.32 * cm),
                            (int)(17.4 * cm), (int)(0.72 * cm),
                            6, 6);

                        // Puce bleue
                        painter.setBrush(QColor("#3b82f6"));
                        painter.drawEllipse(
                            x + (int)(0.4 * cm),
                            y - (int)(0.17 * cm),
                            (int)(0.22 * cm), (int)(0.22 * cm));

                        // Texte — on tronque si trop long pour tenir sur la ligne
                        painter.setFont(QFont("Segoe UI", 11));
                        painter.setPen(QColor("#1e293b"));
                        QRect textRect(
                            x + (int)(0.85 * cm),
                            y - (int)(0.32 * cm),
                            (int)(16.0 * cm),
                            (int)(0.72 * cm));
                        painter.drawText(textRect,
                                         Qt::AlignVCenter | Qt::AlignLeft,
                                         ligne);

                        y += (int)(0.75 * cm);

                        // Saut de page si on approche du bas
                        if (y > (int)(26.0 * cm)) {
                            printer.newPage();
                            y = (int)(1.5 * cm);
                        }
                    }
                }

                // ── Pied de page ────────────────────────────────────────────────────
                int footerY = (int)(27.8 * cm);
                painter.setPen(QColor("#e2e8f0"));
                painter.drawLine(x, footerY, x + (int)(17.4 * cm), footerY);
                painter.setFont(QFont("Segoe UI", 9));
                painter.setPen(QColor("#94a3b8"));
                painter.drawText(x, footerY + (int)(0.45 * cm),
                                 QString("SmartPub — Fiche générée le %1")
                                     .arg(QDate::currentDate().toString("dd/MM/yyyy")));

                painter.end();
                QMessageBox::information(this, "Export PDF",
                                         "✅  Fiche exportée avec succès !\n" + fileName);
            });

    QPushButton *btnClose = new QPushButton("Fermer");
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

    footerLayout->addWidget(btnExport);
    footerLayout->addStretch();
    footerLayout->addWidget(btnClose);
    mainLayout->addWidget(footerFrame);

    dialog->exec();
}

// on_cherchBtnExportDetails_clicked() — supprimée, export PDF géré
// directement par une lambda dans on_cherchVoirDetailsChercheur()

void SmartPub::on_cherchLineEditRecherche_textChanged(const QString &text) {
    if (text.length() >= 2 || text.isEmpty()) {
        QTimer::singleShot(300, this, [this, text]() {
            if (ui->cherchLineEditRecherche->text() == text) {
                on_cherchBtnRecherche_clicked();
            }
        });
    }
}

void SmartPub::handleChercheursNavigation() {
    ui->stackedWidgetModules->setCurrentIndex(0);
    setActiveNavigationButton(0);
    updateProfileName(0);

    ui->cherchStackedWidget->setCurrentIndex(0);
    cherchVueListeActive = true;

    ui->cherchLineEditRecherche->setVisible(true);
    ui->cherchBtnRecherche->setVisible(true);
    ui->cherchBtnTri->setVisible(true);
    ui->cherchBtnExport->setVisible(true);
    ui->cherchBtnStatistiques->setVisible(true);
    if (cherchBtnToggleVue)
        cherchBtnToggleVue->setVisible(true);

    const QString tabActive = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #10b981);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
    )";
    const QString tabInactive = R"(
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
    ui->cherchBtnVueListe->setStyleSheet(tabActive);
    ui->cherchBtnAjouter->setStyleSheet(tabInactive);

    cherchAfficherListeChercheurs();
}

// ============================================================================
// VÉRIFICATION DÉLIVRABILITÉ EMAIL — AbstractAPI
// Clé : 5c897de7e4ae42bda2566ecfdc30f0f7
// ============================================================================
void SmartPub::cherchVerifyEmailDeliverability(const QString &email) {
    if (email.isEmpty()) {
        cherchEmailCheckPending = false;
        return;
    }

    cherchEmailCheckPending = true; // On indique que c'est en cours
    cherchLastVerifiedEmail = email;

    if (cherchErrEmailLabel) {
        cherchErrEmailLabel->setText("⏳ Vérification en cours...");
        cherchErrEmailLabel->setVisible(true);
    }

    QString apiKey = "a7d04d7f998b45e9b0b892b8aa524077";
    QString urlString = QString("https://emailreputation.abstractapi.com/v1/?api_key=%1&email=%2")
                            .arg(apiKey).arg(email);

    QNetworkRequest request((QUrl(urlString)));
    QNetworkReply *reply = cherchNetworkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, email]() {
        // IMPORTANT : La vérification n'est plus en attente
        cherchEmailCheckPending = false;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject jsonObj = QJsonDocument::fromJson(reply->readAll()).object();
            QJsonObject deliverabilityObj = jsonObj["email_deliverability"].toObject();
            QString status = deliverabilityObj["status"].toString();

            if (status == "deliverable") {
                cherchErrEmailLabel->setText("✅ Email valide");
                cherchEmailDeliverabilityOk = true;
            } else {
                cherchErrEmailLabel->setText("❌ Email invalide");
                cherchEmailDeliverabilityOk = false;
            }
        } else {
            cherchErrEmailLabel->setText("⚠️ Erreur réseau");
            // En cas d'erreur réseau, on peut décider de laisser passer (true)
            // ou de bloquer (false). À vous de choisir :
            cherchEmailDeliverabilityOk = false;
        }
        reply->deleteLater();
    });
}

void SmartPub::on_cherchEmailVerificationReply(QNetworkReply *reply)
{
    reply->deleteLater();

    const QString requestedEmail = reply->request().attribute(QNetworkRequest::User).toString();
    const QString currentEmail = ui->cherchLineEditEmail->text().trimmed();
    if (requestedEmail != currentEmail) return;

    cherchEmailCheckPending = false;

    const QString inputBase =
        "QLineEdit { background-color: #f8fafc; border: 2px solid %1; border-radius: 10px; "
        "padding: 12px 16px; font-size: 14px; color: #334155; }"
        "QLineEdit:focus { background-color: %2; border: 2px solid %1; }";

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[EmailVerif] Erreur réseau :" << reply->errorString();
        cherchEmailDeliverabilityOk = true; // fallback
        if (cherchErrEmailLabel) {
            cherchErrEmailLabel->setText("  ⚠️ Service indisponible — format OK");
            cherchErrEmailLabel->setStyleSheet(
                "color: #f59e0b; font-size: 12px; background: #fffbeb; border: 1px solid #fde68a; "
                "border-radius: 8px; padding: 4px 10px;");
            cherchErrEmailLabel->setVisible(true);
        }
        ui->cherchLineEditEmail->setStyleSheet(inputBase.arg("#f59e0b", "#fffbeb"));
        return;
    }

    int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpCode != 200) {
        qWarning() << "[EmailVerif] HTTP error:" << httpCode;
        cherchEmailDeliverabilityOk = true;
        if (cherchErrEmailLabel) {
            cherchErrEmailLabel->setText(QString("  ⚠️ API code %1 — format OK").arg(httpCode));
            cherchErrEmailLabel->setStyleSheet(
                "color: #f59e0b; font-size: 12px; background: #fffbeb; border: 1px solid #fde68a; "
                "border-radius: 8px; padding: 4px 10px;");
            cherchErrEmailLabel->setVisible(true);
        }
        ui->cherchLineEditEmail->setStyleSheet(inputBase.arg("#f59e0b", "#fffbeb"));
        return;
    }

    QByteArray rawData = reply->readAll();
    qDebug() << "[EmailVerif] Réponse brute :" << rawData;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[EmailVerif] JSON invalide :" << parseError.errorString();
        cherchEmailDeliverabilityOk = true;
        if (cherchErrEmailLabel) {
            cherchErrEmailLabel->setText("  ⚠️ Réponse inattendue — format OK");
            cherchErrEmailLabel->setStyleSheet(
                "color: #f59e0b; font-size: 12px; background: #fffbeb; border: 1px solid #fde68a; "
                "border-radius: 8px; padding: 4px 10px;");
            cherchErrEmailLabel->setVisible(true);
        }
        return;
    }

    QJsonObject obj = doc.object();
    QString deliverability = obj.value("deliverability").toString().toUpper();
    bool isValidFormat = obj.value("is_valid_format").toObject().value("value").toBool(false);
    bool isMxFound = obj.value("is_mx_found").toObject().value("value").toBool(false);
    bool isSmtpValid = obj.value("is_smtp_valid").toObject().value("value").toBool(false);
    bool isDisposable = obj.value("is_disposable_email").toObject().value("value").toBool(false);

    qDebug() << "[EmailVerif]" << requestedEmail
             << "deliverability:" << deliverability
             << "mx:" << isMxFound << "smtp:" << isSmtpValid
             << "disposable:" << isDisposable;

    if (deliverability == "DELIVERABLE" && isValidFormat && isMxFound) {
        cherchEmailDeliverabilityOk = true;
        QString label = "✅ Email vérifié — délivrable";
        if (isDisposable) label = "⚠️ Email jetable — accepté mais déconseillé";
        if (cherchErrEmailLabel) {
            cherchErrEmailLabel->setText("  " + label);
            cherchErrEmailLabel->setStyleSheet(
                "color: #059669; font-size: 12px; background: #d1fae5; border: 1px solid #6ee7b7; "
                "border-radius: 8px; padding: 4px 10px;");
            cherchErrEmailLabel->setVisible(true);
        }
        ui->cherchLineEditEmail->setStyleSheet(inputBase.arg("#10b981", "#f0fdf4"));
    }
    else if (deliverability == "RISKY") {
        cherchEmailDeliverabilityOk = true;
        if (cherchErrEmailLabel) {
            cherchErrEmailLabel->setText("  ⚠️ Email accepté (risqué) — vérifiez manuellement");
            cherchErrEmailLabel->setStyleSheet(
                "color: #d97706; font-size: 12px; background: #fef3c7; border: 1px solid #fcd34d; "
                "border-radius: 8px; padding: 4px 10px;");
            cherchErrEmailLabel->setVisible(true);
        }
        ui->cherchLineEditEmail->setStyleSheet(inputBase.arg("#f59e0b", "#fffbeb"));
    }
    else {
        cherchEmailDeliverabilityOk = false;
        QString reason;
        if (!isValidFormat) reason = "format invalide";
        else if (!isMxFound) reason = "domaine introuvable (pas de serveur mail)";
        else if (!isSmtpValid) reason = "boîte inexistante";
        else reason = "non délivrable";
        if (cherchErrEmailLabel) {
            cherchErrEmailLabel->setText("  ❌ Email refusé : " + reason);
            cherchErrEmailLabel->setStyleSheet(
                "color: #dc2626; font-size: 12px; background: #fee2e2; border: 1px solid #fca5a5; "
                "border-radius: 8px; padding: 4px 10px;");
            cherchErrEmailLabel->setVisible(true);
        }
        ui->cherchLineEditEmail->setStyleSheet(inputBase.arg("#ef4444", "#fff1f2"));
    }

}

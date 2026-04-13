#include "trans_secure.h"

#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

// ============================================================================
// Chemin du fichier journal
// ============================================================================

QString TransSecure::cheminFichier()
{
    // Placé dans le dossier Documents de l'utilisateur
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return dir + QDir::separator() + "transactions_securite.txt";
}

// ============================================================================
// Enregistrement d'une opération
// ============================================================================

void TransSecure::logTransaction(const QString &action,
                                 int            id,
                                 const QString &type,
                                 double         montant,
                                 const QString &date,
                                 const QString &categorie,
                                 const QString &statut,
                                 const QString &projet,
                                 const QString &description)
{
    QFile file(cheminFichier());
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QString horodatage = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss");
    const QString separateur = QString(70, '-');

    out << separateur << "\n";
    out << QString("  ACTION      : %1\n").arg(action);
    out << QString("  Horodatage  : %1\n").arg(horodatage);
    out << QString("  ID          : %1\n").arg(id);
    out << QString("  Type        : %1\n").arg(type);
    out << QString("  Montant     : %1 TND\n").arg(QString::number(montant, 'f', 2));
    out << QString("  Date        : %1\n").arg(date);
    out << QString("  Categorie   : %1\n").arg(categorie);
    out << QString("  Statut      : %1\n").arg(statut);
    out << QString("  Projet      : %1\n").arg(projet.isEmpty() ? "—" : projet);
    if (!description.isEmpty())
        out << QString("  Description : %1\n").arg(description);
    out << separateur << "\n\n";

    file.close();
}

// ============================================================================
// Affichage du journal dans une fenêtre
// ============================================================================

void TransSecure::afficherJournal(QWidget *parent)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle("Journal de Sécurité des Transactions");
    dialog->setMinimumSize(750, 550);
    dialog->resize(850, 620);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // --- En-tête ---
    QFrame *header = new QFrame();
    header->setFixedHeight(80);
    header->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        " stop:0 #1e293b, stop:1 #334155); border: none; }");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(24, 12, 24, 12);

    QLabel *titleLabel = new QLabel("🔒  Journal de Sécurité — Transactions Financières");
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    QLabel *subLabel = new QLabel("Historique complet : ajouts, modifications et suppressions");
    subLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 12px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subLabel);
    mainLayout->addWidget(header);

    // --- Contenu ---
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier New", 10));
    textEdit->setStyleSheet(
        "QTextEdit { background-color: #0f172a; color: #e2e8f0;"
        " border: none; padding: 16px; line-height: 1.6; }");

    const QString path = cheminFichier();
    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        const QString contenu = in.readAll();
        file.close();

        if (contenu.trimmed().isEmpty()) {
            textEdit->setPlainText("  Aucune transaction enregistrée pour le moment.");
        } else {
            // Colorisation légère via HTML
            QString html = "<pre style='font-family:Courier New; font-size:10pt; color:#e2e8f0;'>";
            for (const QString &line : contenu.split('\n')) {
                QString escaped = line.toHtmlEscaped();
                if (line.startsWith("  ACTION")) {
                    if (line.contains("AJOUT"))
                        escaped = "<span style='color:#34d399; font-weight:bold;'>" + escaped + "</span>";
                    else if (line.contains("MODIFICATION"))
                        escaped = "<span style='color:#60a5fa; font-weight:bold;'>" + escaped + "</span>";
                    else if (line.contains("SUPPRESSION"))
                        escaped = "<span style='color:#f87171; font-weight:bold;'>" + escaped + "</span>";
                } else if (line.startsWith("  Horodatage")) {
                    escaped = "<span style='color:#94a3b8;'>" + escaped + "</span>";
                } else if (line.startsWith(QString(70, '-').left(3))) {
                    escaped = "<span style='color:#475569;'>" + escaped + "</span>";
                }
                html += escaped + "\n";
            }
            html += "</pre>";
            textEdit->setHtml(html);
        }
    } else {
        textEdit->setPlainText(
            "  Fichier journal introuvable.\n\n"
            "  Il sera créé automatiquement lors de la première opération sur une transaction.\n\n"
            "  Chemin attendu :\n  " + path);
    }

    mainLayout->addWidget(textEdit);

    // --- Pied de page ---
    QFrame *footer = new QFrame();
    footer->setFixedHeight(60);
    footer->setStyleSheet(
        "QFrame { background-color: #1e293b; border-top: 1px solid #334155; }");
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(16, 8, 16, 8);

    QLabel *pathLabel = new QLabel("📁  " + path);
    pathLabel->setStyleSheet("color: #64748b; font-size: 10px;");
    pathLabel->setWordWrap(true);
    footerLayout->addWidget(pathLabel, 1);

    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setFixedSize(100, 36);
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white; border: none;"
        " border-radius: 6px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    QObject::connect(btnFermer, &QPushButton::clicked, dialog, &QDialog::accept);
    footerLayout->addWidget(btnFermer);

    mainLayout->addWidget(footer);

    dialog->exec();
}

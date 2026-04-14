#include "rfid_journal.h"

#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

// ============================================================================
// Chemin du fichier journal
// ============================================================================
QString RfidJournal::cheminFichier()
{
    const QString dir = QStandardPaths::writableLocation(
        QStandardPaths::DocumentsLocation);
    return dir + QDir::separator() + "rfid_passages.txt";
}

// ============================================================================
// Enregistrement d'un passage
// ============================================================================
void RfidJournal::logPassage(const QString &cin,
                              const QString &resultat,
                              const QString &nom)
{
    QFile file(cheminFichier());
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QString horodatage =
        QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss");
    const QString sep = QString(70, '-');

    out << sep << "\n";
    out << "  ACTION      : CARTE SCANNEE\n";
    out << QString("  Horodatage  : %1\n").arg(horodatage);
    out << QString("  CIN scanné  : %1\n").arg(cin);

    if (resultat == "TROUVE") {
        out << "  Résultat    : CHERCHEUR TROUVE\n";
        out << QString("  Nom         : %1\n").arg(nom.isEmpty() ? "—" : nom);
    } else {
        out << "  Résultat    : ACCES BLOQUE — Inconnu\n";
        out << "  Nom         : —\n";
    }

    out << sep << "\n\n";
    file.close();
}

// ============================================================================
// Affichage du journal dans une fenêtre Qt
// ============================================================================
void RfidJournal::afficherJournal(QWidget *parent)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle("Journal RFID — Historique des passages");
    dialog->setMinimumSize(700, 500);
    dialog->resize(800, 580);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // En-tête
    QFrame *header = new QFrame();
    header->setFixedHeight(75);
    header->setStyleSheet(
        "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #1e293b, stop:1 #0f4c75); border: none; }");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(24, 10, 24, 10);

    QLabel *titleLabel = new QLabel("📋  Journal RFID — Historique des passages");
    titleLabel->setStyleSheet("color: white; font-size: 15px; font-weight: bold;");
    QLabel *subLabel = new QLabel(
        "Tous les scans de cartes : chercheurs trouvés et accès bloqués");
    subLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 11px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subLabel);
    mainLayout->addWidget(header);

    // Zone de texte
    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier New", 10));
    textEdit->setStyleSheet(
        "QTextEdit { background-color: #0f172a; color: #e2e8f0;"
        " border: none; padding: 16px; }");

    const QString path = cheminFichier();
    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        const QString contenu = in.readAll();
        file.close();

        if (contenu.trimmed().isEmpty()) {
            textEdit->setPlainText("  Aucun passage enregistré pour le moment.");
        } else {
            QString html =
                "<pre style='font-family:Courier New; font-size:10pt; color:#e2e8f0;'>";
            for (const QString &line : contenu.split('\n')) {
                QString escaped = line.toHtmlEscaped();
                if (line.contains("CHERCHEUR TROUVE")) {
                    escaped = "<span style='color:#34d399; font-weight:bold;'>"
                              + escaped + "</span>";
                } else if (line.contains("ACCES BLOQUE")) {
                    escaped = "<span style='color:#f87171; font-weight:bold;'>"
                              + escaped + "</span>";
                } else if (line.startsWith("  Horodatage")) {
                    escaped = "<span style='color:#94a3b8;'>" + escaped + "</span>";
                } else if (line.startsWith("---")) {
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
            "  Il sera créé automatiquement au premier scan.\n\n"
            "  Chemin attendu :\n  " + path);
    }
    mainLayout->addWidget(textEdit);

    // Pied de page
    QFrame *footer = new QFrame();
    footer->setFixedHeight(56);
    footer->setStyleSheet(
        "QFrame { background-color: #1e293b; border-top: 1px solid #334155; }");
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(16, 8, 16, 8);

    QLabel *pathLabel = new QLabel("📁  " + path);
    pathLabel->setStyleSheet("color: #64748b; font-size: 10px;");
    pathLabel->setWordWrap(true);
    footerLayout->addWidget(pathLabel, 1);

    QPushButton *btnFermer = new QPushButton("Fermer");
    btnFermer->setFixedSize(100, 34);
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        "QPushButton { background-color: #3b82f6; color: white;"
        " border: none; border-radius: 6px; font-weight: 600; }"
        "QPushButton:hover { background-color: #2563eb; }");
    QObject::connect(btnFermer, &QPushButton::clicked, dialog, &QDialog::accept);
    footerLayout->addWidget(btnFermer);
    mainLayout->addWidget(footer);

    dialog->exec();
}

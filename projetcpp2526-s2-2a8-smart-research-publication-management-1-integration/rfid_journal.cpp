#include "rfid_journal.h"

#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <QFrame>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

QString RfidJournal::cheminFichier()
{
    const QString dir = QStandardPaths::writableLocation(
        QStandardPaths::DocumentsLocation);
    return dir + QDir::separator() + QStringLiteral("rfid_passages.txt");
}

void RfidJournal::logPassage(const QString &cin,
                             const QString &resultat,
                             const QString &nom)
{
    QFile file(cheminFichier());
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec(QTextCodec::codecForName("UTF-8"));
#endif

    const QString horodatage =
        QDateTime::currentDateTime().toString(QStringLiteral("dd/MM/yyyy HH:mm:ss"));
    const QString separateur = QString(70, QLatin1Char('-'));

    out << separateur << QLatin1Char('\n');
    out << QStringLiteral("  ACTION      : CARTE SCANNEE\n");
    out << QStringLiteral("  Horodatage  : %1\n").arg(horodatage);
    out << QStringLiteral("  CIN scanné  : %1\n").arg(cin);

    if (resultat == QStringLiteral("TROUVE")) {
        out << QStringLiteral("  Résultat    : CHERCHEUR TROUVE\n");
        out << QStringLiteral("  Nom         : %1\n")
                   .arg(nom.isEmpty() ? QStringLiteral("—") : nom);
    } else {
        out << QStringLiteral("  Résultat    : ACCES BLOQUE — Inconnu\n");
        out << QStringLiteral("  Nom         : —\n");
    }

    out << separateur << QLatin1String("\n\n");
    file.close();
}

void RfidJournal::afficherJournal(QWidget *parent)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(QStringLiteral("Journal RFID — Historique des passages"));
    dialog->setMinimumSize(700, 500);
    dialog->resize(800, 580);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *header = new QFrame();
    header->setFixedHeight(75);
    header->setStyleSheet(
        QStringLiteral(
            "QFrame { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #1e293b, stop:1 #0f4c75); border: none; }"));
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(24, 10, 24, 10);

    QLabel *titleLabel = new QLabel(
        QStringLiteral("Journal RFID — Historique des passages"));
    titleLabel->setStyleSheet(
        QStringLiteral("color: white; font-size: 15px; font-weight: bold;"));
    QLabel *subLabel = new QLabel(
        QStringLiteral(
            "Tous les scans de cartes : chercheurs trouvés et accès bloqués"));
    subLabel->setStyleSheet(
        QStringLiteral("color: rgba(255,255,255,0.7); font-size: 11px;"));
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subLabel);
    mainLayout->addWidget(header);

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont(QStringLiteral("Courier New"), 10));
    textEdit->setStyleSheet(
        QStringLiteral(
            "QTextEdit { background-color: #0f172a; color: #e2e8f0;"
            " border: none; padding: 16px; }"));

    const QString path = cheminFichier();
    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec(QTextCodec::codecForName("UTF-8"));
#endif
        const QString contenu = in.readAll();
        file.close();

        if (contenu.trimmed().isEmpty()) {
            textEdit->setPlainText(
                QStringLiteral("  Aucun passage enregistré pour le moment."));
        } else {
            const QString sep70 = QString(70, QLatin1Char('-'));
            QString html =
                QStringLiteral(
                    "<pre style='font-family:Courier New; font-size:10pt;"
                    " color:#e2e8f0;'>");
            const QStringList lines = contenu.split(QLatin1Char('\n'));
            for (const QString &line : lines) {
                QString escaped = line.toHtmlEscaped();
                if (line.contains(QStringLiteral("CHERCHEUR TROUVE"))) {
                    escaped =
                        QStringLiteral("<span style='color:#34d399; font-weight:bold;'>")
                        + escaped + QStringLiteral("</span>");
                } else if (line.contains(QStringLiteral("ACCES BLOQUE"))) {
                    escaped =
                        QStringLiteral("<span style='color:#f87171; font-weight:bold;'>")
                        + escaped + QStringLiteral("</span>");
                } else if (line.startsWith(QStringLiteral("  Horodatage"))) {
                    escaped = QStringLiteral("<span style='color:#94a3b8;'>") + escaped
                            + QStringLiteral("</span>");
                } else if (line.startsWith(sep70)) {
                    escaped = QStringLiteral("<span style='color:#475569;'>") + escaped
                            + QStringLiteral("</span>");
                }
                html += escaped + QLatin1Char('\n');
            }
            html += QStringLiteral("</pre>");
            textEdit->setHtml(html);
        }
    } else {
        textEdit->setPlainText(
            QStringLiteral("  Fichier journal introuvable.\n\n"
                           "  Il sera créé automatiquement au premier scan.\n\n"
                           "  Chemin attendu :\n  ")
            + path);
    }
    mainLayout->addWidget(textEdit);

    QFrame *footer = new QFrame();
    footer->setFixedHeight(56);
    footer->setStyleSheet(
        QStringLiteral(
            "QFrame { background-color: #1e293b;"
            " border-top: 1px solid #334155; }"));
    QHBoxLayout *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(16, 8, 16, 8);

    QLabel *pathLabel = new QLabel(path);
    pathLabel->setStyleSheet(
        QStringLiteral("color: #64748b; font-size: 10px;"));
    pathLabel->setWordWrap(true);
    footerLayout->addWidget(pathLabel, 1);

    QPushButton *btnFermer = new QPushButton(QStringLiteral("Fermer"));
    btnFermer->setFixedSize(100, 34);
    btnFermer->setCursor(Qt::PointingHandCursor);
    btnFermer->setStyleSheet(
        QStringLiteral(
            "QPushButton { background-color: #3b82f6; color: white;"
            " border: none; border-radius: 6px; font-weight: 600; }"
            "QPushButton:hover { background-color: #2563eb; }"));
    QObject::connect(btnFermer, &QPushButton::clicked, dialog, &QDialog::accept);
    footerLayout->addWidget(btnFermer);
    mainLayout->addWidget(footer);

    dialog->exec();
}

#include "trans_secure.h"
#include "connection.h"

#include <QApplication>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

// ============================================================================
// Chemin du fichier journal — avec fallbacks si OneDrive bloque l'écriture
// ============================================================================

QString TransSecure::cheminFichier()
{
    // Liste de chemins candidats par ordre de préférence
    QStringList candidats;

    // 1. Dossier Documents standard
    candidats << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                     + QDir::separator() + "transactions_securite.txt";

    // 2. Dossier Home de l'utilisateur
    candidats << QDir::homePath() + QDir::separator() + "transactions_securite.txt";

    // 3. Dossier temporaire de l'application
    candidats << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                     + QDir::separator() + "transactions_securite.txt";

    // 4. Dossier temp système
    candidats << QDir::tempPath() + QDir::separator() + "transactions_securite.txt";

    // Si le fichier existe déjà dans un des chemins, on le retourne
    for (const QString &path : candidats) {
        if (QFile::exists(path))
            return path;
    }

    // Sinon, tester lequel est accessible en écriture
    for (const QString &path : candidats) {
        QFileInfo fi(path);
        QDir dir = fi.absoluteDir();
        // Créer le dossier si nécessaire (ex: AppDataLocation)
        if (!dir.exists())
            dir.mkpath(".");
        QFile test(path);
        if (test.open(QIODevice::WriteOnly | QIODevice::Append)) {
            test.close();
            // Si le fichier était vide (juste créé), on le supprime pour laisser
            // initialiserFichier() le recréer proprement
            if (QFileInfo(path).size() == 0)
                QFile::remove(path);
            return path;
        }
    }

    // Dernier recours : Documents (même si ça échoue, on retourne quelque chose)
    return candidats.first();
}

// ============================================================================
// Initialisation du fichier journal (créé s'il n'existe pas)
// ============================================================================

void TransSecure::initialiserFichier()
{
    const QString path = cheminFichier();
    if (QFile::exists(path))
        return;

    // S'assurer que le dossier parent existe
    QFileInfo fi(path);
    fi.absoluteDir().mkpath(".");

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QString ligne = QString(70, '=');
    out << ligne << "\n";
    out << "  JOURNAL DE SECURITE — TRANSACTIONS FINANCIERES\n";
    out << "  Cree le : " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";
    out << "  Ce fichier est permanent. Les entrees ne sont jamais supprimees.\n";
    out << ligne << "\n\n";

    file.close();
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
    initialiserFichier();

    QFile file(cheminFichier());
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const QString horodatage = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss");
    const QString sep = QString(70, '-');

    out << sep << "\n";
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
    out << sep << "\n\n";

    file.close();
}

// ============================================================================
// Charge les transactions depuis la BD et les écrit dans le fichier
// (appelé une seule fois si le fichier vient d'être créé)
// ============================================================================

static void chargerTransactionsBD()
{
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen())
        return;

    QSqlQuery q(db);
    const QString sql =
        "SELECT f.ID_TRANSACTION, f.MONTANT, f.TYPE_TRANS, f.CATEGORIE, "
        "f.DATE_TRANSACTION, f.STATUT, f.DESCRIPTION, p.TITRE "
        "FROM FINANCE f LEFT JOIN PROJET p ON p.ID_PROJET = f.ID_PROJET "
        "ORDER BY f.ID_TRANSACTION";

    if (!q.exec(sql))
        return;

    while (q.next()) {
        int    id          = q.value(0).toInt();
        double montant     = q.value(1).toDouble();
        QString type       = q.value(2).toString();
        QString categorie  = q.value(3).toString();
        QString date;
        QVariant dv = q.value(4);
        QDate d = dv.toDate();
        if (!d.isValid() && dv.toDateTime().isValid())
            d = dv.toDateTime().date();
        date = d.isValid() ? d.toString("dd/MM/yyyy") : dv.toString();
        QString statut      = q.value(5).toString();
        QString description = q.value(6).toString();
        QString projet      = q.value(7).toString();

        TransSecure::logTransaction("EXISTANT (chargé depuis BD)",
                                    id, type, montant, date,
                                    categorie, statut, projet, description);
    }
}

// ============================================================================
// Affichage du journal dans une fenêtre
// ============================================================================

void TransSecure::afficherJournal(QWidget *parent)
{
    // Créer le fichier s'il n'existe pas
    const bool estNouveau = !QFile::exists(cheminFichier());
    initialiserFichier();

    // Si le fichier vient d'être créé, on charge les transactions existantes en BD
    if (estNouveau) {
        chargerTransactionsBD();
    }

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
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);
        const QString contenu = in.readAll();
        file.close();

        const bool aDesTransactions = contenu.contains("ACTION      :");

        if (!aDesTransactions) {
            textEdit->setPlainText(
                "  Aucune transaction enregistrée pour le moment.\n\n"
                "  Les transactions apparaîtront ici dès qu'un ajout,\n"
                "  une modification ou une suppression sera effectuée.");
        } else {
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
                    else if (line.contains("EXISTANT"))
                        escaped = "<span style='color:#a78bfa; font-weight:bold;'>" + escaped + "</span>";
                } else if (line.startsWith("  Horodatage")) {
                    escaped = "<span style='color:#94a3b8;'>" + escaped + "</span>";
                } else if (line.startsWith("  ID") || line.startsWith("  Montant")) {
                    escaped = "<span style='color:#fbbf24;'>" + escaped + "</span>";
                } else if (line.trimmed().startsWith("---")) {
                    escaped = "<span style='color:#475569;'>" + escaped + "</span>";
                } else if (line.trimmed().startsWith("===")) {
                    escaped = "<span style='color:#334155;'>" + escaped + "</span>";
                }

                html += escaped + "\n";
            }
            html += "</pre>";
            textEdit->setHtml(html);
        }
    } else {
        textEdit->setPlainText(
            "  Impossible d'ouvrir le fichier journal.\n\n"
            "  Chemin tenté :\n  " + path + "\n\n"
            "  Vérifiez les permissions d'écriture sur ce dossier.");
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

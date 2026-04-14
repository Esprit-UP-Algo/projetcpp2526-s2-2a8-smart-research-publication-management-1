#include "scenario3.h"
#include "rfid_journal.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

Scenario3::Scenario3(Arduino *arduino)
    : m_arduino(arduino),
      m_dernierResultat(false),
      m_compteurEchecs(0)
{
}

void Scenario3::processIdentification()
{
    processIdentification(m_arduino->readLine());
}

void Scenario3::processIdentification(const QString &rawLine)
{
    QString message = rawLine;
    if (message.isEmpty())
        return;

    message.remove(QChar('\0'));
    message = message.trimmed();

    qDebug() << "[Scenario3] Reçu :" << message;

    QString cin;
    if (message.startsWith(QStringLiteral("RFID:"), Qt::CaseInsensitive)) {
        cin = message.mid(5).trimmed();
    } else {
        qDebug() << "[Scenario3] Format non reconnu.";
        sendInconnu();
        return;
    }

    if (cin.isEmpty()) {
        sendInconnu();
        return;
    }

    m_dernierCin = cin;

    QString nomComplet;
    if (chercheurExiste(cin, nomComplet)) {
        m_dernierNom = nomComplet;
        m_dernierResultat = true;
        m_compteurEchecs = 0;

        qDebug() << "[Scenario3] TROUVÉ :" << nomComplet;

        RfidJournal::logPassage(cin, QStringLiteral("TROUVE"), nomComplet);
        sendTrouve();
    } else {
        m_dernierNom.clear();
        m_dernierResultat = false;
        m_compteurEchecs++;

        qDebug() << "[Scenario3] INCONNU — CIN :" << cin
                 << "— Échecs consécutifs :" << m_compteurEchecs;

        RfidJournal::logPassage(cin, QStringLiteral("INCONNU"));

        if (m_compteurEchecs >= 3) {
            qDebug() << "[Scenario3] ALERTE — 3 tentatives échouées";
            sendAlerte();
            m_compteurEchecs = 0;
        } else {
            sendInconnu();
        }
    }
}

bool Scenario3::chercheurExiste(const QString &cin, QString &nomComplet)
{
    QSqlQuery query;
    query.prepare(
        QStringLiteral("SELECT NOM, PRENOM FROM CHERCHEUR WHERE CIN = :cin"));
    query.bindValue(QStringLiteral(":cin"), cin);

    if (!query.exec()) {
        qDebug() << "[Scenario3] Erreur SQL :" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        const QString nom = query.value(0).toString().trimmed();
        const QString prenom = query.value(1).toString().trimmed();
        nomComplet = prenom + QLatin1Char(' ') + nom;
        return true;
    }
    return false;
}

void Scenario3::sendTrouve()
{
    m_arduino->write_to_arduino(QStringLiteral("TROUVE\n").toUtf8());
    qDebug() << "[Scenario3] >>> CHERCHEUR TROUVÉ <<<";
}

void Scenario3::sendInconnu()
{
    m_arduino->write_to_arduino(QStringLiteral("INCONNU\n").toUtf8());
    qDebug() << "[Scenario3] >>> ACCÈS BLOQUÉ <<<";
}

void Scenario3::sendAlerte()
{
    m_arduino->write_to_arduino(QStringLiteral("ALERTE\n").toUtf8());
    qDebug() << "[Scenario3] >>> ALERTE SÉCURITÉ — 3 échecs <<<";
}

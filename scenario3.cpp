#include "scenario3.h"
#include "rfid_journal.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================================
// Constructeur
// ============================================================================
Scenario3::Scenario3(Arduino* arduino)
    : m_arduino(arduino),
      m_dernierResultat(false),
      m_compteurEchecs(0)
{
}

// ============================================================================
// processIdentification()
// À connecter sur readyRead() quand le module Chercheurs est actif (index 0)
// ============================================================================
void Scenario3::processIdentification()
{
    QString message = m_arduino->readLine();
    if (message.isEmpty())
        return;

    message.remove(QChar('\0'));
    message = message.trimmed();

    qDebug() << "[Scenario3] Reçu :" << message;

    // Format attendu : "RFID:<CIN>"
    if (!message.startsWith("RFID:", Qt::CaseInsensitive)) {
        qDebug() << "[Scenario3] Format non reconnu.";
        sendInconnu();
        return;
    }

    const QString cin = message.mid(5).trimmed();
    if (cin.isEmpty()) {
        sendInconnu();
        return;
    }

    m_dernierCin = cin;

    QString nomComplet;
    if (chercheurExiste(cin, nomComplet)) {
        // ✅ Chercheur trouvé
        m_dernierNom      = nomComplet;
        m_dernierResultat = true;
        m_compteurEchecs  = 0;  // reset compteur

        qDebug() << "[Scenario3] TROUVÉ :" << nomComplet;

        RfidJournal::logPassage(cin, "TROUVE", nomComplet);
        sendTrouve();

    } else {
        // 🚫 Chercheur inconnu
        m_dernierNom      = "";
        m_dernierResultat = false;
        m_compteurEchecs++;

        qDebug() << "[Scenario3] INCONNU — CIN :" << cin
                 << "— Échecs consécutifs :" << m_compteurEchecs;

        RfidJournal::logPassage(cin, "INCONNU");

        if (m_compteurEchecs >= 3) {
            qDebug() << "[Scenario3] ⚠️ ALERTE — 3 tentatives échouées !";
            sendAlerte();
            m_compteurEchecs = 0;  // reset après alerte
        } else {
            sendInconnu();
        }
    }
}

// ============================================================================
// chercheurExiste() — requête SQL sur la table CHERCHEUR
// ============================================================================
bool Scenario3::chercheurExiste(const QString& cin, QString& nomComplet)
{
    QSqlQuery query;
    query.prepare(
        "SELECT NOM, PRENOM "
        "FROM CHERCHEUR "
        "WHERE CIN = :cin"
    );
    query.bindValue(":cin", cin);

    if (!query.exec()) {
        qDebug() << "[Scenario3] Erreur SQL :" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        const QString nom    = query.value(0).toString().trimmed();
        const QString prenom = query.value(1).toString().trimmed();
        nomComplet = prenom + " " + nom;
        return true;
    }
    return false;
}

// ============================================================================
// Commandes vers l'Arduino
// ============================================================================
void Scenario3::sendTrouve()
{
    m_arduino->write_to_arduino("TROUVE\n");
    qDebug() << "[Scenario3] >>> CHERCHEUR TROUVÉ <<<";
}

void Scenario3::sendInconnu()
{
    m_arduino->write_to_arduino("INCONNU\n");
    qDebug() << "[Scenario3] >>> ACCÈS BLOQUÉ <<<";
}

void Scenario3::sendAlerte()
{
    m_arduino->write_to_arduino("ALERTE\n");
    qDebug() << "[Scenario3] >>> ⚠️ ALERTE SÉCURITÉ — 3 échecs <<<";
}

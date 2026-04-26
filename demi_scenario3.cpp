#include "demi_scenario3.h"
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================================
// Constructeur
// ============================================================================
DemiScenario3::DemiScenario3(Arduino* arduino)
    : m_arduino(arduino)
{
}

// ============================================================================
// processInput()
// À appeler depuis le slot readyRead() dans SmartPub.
//
// Flux :
//   1. Lire la ligne série depuis l'Arduino
//   2. Si "FIRE:<id>" → désactiver le laboratoire en BD + envoyer ACK
//   3. Si "TEMP:<val>" → mémoriser la température courante
// ============================================================================
void DemiScenario3::processInput()
{
    if (!m_arduino) return;

    QString message = m_arduino->readLine();
    if (message.isEmpty()) return;

    message = message.trimmed();
    qDebug() << "[DemiScenario3] Recu de l'Arduino :" << message;

    // ── Alerte incendie ───────────────────────────────────────────────
    if (message.startsWith("FIRE:")) {
        const int id_labo = message.mid(5).trimmed().toInt();
        if (id_labo <= 0) {
            qDebug() << "[DemiScenario3] ID laboratoire invalide dans FIRE :" << message;
            return;
        }

        qDebug() << "[DemiScenario3] INCENDIE detecte dans le laboratoire ID :" << id_labo;

        m_incendieDetecte = true;
        m_idLaboEnAlerte  = id_labo;

        if (desactiverLaboratoire(id_labo)) {
            qDebug() << "[DemiScenario3] Laboratoire ID" << id_labo << "passe en Inactif.";
        } else {
            qDebug() << "[DemiScenario3] Echec mise a jour BD pour laboratoire ID :" << id_labo;
        }

        // Accuser réception à l'Arduino
        envoyerAck();
        return;
    }

    // ── Température périodique ────────────────────────────────────────
    if (message.startsWith("TEMP:")) {
        bool ok = false;
        const double temp = message.mid(5).trimmed().toDouble(&ok);
        if (ok) {
            m_derniereTemp = temp;
            qDebug() << "[DemiScenario3] Temperature courante :" << temp << "°C";
        }
        return;
    }

    // ── Messages informatifs ──────────────────────────────────────────
    if (message == "READY") {
        qDebug() << "[DemiScenario3] Arduino pret.";
        return;
    }

    if (message == "RESET:OK") {
        m_incendieDetecte = false;
        m_idLaboEnAlerte  = -1;
        qDebug() << "[DemiScenario3] Etat reinitialise.";
        return;
    }

    if (message.startsWith("ERR:")) {
        qDebug() << "[DemiScenario3] Erreur Arduino :" << message;
        return;
    }
}

// ============================================================================
// desactiverLaboratoire()
// Met DISPONIBILITE = 'indisponible' pour le laboratoire donné.
// Correspond au passage Actif → Inactif dans l'interface SmartPub.
// ============================================================================
bool DemiScenario3::desactiverLaboratoire(int id_labo)
{
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        qDebug() << "[DemiScenario3] Base de donnees non connectee.";
        return false;
    }

    QSqlQuery query(db);
    query.prepare(
        "UPDATE LABORATOIRE "
        "SET DISPONIBILITE = 'indisponible' "
        "WHERE ID_LABORATOIRE = :id"
    );
    query.bindValue(":id", id_labo);

    if (!query.exec()) {
        qDebug() << "[DemiScenario3] Erreur SQL desactiverLaboratoire :"
                 << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "[DemiScenario3] Aucun laboratoire trouve avec ID :" << id_labo;
        return false;
    }

    return true;
}

// ============================================================================
// envoyerAck()
// Envoie "ACK\n" à l'Arduino pour accuser réception de l'alerte incendie
// ============================================================================
void DemiScenario3::envoyerAck()
{
    if (!m_arduino) return;
    m_arduino->write_to_arduino("ACK\n");
    qDebug() << "[DemiScenario3] >>> ACK envoye a l'Arduino";
}

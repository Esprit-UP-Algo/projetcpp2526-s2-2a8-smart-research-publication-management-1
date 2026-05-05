#include "demi_scenario3.h"
#include "connection.h"
#include <QtSerialPort/QSerialPort>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>



//-------------------------------

// ============================================================================
// Constructeur
// Se connecte directement sur readyRead() du QSerialPort avec son propre
// buffer — indépendant de Scenario1 qui a son propre buffer dans Arduino.
// ============================================================================
DemiScenario3::DemiScenario3(Arduino* arduino, QObject* parent)
    : QObject(parent), m_arduino(arduino)
{
    // Connexion directe sur le port série — buffer interne m_buffer
    connect(m_arduino->getserial(), &QSerialPort::readyRead,
            this, &DemiScenario3::onSerialDataReady);
}

// ============================================================================
// activerPourLabo()
// Envoie "START:<id>\n" à l'Arduino pour déclencher une lecture DHT11.
// ============================================================================
void DemiScenario3::activerPourLabo(int id_labo)
{
    if (!m_arduino) return;
    QString cmd = QString("START:%1\n").arg(id_labo);
    m_arduino->write_to_arduino(cmd.toUtf8());
    qDebug() << "[DemiScenario3] >>> Envoi Arduino :" << cmd.trimmed();
}

// ============================================================================
// onSerialDataReady()
// Slot connecté sur QSerialPort::readyRead().
// Accumule les données dans m_buffer et extrait les lignes complètes.
// Seules les trames TEMP:/FIRE:/ERR: sont traitées ici.
// Les autres (RFID:, PORTE_*) sont ignorées — elles appartiennent à Scenario1.
// ============================================================================
void DemiScenario3::onSerialDataReady()
{
    if (!m_arduino || !m_arduino->getserial()) return;

    // Lire les données disponibles dans notre propre buffer
    m_buffer.append(m_arduino->getserial()->peek(m_arduino->getserial()->bytesAvailable()));

    // Extraire les lignes complètes
    int pos;
    while ((pos = m_buffer.indexOf('\n')) != -1) {
        QByteArray lineBytes = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);
        QString line = QString::fromUtf8(lineBytes).trimmed();
        if (!line.isEmpty())
            processLine(line);
    }
}

// ============================================================================
// processLine()
// Traite une ligne reçue de l'Arduino.
// Ignore tout ce qui n'est pas TEMP:/FIRE:/ERR: (appartient à Scenario1).
// ============================================================================
void DemiScenario3::processLine(const QString& line)
{
    if (line.startsWith("TEMP:")) {
        bool ok = false;
        double temp = line.mid(5).trimmed().toDouble(&ok);
        if (ok) {
            m_derniereTemp = temp;
            qDebug() << "[DemiScenario3] Temperature :" << temp << "°C";
        }
        return;
    }

    if (line.startsWith("FIRE:")) {
        int id_labo = line.mid(5).trimmed().toInt();
        if (id_labo <= 0) return;

        qDebug() << "[DemiScenario3] Chaleur detectee — labo ID :" << id_labo;
        m_incendieDetecte = true;
        m_idLaboEnAlerte  = id_labo;

        if (desactiverLaboratoire(id_labo)) {
            qDebug() << "[DemiScenario3] Labo ID" << id_labo << "-> Inactif en BD.";
            emit laboDesactive(id_labo);
        } else {
            qDebug() << "[DemiScenario3] Echec mise a jour BD pour labo ID :" << id_labo;
        }
        return;
    }

    if (line.startsWith("ERR:")) {
        qDebug() << "[DemiScenario3] Erreur Arduino :" << line;
    }
    // RFID:, PORTE_*, READY → ignorés, appartiennent à Scenario1
}

// ============================================================================
// desactiverLaboratoire()
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
        qDebug() << "[DemiScenario3] Erreur SQL :" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "[DemiScenario3] Aucun laboratoire avec ID :" << id_labo;
        return false;
    }

    return true;
}

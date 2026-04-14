#include "scenario3.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>

Scenario3::Scenario3(Arduino *arduino)
    : m_arduino(arduino)
{
}

void Scenario3::processCloture()
{
    QString message = m_arduino->readLine();
    if (message.isEmpty())
        return;

    message.remove(QChar('\0'));
    message = message.trimmed();
    processClotureFromMessage(message);
}

void Scenario3::processClotureFromMessage(const QString &message)
{
    qDebug() << "[Scenario3] Reçu :" << message;

    if (!message.startsWith("CLOTURER:", Qt::CaseInsensitive)) {
        qDebug() << "[Scenario3] Format non reconnu, ignoré.";
        sendError("FORMAT_INVALIDE");
        return;
    }

    bool ok = false;
    const int idProjet = message.mid(9).trimmed().toInt(&ok);

    if (!ok || idProjet <= 0) {
        qDebug() << "[Scenario3] ID projet invalide :" << message.mid(9);
        sendError("ID_INVALIDE");
        return;
    }

    qDebug() << "[Scenario3] Demande de clôture pour projet ID :" << idProjet;

    const QString titre = getProjetEnCours(idProjet);
    if (titre.isNull()) {
        qDebug() << "[Scenario3] Projet introuvable ou non en cours.";
        sendError("PROJET_NON_EN_COURS");
        return;
    }

    if (!cloturerProjet(idProjet)) {
        qDebug() << "[Scenario3] Échec UPDATE PROJET.";
        sendError("DB_ERROR");
        return;
    }

    const int codeEv = creerEvenementCloture(idProjet, titre);
    if (codeEv == -1) {
        qDebug() << "[Scenario3] Avertissement : projet clôturé mais événement non créé.";
    }

    qDebug() << "[Scenario3] Projet clôturé :" << titre
             << "— événement code:" << codeEv;
    sendOk(titre);
}

QString Scenario3::getProjetEnCours(int idProjet)
{
    QSqlQuery query;
    query.prepare(
        "SELECT TITRE "
        "FROM PROJET "
        "WHERE ID_PROJET = :id "
        "  AND ETAT = 'en_cours'"
    );
    query.bindValue(":id", idProjet);

    if (!query.exec()) {
        qDebug() << "[Scenario3] Erreur SQL getProjetEnCours :" << query.lastError().text();
        return QString();
    }

    if (query.next())
        return query.value(0).toString();

    return QString();
}

bool Scenario3::cloturerProjet(int idProjet)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE PROJET "
        "SET ETAT = 'termine', PROGRESSION = 100 "
        "WHERE ID_PROJET = :id"
    );
    query.bindValue(":id", idProjet);

    if (!query.exec()) {
        qDebug() << "[Scenario3] Erreur SQL cloturerProjet :" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

int Scenario3::creerEvenementCloture(int idProjet, const QString &titreProjet)
{
    const int codeEv = 90000 + idProjet;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM EVENEMENT WHERE CODE_EVENEMENT = :code");
    checkQuery.bindValue(":code", codeEv);
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "[Scenario3] Événement de clôture déjà existant pour ce projet.";
        return codeEv;
    }

    const QString nomEv = QString("Clôture : %1").arg(titreProjet).left(200);
    const QString dateEv = QDate::currentDate().toString("dd/MM/yyyy");

    QSqlQuery query;
    query.prepare(
        "INSERT INTO EVENEMENT (CODE_EVENEMENT, NOM, LIEU, DATE_EVENEMENT) "
        "VALUES (:code, :nom, :lieu, TO_DATE(:date, 'DD/MM/YYYY'))"
    );
    query.bindValue(":code", codeEv);
    query.bindValue(":nom",  nomEv);
    query.bindValue(":lieu", QStringLiteral("Salle de conférence"));
    query.bindValue(":date", dateEv);

    if (!query.exec()) {
        qDebug() << "[Scenario3] Erreur SQL creerEvenementCloture :" << query.lastError().text();
        return -1;
    }

    return codeEv;
}

void Scenario3::sendOk(const QString &titreProjet)
{
    const QString safe = titreProjet.left(50).replace('\n', ' ').replace('\r', ' ');
    m_arduino->write_to_arduino(QString("OK:%1\n").arg(safe).toUtf8());
    qDebug() << "[Scenario3] >>> OK — projet clôturé :" << safe;
}

void Scenario3::sendError(const QString &raison)
{
    m_arduino->write_to_arduino(QString("ERR:%1\n").arg(raison).toUtf8());
    qDebug() << "[Scenario3] >>> ERREUR —" << raison;
}

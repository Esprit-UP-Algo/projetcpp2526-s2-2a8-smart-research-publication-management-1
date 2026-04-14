#include "scenario2.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>

// ============================================================================
// Constructeur
// ============================================================================
Scenario2::Scenario2(Arduino *arduino)
    : m_arduino(arduino)
{
}

// ============================================================================
// processCloture()
// À connecter sur le signal readyRead() du QSerialPort de cet Arduino.
// ============================================================================
void Scenario2::processCloture()
{
    processCloture(m_arduino->readLine());
}

void Scenario2::processCloture(const QString &rawLine)
{
    QString message = rawLine;
    if (message.isEmpty())
        return;

    message.remove(QChar('\0'));
    message = message.trimmed();

    qDebug() << "[Scenario2] Reçu :" << message;

    // ── Parser le message ── format attendu : "CLOTURER:<id_projet>"
    if (!message.startsWith("CLOTURER:", Qt::CaseInsensitive)) {
        qDebug() << "[Scenario2] Format non reconnu, ignoré.";
        sendError("FORMAT_INVALIDE");
        return;
    }

    bool ok = false;
    const int idProjet = message.mid(9).trimmed().toInt(&ok);

    if (!ok || idProjet <= 0) {
        qDebug() << "[Scenario2] ID projet invalide :" << message.mid(9);
        sendError("ID_INVALIDE");
        return;
    }

    qDebug() << "[Scenario2] Demande de clôture pour projet ID :" << idProjet;

    // ── Étape 1 : projet existe et est 'en_cours' ?
    const QString titre = getProjetEnCours(idProjet);
    if (titre.isNull()) {
        qDebug() << "[Scenario2] Projet introuvable ou non en cours.";
        sendError("PROJET_NON_EN_COURS");
        return;
    }

    // ── Étape 2 : clôturer le projet
    if (!cloturerProjet(idProjet)) {
        qDebug() << "[Scenario2] Échec UPDATE PROJET.";
        sendError("DB_ERROR");
        return;
    }

    // ── Étape 3 : créer l'événement de clôture
    const int codeEv = creerEvenementCloture(idProjet, titre);
    if (codeEv == -1) {
        qDebug() << "[Scenario2] Avertissement : projet clôturé mais événement non créé.";
        // Non bloquant : le projet est déjà clôturé, on répond OK quand même
    }

    qDebug() << "[Scenario2] Projet clôturé :" << titre
             << "— événement code:" << codeEv;
    sendOk(titre);
}

// ============================================================================
// Étape 1 : projet existe et ETAT = 'en_cours'
// ============================================================================
QString Scenario2::getProjetEnCours(int idProjet)
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
        qDebug() << "[Scenario2] Erreur SQL getProjetEnCours :" << query.lastError().text();
        return QString();
    }

    if (query.next())
        return query.value(0).toString();

    return QString();
}

// ============================================================================
// Étape 2 : UPDATE PROJET SET ETAT='termine', PROGRESSION=100
// ============================================================================
bool Scenario2::cloturerProjet(int idProjet)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE PROJET "
        "SET ETAT = 'termine', PROGRESSION = 100 "
        "WHERE ID_PROJET = :id"
    );
    query.bindValue(":id", idProjet);

    if (!query.exec()) {
        qDebug() << "[Scenario2] Erreur SQL cloturerProjet :" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

// ============================================================================
// Étape 3 : INSERT INTO EVENEMENT — événement de clôture
// CODE_EVENEMENT = 90000 + id_projet (pour éviter les collisions)
// DATE_EVENEMENT = aujourd'hui
// LIEU           = "Salle de conférence"
// ============================================================================
int Scenario2::creerEvenementCloture(int idProjet, const QString &titreProjet)
{
    // Générer un code unique basé sur l'ID projet
    const int codeEv = 90000 + idProjet;

    // Vérifier que ce code n'existe pas déjà
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM EVENEMENT WHERE CODE_EVENEMENT = :code");
    checkQuery.bindValue(":code", codeEv);
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "[Scenario2] Événement de clôture déjà existant pour ce projet.";
        return codeEv; // déjà créé, pas une erreur
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
        qDebug() << "[Scenario2] Erreur SQL creerEvenementCloture :" << query.lastError().text();
        return -1;
    }

    return codeEv;
}

// ============================================================================
// Réponses vers Arduino
// ============================================================================
void Scenario2::sendOk(const QString &titreProjet)
{
    const QString safe = titreProjet.left(50).replace('\n', ' ').replace('\r', ' ');
    m_arduino->write_to_arduino(QString("OK:%1\n").arg(safe).toUtf8());
    qDebug() << "[Scenario2] >>> OK — projet clôturé :" << safe;
}

void Scenario2::sendError(const QString &raison)
{
    m_arduino->write_to_arduino(QString("ERR:%1\n").arg(raison).toUtf8());
    qDebug() << "[Scenario2] >>> ERREUR —" << raison;
}

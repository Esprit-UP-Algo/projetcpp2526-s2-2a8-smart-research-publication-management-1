#include "scenario1.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================
// Constructeur
// ============================================================
Scenario1::Scenario1(Arduino* arduino, int id_laboratoire)
    : arduino(arduino), id_lab(id_laboratoire)
{
}

// ============================================================
// processAccess() — appelée depuis MainWindow sur readyRead()
// ============================================================
void Scenario1::processAccess()
{
    processAccess(arduino->readLine());
}

void Scenario1::processAccess(const QString &rawLine)
{
    QString message = rawLine;
    if (message.isEmpty()) {
        return;
    }

    message.remove(QChar('\0'));
    message = message.trimmed();

    qDebug() << "[Scenario1] Reçu de l'Arduino :" << message;

    // Parser le message — format attendu : "RFID:01020304"
    QString uid = "";
    if (message.startsWith("RFID:", Qt::CaseInsensitive)) {
        uid = message.mid(5).trimmed();
    } else {
        qDebug() << "[Scenario1] Format non reconnu (attendu RFID:CIN), accès refusé.";
        denyAccess();
        return;
    }

    if (uid.isEmpty()) {
        qDebug() << "[Scenario1] UID vide, accès refusé.";
        denyAccess();
        return;
    }

    // 3. Identifier le chercheur via son CIN (l'UID RFID correspond au CIN)
    int id_chercheur = getChercheurId(uid);
    if (id_chercheur == -1) {
        qDebug() << "[Scenario1] Chercheur introuvable pour UID (CIN) :" << uid;
        denyAccess();
        return;
    }

    // 4. Vérifier projet en cours
    if (!checkProjetEnCours(id_chercheur)) {
        qDebug() << "[Scenario1] Aucun projet en cours pour chercheur ayant id :" << id_chercheur;
        denyAccess();
        return;
    }

    // 5. Vérifier que le projet est bien affecté à CE laboratoire
    if (!checkProjetDansLabo(id_chercheur, id_lab)) {
        qDebug() << "[Scenario1] Projet non lié au labo ayant id :" << id_lab;
        denyAccess();
        return;
    }

    // 6. Tout est OK → accès autorisé
    qDebug() << "[Scenario1] Accès AUTORISÉ pour chercheur ayant id :" << id_chercheur;
    grantAccess();
}

// ============================================================
// Étape 1 : chercher le chercheur par CIN (UID RFID)
// ============================================================
int Scenario1::getChercheurId(const QString& cin)
{
    QSqlQuery query;
    query.prepare(
        "SELECT ID_CHERCHEUR "
        "FROM CHERCHEUR "
        "WHERE CIN = :cin"
        );
    query.bindValue(":cin", cin);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL getChercheurId :" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt(); // retourner l'ID trouvé
    }

    return -1; // non trouvé
}

// ============================================================
// Étape 2 : vérifier qu'il a un projet en cours
// ============================================================
bool Scenario1::checkProjetEnCours(int id_chercheur)
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) "
        "FROM CONTRIBUER C "
        "JOIN PROJET P ON C.ID_PROJET = P.ID_PROJET "
        "WHERE C.ID_CHERCHEUR = :id "
        "  AND P.ETAT = 'en_cours' "
        );
    query.bindValue(":id", id_chercheur);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL checkProjetEnCours :" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return (count > 0); // vrai si au moins 1 projet en cours
    }

    return false;
}

// ============================================================
// Étape 3 : vérifier que le projet est bien dans CE labo
// ============================================================
bool Scenario1::checkProjetDansLabo(int id_chercheur, int id_lab)
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) "
        "FROM LABORATOIRE L "
        "JOIN PROJET P ON L.ID_PROJET = P.ID_PROJET "
        "JOIN CONTRIBUER C ON C.ID_PROJET = P.ID_PROJET "
        "WHERE C.ID_CHERCHEUR = :id "
        "  AND L.ID_LABORATOIRE = :id_lab "
        );
    query.bindValue(":id", id_chercheur);
    query.bindValue(":id_lab", id_lab);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL checkProjetDansLabo :" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return (query.value(0).toInt() > 0);
    }

    return false;
}

// ============================================================
// Accès autorisé → envoyer OPEN puis GREEN à l'Arduino
// ============================================================
void Scenario1::grantAccess()
{
    arduino->write_to_arduino("AUTORISE\n");
    qDebug() << "[Scenario1] >>> ACCÈS AUTORISÉ <<<";
    qDebug() << "[Scenario1] Commande envoyée : AUTORISE";
}

void Scenario1::denyAccess()
{
    arduino->write_to_arduino("REFUSE\n");
    qDebug() << "[Scenario1] >>> ACCÈS REFUSÉ <<<";
    qDebug() << "[Scenario1] Commande envoyée : REFUSE";

}

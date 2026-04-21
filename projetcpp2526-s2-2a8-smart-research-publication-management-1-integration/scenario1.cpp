#include "scenario1.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================================
// Constructeur
// ============================================================================
Scenario1::Scenario1(Arduino* arduino, int id_laboratoire)
    : arduino(arduino), id_lab(id_laboratoire)
{
}

// ============================================================================
// processAccess()
// Appelée depuis MainWindow sur readyRead() du QSerialPort.
//
// Flux :
//   1. Lire la ligne série → extraire l'UID RFID
//   2. Chercher le chercheur par CLR_RFID
//   3. Vérifier projet en cours dans le bon labo
//   4. Détecter si c'est une ENTRÉE ou une SORTIE
//   5. Persister en base et envoyer la commande à l'Arduino
// ============================================================================
void Scenario1::processAccess()
{
    // ── Lire une ligne complète depuis l'Arduino ──────────────────────────
    QString message = arduino->readLine();
    if (message.isEmpty())
        return;  // ligne incomplète, on attend le prochain readyRead

    message.remove(QChar('\0'));
    message = message.trimmed();

    qDebug() << "[Scenario1] Reçu de l'Arduino :" << message;

    // ── Parser le message — format attendu : "RFID:<UID>" ─────────────────
    if (!message.startsWith("RFID:", Qt::CaseInsensitive)) {
        qDebug() << "[Scenario1] Format non reconnu (attendu RFID:<UID>), ignoré.";
        return;
    }

    const QString uid = message.mid(5).trimmed();
    if (uid.isEmpty()) {
        qDebug() << "[Scenario1] UID vide, accès refusé.";
        denyAccess();
        return;
    }

    // ── Étape 1 : identifier le chercheur par CLR_RFID ───────────────────
    const int id_chercheur = getChercheurIdByRfid(uid);
    if (id_chercheur == -1) {
        qDebug() << "[Scenario1] Aucun chercheur pour RFID :" << uid;
        denyAccess();
        return;
    }

    // ── Étape 2 : récupérer nom et prénom ────────────────────────────────
    QString nom, prenom;
    if (!getChercheurInfo(id_chercheur, nom, prenom)) {
        qDebug() << "[Scenario1] Impossible de récupérer les infos du chercheur ID :" << id_chercheur;
        denyAccess();
        return;
    }

    // ── Étape 3 : vérifier qu'il a un projet en cours ────────────────────
    if (!checkProjetEnCours(id_chercheur)) {
        qDebug() << "[Scenario1] Aucun projet en cours pour chercheur ID :" << id_chercheur;
        denyAccess();
        return;
    }

    // ── Étape 4 : vérifier que le projet est dans CE labo ────────────────
    if (!checkProjetDansLabo(id_chercheur, id_lab)) {
        qDebug() << "[Scenario1] Projet non affecté au labo ID :" << id_lab;
        denyAccess();
        return;
    }

    // ── Étape 5 : détecter ENTRÉE ou SORTIE ──────────────────────────────
    if (estDansLabo(id_chercheur)) {
        // ─── SORTIE ───────────────────────────────────────────────────────
        qDebug() << "[Scenario1] SORTIE détectée pour" << nom << prenom;

        if (!enregistrerSortie(id_chercheur)) {
            qDebug() << "[Scenario1] Erreur lors de l'enregistrement de la sortie !";
            // On autorise quand même la sortie physique
        }

        m_lastGranted   = true;
        m_lastIsEntree  = false;
        m_lastNomPrenom = nom + " " + prenom;

        grantExit(nom, prenom);

    } else {
        // ─── ENTRÉE ───────────────────────────────────────────────────────
        qDebug() << "[Scenario1] ENTRÉE détectée pour" << nom << prenom;

        if (!enregistrerEntree(id_chercheur)) {
            qDebug() << "[Scenario1] Erreur lors de l'enregistrement de l'entrée !";
        }

        m_lastGranted   = true;
        m_lastIsEntree  = true;
        m_lastNomPrenom = nom + " " + prenom;

        grantEntry(nom, prenom);
    }
}

// ============================================================================
// Étape 1 : chercher le chercheur par CLR_RFID
// ============================================================================
int Scenario1::getChercheurIdByRfid(const QString& rfid)
{
    QSqlQuery query;
    query.prepare(
        "SELECT ID_CHERCHEUR "
        "FROM CHERCHEUR "
        "WHERE CLR_RFID = :rfid"
        );
    query.bindValue(":rfid", rfid);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL getChercheurIdByRfid :" << query.lastError().text();
        return -1;
    }

    if (query.next())
        return query.value(0).toInt();

    return -1;
}

// ============================================================================
// Étape 2 : récupérer nom et prénom du chercheur
// ============================================================================
bool Scenario1::getChercheurInfo(int id_chercheur, QString& nom, QString& prenom)
{
    QSqlQuery query;
    query.prepare(
        "SELECT NOM, PRENOM "
        "FROM CHERCHEUR "
        "WHERE ID_CHERCHEUR = :id"
        );
    query.bindValue(":id", id_chercheur);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL getChercheurInfo :" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        nom    = query.value(0).toString();
        prenom = query.value(1).toString();
        return true;
    }

    return false;
}

// ============================================================================
// Étape 3 : vérifier qu'il a au moins un projet en cours
// ============================================================================
bool Scenario1::checkProjetEnCours(int id_chercheur)
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) "
        "FROM CONTRIBUER C "
        "JOIN PROJET P ON C.ID_PROJET = P.ID_PROJET "
        "WHERE C.ID_CHERCHEUR = :id "
        "  AND P.ETAT = 'en_cours'"
        );
    query.bindValue(":id", id_chercheur);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL checkProjetEnCours :" << query.lastError().text();
        return false;
    }

    if (query.next())
        return query.value(0).toInt() > 0;

    return false;
}

// ============================================================================
// Étape 4 : vérifier que le projet est bien dans CE labo
// ============================================================================
bool Scenario1::checkProjetDansLabo(int id_chercheur, int id_lab)
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) "
        "FROM LABORATOIRE L "
        "JOIN PROJET P      ON L.ID_PROJET = P.ID_PROJET "
        "JOIN CONTRIBUER C  ON C.ID_PROJET = P.ID_PROJET "
        "WHERE C.ID_CHERCHEUR = :id "
        "  AND L.ID_LABORATOIRE = :id_lab"
        );
    query.bindValue(":id",     id_chercheur);
    query.bindValue(":id_lab", id_lab);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL checkProjetDansLabo :" << query.lastError().text();
        return false;
    }

    if (query.next())
        return query.value(0).toInt() > 0;

    return false;
}

// ============================================================================
// Détection ENTRÉE / SORTIE
// DATE_ENTREE_LAB IS NOT NULL → le chercheur est déjà dans le labo (SORTIE)
// DATE_ENTREE_LAB IS NULL     → le chercheur n'est pas dans le labo (ENTRÉE)
// ============================================================================
bool Scenario1::estDansLabo(int id_chercheur)
{
    QSqlQuery query;
    query.prepare(
        "SELECT DATE_ENTREE_LAB "
        "FROM CHERCHEUR "
        "WHERE ID_CHERCHEUR = :id"
        );
    query.bindValue(":id", id_chercheur);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL estDansLabo :" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        // isNull() retourne true si la valeur est NULL en base
        return !query.value(0).isNull();
    }

    return false;
}

// ============================================================================
// Persistance ENTRÉE : enregistrer DATE_ENTREE_LAB = maintenant
//
// CORRECTION : utilisation de SYSDATE (type DATE Oracle) au lieu de
// SYSTIMESTAMP pour éviter les problèmes de soustraction de TIMESTAMP.
// SYSDATE supporte la soustraction directe et retourne un résultat en jours.
// ============================================================================
bool Scenario1::enregistrerEntree(int id_chercheur)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE CHERCHEUR "
        "SET DATE_ENTREE_LAB = SYSDATE "
        "WHERE ID_CHERCHEUR = :id"
        );
    query.bindValue(":id", id_chercheur);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL enregistrerEntree :" << query.lastError().text();
        return false;
    }

    qDebug() << "[Scenario1] DATE_ENTREE_LAB enregistrée pour chercheur ID :" << id_chercheur;
    return true;
}

// ============================================================================
// Persistance SORTIE :
//   1. Calculer durée = SYSDATE - DATE_ENTREE_LAB (en jours → * 86400 → secondes)
//   2. Incrémenter TEMPS_TRAVAIL_PROJET de cette durée
//   3. Remettre DATE_ENTREE_LAB à NULL
//
// CORRECTIONS apportées :
//   - Utilisation de SYSDATE (type DATE) au lieu de SYSTIMESTAMP (type TIMESTAMP).
//     La soustraction de deux DATE en Oracle donne directement un NUMBER en jours.
//     La soustraction de deux TIMESTAMP donne un INTERVAL, qui nécessite
//     EXTRACT() et ne peut pas être multiplié directement par 86400.
//   - Séparation en deux requêtes distinctes pour contourner un bug Oracle/Qt
//     où une seule requête UPDATE avec deux colonnes modifiées et une expression
//     arithmétique peut échouer silencieusement selon le driver ODBC utilisé.
//   - La colonne DATE_ENTREE_LAB est déclarée TIMESTAMP en base mais SYSDATE
//     (type DATE) peut y être stocké ; Oracle convertit automatiquement DATE→TIMESTAMP.
//     La soustraction SYSDATE - DATE_ENTREE_LAB reste valide car Oracle cast
//     le TIMESTAMP en DATE pour l'opération si DATE_ENTREE_LAB a été alimenté
//     par SYSDATE. Pour garantir la cohérence, on force le cast explicite avec TO_DATE.
// ============================================================================
bool Scenario1::enregistrerSortie(int id_chercheur)
{
    // ── Requête 1 : incrémenter TEMPS_TRAVAIL_PROJET ──────────────────────
    // On calcule la durée en secondes : (SYSDATE - DATE_ENTREE_LAB) * 86400
    // SYSDATE - DATE retourne un NUMBER (jours décimaux) en Oracle.
    // ROUND() pour obtenir un entier de secondes.
    // NVL() pour partir de 0 si TEMPS_TRAVAIL_PROJET était NULL.
    // CAST(DATE_ENTREE_LAB AS DATE) garantit que la soustraction est numérique.
    {
        QSqlQuery q1;
        q1.prepare(
            "UPDATE CHERCHEUR "
            "SET TEMPS_TRAVAIL_PROJET = NVL(TEMPS_TRAVAIL_PROJET, 0) "
            "    + ROUND((SYSDATE - CAST(DATE_ENTREE_LAB AS DATE)) * 86400) "
            "WHERE ID_CHERCHEUR = :id "
            "  AND DATE_ENTREE_LAB IS NOT NULL"
            );
        q1.bindValue(":id", id_chercheur);

        if (!q1.exec()) {
            qDebug() << "[Scenario1] Erreur SQL enregistrerSortie (TEMPS_TRAVAIL) :"
                     << q1.lastError().text();
            return false;
        }

        if (q1.numRowsAffected() == 0) {
            qDebug() << "[Scenario1] Aucune ligne affectée pour TEMPS_TRAVAIL — DATE_ENTREE_LAB était NULL ?";
            return false;
        }
    }

    // ── Requête 2 : remettre DATE_ENTREE_LAB à NULL ───────────────────────
    {
        QSqlQuery q2;
        q2.prepare(
            "UPDATE CHERCHEUR "
            "SET DATE_ENTREE_LAB = NULL "
            "WHERE ID_CHERCHEUR = :id"
            );
        q2.bindValue(":id", id_chercheur);

        if (!q2.exec()) {
            qDebug() << "[Scenario1] Erreur SQL enregistrerSortie (DATE_ENTREE_LAB = NULL) :"
                     << q2.lastError().text();
            return false;
        }
    }

    qDebug() << "[Scenario1] Sortie enregistrée avec succès pour chercheur ID :" << id_chercheur;
    return true;
}

// ============================================================================
// Accès autorisé — ENTRÉE
// Envoie "ENTREE:<Nom Prenom>" à l'Arduino
// ============================================================================
void Scenario1::grantEntry(const QString& nom, const QString& prenom)
{
    const QString nomPrenom = (nom + " " + prenom).left(32)
    .replace('\n', ' ').replace('\r', ' ');
    arduino->write_to_arduino(QString("ENTREE:%1\n").arg(nomPrenom).toUtf8());
    qDebug() << "[Scenario1] >>> ENTREE AUTORISÉE —" << nomPrenom;
}

// ============================================================================
// Accès autorisé — SORTIE
// Envoie "SORTIE:<Nom Prenom>" à l'Arduino
// ============================================================================
void Scenario1::grantExit(const QString& nom, const QString& prenom)
{
    const QString nomPrenom = (nom + " " + prenom).left(32)
    .replace('\n', ' ').replace('\r', ' ');
    arduino->write_to_arduino(QString("SORTIE:%1\n").arg(nomPrenom).toUtf8());
    qDebug() << "[Scenario1] >>> SORTIE AUTORISÉE —" << nomPrenom;
}

// ============================================================================
// Accès refusé — Envoie "REFUSE" à l'Arduino
// ============================================================================
void Scenario1::denyAccess()
{
    m_lastGranted   = false;
    m_lastNomPrenom = "";
    arduino->write_to_arduino("REFUSE\n");
    qDebug() << "[Scenario1] >>> ACCÈS REFUSÉ <<<";
}

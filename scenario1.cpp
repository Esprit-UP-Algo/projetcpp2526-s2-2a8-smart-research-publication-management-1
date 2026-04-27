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
// processMessage()
// Point d'entrée unique — appelé depuis readyRead() dans smartpub.cpp.
//
// Lit UNE ligne du buffer série Arduino et dispatche selon le préfixe :
//   "RFID:<uid>"    → handleRfid() — flux accès chercheur complet
//   "PORTE_OUVERTE" → mettreAJourEtatPorte(true)  — servo ouvert
//   "PORTE_FERMEE"  → mettreAJourEtatPorte(false) — servo fermé
//   autres          → ignoré (messages debug Arduino, etc.)
//
// La garde m_enTraitement empêche la réentrance si readyRead() est
// émis pendant le traitement d'une requête SQL (cas rare mais possible
// avec certains drivers Qt sur Windows).
// ============================================================================
void Scenario1::processMessage()
{
    // ── Garde anti-réentrance ─────────────────────────────────────────
    if (m_enTraitement)
        return;
    m_enTraitement = true;

    // ── Lire une ligne complète depuis l'Arduino ──────────────────────
    QString message = arduino->readLine();
    if (message.isEmpty()) {
        m_enTraitement = false;
        return;
    }

    message.remove(QChar('\0'));
    message = message.trimmed();

    qDebug() << "[Scenario1] Reçu de l'Arduino :" << message;

    // ── Dispatch ──────────────────────────────────────────────────────
    if (message.startsWith("RFID:", Qt::CaseInsensitive)) {
        // Trame RFID — flux accès chercheur
        const QString uid = message.mid(5).trimmed();
        if (uid.isEmpty()) {
            qDebug() << "[Scenario1] UID vide, accès refusé.";
            denyAccess();
        } else {
            handleRfid(uid);
        }

    } else if (message == "PORTE_OUVERTE") {
        // Servo SG90 vient d'ouvrir la porte
        qDebug() << "[Scenario1] Porte ouverte — mise à jour ETAT_PORTE = 1 pour labo ID :" << id_lab;
        if (!mettreAJourEtatPorte(true)) {
            qDebug() << "[Scenario1] Erreur mise à jour ETAT_PORTE (ouverture) !";
        }

    } else if (message == "PORTE_FERMEE") {
        // Servo SG90 vient de fermer la porte
        qDebug() << "[Scenario1] Porte fermée — mise à jour ETAT_PORTE = 0 pour labo ID :" << id_lab;
        if (!mettreAJourEtatPorte(false)) {
            qDebug() << "[Scenario1] Erreur mise à jour ETAT_PORTE (fermeture) !";
        }

    } else {
        qDebug() << "[Scenario1] Message non reconnu, ignoré :" << message;
    }

    m_enTraitement = false;
}

// ============================================================================
// handleRfid()
// Traite un UID RFID extrait par processMessage().
//
// Flux :
//   1. Identifier le chercheur par CLR_RFID
//   2. Récupérer nom et prénom
//   3. Vérifier projet en cours
//   4. Vérifier que le projet est dans CE labo
//   5. Détecter ENTRÉE ou SORTIE
//   6. Persister en base et envoyer la commande à l'Arduino
//      (l'Arduino actionne ensuite le servo et le buzzer)
// ============================================================================
void Scenario1::handleRfid(const QString& uid)
{
    // ── Étape 1 : identifier le chercheur par CLR_RFID ───────────────
    const int id_chercheur = getChercheurIdByRfid(uid);
    if (id_chercheur == -1) {
        qDebug() << "[Scenario1] Aucun chercheur pour RFID :" << uid;
        denyAccess();
        return;
    }

    // ── Étape 2 : récupérer nom et prénom ────────────────────────────
    QString nom, prenom;
    if (!getChercheurInfo(id_chercheur, nom, prenom)) {
        qDebug() << "[Scenario1] Impossible de récupérer les infos du chercheur ID :" << id_chercheur;
        denyAccess();
        return;
    }

    // ── Étape 3 : vérifier qu'il a un projet en cours ────────────────
    if (!checkProjetEnCours(id_chercheur)) {
        qDebug() << "[Scenario1] Aucun projet en cours pour chercheur ID :" << id_chercheur;
        denyAccess();
        return;
    }

    // ── Étape 4 : vérifier que le projet est dans CE labo ────────────
    if (!checkProjetDansLabo(id_chercheur, id_lab)) {
        qDebug() << "[Scenario1] Projet non affecté au labo ID :" << id_lab;
        denyAccess();
        return;
    }

    // ── Étape 5 : détecter ENTRÉE ou SORTIE ──────────────────────────
    if (estDansLabo(id_chercheur)) {
        // ─── SORTIE ───────────────────────────────────────────────────
        qDebug() << "[Scenario1] SORTIE détectée pour" << nom << prenom;

        if (!enregistrerSortie(id_chercheur)) {
            qDebug() << "[Scenario1] Erreur lors de l'enregistrement de la sortie !";
            // On autorise quand même la sortie physique
        }

        m_lastGranted   = true;
        m_lastIsEntree  = false;
        m_lastNomPrenom = nom + " " + prenom;

        // Envoie "SORTIE:<nom>" → Arduino ouvre servo + 2 bips verts
        grantExit(nom, prenom);

    } else {
        // ─── ENTRÉE ───────────────────────────────────────────────────
        qDebug() << "[Scenario1] ENTRÉE détectée pour" << nom << prenom;

        if (!enregistrerEntree(id_chercheur)) {
            qDebug() << "[Scenario1] Erreur lors de l'enregistrement de l'entrée !";
        }

        m_lastGranted   = true;
        m_lastIsEntree  = true;
        m_lastNomPrenom = nom + " " + prenom;

        // Envoie "ENTREE:<nom>" → Arduino ouvre servo + 2 bips verts
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
        return !query.value(0).isNull();
    }

    return false;
}

// ============================================================================
// Persistance ENTRÉE : enregistrer DATE_ENTREE_LAB = maintenant
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
// ============================================================================
bool Scenario1::enregistrerSortie(int id_chercheur)
{
    // ── Requête 1 : incrémenter TEMPS_TRAVAIL_PROJET ──────────────────
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

    // ── Requête 2 : remettre DATE_ENTREE_LAB à NULL ───────────────────
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
// mettreAJourEtatPorte()
// Met à jour la colonne ETAT_PORTE de la table LABORATOIRE.
//   etatOuvert = true  → ETAT_PORTE = 1 (porte ouverte)
//   etatOuvert = false → ETAT_PORTE = 0 (porte fermée)
//
// Appelée depuis processMessage() quand l'Arduino envoie "PORTE_OUVERTE"
// ou "PORTE_FERMEE" après actionnement du servo SG90.
// ============================================================================
bool Scenario1::mettreAJourEtatPorte(bool etatOuvert)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE LABORATOIRE "
        "SET ETAT_PORTE = :etat "
        "WHERE ID_LABORATOIRE = :id_lab"
        );
    query.bindValue(":etat",   etatOuvert ? 1 : 0);
    query.bindValue(":id_lab", id_lab);

    if (!query.exec()) {
        qDebug() << "[Scenario1] Erreur SQL mettreAJourEtatPorte :" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "[Scenario1] Aucune ligne affectée pour ETAT_PORTE — ID_LABORATOIRE :" << id_lab;
        return false;
    }

    qDebug() << "[Scenario1] ETAT_PORTE mis à jour :"
             << (etatOuvert ? "OUVERT (1)" : "FERME (0)")
             << "pour labo ID :" << id_lab;
    return true;
}

// ============================================================================
// Accès autorisé — ENTRÉE
// Envoie "ENTREE:<Nom Prenom>" à l'Arduino
// L'Arduino actionne le servo (ouverture porte) + 2 bips courts
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
// L'Arduino actionne le servo (ouverture porte) + 2 bips courts
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
// L'Arduino allume la LED rouge + 1 bip long, porte reste fermée
// ============================================================================
void Scenario1::denyAccess()
{
    m_lastGranted   = false;
    m_lastNomPrenom = "";
    arduino->write_to_arduino("REFUSE\n");
    qDebug() << "[Scenario1] >>> ACCÈS REFUSÉ <<<";
}

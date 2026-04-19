#include "demi_scenario2.h"
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>

// ============================================================================
// Constructeur
// ============================================================================
DemiScenario2::DemiScenario2(Arduino* arduino)
    : m_arduino(arduino)
{
}

// ============================================================================
// formaterPourLed()
// Nettoie le texte : supprime retours à la ligne, tronque à maxLen caractères
// ============================================================================
QString DemiScenario2::formaterPourLed(const QString& texte, int maxLen)
{
    QString s = texte;
    s.replace('\n', ' ').replace('\r', ' ').replace('\0', ' ');
    s = s.simplified();
    if (s.length() > maxLen)
        s = s.left(maxLen - 1) + ".";
    return s;
}

// ============================================================================
// envoyerMessage()
// Envoie "MSG:<texte>\n" à l'Arduino via le port série
// ============================================================================
void DemiScenario2::envoyerMessage(const QString& msg)
{
    if (!m_arduino) return;
    const QString ligne = QString("MSG:%1\n").arg(formaterPourLed(msg));
    m_arduino->write_to_arduino(ligne.toUtf8());
    qDebug() << "[DemiScenario2] >>>" << ligne.trimmed();
}

// ============================================================================
// envoyerListe()
// Envoie chaque message de la liste avec un délai entre chaque
// (l'Arduino gère le timing d'affichage côté .ino)
// ============================================================================
void DemiScenario2::envoyerListe(const QStringList& messages)
{
    for (const QString& msg : messages) {
        envoyerMessage(msg);
    }
}

// ============================================================================
// afficherEvenementsSemaine()
// Table : EVENEMENT — colonnes : NOM, LIEU, DATE_EVENEMENT
// Filtre : événements de la semaine courante (7 prochains jours)
// ============================================================================
QStringList DemiScenario2::afficherEvenementsSemaine()
{
    QStringList messages;
    messages << "=== EVENEMENTS ===";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Evenement: N/A";
        qDebug() << "[DemiScenario2] BD non connectee (evenements)";
        return messages;
    }

    QSqlQuery q(db);
    // Événements des 7 prochains jours
    const QString sql =
        "SELECT NOM, LIEU, DATE_EVENEMENT "
        "FROM EVENEMENT "
        "WHERE DATE_EVENEMENT >= TRUNC(SYSDATE) "
        "  AND DATE_EVENEMENT <  TRUNC(SYSDATE) + 7 "
        "ORDER BY DATE_EVENEMENT "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL evenements :" << q.lastError().text();
        messages << "Evenement: Erreur BD";
        return messages;
    }

    bool found = false;
    while (q.next()) {
        found = true;
        const QString nom  = q.value("NOM").toString().trimmed();
        const QString lieu = q.value("LIEU").toString().trimmed();
        QDate date = q.value("DATE_EVENEMENT").toDate();
        if (!date.isValid()) {
            QDateTime dt = q.value("DATE_EVENEMENT").toDateTime();
            date = dt.date();
        }
        const QString jourSemaine = date.isValid()
            ? date.toString("ddd dd/MM")
            : "";
        const QString msg = QString("Evt: %1 - %2%3")
            .arg(nom)
            .arg(lieu)
            .arg(jourSemaine.isEmpty() ? "" : " (" + jourSemaine + ")");
        messages << msg;
    }

    if (!found)
        messages << "Evenement: Aucun cette semaine";

    return messages;
}

// ============================================================================
// afficherProjetsSemaine()
// Table : PROJET — colonnes : TITRE, ETAT, CODE
// Filtre : projets en cours (ETAT = 'en_cours')
// ============================================================================
QStringList DemiScenario2::afficherProjetsSemaine()
{
    QStringList messages;
    messages << "=== PROJETS ===";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Projet: N/A";
        return messages;
    }

    QSqlQuery q(db);
    const QString sql =
        "SELECT CODE, TITRE, ETAT "
        "FROM PROJET "
        "WHERE ETAT = 'en_cours' "
        "ORDER BY CODE "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL projets :" << q.lastError().text();
        messages << "Projet: Erreur BD";
        return messages;
    }

    bool found = false;
    while (q.next()) {
        found = true;
        const QString code  = q.value("CODE").toString().trimmed();
        const QString titre = q.value("TITRE").toString().trimmed();
        const QString msg   = QString("Projet: %1 - En cours").arg(
            titre.isEmpty() ? code : titre);
        messages << msg;
    }

    if (!found)
        messages << "Projet: Aucun en cours";

    return messages;
}

// ============================================================================
// afficherFinanceSemaine()
// Table : FINANCE — colonnes : TYPE_TRANS, MONTANT, STATUT, DATE_TRANSACTION
// Filtre : transactions récentes (7 derniers jours)
// ============================================================================
QStringList DemiScenario2::afficherFinanceSemaine()
{
    QStringList messages;
    messages << "=== FINANCES ===";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Finance: N/A";
        return messages;
    }

    QSqlQuery q(db);
    const QString sql =
        "SELECT TYPE_TRANS, MONTANT, STATUT "
        "FROM FINANCE "
        "WHERE DATE_TRANSACTION >= TRUNC(SYSDATE) - 7 "
        "ORDER BY DATE_TRANSACTION DESC "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL finances :" << q.lastError().text();
        messages << "Finance: Erreur BD";
        return messages;
    }

    bool found = false;
    while (q.next()) {
        found = true;
        const QString type   = q.value("TYPE_TRANS").toString().trimmed();
        const double montant = q.value("MONTANT").toDouble();
        const QString statut = q.value("STATUT").toString().trimmed();
        const QString msg    = QString("Finance: %1 %2 TND - %3")
            .arg(type)
            .arg(QString::number(montant, 'f', 0))
            .arg(statut.isEmpty() ? "En attente" : statut);
        messages << msg;
    }

    if (!found)
        messages << "Finance: Aucune operation recente";

    return messages;
}

// ============================================================================
// afficherLaboratoireSemaine()
// Table : LABORATOIRE — colonnes : NOM, THEMATIQUE, DISPONIBILITE
// Filtre : laboratoires actifs (DISPONIBILITE = 'disponible')
// ============================================================================
QStringList DemiScenario2::afficherLaboratoireSemaine()
{
    QStringList messages;
    messages << "=== LABORATOIRES ===";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Labo: N/A";
        return messages;
    }

    QSqlQuery q(db);
    const QString sql =
        "SELECT NOM, THEMATIQUE, DISPONIBILITE "
        "FROM LABORATOIRE "
        "WHERE DISPONIBILITE = 'disponible' "
        "ORDER BY NOM "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL laboratoires :" << q.lastError().text();
        messages << "Labo: Erreur BD";
        return messages;
    }

    bool found = false;
    while (q.next()) {
        found = true;
        const QString nom       = q.value("NOM").toString().trimmed();
        const QString thematique = q.value("THEMATIQUE").toString().trimmed();
        const QString msg = QString("Labo: %1 - %2")
            .arg(nom)
            .arg(thematique.isEmpty() ? "Actif" : thematique);
        messages << msg;
    }

    if (!found)
        messages << "Labo: Aucun disponible";

    return messages;
}

// ============================================================================
// afficherPublicationsSemaine()
// Table : PUBLICATION — colonnes : TITRE, AUTEUR, DATE_PUBLICATION, STATUT
// Filtre : publications récentes (30 derniers jours) ou statut 'publie'
// ============================================================================
QStringList DemiScenario2::afficherPublicationsSemaine()
{
    QStringList messages;
    messages << "=== PUBLICATIONS ===";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Pub: N/A";
        return messages;
    }

    QSqlQuery q(db);
    const QString sql =
        "SELECT TITRE, AUTEUR, STATUT "
        "FROM PUBLICATION "
        "WHERE DATE_PUBLICATION >= TRUNC(SYSDATE) - 30 "
        "   OR LOWER(STATUT) = 'publie' "
        "ORDER BY DATE_PUBLICATION DESC "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL publications :" << q.lastError().text();
        messages << "Pub: Erreur BD";
        return messages;
    }

    bool found = false;
    while (q.next()) {
        found = true;
        const QString titre  = q.value("TITRE").toString().trimmed();
        const QString auteur = q.value("AUTEUR").toString().trimmed();
        const QString msg    = QString("Pub: %1%2")
            .arg(titre.isEmpty() ? "Sans titre" : titre)
            .arg(auteur.isEmpty() ? "" : " - " + auteur);
        messages << msg;
    }

    if (!found)
        messages << "Pub: Aucune publication recente";

    return messages;
}

// ============================================================================
// afficherChercheursDisponibles()
// Table : CHERCHEUR — colonnes : NOM, PRENOM, GRADE, DATE_ENTREE_LAB
// Filtre : DATE_ENTREE_LAB IS NULL → chercheur non présent dans un labo
// ============================================================================
QStringList DemiScenario2::afficherChercheursDisponibles()
{
    QStringList messages;
    messages << "=== CHERCHEURS ===";

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Chercheur: N/A";
        return messages;
    }

    QSqlQuery q(db);
    // DATE_ENTREE_LAB IS NULL = chercheur disponible (pas dans un labo en ce moment)
    const QString sql =
        "SELECT NOM, PRENOM, GRADE "
        "FROM CHERCHEUR "
        "WHERE DATE_ENTREE_LAB IS NULL "
        "ORDER BY NOM, PRENOM "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL chercheurs :" << q.lastError().text();
        messages << "Chercheur: Erreur BD";
        return messages;
    }

    bool found = false;
    while (q.next()) {
        found = true;
        const QString nom    = q.value("NOM").toString().trimmed();
        const QString prenom = q.value("PRENOM").toString().trimmed();
        const QString grade  = q.value("GRADE").toString().trimmed();
        const QString msg    = QString("Dr %1 %2 disponible%3")
            .arg(nom)
            .arg(prenom)
            .arg(grade.isEmpty() ? "" : " (" + grade + ")");
        messages << msg;
    }

    if (!found)
        messages << "Chercheur: Tous en labo";

    return messages;
}

// ============================================================================
// afficherProgrammeGlobal()
// Agrège tous les modules et envoie la séquence complète à l'Arduino
// ============================================================================
void DemiScenario2::afficherProgrammeGlobal()
{
    if (!m_arduino) {
        qDebug() << "[DemiScenario2] Arduino non disponible";
        return;
    }

    qDebug() << "[DemiScenario2] Debut affichage programme global";

    // Effacer l'écran avant de commencer
    m_arduino->write_to_arduino("CLEAR\n");

    // Collecter tous les messages par module
    QStringList tousMessages;
    tousMessages << afficherEvenementsSemaine();
    tousMessages << afficherProjetsSemaine();
    tousMessages << afficherFinanceSemaine();
    tousMessages << afficherLaboratoireSemaine();
    tousMessages << afficherPublicationsSemaine();
    tousMessages << afficherChercheursDisponibles();

    // Envoyer la séquence complète à l'Arduino
    envoyerListe(tousMessages);

    // Signal de fin de séquence
    m_arduino->write_to_arduino("DONE\n");

    qDebug() << "[DemiScenario2] Fin affichage programme global —"
             << tousMessages.size() << "messages envoyes";
}

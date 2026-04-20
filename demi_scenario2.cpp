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
    s.replace('|', '-'); // le pipe est utilisé comme séparateur de couleur
    s = s.simplified();
    if (s.length() > maxLen)
        s = s.left(maxLen - 1) + ".";
    return s;
}

// ============================================================================
// envoyerMessage()
// Envoie "MSG:<texte>|<couleur>\n" à l'Arduino via le port série
// Couleurs supportées : RED, GREEN, BLUE, YELLOW, CYAN, WHITE
// ============================================================================
void DemiScenario2::envoyerMessage(const QString& texte, const QString& couleur)
{
    if (!m_arduino) return;
    const QString ligne = QString("MSG:%1|%2\n")
                              .arg(formaterPourLed(texte))
                              .arg(couleur.toUpper());
    m_arduino->write_to_arduino(ligne.toUtf8());
    qDebug() << "[DemiScenario2] >>>" << ligne.trimmed();
}

// ============================================================================
// envoyerListe()
// Chaque élément est au format "<texte>|<couleur>"
// ============================================================================
void DemiScenario2::envoyerListe(const QStringList& messages)
{
    for (const QString& item : messages) {
        // Séparer texte et couleur
        const int sep = item.lastIndexOf('|');
        if (sep > 0) {
            envoyerMessage(item.left(sep), item.mid(sep + 1));
        } else {
            envoyerMessage(item, "WHITE");
        }
    }
}

// ============================================================================
// processInput()
// Lit les messages envoyés par l'Arduino vers Qt (sens Input)
// Format attendu : "BTN:AFFICHER\n" ou "READY\n"
// À appeler depuis le slot readyRead() dans SmartPub
// ============================================================================
void DemiScenario2::processInput()
{
    if (!m_arduino) return;

    const QString message = m_arduino->readLine();
    if (message.isEmpty()) return;

    qDebug() << "[DemiScenario2] Recu de l'Arduino :" << message;

    if (message.trimmed() == "BTN:AFFICHER") {
        // Le bouton sur l'Arduino a été pressé → lancer l'affichage complet
        qDebug() << "[DemiScenario2] Bouton presse -> affichage programme global";
        afficherProgrammeGlobal();
    }
    else if (message.trimmed() == "READY") {
        qDebug() << "[DemiScenario2] Panneau LED pret";
    }
}

// ============================================================================
// afficherEvenementsSemaine()
// Table : EVENEMENT — NOM, LIEU, DATE_EVENEMENT
// Couleur : CYAN
// Filtre  : événements des 7 prochains jours
// ============================================================================
QStringList DemiScenario2::afficherEvenementsSemaine()
{
    QStringList messages;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Evenement: Base non connectee|RED";
        return messages;
    }

    QSqlQuery q(db);
    const QString sql =
        "SELECT NOM, LIEU, DATE_EVENEMENT "
        "FROM EVENEMENT "
        "WHERE DATE_EVENEMENT >= TRUNC(SYSDATE) "
        "  AND DATE_EVENEMENT <  TRUNC(SYSDATE) + 7 "
        "ORDER BY DATE_EVENEMENT "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL evenements :" << q.lastError().text();
        messages << "Evenement: Erreur BD|RED";
        return messages;
    }

    // En-tête de section — couleur CYAN
    messages << "=== EVENEMENTS DE LA SEMAINE ===|CYAN";

    bool found = false;
    while (q.next()) {
        found = true;
        const QString nom  = q.value("NOM").toString().trimmed();
        const QString lieu = q.value("LIEU").toString().trimmed();
        QDate date = q.value("DATE_EVENEMENT").toDate();
        if (!date.isValid())
            date = q.value("DATE_EVENEMENT").toDateTime().date();

        const QString dateStr = date.isValid() ? date.toString("ddd dd/MM") : "";
        const QString msg = QString("Evt: %1 - %2%3")
                                .arg(nom)
                                .arg(lieu)
                                .arg(dateStr.isEmpty() ? "" : " (" + dateStr + ")");
        messages << (msg + "|CYAN");
    }

    if (!found)
        messages << "Evenement: Aucun cette semaine|CYAN";

    return messages;
}

// ============================================================================
// afficherProjetsSemaine()
// Table : PROJET — CODE, TITRE, ETAT
// Couleur : GREEN
// Filtre  : projets en cours (ETAT = 'en_cours')
// ============================================================================
QStringList DemiScenario2::afficherProjetsSemaine()
{
    QStringList messages;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Projet: Base non connectee|RED";
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
        messages << "Projet: Erreur BD|RED";
        return messages;
    }

    messages << "=== PROJETS EN COURS ===|GREEN";

    bool found = false;
    while (q.next()) {
        found = true;
        const QString code  = q.value("CODE").toString().trimmed();
        const QString titre = q.value("TITRE").toString().trimmed();
        const QString msg   = QString("Projet: %1 - En cours")
                                .arg(titre.isEmpty() ? code : titre);
        messages << (msg + "|GREEN");
    }

    if (!found)
        messages << "Projet: Aucun en cours|GREEN";

    return messages;
}

// ============================================================================
// afficherFinanceSemaine()
// Table : FINANCE — TYPE_TRANS, MONTANT, STATUT, DATE_TRANSACTION
// Couleur : YELLOW
// Filtre  : transactions des 7 derniers jours
// ============================================================================
QStringList DemiScenario2::afficherFinanceSemaine()
{
    QStringList messages;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Finance: Base non connectee|RED";
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
        messages << "Finance: Erreur BD|RED";
        return messages;
    }

    messages << "=== FINANCES RECENTES ===|YELLOW";

    bool found = false;
    while (q.next()) {
        found = true;
        const QString type   = q.value("TYPE_TRANS").toString().trimmed();
        const double  montant = q.value("MONTANT").toDouble();
        const QString statut = q.value("STATUT").toString().trimmed();
        const QString msg    = QString("Finance: %1 %2 TND - %3")
                                .arg(type)
                                .arg(QString::number(montant, 'f', 0))
                                .arg(statut.isEmpty() ? "En attente" : statut);
        messages << (msg + "|YELLOW");
    }

    if (!found)
        messages << "Finance: Aucune operation recente|YELLOW";

    return messages;
}

// ============================================================================
// afficherLaboratoireSemaine()
// Table : LABORATOIRE — NOM, THEMATIQUE, DISPONIBILITE
// Couleur : BLUE
// Filtre  : laboratoires disponibles
// ============================================================================
QStringList DemiScenario2::afficherLaboratoireSemaine()
{
    QStringList messages;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Labo: Base non connectee|RED";
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
        messages << "Labo: Erreur BD|RED";
        return messages;
    }

    messages << "=== LABORATOIRES DISPONIBLES ===|BLUE";

    bool found = false;
    while (q.next()) {
        found = true;
        const QString nom        = q.value("NOM").toString().trimmed();
        const QString thematique = q.value("THEMATIQUE").toString().trimmed();
        const QString msg = QString("Labo: %1 - %2")
                                .arg(nom)
                                .arg(thematique.isEmpty() ? "Actif" : thematique);
        messages << (msg + "|BLUE");
    }

    if (!found)
        messages << "Labo: Aucun disponible|BLUE";

    return messages;
}

// ============================================================================
// afficherPublicationsSemaine()
// Table : PUBLICATION — TITRE, AUTEUR, DATE_PUBLICATION, STATUT
// Couleur : WHITE
// Filtre  : publications récentes (30 jours) ou statut 'publie'
// ============================================================================
QStringList DemiScenario2::afficherPublicationsSemaine()
{
    QStringList messages;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Pub: Base non connectee|RED";
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
        messages << "Pub: Erreur BD|RED";
        return messages;
    }

    messages << "=== PUBLICATIONS RECENTES ===|WHITE";

    bool found = false;
    while (q.next()) {
        found = true;
        const QString titre  = q.value("TITRE").toString().trimmed();
        const QString auteur = q.value("AUTEUR").toString().trimmed();
        const QString msg    = QString("Pub: %1%2")
                                .arg(titre.isEmpty() ? "Sans titre" : titre)
                                .arg(auteur.isEmpty() ? "" : " - " + auteur);
        messages << (msg + "|WHITE");
    }

    if (!found)
        messages << "Pub: Aucune publication recente|WHITE";

    return messages;
}

// ============================================================================
// afficherChercheursDisponibles()
// Table : CHERCHEUR — NOM, PRENOM, GRADE, DATE_ENTREE_LAB
// Couleur : GREEN
// Filtre  : DATE_ENTREE_LAB IS NULL → chercheur non présent dans un labo
// ============================================================================
QStringList DemiScenario2::afficherChercheursDisponibles()
{
    QStringList messages;

    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        messages << "Chercheur: Base non connectee|RED";
        return messages;
    }

    QSqlQuery q(db);
    const QString sql =
        "SELECT NOM, PRENOM, GRADE "
        "FROM CHERCHEUR "
        "WHERE DATE_ENTREE_LAB IS NULL "
        "ORDER BY NOM, PRENOM "
        "FETCH FIRST 5 ROWS ONLY";

    if (!q.exec(sql)) {
        qDebug() << "[DemiScenario2] Erreur SQL chercheurs :" << q.lastError().text();
        messages << "Chercheur: Erreur BD|RED";
        return messages;
    }

    messages << "=== CHERCHEURS DISPONIBLES ===|GREEN";

    bool found = false;
    while (q.next()) {
        found = true;
        const QString nom    = q.value("NOM").toString().trimmed();
        const QString prenom = q.value("PRENOM").toString().trimmed();
        const QString grade  = q.value("GRADE").toString().trimmed();
        const QString msg    = QString("Dr %1 %2 disponible%3")
                                .arg(nom).arg(prenom)
                                .arg(grade.isEmpty() ? "" : " (" + grade + ")");
        messages << (msg + "|GREEN");
    }

    if (!found)
        messages << "Chercheur: Tous en labo|GREEN";

    return messages;
}

// ============================================================================
// afficherProgrammeGlobal()
// Agrège tous les modules et envoie la séquence complète au panneau LED
// ============================================================================
void DemiScenario2::afficherProgrammeGlobal()
{
    if (!m_arduino) {
        qDebug() << "[DemiScenario2] Arduino non disponible";
        return;
    }

    qDebug() << "[DemiScenario2] Debut affichage programme global";

    // Effacer le panneau
    m_arduino->write_to_arduino("CLEAR\n");

    // Collecter tous les messages par module avec leurs couleurs
    QStringList tousMessages;
    tousMessages << afficherEvenementsSemaine();
    tousMessages << afficherProjetsSemaine();
    tousMessages << afficherFinanceSemaine();
    tousMessages << afficherLaboratoireSemaine();
    tousMessages << afficherPublicationsSemaine();
    tousMessages << afficherChercheursDisponibles();

    // Envoyer la séquence complète au panneau LED
    envoyerListe(tousMessages);

    // Signal de fin de séquence
    m_arduino->write_to_arduino("DONE\n");

    qDebug() << "[DemiScenario2] Fin affichage —"
             << tousMessages.size() << "messages envoyes";
}

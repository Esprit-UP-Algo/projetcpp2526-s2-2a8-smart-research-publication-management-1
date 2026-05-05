// ============================================================================
// scenario2.cpp — Fusion OLED (ex-DemiScenario2) + DHT11 (ex-DemiScenario3)
// ============================================================================

#include "scenario2.h"
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>

// ============================================================================
// Constructeur
// ============================================================================
Scenario2::Scenario2(Arduino* arduino, QObject* parent)
    : QObject(parent)
    , m_arduino(arduino)
    , m_pendingIndex(0)
    , m_relanceEnAttente(false)
    , m_messageInTransit(false)
    , m_readyDebounce(false)
{
    // Timer de relance périodique (60s)
    m_timer = new QTimer(this);
    m_timer->setInterval(60000);
    connect(m_timer, &QTimer::timeout, this, &Scenario2::onTimerTick);

    // Fallback : si READY n'est pas reçu dans les 6s (boot Arduino 3s + marge),
    // on démarre quand même la boucle d'affichage.
    QTimer::singleShot(6000, this, [this]() {
        if (m_pendingMessages.isEmpty() && m_pendingIndex == 0 && !m_relanceEnAttente) {
            qDebug() << "[Scenario2] Fallback Timer -> demarrage boucle auto";
            demarrerBoucleAuto();
        }
    });
}

// ============================================================================
// Partie A — OLED : boucle d'affichage du programme journalier
// ============================================================================

void Scenario2::demarrerBoucleAuto()
{
    // Démarrer le timer périodique (filet de sécurité toutes les 60s)
    if (!m_timer->isActive()) m_timer->start();
    afficherProgrammeGlobal();
}

void Scenario2::onTimerTick()
{
    // Déclenché toutes les 60s comme filet de sécurité :
    // si l'ACK chain s'est bloqué, on repart proprement.
    qDebug() << "[Scenario2] Timer 60s -> verification/relance boucle";
    if (!m_relanceEnAttente && m_pendingMessages.isEmpty() && m_pendingIndex == 0) {
        afficherProgrammeGlobal();
    }
}

// ── refreshDonnees : relance immédiate après une modification en BD ──────────
void Scenario2::refreshDonnees()
{
    qDebug() << "[Scenario2] refreshDonnees() -> relance avec donnees actualisees";
    m_pendingMessages.clear();
    m_pendingIndex     = 0;
    m_messageInTransit = false;
    if (!m_relanceEnAttente && m_arduino)
        m_arduino->write_to_arduino("DONE\n");
    m_relanceEnAttente = true;
    QTimer::singleShot(2000, this, [this]() {
        m_relanceEnAttente = false;
        afficherProgrammeGlobal();
    });
}

void Scenario2::envoyerProchainMessage()
{
    if (!m_arduino) return;

    if (m_pendingIndex >= m_pendingMessages.size()) {
        // Fin de séquence : laisser le dernier message visible 3s de plus
        // puis signaler à l'Arduino et relancer après 5s d'accueil.
        int nbMsg = m_pendingMessages.size();
        m_pendingMessages.clear();
        m_pendingIndex    = 0;
        m_relanceEnAttente = true;
        QTimer::singleShot(3000, this, [this, nbMsg]() {
            if (m_arduino) m_arduino->write_to_arduino("DONE\n");
            qDebug() << "[Scenario2] Fin affichage —" << nbMsg << "messages envoyes";
            QTimer::singleShot(5000, this, [this]() {
                m_relanceEnAttente = false;
                qDebug() << "[Scenario2] Relance boucle affichage";
                afficherProgrammeGlobal();
            });
        });
        return;
    }

    const QString& item   = m_pendingMessages[m_pendingIndex];
    const int      sep    = item.lastIndexOf('|');
    const QString  texte  = (sep > 0) ? item.left(sep) : item;
    const QString  couleur = (sep > 0) ? item.mid(sep + 1) : "WHITE";

    envoyerMessage(texte, couleur);
    // L'ACK de l'Arduino sera reçu dans processInput()
}

QString Scenario2::formaterPourOled(const QString& texte, int maxLen)
{
    QString s = texte;
    s.replace('\n', ' ').replace('\r', ' ').replace('\0', ' ');
    s.replace('|', '-');
    s = s.simplified();

    // Remplacer les caractères accentués non supportés par Adafruit GFX
    const QString from = QString::fromUtf8(
        "àáâãäåèéêëìíîïòóôõöùúûüýÿçñÀÁÂÃÄÅÈÉÊËÌÍÎÏÒÓÔÕÖÙÚÛÜÝŸÇÑ");
    const QString to =
        "aaaaaaeeeeiiiioooooouuuuyyçnAAAAAEEEEIIIIOOOOOUUUUYYCN";
    for (int i = 0; i < from.length(); ++i)
        s.replace(from[i], to[i]);

    if (s.length() > maxLen)
        s = s.left(maxLen - 1) + ".";
    return s;
}

void Scenario2::envoyerMessage(const QString& texte, const QString& couleur)
{
    if (!m_arduino) return;
    Q_UNUSED(couleur)
    const QString ligne = QString("MSG:%1\n").arg(formaterPourOled(texte));
    m_arduino->write_to_arduino(ligne.toUtf8());
    m_messageInTransit = true;   // on attend l'ACK de l'Arduino
    qDebug() << "[Scenario2/OLED] >>>" << ligne.trimmed();
}

// ── processInput : traite les messages READY et ACK de l'Arduino ──────────
void Scenario2::processInput(const QString& message)
{
    if (!m_arduino || message.isEmpty()) return;

    qDebug() << "[Scenario2] Arduino RAW:" << message;

    const QString msg = message.trimmed();

    // ── ACK : matching EXACT pour rejeter les messages corrompus ──
    // "ACKACK", "REAACK", "ACKREADY" etc. → ignorés
    if (msg.contains("ACK")) {
        if (!m_messageInTransit) {
            qDebug() << "[Scenario2] ACK ignore (aucun message en transit)";
            return;
        }
        // Remettre le flag à false AVANT de différer l'envoi.
        // Garantit que si un 2e ACK arrive dans la même boucle while(true)
        // du dispatcher, m_messageInTransit sera false → il sera ignoré.
        m_messageInTransit = false;
        qDebug() << "[Scenario2] ACK recu — envoi message suivant (differe)";
        if (m_pendingIndex == 0 && m_pendingMessages.isEmpty()) {
            qDebug() << "[Scenario2] ACK ignore (aucune sequence active)";
            return;
        }
        m_pendingIndex++;
        // singleShot(0) : reporte l'envoi au prochain tick Qt.
        QTimer::singleShot(0, this, [this]() {
            envoyerProchainMessage();
        });
    }
    // ── READY : l'Arduino vient de (re)démarrer ──
    else if (msg.contains("READY")) {
        // Debounce : ignorer les READY parasites dans les 5s suivant le premier READY
        if (m_readyDebounce) {
            qDebug() << "[Scenario2] READY ignore (debounce 5s actif)";
            return;
        }
        m_readyDebounce = true;
        QTimer::singleShot(5000, this, [this]() { m_readyDebounce = false; });

        // Si un cycle est actif quand READY arrive → l'Arduino a redémarré
        // en plein milieu. Réinitialiser et relancer proprement.
        if (!m_pendingMessages.isEmpty() || m_pendingIndex > 0 || m_messageInTransit) {
            qDebug() << "[Scenario2] READY recu pendant cycle actif -> reset force + relance";
            m_pendingMessages.clear();
            m_pendingIndex      = 0;
            m_messageInTransit  = false;
            m_relanceEnAttente  = false;
        }
        qDebug() << "[Scenario2] Arduino pret -> demarrage boucle auto";
        if (!m_relanceEnAttente)
            demarrerBoucleAuto();
    }
    else {
        qDebug() << "[Scenario2] Message inconnu:" << msg;
    }
}


// ── Requêtes SQL pour le programme journalier ─────────────────────────────

void Scenario2::afficherProgrammeGlobal()
{
    if (!m_arduino) {
        qDebug() << "[Scenario2] Arduino non disponible";
        return;
    }
    if (!m_pendingMessages.isEmpty() || m_pendingIndex > 0 || m_relanceEnAttente) {
        qDebug() << "[Scenario2] afficherProgrammeGlobal ignore (sequence en cours)";
        return;
    }

    // Nettoyage complet de l'état série avant chaque nouveau cycle
    m_messageInTransit = false;

    qDebug() << "[Scenario2] Debut affichage programme global";

    QStringList tous;
    tous << afficherEvenementsSemaine();
    tous << afficherProjetsSemaine();
    tous << afficherFinanceSemaine();
    tous << afficherLaboratoireSemaine();
    tous << afficherPublicationsSemaine();
    tous << afficherChercheursDisponibles();

    if (tous.isEmpty()) {
        qDebug() << "[Scenario2] Aucun message a afficher, relance dans 10s";
        m_relanceEnAttente = true;
        QTimer::singleShot(10000, this, [this]() {
            m_relanceEnAttente = false;
            afficherProgrammeGlobal();
        });
        return;
    }

    m_pendingMessages = tous;
    m_pendingIndex    = 0;
    envoyerProchainMessage();
}

QStringList Scenario2::afficherEvenementsSemaine()
{
    QStringList msgs;
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) { msgs << "Evenement: Base non connectee|RED"; return msgs; }

    QSqlQuery q(db);
    if (!q.exec("SELECT NOM, LIEU, DATE_EVENEMENT FROM EVENEMENT "
                "WHERE DATE_EVENEMENT >= TRUNC(SYSDATE) "
                "  AND DATE_EVENEMENT <  TRUNC(SYSDATE) + 7 "
                "  AND ROWNUM <= 5 ORDER BY DATE_EVENEMENT"))
    { msgs << "Evenement: Erreur BD|RED"; return msgs; }

    msgs << "=== EVENEMENTS ===|CYAN";
    bool found = false;
    while (q.next()) {
        found = true;
        QString nom  = q.value("NOM").toString().trimmed();
        QString lieu = q.value("LIEU").toString().trimmed();
        QDate   date = q.value("DATE_EVENEMENT").toDate();
        if (!date.isValid()) date = q.value("DATE_EVENEMENT").toDateTime().date();
        QString dateStr = date.isValid() ? date.toString("ddd dd/MM") : "";
        msgs << (QString("Evt: %1 - %2%3").arg(nom, lieu,
                 dateStr.isEmpty() ? "" : " (" + dateStr + ")") + "|CYAN");
    }
    if (!found) msgs << "Evenement: Aucun cette semaine|CYAN";
    return msgs;
}

QStringList Scenario2::afficherProjetsSemaine()
{
    QStringList msgs;
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) { msgs << "Projet: Base non connectee|RED"; return msgs; }

    QSqlQuery q(db);
    if (!q.exec("SELECT CODE, TITRE, ETAT FROM PROJET "
                "WHERE ETAT = 'en_cours' AND ROWNUM <= 5 ORDER BY CODE"))
    { msgs << "Projet: Erreur BD|RED"; return msgs; }

    msgs << "=== PROJETS ===|GREEN";
    bool found = false;
    while (q.next()) {
        found = true;
        QString code  = q.value("CODE").toString().trimmed();
        QString titre = q.value("TITRE").toString().trimmed();
        msgs << (QString("Projet: %1 - En cours")
                 .arg(titre.isEmpty() ? code : titre) + "|GREEN");
    }
    if (!found) msgs << "Projet: Aucun en cours|GREEN";
    return msgs;
}

QStringList Scenario2::afficherFinanceSemaine()
{
    QStringList msgs;
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) { msgs << "Finance: Base non connectee|RED"; return msgs; }

    QSqlQuery q(db);
    if (!q.exec("SELECT TYPE_TRANS, MONTANT, STATUT FROM FINANCE "
                "WHERE DATE_TRANSACTION >= TRUNC(SYSDATE) - 7 "
                "  AND ROWNUM <= 5 ORDER BY DATE_TRANSACTION DESC"))
    { msgs << "Finance: Erreur BD|RED"; return msgs; }

    msgs << "=== FINANCES ===|YELLOW";
    bool found = false;
    while (q.next()) {
        found = true;
        QString type   = q.value("TYPE_TRANS").toString().trimmed();
        double  montant = q.value("MONTANT").toDouble();
        QString statut = q.value("STATUT").toString().trimmed();
        msgs << (QString("Finance: %1 %2 TND - %3")
                 .arg(type, QString::number(montant, 'f', 0),
                      statut.isEmpty() ? "En attente" : statut) + "|YELLOW");
    }
    if (!found) msgs << "Finance: Aucune operation recente|YELLOW";
    return msgs;
}

QStringList Scenario2::afficherLaboratoireSemaine()
{
    QStringList msgs;
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) { msgs << "Labo: Base non connectee|RED"; return msgs; }

    QSqlQuery q(db);
    if (!q.exec("SELECT NOM, THEMATIQUE FROM LABORATOIRE "
                "WHERE DISPONIBILITE = 'disponible' AND ROWNUM <= 5 ORDER BY NOM"))
    { msgs << "Labo: Erreur BD|RED"; return msgs; }

    msgs << "=== LABORATOIRES ===|BLUE";
    bool found = false;
    while (q.next()) {
        found = true;
        QString nom  = q.value("NOM").toString().trimmed();
        QString them = q.value("THEMATIQUE").toString().trimmed();
        msgs << (QString("Labo: %1 - %2").arg(nom, them.isEmpty() ? "Actif" : them)
                 + "|BLUE");
    }
    if (!found) msgs << "Labo: Aucun disponible|BLUE";
    return msgs;
}

QStringList Scenario2::afficherPublicationsSemaine()
{
    QStringList msgs;
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) { msgs << "Pub: Base non connectee|RED"; return msgs; }

    QSqlQuery q(db);
    if (!q.exec("SELECT TITRE, AUTEUR, STATUT FROM PUBLICATION "
                "WHERE (DATE_PUBLICATION >= TRUNC(SYSDATE) - 30 "
                "   OR LOWER(STATUT) = 'publie') AND ROWNUM <= 5 "
                "ORDER BY DATE_PUBLICATION DESC"))
    { msgs << "Pub: Erreur BD|RED"; return msgs; }

    msgs << "=== PUBLICATIONS ===|WHITE";
    bool found = false;
    while (q.next()) {
        found = true;
        QString titre  = q.value("TITRE").toString().trimmed();
        QString auteur = q.value("AUTEUR").toString().trimmed();
        msgs << (QString("Pub: %1%2")
                 .arg(titre.isEmpty() ? "Sans titre" : titre,
                      auteur.isEmpty() ? "" : " - " + auteur) + "|WHITE");
    }
    if (!found) msgs << "Pub: Aucune publication recente|WHITE";
    return msgs;
}

QStringList Scenario2::afficherChercheursDisponibles()
{
    QStringList msgs;
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) { msgs << "Chercheur: Base non connectee|RED"; return msgs; }

    QSqlQuery q(db);
    // Oracle : ROWNUM doit être dans une sous-requête APRÈS ORDER BY,
    // sinon le filtre est appliqué avant le tri → résultats incomplets.
    if (!q.exec("SELECT NOM, PRENOM, GRADE FROM "
                "(SELECT NOM, PRENOM, GRADE FROM CHERCHEUR ORDER BY NOM, PRENOM) "
                "WHERE ROWNUM <= 10"))
    { msgs << "Chercheur: Erreur BD|RED"; return msgs; }

    msgs << "=== CHERCHEURS ===|GREEN";
    bool found = false;
    while (q.next()) {
        found = true;
        QString nom    = q.value("NOM").toString().trimmed();
        QString prenom = q.value("PRENOM").toString().trimmed();
        QString grade  = q.value("GRADE").toString().trimmed();
        msgs << (QString("Dr %1 %2%3").arg(nom, prenom,
                 grade.isEmpty() ? "" : " - " + grade) + "|GREEN");
    }
    if (!found) msgs << "Chercheur: Aucun enregistre|GREEN";
    return msgs;
}


// ============================================================================
// Partie B — DHT11 : détection chaleur laboratoire
// ============================================================================

void Scenario2::activerPourLabo(int id_labo)
{
    if (!m_arduino) return;
    QString cmd = QString("START:%1\n").arg(id_labo);
    m_arduino->write_to_arduino(cmd.toUtf8());
    qDebug() << "[Scenario2/DHT] >>> Envoi Arduino :" << cmd.trimmed();
}

void Scenario2::processLineDHT(const QString& line)
{
    if (line.startsWith("TEMP:")) {
        bool   ok   = false;
        double temp = line.mid(5).trimmed().toDouble(&ok);
        if (ok) {
            m_derniereTemp = temp;
            qDebug() << "[Scenario2/DHT] Temperature :" << temp << "°C";
        }
        return;
    }

    if (line.startsWith("FIRE:")) {
        int id_labo = line.mid(5).trimmed().toInt();
        if (id_labo <= 0) return;

        qDebug() << "[Scenario2/DHT] Chaleur detectee — labo ID :" << id_labo;
        m_incendieDetecte = true;
        m_idLaboEnAlerte  = id_labo;

        if (desactiverLaboratoire(id_labo)) {
            qDebug() << "[Scenario2/DHT] Labo ID" << id_labo << "-> Inactif en BD.";
            emit laboDesactive(id_labo);
        } else {
            qDebug() << "[Scenario2/DHT] Echec BD pour labo ID :" << id_labo;
        }
        return;
    }

    if (line.startsWith("ERR:")) {
        qDebug() << "[Scenario2/DHT] Erreur Arduino :" << line;
    }
}

bool Scenario2::desactiverLaboratoire(int id_labo)
{
    QSqlDatabase db = Connection::instance()->getDatabase();
    if (!db.isOpen()) {
        qDebug() << "[Scenario2/DHT] Base de donnees non connectee.";
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
        qDebug() << "[Scenario2/DHT] Erreur SQL :" << query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        qDebug() << "[Scenario2/DHT] Aucun laboratoire avec ID :" << id_labo;
        return false;
    }
    return true;
}

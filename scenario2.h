#ifndef SCENARIO2_H
#define SCENARIO2_H

// ============================================================================
// SCENARIO2 — Fusion OLED (ex-DemiScenario2) + DHT11 (ex-DemiScenario3)
//             sur une seule carte Arduino Uno
// ============================================================================
//
// Partie A — OLED SH1106 128x64 I2C :
//   Affichage programme journalier SmartPub (événements, projets, finances…)
//   Câblage : OLED SDA → A4 | OLED SCL → A5 | VCC → 3.3V | GND → GND
//
// Partie B — Capteur DHT11 :
//   Détection chaleur laboratoire — passage DISPONIBILITE → 'indisponible'
//   Câblage : DHT11 DATA → D7 | VCC → 5V | GND → GND
//
// Protocole série Qt → Arduino :
//   "MSG:<texte>\n"      → afficher texte sur OLED (Partie A)
//   "CLEAR\n"            → effacer OLED             (Partie A)
//   "DONE\n"             → retour écran accueil     (Partie A)
//   "START:<id_labo>\n"  → lancer lecture DHT11     (Partie B)
//
// Protocole série Arduino → Qt :
//   "READY\n"            → Arduino prêt             (Partie A)
//   "ACK\n"              → message OLED affiché     (Partie A)
//   "TEMP:<val>\n"       → température lue          (Partie B)
//   "FIRE:<id_labo>\n"   → seuil dépassé            (Partie B)
//   "ERR:DHT_READ\n"     → erreur capteur           (Partie B)
// ============================================================================

#include "arduino.h"
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTimer>
#include <QObject>

class Scenario2 : public QObject {
    Q_OBJECT

public:
    explicit Scenario2(Arduino* arduino, QObject* parent = nullptr);

    // ── Partie A : OLED ──────────────────────────────────────────────
    void processInput(const QString& message);   // traite READY / ACK
    void afficherProgrammeGlobal();
    void demarrerBoucleAuto();
    bool isRunning() const {
        return !m_pendingMessages.isEmpty()
            || m_pendingIndex > 0
            || m_relanceEnAttente;
    }

    QStringList afficherEvenementsSemaine();
    QStringList afficherProjetsSemaine();
    QStringList afficherFinanceSemaine();
    QStringList afficherLaboratoireSemaine();
    QStringList afficherPublicationsSemaine();
    QStringList afficherChercheursDisponibles();

    // ── Partie B : DHT11 ─────────────────────────────────────────────
    void activerPourLabo(int id_labo);           // envoie "START:<id>"
    void processLineDHT(const QString& line);    // traite TEMP/FIRE/ERR

    double getDerniereTemperature() const { return m_derniereTemp; }
    bool   isIncendieDetecte()      const { return m_incendieDetecte; }
    int    getIdLaboEnAlerte()      const { return m_idLaboEnAlerte; }

    // ── Mise à jour BD → OLED ────────────────────────────────────────
    // Appeler après tout ajout/modification/suppression en BD pour que
    // l'OLED reflète les nouvelles données au prochain cycle.
    void refreshDonnees();   // relance un nouveau cycle immédiatement

signals:
    // Émis quand un laboratoire est passé en Inactif en BD (Partie B)
    void laboDesactive(int id_labo);

private slots:
    void onTimerTick();   // relance la boucle OLED toutes les 60s

private:
    // ── Partie A ──
    Arduino*    m_arduino;
    QTimer*     m_timer;
    QStringList m_pendingMessages;
    int         m_pendingIndex;
    bool        m_relanceEnAttente;
    bool        m_messageInTransit; // vrai entre envoi MSG et réception ACK
    bool        m_readyDebounce;    // ignore les READY parasites pendant 2s


    void envoyerProchainMessage();
    void envoyerMessage(const QString& texte, const QString& couleur = "WHITE");
    static QString formaterPourOled(const QString& texte, int maxLen = 36);

    // ── Partie B ──
    double m_derniereTemp    = 0.0;
    bool   m_incendieDetecte = false;
    int    m_idLaboEnAlerte  = -1;

    bool desactiverLaboratoire(int id_labo);
};

#endif // SCENARIO2_H

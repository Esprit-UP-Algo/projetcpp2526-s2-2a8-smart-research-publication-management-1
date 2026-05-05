#ifndef SCENARIO1_H
#define SCENARIO1_H

// ============================================================================
// SCENARIO 1 — Controle d'acces RFID au laboratoire (Entree / Sortie)
<<<<<<< HEAD
// v6 : corrections RFID + ajout servo SG90 + buzzer + colonne ETAT_PORTE
//
// ARCHITECTURE v6 :
//   Un seul point d'entrée : processMessage()
//   Appelé depuis readyRead() dans smartpub.cpp, il lit UNE ligne du
//   buffer série et la dispatche selon son préfixe :
//     "RFID:<uid>"    → flux acces chercheur (inchangé v5)
//     "PORTE_OUVERTE" → mettreAJourEtatPorte(true)
//     "PORTE_FERMEE"  → mettreAJourEtatPorte(false)
//
=======
// v7 : séparation signal nom / signal moteur + signal Qt pour popup + affichage
//
// ARCHITECTURE v7 :
//   Un seul point d'entrée : processMessage()
//   Appelé depuis readyRead() dans smartpub.cpp, il lit UNE ligne du
//   buffer série et la dispatche selon son préfixe :
//     "RFID:<uid>"    → flux acces chercheur (inchangé v6)
//     "PORTE_OUVERTE" → mettreAJourEtatPorte(true)  + émet porteStatusChanged
//     "PORTE_FERMEE"  → mettreAJourEtatPorte(false) + émet porteStatusChanged
//
//   Protocole Qt → Arduino (v7) :
//     "ENTREE:<nom>"  → LCD + buzzer (pas de servo)
//     "SORTIE:<nom>"  → LCD + buzzer (pas de servo)
//     "OUVRIR_PORTE"  → commande servo uniquement
//     "REFUSE"        → LED rouge + bip long
//
>>>>>>> b4543ea (integration des scenario)
//   processAccess() est conservé comme alias de processMessage()
//   pour compatibilité avec le code existant dans smartpub.cpp.
// ============================================================================

#include "arduino.h"
#include <QObject>
#include <QString>
#include <QSqlDatabase>

class Scenario1 : public QObject {
    Q_OBJECT
public:
<<<<<<< HEAD
    explicit Scenario1(Arduino* arduino, int id_laboratoire);

    // Point d'entrée principal — à appeler depuis readyRead()
    // Lit une ligne du buffer et dispatche selon le type de message.
    void processMessage();

=======
    explicit Scenario1(Arduino* arduino, int id_laboratoire, QObject* parent = nullptr);

    // Point d'entrée principal — à appeler depuis readyRead()
    void processMessage();

    // Variante acceptant une ligne déjà lue (utilisée depuis smartpub.cpp)
    void processLine(const QString& line);

>>>>>>> b4543ea (integration des scenario)
    // Alias conservé pour compatibilité avec smartpub.cpp existant
    void processAccess() { processMessage(); }

    // Accesseurs pour l'interface Qt
    bool    getLastAccessGranted()      const { return m_lastGranted;   }
    QString getLastChercheurNomPrenom() const { return m_lastNomPrenom; }
    bool    getLastIsEntree()           const { return m_lastIsEntree;  }
<<<<<<< HEAD
=======

signals:
    // Émis quand la porte change d'état (confirmation Arduino reçue)
    // ouvert=true → porte ouverte, nomChercheur = nom du dernier accès autorisé
    // ouvert=false → porte fermée
    void porteStatusChanged(bool ouvert, const QString& nomChercheur, bool isEntree);
>>>>>>> b4543ea (integration des scenario)

private:
    Arduino* arduino;
    int      id_lab;

<<<<<<< HEAD
    bool    m_lastGranted   = false;
    QString m_lastNomPrenom;
    bool    m_lastIsEntree  = true;
    bool    m_enTraitement  = false;   // garde anti-reentrance readyRead
=======
    bool    m_lastGranted      = false;
    QString m_lastNomPrenom;
    bool    m_lastIsEntree     = true;
    bool    m_enTraitement     = false;   // garde anti-reentrance readyRead
    QString m_pendingNomPrenom;           // nom en attente de confirmation porte
    bool    m_pendingIsEntree  = true;    // direction en attente
>>>>>>> b4543ea (integration des scenario)

    // ── Flux RFID — accès chercheur ───────────────────────────────────
    void handleRfid(const QString& uid);

    int  getChercheurIdByRfid (const QString& rfid);
    bool getChercheurInfo     (int id_chercheur, QString& nom, QString& prenom);
    bool checkProjetEnCours   (int id_chercheur);
    bool checkProjetDansLabo  (int id_chercheur, int id_lab);
    bool estDansLabo          (int id_chercheur);
    bool enregistrerEntree    (int id_chercheur);
    bool enregistrerSortie    (int id_chercheur);

    // ── Flux porte — servo SG90 ───────────────────────────────────────
    bool mettreAJourEtatPorte(bool etatOuvert);

    // ── Commandes Arduino ─────────────────────────────────────────────
    void grantEntry (const QString& nom, const QString& prenom);
    void grantExit  (const QString& nom, const QString& prenom);
    void denyAccess ();
};

#endif // SCENARIO1_H

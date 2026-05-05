#ifndef FIN_KEYPAD_DELETE_H
#define FIN_KEYPAD_DELETE_H

// =========================================================================
// fin_keypad_delete.h
// =========================================================================
// Suppression sécurisée d'une transaction financière via clavier HX-543.
// Le code OTP est envoyé par email au DG via SMTP Gmail port 587 STARTTLS
// (QSslSocket natif Qt — aucune dépendance externe, fonctionne derrière
// tous les firewalls d'entreprise/université).
// =========================================================================

#include <QObject>
#include <QString>
#include <QTimer>

class Arduino;
class SmartPub;

class FinKepadDelete : public QObject
{
    Q_OBJECT

public:
    explicit FinKepadDelete(Arduino *arduino, SmartPub *parent = nullptr);

    // dgEmail : adresse email du Directeur Général
    void demanderSuppression(int transactionId, const QString &dgEmail);
    void annuler();

    // Appeler depuis readyRead — retourne true si la ligne est consommée
    bool traiterLigneSerie(const QString &ligne);

    bool enCours() const { return m_enCours; }
    int  transactionId() const { return m_transId; }

signals:
    void suppressionReussie(int transactionId);
    void suppressionRefusee(int transactionId);
    void suppressionAnnulee(int transactionId);
    void statutMessage(const QString &msg);

private slots:
    void onTimeoutCode();

private:
    QString genererCode() const;
    void envoyerEmailDG(const QString &email, const QString &code, int transId);
    void envoyerArduino(const QByteArray &cmd);
    void supprimerEnBD();
    void reinitialiser();

    Arduino  *m_arduino;
    SmartPub *m_parent;
    QTimer   *m_timer;

    bool    m_enCours    = false;
    int     m_transId    = 0;
    QString m_code;
    QString m_dgEmail;
    int     m_tentatives = 0;
    QString m_affichage;              // étoiles affichées pendant la saisie

    static constexpr int MAX_TENTATIVES    = 3;
    static constexpr int TIMEOUT_SAISIE_MS = 120'000; // 2 minutes
};

#endif // FIN_KEYPAD_DELETE_H
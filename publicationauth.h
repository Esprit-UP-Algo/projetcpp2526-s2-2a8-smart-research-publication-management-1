#ifndef PUBLICATIONAUTH_H
#define PUBLICATIONAUTH_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

class PublicationAuthService {
public:
    bool authenticate(const QString &email, const QString &password, QString *errorMessage) const;
};

class PublicationLoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit PublicationLoginDialog(QWidget *parent = nullptr);

private slots:
    void onLoginClicked();

private:
    void setupUI();

    PublicationAuthService m_authService;
    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    QLabel *m_errorLabel;
    QPushButton *m_loginButton;
    QPushButton *m_cancelButton;
};

#endif // PUBLICATIONAUTH_H

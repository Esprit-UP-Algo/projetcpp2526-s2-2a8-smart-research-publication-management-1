#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class Connection
{
private:
    QSqlDatabase db;

    // Constructeur privé pour le pattern Singleton
    Connection();

    // Empêcher la copie
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

public:
    // Méthode statique pour obtenir l'instance unique
    static Connection& getInstance();

    // Méthode pour créer la connexion
    bool createConnection();

    // Méthode pour obtenir la base de données
    QSqlDatabase getDatabase();

    // Destructeur
    ~Connection();
};

#endif // CONNECTION_H

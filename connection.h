#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class Connection
{
private:
    // Constructeur privé pour le patron Singleton
    Connection();

    // Empêcher la copie et l'assignation
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    QSqlDatabase db;

public:
    // Destructeur
    ~Connection();

    // Méthode pour obtenir l'instance unique (Singleton)
    static Connection& getInstance();

    // Méthode pour créer/obtenir la connexion
    bool createConnection();

    // Méthode pour obtenir la base de données
    QSqlDatabase getDatabase();

    // Méthode pour fermer la connexion
    void closeConnection();

    // Méthode pour vérifier si la connexion est ouverte
    bool isOpen() const;
};

#endif // CONNECTION_H

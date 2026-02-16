#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

class Connection
{
private:
    // Constructeur privé pour le pattern Singleton
    Connection();
    
    // Supprimer le constructeur de copie et l'opérateur d'affectation
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    
    QSqlDatabase db;

public:
    // Destructeur
    ~Connection();
    
    // Méthode static pour obtenir l'instance unique (Singleton)
    static Connection& getInstance();
    
    // Méthode pour créer la connexion
    bool createConnection();
    
    // Méthode pour fermer la connexion
    void closeConnection();
    
    // Méthode pour obtenir la base de données
    QSqlDatabase getDatabase();
    
    // Méthode pour vérifier si la connexion est ouverte
    bool isOpen() const;
};

#endif // CONNECTION_H

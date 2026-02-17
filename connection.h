#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class Connection
{
public:
    // Pattern Singleton : constructeur privé et méthode statique pour obtenir l'instance
    static Connection& getInstance();
    
    // Empêcher la copie
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    
    // Méthode pour établir la connexion
    bool createconnect();
    
    // Méthode pour obtenir la base de données
    QSqlDatabase database();
    
    // Méthode pour fermer la connexion
    void closeConnection();

private:
    Connection();  // Constructeur privé
    ~Connection() = default;
};

#endif // CONNECTION_H
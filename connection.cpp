#include "connection.h"
#include <QDebug>

// Constructeur privé
Connection::Connection()
{
}

// Destructeur
Connection::~Connection()
{
    closeConnection();
}

// Méthode static pour obtenir l'instance unique (Singleton)
Connection& Connection::getInstance()
{
    static Connection instance; // Créée une seule fois (Singleton)
    return instance;
}

// Méthode pour créer la connexion
bool Connection::createConnection()
{
    // Vérifier si la connexion existe déjà
    if (db.isValid() && db.isOpen())
    {
        qDebug() << "Connexion déjà établie";
        return true;
    }
    
    // Créer la connexion ODBC
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Source_Projet2A"); // Nom de la source de données ODBC
    db.setUserName("Karim");               // Nom d'utilisateur Oracle
    db.setPassword("karim123");            // Mot de passe Oracle
    
    // Tenter d'ouvrir la connexion
    if (db.open())
    {
        qDebug() << "Connexion à la base de données réussie!";
        return true;
    }
    else
    {
        qDebug() << "Erreur de connexion à la base de données:";
        qDebug() << "Driver error:" << db.lastError().driverText();
        qDebug() << "Database error:" << db.lastError().databaseText();
        return false;
    }
}

// Méthode pour fermer la connexion
void Connection::closeConnection()
{
    if (db.isOpen())
    {
        QString connectionName = db.connectionName();
        db.close();
        QSqlDatabase::removeDatabase(connectionName);
        qDebug() << "Connexion fermée";
    }
}

// Méthode pour obtenir la base de données
QSqlDatabase Connection::getDatabase()
{
    return db;
}

// Méthode pour vérifier si la connexion est ouverte
bool Connection::isOpen() const
{
    return db.isOpen();
}

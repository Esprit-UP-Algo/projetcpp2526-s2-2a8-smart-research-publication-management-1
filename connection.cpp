#include "connection.h"

// Constructeur privé
Connection::Connection()
{
    // Initialisation de la base de données dans le constructeur
}

// Destructeur
Connection::~Connection()
{
    closeConnection();
}

// Méthode getInstance pour le patron Singleton
Connection& Connection::getInstance()
{
    static Connection instance; // Instance unique créée une seule fois
    return instance;
}

// Méthode pour créer la connexion
bool Connection::createConnection()
{
    // Vérifier si la connexion existe déjà
    if (db.isValid() && db.isOpen())
    {
        qDebug() << "La connexion existe déjà et est ouverte";
        return true;
    }

    // Créer la connexion si elle n'existe pas
    if (!db.isValid())
    {
        db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName("Source_Projet2A8"); // Nom de la source de données ODBC
        db.setUserName("Nourchène");             // Nom d'utilisateur
        db.setPassword("1234567");               // Mot de passe
    }

    // Tenter d'ouvrir la connexion
    if (db.open())
    {
        qDebug() << "Connexion établie avec succès !";
        qDebug() << "Base de données:" << db.databaseName();
        qDebug() << "Driver:" << db.driverName();
        return true;
    }
    else
    {
        qDebug() << "Erreur de connexion:" << db.lastError().text();
        return false;
    }
}

// Méthode pour obtenir la base de données
QSqlDatabase Connection::getDatabase()
{
    return db;
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

// Méthode pour vérifier si la connexion est ouverte
bool Connection::isOpen() const
{
    return db.isOpen();
}

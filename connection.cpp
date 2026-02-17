#include "connection.h"

// Constructeur privé
Connection::Connection()
{
    // Initialisation de la base de données
}

// Méthode statique pour obtenir l'instance unique (Singleton)
Connection& Connection::getInstance()
{
    static Connection instance; // Création unique de l'instance
    return instance;
}

// Méthode pour créer la connexion
bool Connection::createConnection()
{
    bool test = false;

    // Configuration de la connexion ODBC
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("Source_Projet2A"); // Nom de la source de données ODBC
    db.setUserName("Salmaaa");             // Nom d'utilisateur Oracle
    db.setPassword("soso");            // Mot de passe Oracle

    // Tentative d'ouverture de la connexion
    if (db.open())
    {
        test = true;
        qDebug() << "✓ Connexion à la base de données établie avec succès";
        qDebug() << "Base de données:" << db.databaseName();
        qDebug() << "Driver:" << db.driverName();
    }
    else
    {
        qDebug() << "✗ Erreur de connexion à la base de données";
        qDebug() << "Erreur:" << db.lastError().text();
        qDebug() << "Driver disponible:" << db.driverName();
    }

    return test;
}

// Méthode pour obtenir la base de données
QSqlDatabase Connection::getDatabase()
{
    return db;
}

// Destructeur
Connection::~Connection()
{
    if (db.isOpen())
    {
        db.close();
        qDebug() << "Connexion à la base de données fermée";
    }
}

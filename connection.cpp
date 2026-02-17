#include "connection.h"
#include <QSqlError>
#include <QDebug>

Connection::Connection()
{
    // Constructeur vide car nous utilisons le pattern Singleton
    // La connexion est établie via la méthode getInstance()
}

Connection& Connection::getInstance()
{
    static Connection instance; // Instance unique (Singleton)
    return instance;
}

bool Connection::createconnect()
{
    // Vérifier si le driver ODBC est disponible
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        qDebug() << "Driver ODBC non disponible!";
        return false;
    }
    
    // Récupérer l'instance de la base de données ou en créer une nouvelle
    QSqlDatabase db;
    
    if (QSqlDatabase::contains("oracle_connection")) {
        db = QSqlDatabase::database("oracle_connection");
    } else {
        db = QSqlDatabase::addDatabase("QODBC", "oracle_connection");
    }
    
    // Configuration des paramètres de connexion
    db.setDatabaseName("Source_Projet2A");  // Nom de votre source ODBC
    db.setUserName("memed832");               // Utilisateur créé dans Oracle
    db.setPassword("mdp");           // Mot de passe
    
    // Tentative d'ouverture de la connexion
    if (db.open()) {
        qDebug() << "Connexion à la base de données établie avec succès!";
        qDebug() << "Source ODBC: Source_Projet2A";
        qDebug() << "Utilisateur: memed832";
        
        // Créer les tables si elles n'existent pas
        QSqlQuery query(db);
        
        // Table des chercheurs
        bool chercheursTable = query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP TABLE CHERCHEURS CASCADE CONSTRAINTS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        
        query.exec(
            "CREATE TABLE CHERCHEURS ("
            "   ID NUMBER PRIMARY KEY,"
            "   NOM VARCHAR2(100) NOT NULL,"
            "   PRENOM VARCHAR2(100) NOT NULL,"
            "   GRADE VARCHAR2(50),"
            "   EMAIL VARCHAR2(100) UNIQUE,"
            "   CIN VARCHAR2(20) UNIQUE,"
            "   DATE_CREATION DATE DEFAULT SYSDATE,"
            "   CARRIERE VARCHAR2(50),"
            "   AGE NUMBER,"
            "   PHOTO_PATH VARCHAR2(255)"
            ")"
        );
        
        // Table des projets
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP TABLE PROJETS CASCADE CONSTRAINTS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        
        query.exec(
            "CREATE TABLE PROJETS ("
            "   ID NUMBER PRIMARY KEY,"
            "   CODE VARCHAR2(50) NOT NULL,"
            "   TITRE VARCHAR2(200) NOT NULL,"
            "   DATE_DEBUT DATE NOT NULL,"
            "   DATE_FIN DATE NOT NULL,"
            "   RESPONSABLE VARCHAR2(100),"
            "   ETAT VARCHAR2(50),"
            "   PROGRESSION VARCHAR2(10),"
            "   DESCRIPTION CLOB"
            ")"
        );
        
        // Table des publications
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP TABLE PUBLICATIONS CASCADE CONSTRAINTS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        
        query.exec(
            "CREATE TABLE PUBLICATIONS ("
            "   ID NUMBER PRIMARY KEY,"
            "   TITRE VARCHAR2(500) NOT NULL,"
            "   AUTEURS VARCHAR2(500) NOT NULL,"
            "   DATE_PUBLICATION DATE,"
            "   REVUE VARCHAR2(200),"
            "   STATUT VARCHAR2(50)"
            ")"
        );
        
        // Table des transactions financières
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP TABLE TRANSACTIONS CASCADE CONSTRAINTS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        
        query.exec(
            "CREATE TABLE TRANSACTIONS ("
            "   ID NUMBER PRIMARY KEY,"
            "   PROJET VARCHAR2(100),"
            "   TYPE VARCHAR2(50),"
            "   MONTANT NUMBER(10,2),"
            "   DATE_TRANSACTION DATE,"
            "   CATEGORIE VARCHAR2(100),"
            "   STATUT VARCHAR2(50),"
            "   DESCRIPTION CLOB"
            ")"
        );
        
        // Table des événements
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP TABLE EVENEMENTS CASCADE CONSTRAINTS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        
        query.exec(
            "CREATE TABLE EVENEMENTS ("
            "   ID NUMBER PRIMARY KEY,"
            "   NOM VARCHAR2(200) NOT NULL,"
            "   LIEU VARCHAR2(200),"
            "   DATE_EVENEMENT DATE,"
            "   DESCRIPTION CLOB"
            ")"
        );
        
        // Créer des séquences pour l'auto-incrémentation
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_CHERCHEURS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        query.exec("CREATE SEQUENCE SEQ_CHERCHEURS START WITH 1 INCREMENT BY 1");
        
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_PROJETS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        query.exec("CREATE SEQUENCE SEQ_PROJETS START WITH 1 INCREMENT BY 1");
        
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_PUBLICATIONS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        query.exec("CREATE SEQUENCE SEQ_PUBLICATIONS START WITH 1 INCREMENT BY 1");
        
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_TRANSACTIONS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        query.exec("CREATE SEQUENCE SEQ_TRANSACTIONS START WITH 1 INCREMENT BY 1");
        
        query.exec(
            "BEGIN "
            "   EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_EVENEMENTS'; "
            "EXCEPTION "
            "   WHEN OTHERS THEN NULL; "
            "END;"
        );
        query.exec("CREATE SEQUENCE SEQ_EVENEMENTS START WITH 1 INCREMENT BY 1");
        
        qDebug() << "Tables créées avec succès pour l'utilisateur memed832!";
        
        return true;
    } else {
        qDebug() << "Échec de la connexion à la base de données:";
        qDebug() << "Source ODBC: Source_Projet2A";
        qDebug() << "Utilisateur: memed832";
        qDebug() << "Erreur:" << db.lastError().text();
        return false;
    }
}

QSqlDatabase Connection::database()
{
    return QSqlDatabase::database("oracle_connection");
}

void Connection::closeConnection()
{
    if (QSqlDatabase::contains("oracle_connection")) {
        QSqlDatabase::database("oracle_connection").close();
        QSqlDatabase::removeDatabase("oracle_connection");
        qDebug() << "Connexion fermée";
    }
}
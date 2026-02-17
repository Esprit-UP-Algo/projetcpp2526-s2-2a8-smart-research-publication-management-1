#include "connection.h"

Connection::Connection()
{

}

bool Connection::createconnect()
{
    bool test = false;
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("source_SMARTPUB"); // TODO: Remplacer par le nom de votre source de données ODBC
    db.setUserName("malekk");         // TODO: Remplacer par votre nom d'utilisateur
    db.setPassword("souix");        // TODO: Remplacer par votre mot de passe

    if (db.open())
    {
        test = true;
    }

    return test;
}


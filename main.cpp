// main.cpp
#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Smart Research - Gestion des Chercheurs");
    a.setOrganizationName("SmartResearch");

    // Établir la connexion à la base de données avec le pattern Singleton
    Connection& connection = Connection::getInstance();
    bool connectionSuccess = connection.createConnection();
    
    if (connectionSuccess)
    {
        // Connexion réussie - afficher la fenêtre principale
        MainWindow w;
        w.show();
        
        QMessageBox::information(nullptr, QObject::tr("Connexion établie"),
                    QObject::tr("Connexion à la base de données réussie.\n"
                                "Bienvenue dans Smart Research!"), 
                    QMessageBox::Ok);
        
        return a.exec();
    }
    else
    {
        // Échec de la connexion - afficher un message d'erreur
        QMessageBox::critical(nullptr, QObject::tr("Erreur de connexion"),
                    QObject::tr("Impossible de se connecter à la base de données.\n"
                                "Veuillez vérifier:\n"
                                "- La source de données ODBC 'Source_Projet2A'\n"
                                "- Les identifiants de connexion\n"
                                "- Le service Oracle"), 
                    QMessageBox::Cancel);
        
        return -1;
    }
}

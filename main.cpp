#include "smartpub.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("SmartPub - Gestion des Publications de Recherche");
    a.setOrganizationName("SmartResearch");
    
    // Utilisation du pattern Singleton pour obtenir l'instance de connexion
    Connection& connection = Connection::getInstance();
    bool test = connection.createconnect();
    
    if(test)
    {
        SmartPub w;
        w.show();
        
        QMessageBox::information(nullptr, QObject::tr("Base de données ouverte"),
                    QObject::tr("Connexion réussie à la base de données.\n"
                                "L'application va maintenant démarrer."), QMessageBox::Ok);
        
        return a.exec();
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Base de données fermée"),
                    QObject::tr("Échec de la connexion à la base de données.\n"
                                "Vérifiez vos paramètres de connexion.\n"
                                "L'application va se fermer."), QMessageBox::Ok);
        
        return -1;  // Retourne -1 pour indiquer une erreur
    }
}
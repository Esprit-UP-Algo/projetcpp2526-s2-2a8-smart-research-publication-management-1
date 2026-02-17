#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Utilisation du patron Singleton pour la connexion
    Connection& connection = Connection::getInstance();
    bool test = connection.createConnection();

    if (test)
    {
        // Connexion réussie
        QMessageBox::information(nullptr, QObject::tr("Base de données"),
                                 QObject::tr("Connexion établie avec succès !\n"
                                             "Base de données : Source_Projet2A8\n"
                                             "Cliquez sur OK pour continuer."),
                                 QMessageBox::Ok);

        MainWindow w;
        w.show();

        return a.exec();
    }
    else
    {
        // Échec de connexion
        QMessageBox::critical(nullptr, QObject::tr("Erreur de connexion"),
                              QObject::tr("Impossible de se connecter à la base de données.\n"
                                          "Vérifiez vos paramètres de connexion.\n"
                                          "Cliquez sur OK pour quitter."),
                              QMessageBox::Ok);

        return -1;
    }
}

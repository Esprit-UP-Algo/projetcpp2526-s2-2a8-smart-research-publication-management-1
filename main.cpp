#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Utilisation du pattern Singleton pour la connexion
    Connection& connection = Connection::getInstance();
    bool test = connection.createConnection();

    MainWindow w;

    if (test)
    {
        // Connexion réussie - afficher la fenêtre principale
        w.show();
        QMessageBox::information(nullptr, QObject::tr("Connexion établie"),
                                 QObject::tr("✓ Connexion à la base de données réussie.\n"
                                             "Base: Source_Projet2A\n"

                                             "Utilisateur: Salmaaa\n\n"
                                             "Click OK pour continuer."),
                                 QMessageBox::Ok);
    }
    else
    {
        // Connexion échouée - afficher l'erreur
        QMessageBox::critical(nullptr, QObject::tr("Erreur de connexion"),
                              QObject::tr("✗ La connexion à la base de données a échoué.\n\n"
                                          "Vérifiez que:\n"
                                          "1. Oracle SQL Developer est en cours d'exécution\n"
                                          "2. La source ODBC 'Source_Projet2A' est configurée\n"
                                          "3. Les identifiants sont corrects (Salmaaa/21326619)\n"
                                          "4. Le driver ODBC Oracle est installé\n\n"
                                          "Click OK pour quitter."),
                              QMessageBox::Ok);
        return -1; // Quitter l'application si la connexion échoue
    }

    return a.exec();
}

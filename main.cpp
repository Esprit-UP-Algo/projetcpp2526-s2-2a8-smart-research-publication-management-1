// main.cpp (inchangé)
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Smart Research - Gestion des Chercheurs");
    a.setOrganizationName("SmartResearch");

    MainWindow w;
    w.show();

    return a.exec();
}

#include "smartpub.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("SmartPub - Gestion des Publications de Recherche");
    a.setOrganizationName("SmartResearch");

    SmartPub w;
    w.show();

    return a.exec();
}

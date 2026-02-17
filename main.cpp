#include "smartpub.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SmartPub w;  // Changed from MainWindow to SmartPub
    w.show();
    return a.exec();
}

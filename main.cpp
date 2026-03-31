#include "smartpub.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Establishing connection BEFORE creating SmartPub 
    // to fix data not loading on launch
    Connection* c = Connection::instance();
    bool test=c->createConnect();
    
    SmartPub w;  // Changed from MainWindow to SmartPub
    
    if(test)
    {w.show();
        QMessageBox::information(nullptr, QObject::tr("database is open"),
                                 QObject::tr("connection successful.\n"
                                             "Click Cancel to exit."), QMessageBox::Cancel);

    }
    else
        QMessageBox::critical(nullptr, QObject::tr("database is not open"),
                               QObject::tr("connection failed.\n"
                                           "Click Cancel to exit."), QMessageBox::Cancel);



    return a.exec();
}

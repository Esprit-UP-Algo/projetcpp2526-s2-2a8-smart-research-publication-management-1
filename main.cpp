#include "finance.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    finance w;
    w.show();
    return a.exec();
}

#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // translation (existing code)
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "sans_titre_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // load stylesheet from resources (style.qss embedded)
    QFile f(":/style.qss");
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        a.setStyleSheet(f.readAll());
    }

    MainWindow w;
    w.show();
    return a.exec();
}

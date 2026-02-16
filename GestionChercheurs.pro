QT       += core gui widgets charts
QT += sql

CONFIG += c++17

SOURCES += \
    connection.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    connection.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    ressources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

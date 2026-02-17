QT       += core gui widgets
QT += sql
CONFIG += c++17

TARGET = SmartResearch
TEMPLATE = app

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

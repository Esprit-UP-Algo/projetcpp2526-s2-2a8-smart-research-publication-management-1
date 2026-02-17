#-------------------------------------------------
# Projet de Gestion de Projets avec Oracle Database
# Configuration Qt Project File (.pro)
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Pour Qt 6, ajouter aussi :
# greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

TARGET = ProjetApp
TEMPLATE = app

# Configuration C++11 (minimum requis pour le Singleton thread-safe)
CONFIG += c++11

# Définir les sources
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    connection.cpp

# Définir les headers
HEADERS += \
    mainwindow.h \
    connection.h

# Définir les formulaires UI
FORMS += \
    mainwindow.ui

# Options de compilation (optionnel)
DEFINES += QT_DEPRECATED_WARNINGS

# Icône de l'application (optionnel)
# RC_ICONS = myapp.ico

# Ressources (optionnel)
# RESOURCES += resources.qrc

# Configuration par défaut
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

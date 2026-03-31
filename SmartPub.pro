QT += core gui widgets charts printsupport
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connection.cpp \
    main.cpp \
    smartpub.cpp \
<<<<<<< HEAD
    ai_service.cpp \
    reminder.cpp \
    finance.cpp \
    evenement.cpp \
=======
>>>>>>> 6e05745b67fc1f6812746e031777202ee10efaa6
    publicationauth.cpp \
    promotionengine.cpp \
    matchmakingengine.cpp

HEADERS += \
    connection.h \
    smartpub.h \
<<<<<<< HEAD
    ai_service.h \
    reminder.h \
=======
>>>>>>> 6e05745b67fc1f6812746e031777202ee10efaa6
    publicationauth.h \
    promotionengine.h \
    matchmakingengine.h \
    chercheur.h \
    publication.h \
    projet.h \
    evenement.h \
    laboratoire.h \
    finance.h

FORMS += \
    smartpub.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources.qrc

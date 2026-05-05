QT += core gui widgets charts printsupport
QT += sql
QT += network
QT += core gui  serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    arduino.cpp \
    connection.cpp \
    main.cpp \
    scenario1.cpp \
<<<<<<< HEAD
    demi_scenario2.cpp \
    demi_scenario3.cpp \
=======
    scenario2.cpp \
>>>>>>> b4543ea (integration des scenario)
    smartpub.cpp \
    chercheur.cpp \
    publication.cpp \
    laboratoire.cpp \
    projet.cpp \
    ai_service.cpp \
    reminder.cpp \
    finance.cpp \
    evenement.cpp \
    publicationauth.cpp \
    promotionengine.cpp \
    matchmakingengine.cpp \
    scoringengine.cpp \
    collaborationengine.cpp \
    trans_secure.cpp \
    calender.cpp \
    upload.cpp \
    osnotification.cpp \
    fin_keypad_delete.cpp

HEADERS += \
    arduino.h \
    connection.h \
    scenario1.h \
<<<<<<< HEAD
    demi_scenario2.h \
    demi_scenario3.h \
=======
    scenario2.h \
>>>>>>> b4543ea (integration des scenario)
    smartpub.h \
    ai_service.h \
    reminder.h \
    publicationauth.h \
    promotionengine.h \
    matchmakingengine.h \
    scoringengine.h \
    collaborationengine.h \
    trans_secure.h \
    calender.h \
    chercheur.h \
    publication.h \
    projet.h \
    evenement.h \
    laboratoire.h \
    finance.h \
    upload.h \
    osnotification.h \
    fin_keypad_delete.h

FORMS += \
    smartpub.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ressources.qrc

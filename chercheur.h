#ifndef CHERCHEUR_H
#define CHERCHEUR_H

#include <QString>
#include <QDateTime>
#include <QList>

struct ChercheurData {
    QString nom;
    QString prenom;
    QString grade;
    QString email;
    QString cin;
    QDateTime dateCreation;
    QString carriere;
    QList<int> projetsIds;
    int age;
    QString photoPath;
};

#endif // CHERCHEUR_H

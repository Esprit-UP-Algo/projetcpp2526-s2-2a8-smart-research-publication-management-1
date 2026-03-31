#ifndef PUBLICATION_H
#define PUBLICATION_H

#include <QString>

// Structure pour le module Publications
struct PublicationData {
    int id;
    QString titre;
    QString auteur;
    QString date;
    QString statut;
    
    PublicationData() : id(0) {}
};

#endif // PUBLICATION_H

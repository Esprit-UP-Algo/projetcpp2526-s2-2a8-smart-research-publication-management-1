-- ============================================================================
-- Script d'insertion de données de test pour le module LABORATOIRE
-- À exécuter dans Oracle SQL Developer après SmartPub1.sql
-- ============================================================================

-- Suppression des données existantes (optionnel)
DELETE FROM PARTICIPER WHERE ID_LABORATOIRE IN (SELECT ID_LABORATOIRE FROM LABORATOIRE);
DELETE FROM LABORATOIRE;
COMMIT;

-- Réinitialiser la séquence (optionnel)
BEGIN
  EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_LABORATOIRE';
  EXECUTE IMMEDIATE 'CREATE SEQUENCE SEQ_LABORATOIRE START WITH 1 INCREMENT BY 1 NOCACHE NOCYCLE';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- ============================================================================
-- INSERTION DE 10 LABORATOIRES DE TEST
-- ============================================================================

-- Lab 1: IA - Excellent (Actif, grande capacité)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Lab Intelligence Artificielle Avancée', 'Intelligence Artificielle', 'disponible', 
        'Bâtiment A, Campus Universitaire, Tunis', NULL);

-- Lab 2: Biotech - Bon (Actif, capacité moyenne)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Centre de Biotechnologie Médicale', 'Biotechnologie', 'disponible', 
        'Pôle Technologique, Sfax', NULL);

-- Lab 3: Nano - Moyen (En Construction)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Laboratoire de Nanotechnologie', 'Nanotechnologie', 'indisponible', 
        'Zone Industrielle, Sousse', NULL);

-- Lab 4: Énergies - Excellent (Actif, grande capacité)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Institut des Énergies Renouvelables', 'Énergies Renouvelables', 'disponible', 
        'Technopole Borj Cedria, Tunis', NULL);

-- Lab 5: Robotique - Bon (Actif)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Lab Robotique et Systèmes Intelligents', 'Robotique', 'disponible', 
        'ENIT, Tunis', NULL);

-- Lab 6: Chimie - Moyen (En Rénovation)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Laboratoire de Chimie Analytique', 'Chimie', 'indisponible', 
        'Faculté des Sciences, Monastir', NULL);

-- Lab 7: Physique - Excellent (Actif)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Centre de Physique Quantique', 'Physique Quantique', 'disponible', 
        'INSAT, Tunis', NULL);

-- Lab 8: Data Science - Bon (Actif)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Lab Sciences de Données et Big Data', 'Sciences de Données', 'disponible', 
        'Sup\'Com, Ariana', NULL);

-- Lab 9: IA - Faible (Inactif, petite capacité)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Lab IA Expérimental', 'Intelligence Artificielle', 'indisponible', 
        'Annexe B, Campus El Manar', NULL);

-- Lab 10: Biotech - Moyen (Actif, capacité limitée)
INSERT INTO LABORATOIRE (NOM, THEMATIQUE, DISPONIBILITE, ADRESSE, ID_PROJET)
VALUES ('Unité de Recherche Biotechnologique', 'Biotechnologie', 'disponible', 
        'Institut Pasteur, Tunis', NULL);

COMMIT;

-- Vérification
SELECT ID_LABORATOIRE, NOM, THEMATIQUE, DISPONIBILITE 
FROM LABORATOIRE 
ORDER BY ID_LABORATOIRE;

/

-- ============================================================
-- SCENARIO 1 — Ajouts pour controle acces RFID au laboratoire
-- ============================================================

-- Colonne CLR_RFID : code UID hexadecimal de la carte RFID
-- ex: 'A1B2C3D4'  (8 caracteres max pour carte MIFARE 1K)
BEGIN
  EXECUTE IMMEDIATE 'ALTER TABLE CHERCHEUR ADD (CLR_RFID VARCHAR2(50))';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- Contrainte UNIQUE sur CLR_RFID (une carte = un chercheur)
BEGIN
  EXECUTE IMMEDIATE 'ALTER TABLE CHERCHEUR ADD CONSTRAINT UNQ_CHERCHEUR_RFID UNIQUE (CLR_RFID)';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- Colonne DATE_ENTREE_LAB : horodatage d'entree dans le labo
-- NULL     => le chercheur n'est PAS dans le labo (ou deja sorti)
-- NOT NULL => le chercheur est ACTUELLEMENT dans le labo
BEGIN
  EXECUTE IMMEDIATE 'ALTER TABLE CHERCHEUR ADD (DATE_ENTREE_LAB TIMESTAMP)';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- Colonne TEMPS_TRAVAIL_PROJET : duree cumulee en secondes
-- Incrementee a chaque sortie du labo : duree = sortie - entree
BEGIN
  EXECUTE IMMEDIATE 'ALTER TABLE CHERCHEUR ADD (TEMPS_TRAVAIL_PROJET NUMBER DEFAULT 0)';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- ============================================================
-- SCENARIO 1 v6 — Ajout colonne ETAT_PORTE dans LABORATOIRE
-- ============================================================
-- Colonne ETAT_PORTE : etat physique de la porte du laboratoire
-- Mise a jour automatiquement par le servo SG90 via l'Arduino :
--   0 = porte FERMEE (valeur par defaut)
--   1 = porte OUVERTE
-- La valeur est mise a jour dans la table LABORATOIRE par
-- Scenario1::mettreAJourEtatPorte() apres reception des messages
-- "PORTE_OUVERTE" et "PORTE_FERMEE" envoyes par l'Arduino.
BEGIN
  EXECUTE IMMEDIATE 'ALTER TABLE LABORATOIRE ADD (ETAT_PORTE NUMBER(1) DEFAULT 0 NOT NULL)';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- Contrainte CHECK : ETAT_PORTE ne peut valoir que 0 ou 1
BEGIN
  EXECUTE IMMEDIATE 'ALTER TABLE LABORATOIRE ADD CONSTRAINT CHK_ETAT_PORTE CHECK (ETAT_PORTE IN (0, 1))';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

-- Initialiser toutes les portes a FERMEE (0) pour les lignes existantes
-- (la clause DEFAULT 0 couvre les nouvelles insertions)
BEGIN
  EXECUTE IMMEDIATE 'UPDATE LABORATOIRE SET ETAT_PORTE = 0 WHERE ETAT_PORTE IS NULL';
  EXECUTE IMMEDIATE 'COMMIT';
EXCEPTION WHEN OTHERS THEN NULL;
END;
/

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
-- NULL  => le chercheur n'est PAS dans le labo (ou deja sorti)
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
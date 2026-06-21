-- init_db.sql
-- Initialisation de la base "mydiscord" (schema + donnees de test)
-- Execution dans pgAdmin4 :
--   1. Clic droit sur la base "mydiscord" > Query Tool
--   2. Ouvrir ce fichier (File > Open) ou coller le contenu
--   3. Appuyer sur F5 ou le bouton "Execute"

BEGIN;

DROP TABLE IF EXISTS fichier     CASCADE;
DROP TABLE IF EXISTS reaction    CASCADE;
DROP TABLE IF EXISTS message     CASCADE;
DROP TABLE IF EXISTS canal       CASCADE;
DROP TABLE IF EXISTS utilisateur CASCADE;
DROP TABLE IF EXISTS role        CASCADE;
DROP TYPE  IF EXISTS statut_utilisateur;

CREATE TYPE statut_utilisateur AS ENUM ('en ligne', 'hors ligne', 'occupe', 'banni');

CREATE TABLE role (
    id_role     SERIAL PRIMARY KEY,
    nom_role    VARCHAR(50) NOT NULL UNIQUE
);

CREATE TABLE utilisateur (
    id_utilisateur  SERIAL PRIMARY KEY,
    email           VARCHAR(150) NOT NULL UNIQUE,
    nom             VARCHAR(100) NOT NULL,
    prenom          VARCHAR(100) NOT NULL,
    mdp_hash        VARCHAR(255) NOT NULL,
    statut          statut_utilisateur NOT NULL DEFAULT 'hors ligne',
    id_role         INTEGER NOT NULL REFERENCES role(id_role),
    created_at      TIMESTAMP NOT NULL DEFAULT now()
);

CREATE TABLE canal (
    id_canal        SERIAL PRIMARY KEY,
    nom             VARCHAR(100) NOT NULL,
    est_prive       BOOLEAN NOT NULL DEFAULT false,
    id_createur     INTEGER NOT NULL REFERENCES utilisateur(id_utilisateur),
    created_at      TIMESTAMP NOT NULL DEFAULT now()
);

CREATE TABLE message (
    id_message      SERIAL PRIMARY KEY,
    contenu_chiffre TEXT NOT NULL,
    contenu_iv      VARCHAR(64) NOT NULL,
    date_envoi      TIMESTAMP NOT NULL DEFAULT now(),
    id_auteur       INTEGER NOT NULL REFERENCES utilisateur(id_utilisateur),
    id_canal        INTEGER NOT NULL REFERENCES canal(id_canal)
);

CREATE TABLE reaction (
    id_reaction     SERIAL PRIMARY KEY,
    emoji           VARCHAR(10) NOT NULL,
    id_message      INTEGER NOT NULL REFERENCES message(id_message),
    id_utilisateur  INTEGER NOT NULL REFERENCES utilisateur(id_utilisateur),
    UNIQUE (id_message, id_utilisateur, emoji)
);

CREATE TABLE fichier (
    id_fichier      SERIAL PRIMARY KEY,
    nom_fichier     VARCHAR(255) NOT NULL,
    chemin          VARCHAR(500) NOT NULL,
    type_mime       VARCHAR(100) NOT NULL,
    taille_octets   BIGINT NOT NULL,
    id_message      INTEGER NOT NULL REFERENCES message(id_message)
);

COMMIT;

-- ===== Donnees de test =====
BEGIN;

INSERT INTO role (nom_role) VALUES
    ('admin'), ('utilisateur'), ('moderateur');

INSERT INTO utilisateur (email, nom, prenom, mdp_hash, statut, id_role) VALUES
    ('alice@example.com', 'Dupont',  'Alice', 'hash_alice', 'en ligne',   1),
    ('bob@example.com',   'Martin',  'Bob',   'hash_bob',   'en ligne',   2),
    ('chloe@example.com', 'Bernard', 'Chloe', 'hash_chloe', 'hors ligne', 2);

INSERT INTO canal (nom, est_prive, id_createur) VALUES
    ('general',          false, 1),
    ('prive-alice-bob',  true,  1);

INSERT INTO message (contenu_chiffre, contenu_iv, id_auteur, id_canal) VALUES
    ('Q2hpZmZyZTpTYWx1dCB0b3Vz', 'iv0001', 1, 1),
    ('Q2hpZmZyZTpCaWVudmVudWU=', 'iv0002', 2, 1);

INSERT INTO reaction (emoji, id_message, id_utilisateur) VALUES
    (U&'\+01F44D', 1, 2),
    (U&'\+01F389', 1, 3);

INSERT INTO fichier (nom_fichier, chemin, type_mime, taille_octets, id_message) VALUES
    ('photo.png', '/uploads/photo.png', 'image/png', 204800, 1);

COMMIT;
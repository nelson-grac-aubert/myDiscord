BEGIN;

DROP TABLE IF EXISTS file      CASCADE;
DROP TABLE IF EXISTS reaction  CASCADE;
DROP TABLE IF EXISTS message   CASCADE;
DROP TABLE IF EXISTS channel   CASCADE;
DROP TABLE IF EXISTS whitelist CASCADE;
DROP TABLE IF EXISTS blacklist CASCADE;
DROP TABLE IF EXISTS "user"    CASCADE;
DROP TABLE IF EXISTS role      CASCADE;
DROP TYPE  IF EXISTS user_status;

CREATE TYPE user_status AS ENUM ('online', 'offline', 'busy', 'banned');

CREATE TABLE role (
    id_role     SERIAL PRIMARY KEY,
    role_name   VARCHAR(50) NOT NULL UNIQUE
);

CREATE TABLE "user" (
    id_user       SERIAL PRIMARY KEY,
    email         VARCHAR(150) NOT NULL UNIQUE,
    last_name     VARCHAR(100) NOT NULL,
    first_name    VARCHAR(100) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    status        user_status  NOT NULL DEFAULT 'offline',
    id_role       INTEGER      NOT NULL REFERENCES role(id_role),
    created_at    TIMESTAMP    NOT NULL DEFAULT now()
);

CREATE TABLE channel (
    id_channel  SERIAL PRIMARY KEY,
    name        VARCHAR(100) NOT NULL,
    is_private  BOOLEAN      NOT NULL DEFAULT false,
    id_creator  INTEGER      NOT NULL REFERENCES "user"(id_user),
    created_at  TIMESTAMP    NOT NULL DEFAULT now()
);

CREATE TABLE message (
    id_message        SERIAL PRIMARY KEY,
    encrypted_content TEXT         NOT NULL,
    content_iv        VARCHAR(64)  NOT NULL,
    sent_at           TIMESTAMP    NOT NULL DEFAULT now(),
    id_author         INTEGER      NOT NULL REFERENCES "user"(id_user),
    id_channel        INTEGER      NOT NULL REFERENCES channel(id_channel)
);

CREATE TABLE reaction (
    id_reaction SERIAL PRIMARY KEY,
    emoji       VARCHAR(10) NOT NULL,
    id_message  INTEGER     NOT NULL REFERENCES message(id_message),
    id_user     INTEGER     NOT NULL REFERENCES "user"(id_user),
    UNIQUE (id_message, id_user, emoji)
);

CREATE TABLE file (
    id_file     SERIAL PRIMARY KEY,
    file_name   VARCHAR(255) NOT NULL,
    path        VARCHAR(500) NOT NULL,
    mime_type   VARCHAR(100) NOT NULL,
    size_bytes  BIGINT       NOT NULL,
    id_message  INTEGER      NOT NULL REFERENCES message(id_message)
);

CREATE TABLE whitelist (
    id_whitelist SERIAL PRIMARY KEY,
    id_user      INTEGER      NOT NULL REFERENCES "user"(id_user),
    reason       VARCHAR(500),
    added_at     TIMESTAMP    NOT NULL DEFAULT now(),
    added_by     INTEGER      NOT NULL REFERENCES "user"(id_user)
);

CREATE TABLE blacklist (
    id_blacklist SERIAL PRIMARY KEY,
    id_user      INTEGER      NOT NULL REFERENCES "user"(id_user),
    reason       VARCHAR(500),
    banned_at    TIMESTAMP    NOT NULL DEFAULT now(),
    banned_by    INTEGER      NOT NULL REFERENCES "user"(id_user)
);

COMMIT;

-- Test data 
BEGIN;

INSERT INTO role (role_name) VALUES
    ('admin'), ('user'), ('moderator');

INSERT INTO "user" (email, last_name, first_name, password_hash, status, id_role) VALUES
    ('alice@example.com', 'Dupont',  'Alice', 'hash_alice', 'online',  1),
    ('bob@example.com',   'Martin',  'Bob',   'hash_bob',   'online',  2),
    ('chloe@example.com', 'Bernard', 'Chloe', 'hash_chloe', 'offline', 2);

INSERT INTO channel (name, is_private, id_creator) VALUES
    ('general',       false, 1),
    ('private-alice-bob', true,  1);

INSERT INTO message (encrypted_content, content_iv, id_author, id_channel) VALUES
    ('Q2hpZmZyZTpTYWx1dCB0b3Vz', 'iv0001', 1, 1),
    ('Q2hpZmZyZTpCaWVudmVudWU=', 'iv0002', 2, 1);

INSERT INTO reaction (emoji, id_message, id_user) VALUES
    (U&'\+01F44D', 1, 2),
    (U&'\+01F389', 1, 3);

INSERT INTO file (file_name, path, mime_type, size_bytes, id_message) VALUES
    ('photo.png', '/uploads/photo.png', 'image/png', 204800, 1);

INSERT INTO whitelist (id_user, reason, added_by) VALUES
    (2, 'Trusted beta tester', 1);

INSERT INTO blacklist (id_user, reason, banned_by) VALUES
    (3, 'Spam activity detected', 1);

COMMIT;
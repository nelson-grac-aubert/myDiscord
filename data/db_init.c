#include <stdio.h>
#include <stdlib.h>
#include "db_init.h"

static int exec_sql(PGconn *conn, const char *sql, const char *label)
{
    PGresult *res = PQexec(conn, sql);
    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        fprintf(stderr, "[FAIL]  %-38s : %s\n", label, PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    printf("[OK]    %-38s\n", label);
    PQclear(res);
    return 0;
}

#define EXEC(sql, label) \
    if (exec_sql(conn, sql, label) != 0) { \
        PQexec(conn, "ROLLBACK;"); \
        fprintf(stderr, "[ROLLBACK] Transaction cancelled.\n"); \
        return -1; \
    }

int initializeData(PGconn *conn)
{
    printf("--- TRANSACTION 1 : schema ---\n\n");

    EXEC("BEGIN;", "BEGIN")

    /* legacy French tables */
    EXEC("DROP TABLE IF EXISTS fichier     CASCADE;", "DROP fichier")
    EXEC("DROP TABLE IF EXISTS canal       CASCADE;", "DROP canal")
    EXEC("DROP TABLE IF EXISTS utilisateur CASCADE;", "DROP utilisateur")
    EXEC("DROP TYPE  IF EXISTS statut_utilisateur;",  "DROP statut_utilisateur")

    /* English tables */
    EXEC("DROP TABLE IF EXISTS file        CASCADE;", "DROP file")
    EXEC("DROP TABLE IF EXISTS reaction    CASCADE;", "DROP reaction")
    EXEC("DROP TABLE IF EXISTS message     CASCADE;", "DROP message")
    EXEC("DROP TABLE IF EXISTS channel     CASCADE;", "DROP channel")
    EXEC("DROP TABLE IF EXISTS whitelist   CASCADE;", "DROP whitelist")
    EXEC("DROP TABLE IF EXISTS blacklist   CASCADE;", "DROP blacklist")
    EXEC("DROP TABLE IF EXISTS \"user\"      CASCADE;", "DROP user")
    EXEC("DROP TABLE IF EXISTS role        CASCADE;", "DROP role")
    EXEC("DROP TYPE  IF EXISTS user_status;",         "DROP user_status")

    EXEC(
        "CREATE TYPE user_status AS ENUM "
        "('online','offline','busy','banned');",
        "CREATE TYPE user_status")

    EXEC(
        "CREATE TABLE role ("
        "  id_role   SERIAL PRIMARY KEY,"
        "  role_name VARCHAR(50) NOT NULL UNIQUE"
        ");",
        "CREATE TABLE role")

    EXEC(
        "CREATE TABLE \"user\" ("
        "  id_user       SERIAL PRIMARY KEY,"
        "  email         VARCHAR(150) NOT NULL UNIQUE,"
        "  last_name     VARCHAR(100) NOT NULL,"
        "  first_name    VARCHAR(100) NOT NULL,"
        "  password_hash VARCHAR(255) NOT NULL,"
        "  status        user_status  NOT NULL DEFAULT 'offline',"
        "  id_role       INTEGER      NOT NULL REFERENCES role(id_role),"
        "  created_at    TIMESTAMP    NOT NULL DEFAULT now()"
        ");",
        "CREATE TABLE user")

    EXEC(
        "CREATE TABLE channel ("
        "  id_channel SERIAL PRIMARY KEY,"
        "  name       VARCHAR(100) NOT NULL,"
        "  is_private BOOLEAN      NOT NULL DEFAULT false,"
        "  id_creator INTEGER      NOT NULL REFERENCES \"user\"(id_user),"
        "  created_at TIMESTAMP    NOT NULL DEFAULT now()"
        ");",
        "CREATE TABLE channel")

    EXEC(
        "CREATE TABLE message ("
        "  id_message        SERIAL PRIMARY KEY,"
        "  encrypted_content TEXT        NOT NULL,"
        "  content_iv        VARCHAR(64) NOT NULL,"
        "  sent_at           TIMESTAMP   NOT NULL DEFAULT now(),"
        "  id_author         INTEGER     NOT NULL REFERENCES \"user\"(id_user),"
        "  id_channel        INTEGER     NOT NULL REFERENCES channel(id_channel)"
        ");",
        "CREATE TABLE message")

    EXEC(
        "CREATE TABLE reaction ("
        "  id_reaction SERIAL PRIMARY KEY,"
        "  emoji       VARCHAR(10) NOT NULL,"
        "  id_message  INTEGER     NOT NULL REFERENCES message(id_message),"
        "  id_user     INTEGER     NOT NULL REFERENCES \"user\"(id_user),"
        "  UNIQUE (id_message, id_user, emoji)"
        ");",
        "CREATE TABLE reaction")

    EXEC(
        "CREATE TABLE file ("
        "  id_file    SERIAL PRIMARY KEY,"
        "  file_name  VARCHAR(255) NOT NULL,"
        "  path       VARCHAR(500) NOT NULL,"
        "  mime_type  VARCHAR(100) NOT NULL,"
        "  size_bytes BIGINT       NOT NULL,"
        "  id_message INTEGER      NOT NULL REFERENCES message(id_message)"
        ");",
        "CREATE TABLE file")

    EXEC(
        "CREATE TABLE whitelist ("
        "  id_whitelist SERIAL PRIMARY KEY,"
        "  id_user      INTEGER      NOT NULL REFERENCES \"user\"(id_user),"
        "  reason       VARCHAR(500),"
        "  added_at     TIMESTAMP    NOT NULL DEFAULT now(),"
        "  added_by     INTEGER      NOT NULL REFERENCES \"user\"(id_user)"
        ");",
        "CREATE TABLE whitelist")

    EXEC(
        "CREATE TABLE blacklist ("
        "  id_blacklist SERIAL PRIMARY KEY,"
        "  id_user      INTEGER      NOT NULL REFERENCES \"user\"(id_user),"
        "  reason       VARCHAR(500),"
        "  banned_at    TIMESTAMP    NOT NULL DEFAULT now(),"
        "  banned_by    INTEGER      NOT NULL REFERENCES \"user\"(id_user)"
        ");",
        "CREATE TABLE blacklist")

    EXEC("COMMIT;", "COMMIT schema")

    printf("\n--- TRANSACTION 2 : data ---\n\n");

    EXEC("BEGIN;", "BEGIN")

    EXEC(
        "INSERT INTO role (role_name) VALUES "
        "('admin'),('user'),('moderator');",
        "INSERT role (3 rows)")

    EXEC(
        "INSERT INTO \"user\" (email, last_name, first_name, password_hash, status, id_role) VALUES "
        "('alice@example.com', 'Dupont',  'Alice', 'hash_alice', 'online',  1),"
        "('bob@example.com',   'Martin',  'Bob',   'hash_bob',   'online',  2),"
        "('chloe@example.com', 'Bernard', 'Chloe', 'hash_chloe', 'offline', 2);",
        "INSERT user (3 rows)")

    EXEC(
        "INSERT INTO channel (name, is_private, id_creator) VALUES "
        "('general',           false, 1),"
        "('private-alice-bob', true,  1);",
        "INSERT channel (2 rows)")

    EXEC(
        "INSERT INTO message (encrypted_content, content_iv, id_author, id_channel) VALUES "
        "('Q2hpZmZyZTpTYWx1dCB0b3Vz', 'iv0001', 1, 1),"
        "('Q2hpZmZyZTpCaWVudmVudWU=', 'iv0002', 2, 1);",
        "INSERT message (2 rows)")

    EXEC(
        "INSERT INTO reaction (emoji, id_message, id_user) VALUES "
        "(E'\\U0001F44D', 1, 2),"
        "(E'\\U0001F389', 1, 3);",
        "INSERT reaction (2 rows)")

    EXEC(
        "INSERT INTO file (file_name, path, mime_type, size_bytes, id_message) VALUES "
        "('photo.png', '/uploads/photo.png', 'image/png', 204800, 1);",
        "INSERT file (1 row)")

    EXEC(
        "INSERT INTO whitelist (id_user, reason, added_by) VALUES "
        "(2, 'Trusted beta tester', 1);",
        "INSERT whitelist (1 row)")

    EXEC(
        "INSERT INTO blacklist (id_user, reason, banned_by) VALUES "
        "(3, 'Spam activity detected', 1);",
        "INSERT blacklist (1 row)")

    EXEC("COMMIT;", "COMMIT data")

    printf("\n[SUCCESS] Database mydiscord initialized.\n");
    printf("  Tables : role, user, channel, message, reaction, file, whitelist, blacklist\n\n");
    return 0;
}
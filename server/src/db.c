#include "../include/db.h"
#include "../include/crypto.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int exec_has_rows(PGresult *res)
{
    return PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0;
}

int db_user_register(PGconn *db, const char *email,
                     const char *first_name, const char *last_name,
                     const char *password, int id_role)
{
    char salt[CRYPTO_SALT_LEN + 1];
    char hash[CRYPTO_HASH_LEN];

    if (crypto_gen_salt(salt) != 0)
        return -1;
    if (crypto_hash_password(password, salt, hash) != 0)
        return -1;

    /* password_hash stores "salt:hash" so we can retrieve salt at login */
    char stored[CRYPTO_SALT_LEN + CRYPTO_HASH_LEN + 2];
    snprintf(stored, sizeof(stored), "%s:%s", salt, hash);

    char id_role_str[8];
    snprintf(id_role_str, sizeof(id_role_str), "%d", id_role);

    const char *params[] = { email, last_name, first_name, stored, id_role_str };
    PGresult *res = PQexecParams(db,
        "INSERT INTO \"user\" (email, last_name, first_name, password_hash, id_role) "
        "VALUES ($1, $2, $3, $4, $5) RETURNING id_user",
        5, NULL, params, NULL, NULL, 0);

    if (!exec_has_rows(res)) {
        fprintf(stderr, "[db] register failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return -1;
    }

    int id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return id;
}

int db_user_login(PGconn *db, const char *email, const char *password)
{
    const char *params[] = { email };
    PGresult *res = PQexecParams(db,
        "SELECT id_user, password_hash FROM \"user\" WHERE email = $1",
        1, NULL, params, NULL, NULL, 0);

    if (!exec_has_rows(res)) {
        PQclear(res);
        return -1;
    }

    int user_id = atoi(PQgetvalue(res, 0, 0));
    char stored[CRYPTO_SALT_LEN + CRYPTO_HASH_LEN + 2];
    strncpy(stored, PQgetvalue(res, 0, 1), sizeof(stored) - 1);
    stored[sizeof(stored) - 1] = '\0';
    PQclear(res);

    /* Split "salt:hash" */
    char *colon = strchr(stored, ':');
    if (!colon)
        return -1;

    *colon = '\0';
    char *salt = stored;
    char *hash = colon + 1;

    if (!crypto_verify_password(password, salt, hash))
        return -1;

    return user_id;
}

int db_user_is_banned(PGconn *db, int user_id)
{
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", user_id);

    const char *params[] = { id_str };
    PGresult *res = PQexecParams(db,
        "SELECT 1 FROM blacklist WHERE id_user = $1",
        1, NULL, params, NULL, NULL, 0);

    int banned = exec_has_rows(res) ? 1 : 0;
    PQclear(res);
    return banned;
}

int db_message_insert(PGconn *db, int user_id, int channel_id, const char *content)
{
    char uid[16], cid[16];
    snprintf(uid, sizeof(uid), "%d", user_id);
    snprintf(cid, sizeof(cid), "%d", channel_id);

    /* No encryption yet: content_iv is placeholder */
    const char *params[] = { content, "iv_placeholder", uid, cid };
    PGresult *res = PQexecParams(db,
        "INSERT INTO message (encrypted_content, content_iv, id_author, id_channel) "
        "VALUES ($1, $2, $3, $4) RETURNING id_message",
        4, NULL, params, NULL, NULL, 0);

    if (!exec_has_rows(res)) {
        fprintf(stderr, "[db] message_insert failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return -1;
    }

    int id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return id;
}

int db_message_history(PGconn *db, int channel_id, int limit,
                       char messages_out[][512], int max_rows)
{
    char cid[16], lim[16];
    snprintf(cid, sizeof(cid), "%d", channel_id);
    snprintf(lim, sizeof(lim), "%d", limit < max_rows ? limit : max_rows);

    const char *params[] = { cid, lim };
    PGresult *res = PQexecParams(db,
        "SELECT u.email, m.encrypted_content, m.id_message FROM message m "
        "JOIN \"user\" u ON u.id_user = m.id_author "
        "WHERE m.id_channel = $1 ORDER BY m.sent_at DESC LIMIT $2",
        2, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "[db] message_history failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return -1;
    }

    int count = PQntuples(res);
    for (int i = 0; i < count; i++)
        snprintf(messages_out[i], 512, "%d|%s|%s|%s", channel_id,
                 PQgetvalue(res, i, 2), PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));

    PQclear(res);
    return count;
}

int db_channel_create(PGconn *db, const char *name, int is_private, int creator_id)
{
    char priv[2], cid[16];
    snprintf(priv, sizeof(priv), "%d", is_private);
    snprintf(cid, sizeof(cid), "%d", creator_id);

    const char *params[] = { name, priv, cid };
    PGresult *res = PQexecParams(db,
        "INSERT INTO channel (name, is_private, id_creator) "
        "VALUES ($1, $2::boolean, $3) RETURNING id_channel",
        3, NULL, params, NULL, NULL, 0);

    if (!exec_has_rows(res)) {
        fprintf(stderr, "[db] channel_create failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return -1;
    }

    int id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return id;
}

int db_channel_delete(PGconn *db, int channel_id, int requester_id)
{
    char cid[16], rid[16];
    snprintf(cid, sizeof(cid), "%d", channel_id);
    snprintf(rid, sizeof(rid), "%d", requester_id);

    /* Only the creator can delete */
    const char *params[] = { cid, rid };
    PGresult *res = PQexecParams(db,
        "DELETE FROM channel WHERE id_channel = $1 AND id_creator = $2",
        2, NULL, params, NULL, NULL, 0);

    /* A DELETE with a non-matching WHERE still reports PGRES_COMMAND_OK with
       0 rows affected, so a bad channel_id or a non-creator request must be
       distinguished by the affected row count, not just the result status */
    int ok = (PQresultStatus(res) == PGRES_COMMAND_OK &&
              atoi(PQcmdTuples(res)) > 0) ? 0 : -1;
    if (ok != 0)
        fprintf(stderr, "[db] channel_delete failed or forbidden: %s\n", PQerrorMessage(db));
    PQclear(res);
    return ok;
}

int db_channel_list(PGconn *db, int user_id, char channels_out[][100], int max_rows)
{
    char uid[16];
    snprintf(uid, sizeof(uid), "%d", user_id);

    const char *params[] = { uid };
    PGresult *res = PQexecParams(db,
        "SELECT id_channel, name, is_private FROM channel "
        "WHERE is_private = false OR id_creator = $1 "
        "ORDER BY id_channel",
        1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "[db] channel_list failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return -1;
    }

    int count = PQntuples(res);
    if (count > max_rows) count = max_rows;
    for (int i = 0; i < count; i++)
        snprintf(channels_out[i], 100, "%s|%s|%s",
                 PQgetvalue(res, i, 0),
                 PQgetvalue(res, i, 1),
                 PQgetvalue(res, i, 2));

    PQclear(res);
    return count;
}

int db_channel_join(PGconn *db, int user_id, int channel_id)
{
    /* No membership table yet: placeholder for future many-to-many */
    (void)db; (void)user_id; (void)channel_id;
    return 0;
}

int db_channel_leave(PGconn *db, int user_id, int channel_id)
{
    (void)db; (void)user_id; (void)channel_id;
    return 0;
}

int db_user_ban(PGconn *db, int target_id, int banned_by, const char *reason)
{
    char tid[16], bid[16];
    snprintf(tid, sizeof(tid), "%d", target_id);
    snprintf(bid, sizeof(bid), "%d", banned_by);

    const char *params[] = { tid, reason, bid };
    PGresult *res = PQexecParams(db,
        "INSERT INTO blacklist (id_user, reason, banned_by) VALUES ($1, $2, $3)",
        3, NULL, params, NULL, NULL, 0);

    int ok = PQresultStatus(res) == PGRES_COMMAND_OK ? 0 : -1;
    if (ok != 0)
        fprintf(stderr, "[db] user_ban failed: %s\n", PQerrorMessage(db));
    PQclear(res);
    return ok;
}
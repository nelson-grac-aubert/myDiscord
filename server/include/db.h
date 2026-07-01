#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

/* Returns id_user on success, -1 on failure (email already taken, db error) */
int db_user_register(PGconn *db, const char *email,
                     const char *first_name, const char *last_name,
                     const char *password, int id_role);

/* Returns id_user on success, -1 if not found or wrong password */
int db_user_login(PGconn *db, const char *email, const char *password);

/* Returns 1 if user is banned, 0 if not, -1 on error */
int db_user_is_banned(PGconn *db, int user_id);

/* Returns the new id_message on success, -1 on error */
int db_message_insert(PGconn *db, int user_id, int channel_id,
                      const char *content);

/* Fills messages_out with last limit messages in channel.
   Each row: "id_message|username|content\0". Returns row count, -1 on error. */
int db_message_history(PGconn *db, int channel_id, int limit,
                       char messages_out[][512], int max_rows);

/* Returns id_channel on success, -1 on failure */
int db_channel_create(PGconn *db, const char *name, int is_private, int creator_id);

/* Returns 0 on success, -1 on failure */
int db_channel_delete(PGconn *db, int channel_id, int requester_id);

/* Fills channels_out with accessible channel names.
   Returns row count, -1 on error. */
int db_channel_list(PGconn *db, int user_id,
                    char channels_out[][100], int max_rows);

/* Returns 0 on success, -1 on failure */
int db_channel_join(PGconn *db, int user_id, int channel_id);
int db_channel_leave(PGconn *db, int user_id, int channel_id);

/* Fills users_out with usernames in channel.
   Returns row count, -1 on error. */
int db_user_list(PGconn *db, int channel_id,
                 char users_out[][100], int max_rows);

/* Returns 0 on success, -1 on failure */
int db_user_ban(PGconn *db, int target_id, int banned_by, const char *reason);

#endif /* DB_H */
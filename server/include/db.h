#ifndef DB_H
#define DB_H

#include <libpq-fe.h>

/* Returns id_user on success, -1 on failure (email already taken, db error) */
int db_user_register(PGconn *db, const char *email,
                     const char *first_name, const char *last_name,
                     const char *password, int id_role);

/* Returns id_user on success, -1 if not found or wrong password.
   role_id_out is filled with the user's id_role, username_out (>= 100 bytes)
   with their first_name, on success. */
int db_user_login(PGconn *db, const char *email, const char *password,
                 int *role_id_out, char *username_out);

/* Returns 1 if user is banned, 0 if not, -1 on error */
int db_user_is_banned(PGconn *db, int user_id);

/* Returns the user's id_role, or -1 if not found/on error */
int db_user_get_role(PGconn *db, int user_id);

/* Fills out with "id_user:first_name:is_banned:id_role" for every
   registered account (regardless of whether they're currently connected).
   Returns row count, -1 on error. */
int db_user_list_all(PGconn *db, char out[][170], int max_rows);

/* Returns the new id_message on success, -1 on error.
   timestamp_out must be at least 6 bytes ("HH:MM\0"), filled with the
   message's actual sent_at as recorded by the database. */
int db_message_insert(PGconn *db, int user_id, int channel_id,
                      const char *content, char *timestamp_out);

/* Fills messages_out with last limit messages in channel, oldest first.
   Each row: "id_message|HH:MM|username|content\0". Returns row count, -1 on error. */
int db_message_history(PGconn *db, int channel_id, int limit,
                       char messages_out[][512], int max_rows);

/* Deletes a message if requester_id is its author. On success returns 0 and
   fills channel_id_out with the channel it belonged to (needed to scope the
   deletion broadcast). Returns -1 if not found or not the author. */
int db_message_delete(PGconn *db, int message_id, int requester_id, int *channel_id_out);

/* Returns id_channel on success, -1 on failure */
int db_channel_create(PGconn *db, const char *name, int is_private, int creator_id);

/* Deletes the channel if requester_id created it, or unconditionally if
   is_admin is set (admins may delete any channel). Returns 0 on success,
   -1 on failure/forbidden. */
int db_channel_delete(PGconn *db, int channel_id, int requester_id, int is_admin);

/* Fills channels_out with accessible channel names.
   Returns row count, -1 on error. */
int db_channel_list(PGconn *db, int user_id,
                    char channels_out[][100], int max_rows);

/* Returns 0 on success, -1 on failure */
int db_channel_join(PGconn *db, int user_id, int channel_id);
int db_channel_leave(PGconn *db, int user_id, int channel_id);

/* Returns 0 on success, -1 on failure */
int db_user_ban(PGconn *db, int target_id, int banned_by, const char *reason);

/* Removes all blacklist entries for target_id. Returns 0 on success,
   -1 if the user wasn't banned or on db error. */
int db_user_unban(PGconn *db, int target_id);

/* Returns the id_channel a message belongs to, or -1 if not found/on error */
int db_message_get_channel_id(PGconn *db, int message_id);

/* Sets user_id's reaction on message_id to emoji, replacing any existing
   reaction from that same user on that same message (a user can only have
   one active reaction per message). Returns 0 on success, -1 on error. */
int db_reaction_set(PGconn *db, int message_id, int user_id, const char *emoji);

/* Removes user_id's reaction from message_id (whatever emoji it was).
   Returns 0 if a reaction was removed, -1 if none existed/on error. */
int db_reaction_remove(PGconn *db, int message_id, int user_id);

/* Fills out with "user_id:emoji" for every reaction on message_id.
   Returns row count, -1 on error. */
int db_reaction_list_for_message(PGconn *db, int message_id, char out[][24], int max_rows);

#endif /* DB_H */
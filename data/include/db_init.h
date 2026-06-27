#ifndef DB_INIT_H
#define DB_INIT_H

#include <libpq-fe.h>

/* Connexion globale — définie dans main.c */
extern PGconn *g_db_conn;

/* Utilisateur connecté — mis à jour après login */
extern int g_current_user_id;

int initializeData(PGconn *conn);

#endif
#ifndef DB_INIT_H
#define DB_INIT_H

#include <libpq-fe.h>

/* Cree le schema et insere les donnees de test.
 * Retourne 0 en succes, -1 en cas d'erreur. */
int initializeData(PGconn *conn);

#endif
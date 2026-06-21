#ifndef DB_INIT_H
#define DB_INIT_H

#include <libpq-fe.h>

int initializeData(PGconn *conn);

#endif
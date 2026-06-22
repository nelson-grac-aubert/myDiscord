#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <libpq-fe.h>
#include "packet.h"

#define SERVER_PORT  8080
#define MAX_CLIENTS  64

/* One entry per connected client */
typedef struct {
    SOCKET  sock;
    char    ip[16];
    int     id;
    int     user_id;   /* -1 until AUTH_LOGIN succeeds */
} ClientInfo;

/* Global server state passed around to all handlers */
typedef struct {
    SOCKET   listen_sock;
    PGconn  *db;
} ServerState;

/* Initialize Winsock, bind the listen socket, connect to the DB */
int  server_init(ServerState *s, int port, const char *conninfo);

/* Blocking accept loop — spawns one thread per client */
void server_run(ServerState *s);

/* Graceful shutdown — closes socket and DB connection */
void server_cleanup(ServerState *s);

#endif /* SERVER_H */
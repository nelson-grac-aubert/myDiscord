#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <libpq-fe.h>
#include "packet.h"

#define SERVER_PORT 8080
#define MAX_CLIENTS 64

/* Matches the seeded rows in the `role` table (data/myDiscord.sql) */
#define ROLE_ADMIN     1
#define ROLE_USER      2
#define ROLE_MODERATOR 3

typedef struct {
    SOCKET sock;
    char ip[16];
    int id;
    int user_id; /* -1 until AUTH_LOGIN succeeds */
    int channel_id; /* current channel, -1 if none */
    char email[150]; /* set once AUTH_LOGIN/AUTH_REGISTER succeeds, used as display name */
    int role_id; /* -1 until AUTH_LOGIN/AUTH_REGISTER succeeds */
} ClientInfo;

/* Thread-safe registry of all currently connected clients */
typedef struct {
    ClientInfo *clients[MAX_CLIENTS];
    int count;
    HANDLE mutex;
} ClientRegistry;

typedef struct {
    SOCKET listen_sock;
    PGconn *db;
    ClientRegistry registry;
} ServerState;

int server_init(ServerState *s, int port, const char *conninfo);
void server_run(ServerState *s);
void server_cleanup(ServerState *s);

/* Registry operations */
int registry_add(ClientRegistry *reg, ClientInfo *client);
void registry_remove(ClientRegistry *reg, ClientInfo *client);

/* Send a packet to all clients in a given channel */
void broadcast_to_channel(ClientRegistry *reg, int channel_id, const Packet *pkt);

/* Send every authenticated, currently-connected client the live, server-wide
   roster of online users (independent of which channel anyone is viewing) */
void broadcast_user_list(ServerState *s);

#endif /* SERVER_H */
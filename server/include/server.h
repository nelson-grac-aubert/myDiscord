#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <libpq-fe.h>
#include "packet.h"

#define SERVER_PORT 8080
#define MAX_CLIENTS 64

typedef struct {
    SOCKET sock;
    char ip[16];
    int id;
    int user_id; /* -1 until AUTH_LOGIN succeeds */
    int channel_id; /* current channel, -1 if none */
    char email[150]; /* set once AUTH_LOGIN/AUTH_REGISTER succeeds, used as display name */
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

#endif /* SERVER_H */
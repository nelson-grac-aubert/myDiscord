#include "../include/server.h"
#include "../include/client_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int init_winsock(void)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return -1;
    }
    return 0;
}

static SOCKET create_listen_socket(int port)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((u_short)port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
        fprintf(stderr, "listen() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

static PGconn *connect_db(const char *conninfo)
{
    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "DB connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }
    printf("[server] connected to database\n");
    return conn;
}

static int registry_init(ClientRegistry *reg)
{
    memset(reg->clients, 0, sizeof(reg->clients));
    reg->count = 0;
    reg->mutex = CreateMutex(NULL, FALSE, NULL);
    return reg->mutex == NULL ? -1 : 0;
}

int registry_add(ClientRegistry *reg, ClientInfo *client)
{
    WaitForSingleObject(reg->mutex, INFINITE);

    int added = 0;
    if (reg->count < MAX_CLIENTS) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (reg->clients[i] == NULL) {
                reg->clients[i] = client;
                reg->count++;
                added = 1;
                break;
            }
        }
    }

    ReleaseMutex(reg->mutex);
    return added ? 0 : -1;
}

void registry_remove(ClientRegistry *reg, ClientInfo *client)
{
    WaitForSingleObject(reg->mutex, INFINITE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (reg->clients[i] == client) {
            reg->clients[i] = NULL;
            reg->count--;
            break;
        }
    }

    ReleaseMutex(reg->mutex);
}

void broadcast_to_channel(ClientRegistry *reg, int channel_id, const Packet *pkt)
{
    char buf[PACKET_MAX_SIZE];
    if (packet_serialize(pkt, buf, sizeof(buf)) != 0)
        return;

    WaitForSingleObject(reg->mutex, INFINITE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        ClientInfo *c = reg->clients[i];
        if (c != NULL && c->channel_id == channel_id)
            send(c->sock, buf, (int)strlen(buf), 0);
    }

    ReleaseMutex(reg->mutex);
}

typedef struct {
    ClientInfo *client;
    ServerState *server;
} ThreadArgs;

static void handle_client_packets(ClientInfo *client, ServerState *s)
{
    char buffer[PACKET_MAX_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client->sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            printf("[server] client #%d disconnected\n", client->id);
            break;
        }

        Packet pkt;
        if (packet_deserialize(buffer, &pkt) != 0) {
            fprintf(stderr, "[server] malformed packet from #%d\n", client->id);
            continue;
        }

        client_handler_dispatch(&pkt, client, s);
    }
}

static DWORD WINAPI client_thread(LPVOID arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    ClientInfo *client = args->client;
    ServerState *s = args->server;
    free(args);

    printf("[server] client #%d connected from %s\n", client->id, client->ip);

    registry_add(&s->registry, client);
    handle_client_packets(client, s);
    registry_remove(&s->registry, client);

    closesocket(client->sock);
    free(client);
    return 0;
}

static ClientInfo *make_client(SOCKET sock, struct sockaddr_in *addr, int id)
{
    ClientInfo *client = malloc(sizeof(ClientInfo));
    if (!client)
        return NULL;

    client->sock = sock;
    client->id = id;
    client->user_id = -1;
    client->channel_id = -1;
    strncpy(client->ip, inet_ntoa(addr->sin_addr), sizeof(client->ip) - 1);
    client->ip[sizeof(client->ip) - 1] = '\0';
    return client;
}

static int spawn_client_thread(ClientInfo *client, ServerState *s)
{
    ThreadArgs *args = malloc(sizeof(ThreadArgs));
    if (!args)
        return -1;

    args->client = client;
    args->server = s;

    HANDLE thread = CreateThread(NULL, 0, client_thread, args, 0, NULL);
    if (thread == NULL) {
        fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
        free(args);
        return -1;
    }

    CloseHandle(thread);
    return 0;
}

int server_init(ServerState *s, int port, const char *conninfo)
{
    if (init_winsock() != 0)
        return -1;

    s->listen_sock = create_listen_socket(port);
    if (s->listen_sock == INVALID_SOCKET) {
        WSACleanup();
        return -1;
    }

    s->db = connect_db(conninfo);
    if (s->db == NULL) {
        closesocket(s->listen_sock);
        WSACleanup();
        return -1;
    }

    if (registry_init(&s->registry) != 0) {
        PQfinish(s->db);
        closesocket(s->listen_sock);
        WSACleanup();
        return -1;
    }

    printf("[server] listening on port %d\n", port);
    return 0;
}

void server_run(ServerState *s)
{
    static int next_id = 0;

    while (1) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);

        SOCKET client_sock = accept(s->listen_sock,
                                    (struct sockaddr *)&client_addr,
                                    &addr_len);
        if (client_sock == INVALID_SOCKET) {
            fprintf(stderr, "accept() failed: %d\n", WSAGetLastError());
            continue;
        }

        ClientInfo *client = make_client(client_sock, &client_addr, ++next_id);
        if (!client) {
            closesocket(client_sock);
            continue;
        }

        if (spawn_client_thread(client, s) != 0) {
            closesocket(client_sock);
            free(client);
        }
    }
}

void server_cleanup(ServerState *s)
{
    CloseHandle(s->registry.mutex);
    PQfinish(s->db);
    closesocket(s->listen_sock);
    WSACleanup();
    printf("[server] shutdown\n");
}
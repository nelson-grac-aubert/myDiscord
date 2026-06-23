#include "../include/server.h"
#include "../include/client_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

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

    handle_client_packets(client, s);

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
        fprintf(stderr, "CreateThread failed: %d\n", GetLastError());
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
    PQfinish(s->db);
    closesocket(s->listen_sock);
    WSACleanup();
    printf("[server] shutdown\n");
}
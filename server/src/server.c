#include "../include/server.h"
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

    /* Allow immediate reuse of the port after restart */
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((u_short)port);

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

/* Each client thread receives this — freed by the thread on exit */
typedef struct {
    ClientInfo  *client;
    ServerState *server;
} ThreadArgs;

static DWORD WINAPI client_thread(LPVOID arg)
{
    ThreadArgs  *args   = (ThreadArgs *)arg;
    ClientInfo  *client = args->client;
    ServerState *s      = args->server;
    free(args);

    char buffer[PACKET_MAX_SIZE];
    int  bytes;

    printf("[server] client #%d connected from %s\n", client->id, client->ip);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(client->sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            printf("[server] client #%d disconnected\n", client->id);
            break;
        }

        Packet pkt;
        if (packet_deserialize(buffer, &pkt) != 0) {
            fprintf(stderr, "[server] malformed packet from #%d\n", client->id);
            continue;
        }

        /* Placeholder — client_handler_dispatch() comes next */
        printf("[server] packet type=%d from client #%d\n",
               pkt.type, client->id);
    }

    closesocket(client->sock);
    free(client);
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

        /* ClientInfo is freed by the thread when the client disconnects */
        ClientInfo *client = malloc(sizeof(ClientInfo));
        client->sock    = client_sock;
        client->id      = ++next_id;
        client->user_id = -1;
        strncpy(client->ip, inet_ntoa(client_addr.sin_addr),
                sizeof(client->ip) - 1);

        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->client = client;
        args->server = s;

        HANDLE thread = CreateThread(NULL, 0, client_thread, args, 0, NULL);
        if (thread == NULL) {
            fprintf(stderr, "CreateThread failed: %d\n", GetLastError());
            closesocket(client_sock);
            free(client);
            free(args);
            continue;
        }

        /* Thread manages itself — we don't need the handle */
        CloseHandle(thread);
    }
}

void server_cleanup(ServerState *s)
{
    PQfinish(s->db);
    closesocket(s->listen_sock);
    WSACleanup();
    printf("[server] shutdown\n");
}
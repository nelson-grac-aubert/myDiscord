#include "../include/client_socket.h"
#include <stdio.h>
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

static SOCKET connect_to_server(const char *ip, int port)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "socket() failed: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((u_short)port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "connect() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

/* Listener thread : blocks on recv(), parses packets, calls the callback */
static DWORD WINAPI listener_thread(LPVOID arg)
{
    ClientSocket *cs = (ClientSocket *)arg;
    char          buffer[PACKET_MAX_SIZE];
    int           bytes;

    while (cs->running) {
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(cs->sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            /* Server closed connection or error */
            printf("[socket] disconnected from server\n");
            cs->running = 0;
            break;
        }

        /* Parse the raw string into a Packet */
        Packet pkt;
        if (packet_deserialize(buffer, &pkt) != 0) {
            fprintf(stderr, "[socket] malformed packet: %s\n", buffer);
            continue;
        }

        /* Forward to the callback if one was registered */
        if (cs->on_packet != NULL)
            cs->on_packet(&pkt);
    }

    return 0;
}

int client_socket_connect(ClientSocket *cs, const char *ip, int port,
                          PacketCallback on_packet)
{
    if (init_winsock() != 0)
        return -1;

    cs->sock            = connect_to_server(ip, port);
    cs->listener_thread = NULL;
    cs->on_packet       = on_packet;
    cs->running         = 0;

    if (cs->sock == INVALID_SOCKET) {
        WSACleanup();
        return -1;
    }

    printf("[socket] connected to %s:%d\n", ip, port);
    return 0;
}

int client_socket_start_listener(ClientSocket *cs)
{
    cs->running = 1;

    cs->listener_thread = CreateThread(NULL, 0, listener_thread, cs, 0, NULL);
    if (cs->listener_thread == NULL) {
        fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
        cs->running = 0;
        return -1;
    }

    /* We don't need to join it : it runs until the connection drops */
    CloseHandle(cs->listener_thread);
    return 0;
}

int client_socket_send(ClientSocket *cs, const Packet *pkt)
{
    char raw[PACKET_MAX_SIZE];

    if (packet_serialize(pkt, raw, sizeof(raw)) != 0) {
        fprintf(stderr, "[socket] serialize failed\n");
        return -1;
    }

    int sent = send(cs->sock, raw, strlen(raw), 0);
    if (sent == SOCKET_ERROR) {
        fprintf(stderr, "[socket] send() failed: %d\n", WSAGetLastError());
        return -1;
    }

    return 0;
}

void client_socket_disconnect(ClientSocket *cs)
{
    cs->running = 0;
    closesocket(cs->sock);
    WSACleanup();
    printf("[socket] disconnected\n");
}
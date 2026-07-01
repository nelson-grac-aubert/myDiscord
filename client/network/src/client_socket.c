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

/* Reassembly buffer size : must hold several queued packets, since a burst
   of back-to-back sends (e.g. MSG_HISTORY) can be coalesced into one recv() */
#define RECV_ACCUM_SIZE (PACKET_MAX_SIZE * 16)

/* Listener thread : blocks on recv(), reassembles the TCP byte stream into
   newline-delimited packets, and calls the callback for each complete one.
   TCP has no message boundaries, so a single recv() call can return zero,
   one, or several whole/partial packets - each recv() must NOT be assumed
   to correspond to exactly one packet. */
static DWORD WINAPI listener_thread(LPVOID arg)
{
    ClientSocket *cs = (ClientSocket *)arg;
    char          accum[RECV_ACCUM_SIZE];
    int           accum_len = 0;
    char          recv_buf[PACKET_MAX_SIZE];
    int           bytes;

    while (cs->running) {
        bytes = recv(cs->sock, recv_buf, sizeof(recv_buf) - 1, 0);

        if (bytes <= 0) {
            /* Server closed connection or error */
            printf("[socket] disconnected from server\n");
            cs->running = 0;
            break;
        }

        if (accum_len + bytes >= (int)sizeof(accum)) {
            fprintf(stderr, "[socket] reassembly buffer overflow, dropping backlog\n");
            accum_len = 0;
            continue;
        }

        memcpy(accum + accum_len, recv_buf, bytes);
        accum_len += bytes;
        accum[accum_len] = '\0';

        /* Process every complete '\n'-terminated packet currently buffered */
        char *line_start = accum;
        char *nl;
        while ((nl = memchr(line_start, '\n', (accum + accum_len) - line_start)) != NULL) {
            *nl = '\0';

            Packet pkt;
            if (packet_deserialize(line_start, &pkt) != 0)
                fprintf(stderr, "[socket] malformed packet: %s\n", line_start);
            else if (cs->on_packet != NULL)
                cs->on_packet(&pkt);

            line_start = nl + 1;
        }

        /* Keep any trailing partial packet buffered for the next recv() */
        int leftover = (accum + accum_len) - line_start;
        memmove(accum, line_start, leftover);
        accum_len = leftover;
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
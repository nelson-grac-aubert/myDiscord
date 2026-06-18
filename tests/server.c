#include <winsock2.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT        8080
#define BUFFER_SIZE 256

static int init_winsock(void)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }
    return 0;
}

static SOCKET create_listen_socket(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, 3) == SOCKET_ERROR) {
        printf("listen() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

int main(void)
{
    if (init_winsock() != 0)
        return 1;

    SOCKET server = create_listen_socket();
    if (server == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);
    printf("Waiting for clients (Ctrl+C to stop)\n\n");

    while (1) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);

        SOCKET client = accept(server, (struct sockaddr *)&client_addr, &addr_len);
        if (client == INVALID_SOCKET) {
            printf("accept() failed: %d\n", WSAGetLastError());
            continue;
        }

        // Print who connected
        printf(">> %s connected\n", inet_ntoa(client_addr.sin_addr));

        // Receive message
        char buffer[BUFFER_SIZE] = {0};
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            printf(">> received: %s\n", buffer);
            send(client, "PONG\n", 5, 0);
            printf(">> sent: PONG\n\n");
        }

        closesocket(client);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
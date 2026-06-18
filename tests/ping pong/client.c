#include <winsock2.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT        8080
#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: client.exe <server_ip>\n");
        printf("Example: client.exe 10.10.7.209\n");
        return 1;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    printf("Connecting to %s:%d...\n", argv[1], PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("connect() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected!\n");

    // Send PING
    send(sock, "PING\n", 5, 0);
    printf("Sent: PING\n");

    // Wait for PONG
    char buffer[BUFFER_SIZE] = {0};
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
        printf("Received: %s\n", buffer);
    else
        printf("No response from server\n");

    closesocket(sock);
    WSACleanup();
    return 0;
}
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT         8080
#define BUFFER_SIZE  256
#define MAX_CLIENTS  64

// Data passed to each client thread
typedef struct {
    SOCKET      socket;
    char        ip[16];
    int         id;
} ClientInfo;

static int client_count = 0;

static DWORD WINAPI handle_client(LPVOID arg)
{
    ClientInfo *info = (ClientInfo *)arg;
    char        buffer[BUFFER_SIZE];
    int         bytes;

    printf("[thread %d] %s connected\n", info->id, info->ip);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(info->socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes <= 0) {
            printf("[thread %d] %s disconnected\n", info->id, info->ip);
            break;
        }

        printf("[thread %d] %s says: %s\n", info->id, info->ip, buffer);

        // Echo PONG back
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "PONG from server (you are client #%d)\n", info->id);
        send(info->socket, response, strlen(response), 0);
    }

    closesocket(info->socket);
    free(info);
    return 0;
}

static SOCKET create_listen_socket(void)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    // Allow reusing the port immediately after restart
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("listen() failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

int main(void)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

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

        // Prepare info passed to the thread
        ClientInfo *info = malloc(sizeof(ClientInfo));
        info->socket = client;
        info->id     = ++client_count;
        strncpy(info->ip, inet_ntoa(client_addr.sin_addr), sizeof(info->ip) - 1);

        // Spawn a dedicated thread — server loop is immediately free again
        HANDLE thread = CreateThread(NULL, 0, handle_client, info, 0, NULL);
        if (thread == NULL) {
            printf("CreateThread failed: %d\n", GetLastError());
            closesocket(client);
            free(info);
            continue;
        }

        // We don't need the handle, the thread manages itself
        CloseHandle(thread);
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
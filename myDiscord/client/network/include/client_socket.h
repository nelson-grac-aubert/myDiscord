#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <winsock2.h>
#include <windows.h>
#include "../include/packet.h"

/* Callback type : called from the listener thread on each incoming packet */
typedef void (*PacketCallback)(const Packet *pkt);

typedef struct {
    SOCKET         sock;
    HANDLE         listener_thread;
    PacketCallback on_packet;    /* called whenever a packet arrives */
    int            running;      /* set to 0 to stop the listener thread */
} ClientSocket;

/*
 * Initialize Winsock and connect to the server.
 * on_packet : function called each time a packet is received (can be NULL).
 * Returns 0 on success, -1 on failure.
 */
int  client_socket_connect(ClientSocket *cs, const char *ip, int port,
                           PacketCallback on_packet);

/*
 * Start the background listener thread.
 * Must be called after client_socket_connect().
 * Returns 0 on success, -1 on failure.
 */
int  client_socket_start_listener(ClientSocket *cs);

/*
 * Serialize and send a packet to the server.
 * Returns 0 on success, -1 on failure.
 */
int  client_socket_send(ClientSocket *cs, const Packet *pkt);

/*
 * Stop the listener thread and close the connection.
 */
void client_socket_disconnect(ClientSocket *cs);

#endif /* CLIENT_SOCKET_H */
#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "server.h"

/* Entry point: parse packet type and route to the correct handler */
void client_handler_dispatch(const Packet *pkt, ClientInfo *client, ServerState *s);

/* Auth */
void handler_register(const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_login   (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_logout  (const Packet *pkt, ClientInfo *client, ServerState *s);

/* Messaging */
void handler_msg_send   (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_msg_history(const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_reaction   (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_msg_delete (const Packet *pkt, ClientInfo *client, ServerState *s);

/* Channels */
void handler_channel_list  (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_channel_create(const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_channel_delete(const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_channel_join  (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_channel_leave (const Packet *pkt, ClientInfo *client, ServerState *s);

/* Users */
void handler_user_list  (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_user_ban   (const Packet *pkt, ClientInfo *client, ServerState *s);
void handler_user_unban (const Packet *pkt, ClientInfo *client, ServerState *s);

#endif /* CLIENT_HANDLER_H */
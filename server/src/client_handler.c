#include "../include/client_handler.h"
#include <stdio.h>
#include <string.h>

static int send_packet(ClientInfo *client, const Packet *pkt)
{
    char buf[PACKET_MAX_SIZE];
    if (packet_serialize(pkt, buf, sizeof(buf)) != 0)
        return -1;
    return send(client->sock, buf, (int)strlen(buf), 0);
}

static void reply_ok(ClientInfo *client, const char *msg)
{
    Packet pkt;
    packet_build(&pkt, SERVER_OK, 1, msg);
    send_packet(client, &pkt);
}

static void reply_error(ClientInfo *client, const char *reason)
{
    Packet pkt;
    packet_build(&pkt, SERVER_ERROR, 1, reason);
    send_packet(client, &pkt);
}

void client_handler_dispatch(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    switch (pkt->type) {
        case AUTH_REGISTER: handler_register(pkt, client, s); break;
        case AUTH_LOGIN: handler_login(pkt, client, s); break;
        case AUTH_LOGOUT: handler_logout(pkt, client, s); break;
        case MSG_SEND: handler_msg_send(pkt, client, s); break;
        case MSG_HISTORY: handler_msg_history(pkt, client, s); break;
        case MSG_REACTION: handler_reaction(pkt, client, s); break;
        case CHANNEL_LIST: handler_channel_list(pkt, client, s); break;
        case CHANNEL_CREATE: handler_channel_create(pkt, client, s); break;
        case CHANNEL_DELETE: handler_channel_delete(pkt, client, s); break;
        case CHANNEL_JOIN: handler_channel_join(pkt, client, s); break;
        case CHANNEL_LEAVE: handler_channel_leave(pkt, client, s); break;
        case USER_LIST: handler_user_list(pkt, client, s); break;
        case USER_BAN: handler_user_ban(pkt, client, s); break;
        default:
            fprintf(stderr, "[handler] unknown packet type %d\n", pkt->type);
            reply_error(client, "unknown packet type");
            break;
    }
}

void handler_register(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: validate fields, hash password, insert user into DB */
    printf("[handler] REGISTER from client #%d\n", client->id);
    reply_ok(client, "register stub");
}

void handler_login(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: look up user in DB, verify password hash, set client->user_id */
    printf("[handler] LOGIN from client #%d\n", client->id);
    reply_ok(client, "login stub");
}

void handler_logout(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: clear session, notify other clients */
    printf("[handler] LOGOUT from client #%d\n", client->id);
    client->user_id = -1;
    reply_ok(client, "logout stub");
}

void handler_msg_send(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: persist message to DB, broadcast SERVER_PUSH to channel members */
    printf("[handler] MSG_SEND from client #%d\n", client->id);
    reply_ok(client, "msg_send stub");
}

void handler_msg_history(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: query last N messages in channel, send back as SERVER_PUSH packets */
    printf("[handler] MSG_HISTORY from client #%d\n", client->id);
    reply_ok(client, "msg_history stub");
}

void handler_reaction(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: persist reaction to DB, broadcast to channel members */
    printf("[handler] REACTION from client #%d\n", client->id);
    reply_ok(client, "reaction stub");
}

void handler_channel_list(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: query channels accessible by client->user_id */
    printf("[handler] CHANNEL_LIST from client #%d\n", client->id);
    reply_ok(client, "channel_list stub");
}

void handler_channel_create(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: insert channel into DB, broadcast update to connected clients */
    printf("[handler] CHANNEL_CREATE from client #%d\n", client->id);
    reply_ok(client, "channel_create stub");
}

void handler_channel_delete(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: check permissions, delete channel from DB, notify members */
    printf("[handler] CHANNEL_DELETE from client #%d\n", client->id);
    reply_ok(client, "channel_delete stub");
}

void handler_channel_join(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: add user to channel membership in DB */
    printf("[handler] CHANNEL_JOIN from client #%d\n", client->id);
    reply_ok(client, "channel_join stub");
}

void handler_channel_leave(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: remove user from channel membership in DB */
    printf("[handler] CHANNEL_LEAVE from client #%d\n", client->id);
    reply_ok(client, "channel_leave stub");
}

void handler_user_list(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: query users in given channel */
    printf("[handler] USER_LIST from client #%d\n", client->id);
    reply_ok(client, "user_list stub");
}

void handler_user_ban(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    /* TODO: check admin permissions, set banned flag in DB, disconnect target */
    printf("[handler] USER_BAN from client #%d\n", client->id);
    reply_ok(client, "user_ban stub");
}
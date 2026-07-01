#include "../include/client_handler.h"
#include "../include/db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

static int client_is_authenticated(ClientInfo *client)
{
    return client->user_id != -1;
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
        case MSG_DELETE: handler_msg_delete(pkt, client, s); break;
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
    if (pkt->field_count < 4) {
        reply_error(client, "register: missing fields");
        return;
    }

    const char *email      = pkt->fields[0];
    const char *first_name = pkt->fields[1];
    const char *last_name  = pkt->fields[2];
    const char *password   = pkt->fields[3];

    int user_id = db_user_register(s->db, email, first_name, last_name, password, 1);
    if (user_id == -1) {
        reply_error(client, "register: email already taken or db error");
        return;
    }

    client->user_id = user_id;
    strncpy(client->email, email, sizeof(client->email) - 1);
    client->email[sizeof(client->email) - 1] = '\0';
    printf("[handler] REGISTER ok: client #%d is now user %d\n", client->id, user_id);

    char resp[32];
    snprintf(resp, sizeof(resp), "%d", user_id);
    reply_ok(client, resp);
    broadcast_user_list(s);
}

void handler_login(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (pkt->field_count < 2) {
        reply_error(client, "login: missing fields");
        return;
    }

    const char *email    = pkt->fields[0];
    const char *password = pkt->fields[1];

    int user_id = db_user_login(s->db, email, password);
    if (user_id == -1) {
        reply_error(client, "login: invalid credentials");
        return;
    }

    if (db_user_is_banned(s->db, user_id)) {
        reply_error(client, "login: account banned");
        return;
    }

    client->user_id = user_id;
    strncpy(client->email, email, sizeof(client->email) - 1);
    client->email[sizeof(client->email) - 1] = '\0';
    printf("[handler] LOGIN ok: client #%d is now user %d\n", client->id, user_id);

    char resp[32];
    snprintf(resp, sizeof(resp), "%d", user_id);
    reply_ok(client, resp);
    broadcast_user_list(s);
}

void handler_logout(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt;
    printf("[handler] LOGOUT: user %d\n", client->user_id);
    client->user_id    = -1;
    client->channel_id = -1;
    reply_ok(client, "logged out");
    broadcast_user_list(s);
}

void handler_msg_send(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "msg_send: not authenticated");
        return;
    }

    if (pkt->field_count < 2) {
        reply_error(client, "msg_send: missing fields");
        return;
    }

    if (client->channel_id == -1) {
        reply_error(client, "msg_send: not in a channel");
        return;
    }

    /* fields[0] = channel_id (ignored, we use client->channel_id)
       fields[1] = content */
    const char *content = pkt->fields[1];

    char timestamp[8];
    int message_id = db_message_insert(s->db, client->user_id, client->channel_id,
                                       content, timestamp);
    if (message_id == -1) {
        reply_error(client, "msg_send: db error");
        return;
    }

    /* Broadcast "channel_id|message_id|HH:MM|sender_email|content" so
       receivers can route the message to the right channel, show who
       actually sent it and when (including to the sender's own client),
       and dedupe it against the same message re-arriving via a later
       MSG_HISTORY fetch */
    char push_payload[PACKET_FIELD_SIZE];
    snprintf(push_payload, PACKET_FIELD_SIZE, "%d|%d|%s|%s|%.320s",
             client->channel_id, message_id, timestamp, client->email, content);

    Packet push;
    packet_build(&push, SERVER_PUSH, 1, push_payload);
    broadcast_to_channel(&s->registry, client->channel_id, &push);
}

void handler_msg_delete(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "msg_delete: not authenticated");
        return;
    }

    if (pkt->field_count < 1) {
        reply_error(client, "msg_delete: missing message_id");
        return;
    }

    int message_id = atoi(pkt->fields[0]);
    int channel_id = -1;
    if (db_message_delete(s->db, message_id, client->user_id, &channel_id) != 0) {
        reply_error(client, "msg_delete: forbidden or not found");
        return;
    }

    reply_ok(client, "deleted");

    /* Tell everyone currently in that channel (including the requester) to
       drop the message, rather than removing it locally before the server
       confirms - keeps every client's view consistent */
    char payload[32];
    snprintf(payload, sizeof(payload), "DELMSG:%d", message_id);
    Packet push;
    packet_build(&push, SERVER_PUSH, 1, payload);
    broadcast_to_channel(&s->registry, channel_id, &push);
}

void handler_msg_history(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "msg_history: not authenticated");
        return;
    }

    if (pkt->field_count < 2) {
        reply_error(client, "msg_history: missing fields");
        return;
    }

    int channel_id = atoi(pkt->fields[0]);
    int limit      = atoi(pkt->fields[1]);

    char rows[50][512];
    int count = db_message_history(s->db, channel_id, limit, rows, 50);
    if (count < 0) {
        reply_error(client, "msg_history: db error");
        return;
    }

    for (int i = 0; i < count; i++) {
        Packet push;
        packet_build(&push, SERVER_PUSH, 1, rows[i]);
        send_packet(client, &push);
    }

    reply_ok(client, "msg_history: done");
}

void handler_reaction(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt; (void)s;
    if (!client_is_authenticated(client)) {
        reply_error(client, "reaction: not authenticated");
        return;
    }
    reply_ok(client, "reaction stub");
}

void handler_channel_list(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt;
    if (!client_is_authenticated(client)) {
        reply_error(client, "channel_list: not authenticated");
        return;
    }

    char rows[64][100];
    int count = db_channel_list(s->db, client->user_id, rows, 64);
    if (count < 0) {
        reply_error(client, "channel_list: db error");
        return;
    }

    for (int i = 0; i < count; i++) {
        /* Prefix with "CHAN:" so client can distinguish from message pushes */
        char prefixed[105]; /* rows[i] is max 100 chars, "CHAN:" adds 5 */
        snprintf(prefixed, sizeof(prefixed), "CHAN:%.99s", rows[i]);
        Packet push;
        packet_build(&push, SERVER_PUSH, 1, prefixed);
        send_packet(client, &push);
    }

    reply_ok(client, "channel_list: done");
}

void handler_channel_create(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "channel_create: not authenticated");
        return;
    }

    if (pkt->field_count < 2) {
        reply_error(client, "channel_create: missing fields");
        return;
    }

    int is_private = atoi(pkt->fields[1]);
    int channel_id = db_channel_create(s->db, pkt->fields[0], is_private, client->user_id);
    if (channel_id == -1) {
        reply_error(client, "channel_create: db error");
        return;
    }

    char resp[16];
    snprintf(resp, sizeof(resp), "%d", channel_id);
    reply_ok(client, resp);
}

void handler_channel_delete(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "channel_delete: not authenticated");
        return;
    }

    if (pkt->field_count < 1) {
        reply_error(client, "channel_delete: missing channel_id");
        return;
    }

    int channel_id = atoi(pkt->fields[0]);
    int ok = db_channel_delete(s->db, channel_id, client->user_id);
    if (ok != 0) {
        reply_error(client, "channel_delete: forbidden or db error");
        return;
    }

    reply_ok(client, "channel deleted");
}

void handler_channel_join(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "channel_join: not authenticated");
        return;
    }

    if (pkt->field_count < 1) {
        reply_error(client, "channel_join: missing channel_id");
        return;
    }

    int channel_id = atoi(pkt->fields[0]);
    db_channel_join(s->db, client->user_id, channel_id);
    client->channel_id = channel_id;

    printf("[handler] user %d joined channel %d\n", client->user_id, channel_id);
    reply_ok(client, "joined");
}

void handler_channel_leave(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "channel_leave: not authenticated");
        return;
    }

    if (pkt->field_count < 1) {
        reply_error(client, "channel_leave: missing channel_id");
        return;
    }

    int channel_id = atoi(pkt->fields[0]);
    db_channel_leave(s->db, client->user_id, channel_id);

    if (client->channel_id == channel_id)
        client->channel_id = -1;

    reply_ok(client, "left");
}

void handler_user_list(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    (void)pkt;
    if (!client_is_authenticated(client)) {
        reply_error(client, "user_list: not authenticated");
        return;
    }

    broadcast_user_list(s);
    reply_ok(client, "user_list: done");
}

void handler_user_ban(const Packet *pkt, ClientInfo *client, ServerState *s)
{
    if (!client_is_authenticated(client)) {
        reply_error(client, "user_ban: not authenticated");
        return;
    }

    if (pkt->field_count < 2) {
        reply_error(client, "user_ban: missing fields");
        return;
    }

    int target_id      = atoi(pkt->fields[0]);
    const char *reason = pkt->fields[1];

    int ok = db_user_ban(s->db, target_id, client->user_id, reason);
    if (ok != 0) {
        reply_error(client, "user_ban: db error");
        return;
    }

    printf("[handler] user %d banned user %d\n", client->user_id, target_id);
    reply_ok(client, "banned");
}
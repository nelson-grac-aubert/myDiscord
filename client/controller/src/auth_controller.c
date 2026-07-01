#include "../include/auth_controller.h"
#include "../../network/include/client_socket.h"
#include "../../network/include/packet.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ClientSocket g_client_socket;

static AuthResult g_result  = AUTH_RESULT_PENDING;
static char g_error[512]    = {0};
static int g_user_id        = -1;
static int g_role_id        = -1;
static int g_auth_done      = 0; /* 1 once login/register succeeded */

static PacketCallback g_chat_callback = NULL;

void auth_controller_set_chat_callback(PacketCallback cb)
{
    g_chat_callback = cb;
}

static void on_packet_received(const Packet *pkt)
{
    /* During auth phase, handle SERVER_OK/ERROR ourselves */
    if (!g_auth_done) {
        if (pkt->type == SERVER_OK) {
            g_user_id  = pkt->field_count > 0 ? atoi(pkt->fields[0]) : -1;
            g_role_id  = pkt->field_count > 1 ? atoi(pkt->fields[1]) : -1;
            g_result   = AUTH_RESULT_OK;
            g_auth_done = 1;
            printf("[auth] ok, user_id=%d, role_id=%d\n", g_user_id, g_role_id);
            return;
        } else if (pkt->type == SERVER_ERROR) {
            strncpy(g_error,
                    pkt->field_count > 0 ? pkt->fields[0] : "unknown error",
                    sizeof(g_error) - 1);
            g_error[sizeof(g_error) - 1] = '\0';
            g_result = AUTH_RESULT_ERROR;
            printf("[auth] error: %s\n", g_error);
            return;
        }
    }

    /* Once authenticated, forward everything to chat controller */
    if (g_chat_callback)
        g_chat_callback(pkt);
}

int auth_controller_connect(const char *ip, int port)
{
    g_result    = AUTH_RESULT_PENDING;
    g_user_id   = -1;
    g_role_id   = -1;
    g_auth_done = 0;
    g_error[0]  = '\0';

    if (client_socket_connect(&g_client_socket, ip, port, on_packet_received) != 0)
        return -1;

    return client_socket_start_listener(&g_client_socket);
}

void auth_controller_login(UIState *state)
{
    g_result    = AUTH_RESULT_PENDING;
    g_auth_done = 0;

    Packet pkt;
    packet_build(&pkt, AUTH_LOGIN, 2, state->text_email, state->text_password);
    client_socket_send(&g_client_socket, &pkt);
}

void auth_controller_register(UIState *state)
{
    g_result    = AUTH_RESULT_PENDING;
    g_auth_done = 0;

    Packet pkt;
    packet_build(&pkt, AUTH_REGISTER, 4,
                 state->text_email,
                 state->text_username,
                 state->text_username,
                 state->text_password);
    client_socket_send(&g_client_socket, &pkt);
}

AuthResult auth_controller_get_result(void)    { return g_result; }
int auth_controller_get_user_id(void)          { return g_user_id; }
int auth_controller_get_role_id(void)          { return g_role_id; }

const char *auth_controller_get_error(void)
{
    return g_error[0] != '\0' ? g_error : NULL;
}

void auth_controller_disconnect(void)
{
    g_auth_done = 0;
    client_socket_disconnect(&g_client_socket);
}
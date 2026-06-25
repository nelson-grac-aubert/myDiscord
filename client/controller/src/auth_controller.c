#include "../include/auth_controller.h"
#include "../../network/include/client_socket.h"
#include "../../network/include/packet.h"
#include <stdio.h>
#include <string.h>

static ClientSocket g_socket;
static AuthResult g_result = AUTH_RESULT_PENDING;
static char g_error[512] = {0};
static int g_user_id = -1;

static void on_packet_received(const Packet *pkt)
{
    if (pkt->type == SERVER_OK) {
        g_user_id = pkt->field_count > 0 ? atoi(pkt->fields[0]) : -1;
        g_result = AUTH_RESULT_OK;
        printf("[auth] login/register ok, user_id=%d\n", g_user_id);
    } else if (pkt->type == SERVER_ERROR) {
        strncpy(g_error, pkt->field_count > 0 ? pkt->fields[0] : "unknown error", sizeof(g_error) - 1);
        g_error[sizeof(g_error) - 1] = '\0';
        g_result = AUTH_RESULT_ERROR;
        printf("[auth] error: %s\n", g_error);
    }
}

int auth_controller_connect(const char *ip, int port)
{
    g_result  = AUTH_RESULT_PENDING;
    g_user_id = -1;
    g_error[0] = '\0';

    if (client_socket_connect(&g_socket, ip, port, on_packet_received) != 0)
        return -1;

    return client_socket_start_listener(&g_socket);
}

void auth_controller_login(UIState *state)
{
    g_result = AUTH_RESULT_PENDING;

    Packet pkt;
    packet_build(&pkt, AUTH_LOGIN, 2, state->text_email, state->text_password);
    client_socket_send(&g_socket, &pkt);
}

void auth_controller_register(UIState *state)
{
    g_result = AUTH_RESULT_PENDING;

    /* fields: email, first_name, last_name, password
       username field used as first_name for now */
    Packet pkt;
    packet_build(&pkt, AUTH_REGISTER, 4,
                 state->text_email,
                 state->text_username,
                 state->text_username,
                 state->text_password);
    client_socket_send(&g_socket, &pkt);
}

AuthResult auth_controller_get_result(void)
{
    return g_result;
}

const char *auth_controller_get_error(void)
{
    return g_error[0] != '\0' ? g_error : NULL;
}

int auth_controller_get_user_id(void)
{
    return g_user_id;
}

void auth_controller_disconnect(void)
{
    client_socket_disconnect(&g_socket);
}
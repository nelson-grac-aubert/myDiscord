#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "client_socket.h"
#include "ui_login.h"

/* Shared socket used by all controllers */
extern ClientSocket g_client_socket;

typedef enum {
    AUTH_RESULT_PENDING,
    AUTH_RESULT_OK,
    AUTH_RESULT_ERROR
} AuthResult;

int auth_controller_connect(const char *ip, int port);
void auth_controller_login(UIState *state);
void auth_controller_register(UIState *state);

/* Register a secondary callback for non-auth packets (SERVER_PUSH etc.) */
void auth_controller_set_chat_callback(PacketCallback cb);

AuthResult auth_controller_get_result(void);
const char *auth_controller_get_error(void);
int auth_controller_get_user_id(void);
void auth_controller_disconnect(void);

#endif /* AUTH_CONTROLLER_H */
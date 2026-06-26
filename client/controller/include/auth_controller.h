#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "ui_login.h"

typedef enum {
    AUTH_RESULT_PENDING,
    AUTH_RESULT_OK,
    AUTH_RESULT_ERROR
} AuthResult;

/* Connect to the server. Must be called before login or register. */
int auth_controller_connect(const char *ip, int port);

/* Send AUTH_LOGIN to the server with the current UI state credentials.
   Returns AUTH_RESULT_PENDING immediately, result arrives via callback. */
void auth_controller_login(UIState *state);

/* Send AUTH_REGISTER to the server with the current UI state credentials. */
void auth_controller_register(UIState *state);

/* Returns the latest result from the server (polled each frame). */
AuthResult auth_controller_get_result(void);

/* Returns the last error message from the server, or NULL if none. */
const char *auth_controller_get_error(void);

/* Returns the user_id received after a successful login/register. */
int auth_controller_get_user_id(void);

/* Disconnect from the server. */
void auth_controller_disconnect(void);

#endif /* AUTH_CONTROLLER_H */
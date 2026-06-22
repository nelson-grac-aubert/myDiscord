#include "auth_controller.h"
#include "../model/include/user.h"
#include <string.h>

int auth_controller_login(const char *email, const char *password, User **out_user) {
    // TEMPORAIRE : En attendant que le pote du réseau connecte le vrai socket,
    // on simule une réponse positive si l'email contient "@"
    if (strchr(email, '@') != NULL) {
        // *out_user = user_create("UtilisateurTest", email, "hash", 1);
        return 1; // Succès de la connexion
    }
    return 0; // Échec
}
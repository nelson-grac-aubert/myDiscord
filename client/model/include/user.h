#ifndef USER_MODEL_H
#define USER_MODEL_H

#define MAX_USERS 64

typedef struct {
    int id;
    char username[32];
    int is_online;
} User;

// Initialise la liste des utilisateurs connectés
void user_model_init(void);

// Copie la liste des utilisateurs en ligne dans out_users
int user_model_get_online(User* out_users, int max_out);

// Gestion de la liste des utilisateurs
void user_model_add(int id, const char* username, int is_online);
void user_model_set_status(int id, int is_online);

#endif /* USER_MODEL_H */
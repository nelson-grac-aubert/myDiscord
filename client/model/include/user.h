#ifndef USER_MODEL_H
#define USER_MODEL_H

#define MAX_USERS 64

typedef struct {
    int id;
    char username[32];
    int is_online;
} User;

// Initializes the connected users list
void user_model_init(void);

// Copies the list of online users into out_users
int user_model_get_online(User* out_users, int max_out);

// Copies the list of offline (registered but not connected) users into out_users
int user_model_get_offline(User* out_users, int max_out);

// User list management
void user_model_add(int id, const char* username, int is_online);
void user_model_set_status(int id, int is_online);

#endif /* USER_MODEL_H */
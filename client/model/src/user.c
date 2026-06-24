#include "../include/user.h"
#include <string.h>

static User local_users[MAX_USERS];
static int local_user_count = 0;

void user_model_init(void) {
    local_user_count = 0;
    
    // Simulation des membres présents dans la sidebar
    user_model_add(1, "Nexus_One", 1);
    user_model_add(2, "SARA_X", 1);
    user_model_add(3, "AlexK", 1);
}

int user_model_get_online(User* out_users, int max_out) {
    int count = 0;
    for (int i = 0; i < local_user_count; i++) {
        if (local_users[i].is_online) {
            if (count < max_out) {
                out_users[count] = local_users[i];
                count++;
            }
        }
    }
    return count;
}

void user_model_add(int id, const char* username, int is_online) {
    if (local_user_count >= MAX_USERS) return;

    User *u = &local_users[local_user_count];
    u->id = id;
    strncpy(u->username, username, 31);
    u->username[31] = '\0';
    u->is_online = is_online;

    local_user_count++;
}

void user_model_set_status(int id, int is_online) {
    for (int i = 0; i < local_user_count; i++) {
        if (local_users[i].id == id) {
            local_users[i].is_online = is_online;
            break;
        }
    }
}
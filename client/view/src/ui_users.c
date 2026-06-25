#include "../include/ui_users.h"
#include "user.h"
#include "../include/ui_login.h" // For draw_text
#include <stdio.h>

void users_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, TTF_Font *font_sub, SDL_Color green_online, SDL_Color dark_gray) {
    if (layout->sidebar_members.w > 0) {
        User online_users[MAX_USERS];
        int count = user_model_get_online(online_users, MAX_USERS);

        char total_str[32];
        snprintf(total_str, sizeof(total_str), "ONLINE — %d", count);
        draw_text(renderer, font_sub, total_str, layout->sidebar_members.x + 20, 20, dark_gray);

        int user_y = 55;
        for (int i = 0; i < count; i++) {
            char name_str[64];
            snprintf(name_str, sizeof(name_str), "●  %s", online_users[i].username);
            draw_text(renderer, font_main, name_str, layout->sidebar_members.x + 20, user_y, green_online);
            user_y += 30;
        }
    }
}
#include "../include/ui_users.h"
#include "user.h"
#include "../include/ui_login.h" // For draw_text
#include <stdio.h>

void users_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, TTF_Font *font_sub, SDL_Color green_online, SDL_Color dark_gray) {
    if (layout->sidebar_members.w > 0) {
        User online_users[MAX_USERS];
        int online_count = user_model_get_online(online_users, MAX_USERS);

        char online_str[32];
        snprintf(online_str, sizeof(online_str), "ONLINE — %d", online_count);
        draw_text(renderer, font_sub, online_str, layout->sidebar_members.x + 20, 20, dark_gray);

        int user_y = 55;
        for (int i = 0; i < online_count; i++) {
            draw_text(renderer, font_main, online_users[i].username, layout->sidebar_members.x + 20, user_y, green_online);
            user_y += 30;
        }

        /* NOTE: this layout (row height 30, header-to-first-row gap 35) is
           duplicated in chat_controller_handle_right_click's hit-test for
           the ban context menu - keep both in sync. */
        User offline_users[MAX_USERS];
        int offline_count = user_model_get_offline(offline_users, MAX_USERS);

        user_y += 20;
        char offline_str[32];
        snprintf(offline_str, sizeof(offline_str), "OFFLINE — %d", offline_count);
        draw_text(renderer, font_sub, offline_str, layout->sidebar_members.x + 20, user_y, dark_gray);
        user_y += 35;

        for (int i = 0; i < offline_count; i++) {
            draw_text(renderer, font_main, offline_users[i].username, layout->sidebar_members.x + 20, user_y, dark_gray);
            user_y += 30;
        }
    }
}
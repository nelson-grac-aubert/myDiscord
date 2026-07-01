#include "../include/ui_users.h"
#include "user.h"
#include "../include/ui_login.h" // For draw_text
#include <stdio.h>

static void record_member_row(ChatLayout *layout, int user_y, int user_id, int is_banned) {
    if (layout->member_row_count >= MAX_MEMBER_ROWS)
        return;
    layout->member_row_rect[layout->member_row_count] =
        (SDL_Rect){layout->sidebar_members.x + 8, user_y - 4, layout->sidebar_members.w - 16, 28};
    layout->member_row_user_id[layout->member_row_count] = user_id;
    layout->member_row_is_banned[layout->member_row_count] = is_banned;
    layout->member_row_count++;
}

static void draw_member_row(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main,
                            const User *u, int user_y, SDL_Color normal_color) {
    if (u->is_banned) {
        char label[64];
        snprintf(label, sizeof(label), "%s (banned)", u->username);
        SDL_Color banned_color = {0xE0, 0x50, 0x50, 0xFF};
        draw_text(renderer, font_main, label, layout->sidebar_members.x + 20, user_y, banned_color);
    } else {
        draw_text(renderer, font_main, u->username, layout->sidebar_members.x + 20, user_y, normal_color);
    }
    record_member_row(layout, user_y, u->id, u->is_banned);
}

void users_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, TTF_Font *font_sub, SDL_Color green_online, SDL_Color dark_gray) {
    layout->member_row_count = 0;

    if (layout->sidebar_members.w > 0) {
        User online_users[MAX_USERS];
        int online_count = user_model_get_online(online_users, MAX_USERS);

        char online_str[32];
        snprintf(online_str, sizeof(online_str), "ONLINE — %d", online_count);
        draw_text(renderer, font_sub, online_str, layout->sidebar_members.x + 20, 20, dark_gray);

        int user_y = 55;
        for (int i = 0; i < online_count; i++) {
            draw_member_row(renderer, layout, font_main, &online_users[i], user_y, green_online);
            user_y += 30;
        }

        User offline_users[MAX_USERS];
        int offline_count = user_model_get_offline(offline_users, MAX_USERS);

        user_y += 20;
        char offline_str[32];
        snprintf(offline_str, sizeof(offline_str), "OFFLINE — %d", offline_count);
        draw_text(renderer, font_sub, offline_str, layout->sidebar_members.x + 20, user_y, dark_gray);
        user_y += 35;

        for (int i = 0; i < offline_count; i++) {
            draw_member_row(renderer, layout, font_main, &offline_users[i], user_y, dark_gray);
            user_y += 30;
        }
    }
}
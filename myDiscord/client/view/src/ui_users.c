#include "../include/ui_users.h"
#include "../include/ui_chat_components.h"

void users_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, TTF_Font *font_sub, SDL_Color green_online, SDL_Color dark_gray) {
    if (layout->sidebar_members.w > 0) {
        components_draw_text(renderer, font_sub, "EN LIGNE — 3", layout->sidebar_members.x + 20, 20, dark_gray);
        components_draw_text(renderer, font_main, "●  Nexus_One", layout->sidebar_members.x + 20, 55, green_online);
        components_draw_text(renderer, font_main, "●  SARA_X", layout->sidebar_members.x + 20, 85, green_online);
        components_draw_text(renderer, font_main, "●  AlexK", layout->sidebar_members.x + 20, 115, green_online);
    }
}
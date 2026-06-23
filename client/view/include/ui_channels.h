#ifndef UI_CHANNELS_H
#define UI_CHANNELS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "ui_chat.h" // Nécessaire pour ChatLayout

extern SDL_Rect btn_options;
extern SDL_Rect btn_logout;

void channels_update_layout(ChatLayout *layout, int win_h);
void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray, SDL_Color red_delete);

#endif
#ifndef UI_CHANNELS_H
#define UI_CHANNELS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "ui_chat.h" // Nécessaire pour ChatLayout

// MODIFICATION : btn_options a été retiré car supprimé du projet
extern SDL_Rect btn_logout;

void channels_clear_textures(void);
void channels_update_layout(ChatLayout *layout, int win_h);

// MODIFICATION : red_delete a été supprimé des paramètres
void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray);

#endif
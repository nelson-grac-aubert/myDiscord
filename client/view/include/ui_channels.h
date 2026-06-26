#ifndef UI_CHANNELS_H
#define UI_CHANNELS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../include/ui_chat.h" // Required for ChatLayout

// SÉCURITÉ : Les déclarations globales 'extern SDL_Rect' ont été supprimées 
// car ces boutons sont désormais centralisés et gérés dans 'ChatLayout'.

// Updates layout bounds for specific channel views (add channel, logout buttons)
void channels_update_layout(ChatLayout *layout, int win_h);

// Clears cached channel textures (kept for compatibility layers)
void channels_clear_textures(void);

// Renders the left-hand channel list sidebar
void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, 
                           TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_emoji,
                           int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray);

#endif /* UI_CHANNELS_H */
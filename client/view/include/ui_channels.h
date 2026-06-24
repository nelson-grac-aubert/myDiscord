#ifndef UI_CHANNELS_H
#define UI_CHANNELS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../include/ui_chat.h" // Nécessaire pour ChatLayout

extern SDL_Rect btn_logout;
extern SDL_Rect btn_add_channel;

// Met à jour la position des boutons spécifiques aux salons (add, logout)
void channels_update_layout(ChatLayout *layout, int win_h);

// Vide (conservée pour compatibilité)
void channels_clear_textures(void);

// Rendu graphique de la barre des salons
void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, 
                           TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, 
                           int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray);

#endif /* UI_CHANNELS_H */
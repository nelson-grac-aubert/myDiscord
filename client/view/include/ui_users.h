#ifndef UI_USERS_H
#define UI_USERS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../include/ui_chat.h" // Required for ChatLayout

// Renders the right-hand sidebar showing active and online members
void users_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, 
                        TTF_Font *font_main, TTF_Font *font_sub, 
                        SDL_Color green_online, SDL_Color dark_gray);

#endif /* UI_USERS_H */
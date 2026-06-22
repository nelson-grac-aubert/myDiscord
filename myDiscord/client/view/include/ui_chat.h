#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// 1. La structure pour stocker la géométrie des blocs
typedef struct {
    int window_w;
    int window_h;
    
    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;
    
    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;
} ChatLayout;

// 2. Les prototypes (déclarations) des fonctions
void update_chat_layout(ChatLayout *layout, int win_w, int win_h);

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

void chat_ui_init_and_run(void);

#endif /* UI_CHAT_H */
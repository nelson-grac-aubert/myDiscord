#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "message.h"
#include "variables.h" // Inclus pour intégrer ChatHoverState et les constantes

typedef struct {
    int window_w;
    int window_h;
    
    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;
    
    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;
    
    // Nouveaux rectangles pour tes boutons multimédias
    SDL_Rect btn_microphone;
    SDL_Rect btn_file_transfer;
    
    int is_input_focused;
    char input_buffer[MAX_MSG_LENGTH];
    
    int show_create_modal;
    char modal_name_buffer[32];
    int modal_is_private;
    
    int menu_type;
    int menu_x;
    int menu_y;
    SDL_Rect menu_rect;

    // --- AJOUTS MULTIMÉDIA & HOVER ET CENTRALISÉS ---
    int is_mic_muted;
    SDL_Texture *tex_mic_on;
    SDL_Texture *tex_mic_off;
    SDL_Texture *tex_file;
    
    ChatHoverState hover; // Permet de savoir en permanence ce qui est survolé
} ChatLayout;

void update_chat_layout(ChatLayout *layout, int win_w, int win_h);
void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);
int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

#endif /* UI_CHAT_H */
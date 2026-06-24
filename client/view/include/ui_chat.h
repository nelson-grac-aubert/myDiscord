#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "message.h" // Nécessaire pour MAX_MSG_LENGTH

typedef struct {
    int window_w;
    int window_h;
    
    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;
    
    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;
    
    int is_input_focused;
    char input_buffer[MAX_MSG_LENGTH];
    
    // Variables d'état pour la modale locale de création de salon
    int show_create_modal;
    char modal_name_buffer[32];
    int modal_is_private;
    
    // Menu contextuel (clic droit optionnel)
    int menu_type;
    int menu_x;
    int menu_y;
    SDL_Rect menu_rect;
} ChatLayout;

// Recalcule les rectangles d'affichage selon la taille de la fenêtre
void update_chat_layout(ChatLayout *layout, int win_w, int win_h);

// Rendu de toute l'interface (appelle en interne le dessin des salons et membres)
void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

// La boucle interne du chat appelée par le State Machine de ui_welcome
int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

#endif /* UI_CHAT_H */
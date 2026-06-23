#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_MSG_LENGTH 256

typedef struct {
    int window_w;
    int window_h;
    
    SDL_Rect sidebar_servers;    // Reste présente si tu t'en sers pour l'arrière-plan global
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;
    
    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;

    char input_buffer[MAX_MSG_LENGTH]; 
    int is_input_focused;              

    int show_create_modal;        
    char modal_name_buffer[32];   
    int modal_is_private;         
    int modal_focused_field;      

    // AJOUTS POUR LE CLIC DROIT CONTEXTUEL
    int menu_type;          // 0 = Aucun, 1 = Salon, 2 = Message
    int menu_x, menu_y;     // Coordonnées du clic
    int target_index;       // Index du salon ou message ciblé
    SDL_Rect menu_rect;     // Zone de collision pour cliquer sur "Supprimer"
} ChatLayout;

void update_chat_layout(ChatLayout *layout, int win_w, int win_h);
void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

// ➡️ CORRECTION : Changement du type de retour pour récupérer le choix de déconnexion
int chat_ui_init_and_run(void);

#endif /* UI_CHAT_H */
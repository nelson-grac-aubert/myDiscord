#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_MSG_LENGTH 256
#define MAX_MESSAGES 50

// Structure pour stocker un message en mémoire (Front uniquement)
typedef struct {
    char username[32];
    char text[MAX_MSG_LENGTH];
} ChatMessage;

// Structure pour stocker la géométrie et l'état interactif du chat
typedef struct {
    int window_w;
    int window_h;
    
    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;
    
    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;

    // --- ÉTAT INTERACTIF LOCAL (AJOUT) ---
    char input_buffer[MAX_MSG_LENGTH]; // Texte en cours de saisie
    int is_input_focused;               // 1 si la barre est sélectionnée, 0 sinon
    
    ChatMessage messages[MAX_MESSAGES]; // Tableau historique des messages
    int message_count;                  // Nombre actuel de messages affichés
} ChatLayout;

// Prototypes des fonctions
void update_chat_layout(ChatLayout *layout, int win_w, int win_h);

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

void chat_ui_init_and_run(void);

#endif /* UI_CHAT_H */
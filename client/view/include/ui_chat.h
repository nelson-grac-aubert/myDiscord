#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_MSG_LENGTH 256

typedef struct {
    int window_w;
    int window_h;

    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;

    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;

    char input_buffer[MAX_MSG_LENGTH];
    int  is_input_focused;

    /* channel creation modal */
    int  show_create_modal;
    char modal_name_buffer[32];
    int  modal_is_private;
    int  modal_focused_field;

    /* right-click context menu: 0 = none, 1 = channel, 2 = message */
    int      menu_type;
    int      menu_x, menu_y;
    int      target_index;
    SDL_Rect menu_rect;

    /* ======================================================= */
    /* ➡️ AJOUTS : NOUVELLES FONCTIONNALITÉS (IMAGES EN ASSETS/) */
    /* ======================================================= */
    
    // Textures des icônes chargées depuis assets/
    SDL_Texture* tex_add_channel;
    SDL_Texture* tex_add_file;
    SDL_Texture* tex_microphone;

    // Rectangles de collision pour les clics souris
    SDL_Rect rect_btn_add_channel;
    SDL_Rect rect_btn_add_file;
    SDL_Rect rect_btn_microphone;

    // Gestion de l'envoi de fichier
    int  has_attached_file;
    char attached_file_name[256];
    SDL_Rect rect_file_preview_close; // Petit bouton [X] pour annuler le fichier

    // Gestion de la pop-up de mention @
    int      show_mention_popup;
    int      selected_mention_index;
    SDL_Rect mention_popup_rect;

    // Gestion du menu microphone et audio
    int      show_audio_menu;
    int      selected_micro_index;
    int      micro_count;
    char     micro_names[10][128]; // Stocke jusqu'à 10 périphériques détectés
    SDL_Rect audio_menu_rect;

} ChatLayout;

void update_chat_layout(ChatLayout *layout, int win_w, int win_h);
void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub,
                         int mx, int my);

#endif /* UI_CHAT_H */
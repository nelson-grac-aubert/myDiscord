#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "message.h"
#include "variables.h" // Included to integrate ChatHoverState and constants

typedef struct
{
    int window_w;
    int window_h;

    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;

    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;

    // Geometry bounds for multimedia buttons
    SDL_Rect btn_call;
    SDL_Rect btn_file_transfer;
    
    // FIX : Ajout des rectangles de collision pour le Log Out et le bouton "+"
    SDL_Rect btn_logout;
    SDL_Rect btn_add_channel;

    int is_input_focused;
    char input_buffer[MAX_MSG_LENGTH];

    int show_create_modal;
    char modal_name_buffer[32];
    int modal_is_private;
    int modal_focused_field; // Added for field focus management
    int target_index;        // Added for deletion/context targets

    int menu_type;
    int menu_x;
    int menu_y;
    SDL_Rect menu_rect;

    // --- MULTIMEDIA & CENTRALIZED HOVER EXTENSIONS ---
    int is_mic_muted;
    SDL_Texture *tex_mic_on;
    SDL_Texture *tex_mic_off;
    SDL_Texture *tex_file;

    ChatHoverState hover;
    int hover_channel_delete_index;
    int hover_message_delete_index;
    SDL_Rect btn_delete_channel_rect;
    SDL_Rect btn_delete_message_rect;
    int show_context_menu;
    int context_menu_x;
    int context_menu_y;
    char modal_buffer[256];
} ChatLayout;

// Global coordinates for modal layouts shared with controllers
extern SDL_Rect modal_input_rect;
extern SDL_Rect modal_toggle_rect;
extern SDL_Rect modal_btn_cancel;
extern SDL_Rect modal_btn_confirm;

// Recalculates responsive zones when window resizes
void ui_chat_handle_resize(ChatLayout *layout, int win_w, int win_h);

// Main rendering routine for the central Discord dashboard view
void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

#endif /* UI_CHAT_H */
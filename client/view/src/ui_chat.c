#include "ui_chat.h"
#include "variables.h"
#include "channel.h"
#include "message.h"
#include "ui_channels.h"
#include "ui_users.h"
#include "ui_login.h"
#include "chat_controller.h"
#include <stdio.h>
#include <string.h>

// Déclarations externes des rectangles pour la modale partagée
SDL_Rect modal_bg_rect, modal_input_rect, modal_toggle_rect, modal_btn_ok, modal_btn_cancel;

static void compute_modal_rects(int win_w, int win_h) {
    int modal_w = 440, modal_h = 300;
    modal_bg_rect = (SDL_Rect){(win_w - modal_w) / 2, (win_h - modal_h) / 2, modal_w, modal_h};
    modal_input_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 90, 380, 40};
    modal_toggle_rect = (SDL_Rect){modal_bg_rect.x + 340, modal_bg_rect.y + 160, 50, 26};
    modal_btn_cancel = (SDL_Rect){modal_bg_rect.x + 210, modal_bg_rect.y + 230, 90, 40};
    modal_btn_ok = (SDL_Rect){modal_bg_rect.x + 310, modal_bg_rect.y + 230, 100, 40};
}

void update_chat_layout(ChatLayout *layout, int win_w, int win_h) {
    layout->window_w = win_w; 
    layout->window_h = win_h;
    
    layout->sidebar_servers = (SDL_Rect){0, 0, 72, win_h};
    layout->sidebar_channels = (SDL_Rect){72, 0, 240, win_h};
    
    int chat_x = 72 + 240;
    int members_w = 240;
    
    layout->sidebar_members = (SDL_Rect){win_w - members_w, 0, members_w, win_h};
    layout->chat_area = (SDL_Rect){chat_x, 0, win_w - chat_x - members_w, win_h};
    layout->chat_top_bar = (SDL_Rect){chat_x, 0, layout->chat_area.w, 48};
    layout->chat_input_bar = (SDL_Rect){chat_x + 60, win_h - 60, layout->chat_area.w - 120, 44};
    
    // Positionnement géométrique des icônes à l'intérieur/bord de la barre d'input
    layout->btn_file_transfer = (SDL_Rect){chat_x + 15, win_h - 58, 36, 36};
    layout->btn_microphone = (SDL_Rect){win_w - members_w - 50, win_h - 58, 36, 36};

    channels_update_layout(layout, win_h);
    compute_modal_rects(win_w, win_h);
}

static void draw_chat_messages(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, SDL_Color white) {
    Channel *active_ch = channel_model_get_active();
    if (!active_ch) return;

    int start_y = CHAT_MESSAGES_START_Y;
    Message active_msgs[MAX_MESSAGES];
    
    int msg_count = message_model_get_for_channel(active_ch->id, active_msgs, MAX_MESSAGES);

    for (int i = 0; i < msg_count; i++) {
        char formatted[512];
        snprintf(formatted, sizeof(formatted), "%s : %s", active_msgs[i].username, active_msgs[i].text);
        draw_text(renderer, font_main, formatted, layout->chat_area.x + 20, start_y, white);
        start_y += MESSAGE_ITEM_HEIGHT;
    }
}

static void draw_local_modal(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, SDL_Color white, SDL_Color gray, SDL_Color dark_gray) {
    SDL_SetRenderDrawColor(renderer, 0x31, 0x33, 0x38, 255); 
    SDL_RenderFillRect(renderer, &modal_bg_rect);
    SDL_SetRenderDrawColor(renderer, 0x1E, 0x1F, 0x22, 255); 
    SDL_RenderDrawRect(renderer, &modal_bg_rect);
    
    draw_text(renderer, font_title, "Créer un salon", modal_bg_rect.x + 30, modal_bg_rect.y + 25, white);
    SDL_SetRenderDrawColor(renderer, 0x1E, 0x1F, 0x22, 255); 
    SDL_RenderFillRect(renderer, &modal_input_rect);
    draw_text(renderer, font_main, "NOM DU SALON", modal_bg_rect.x + 30, modal_bg_rect.y + 70, gray);
    
    if (strlen(layout->modal_name_buffer) > 0) {
        draw_text(renderer, font_main, layout->modal_name_buffer, modal_input_rect.x + 10, modal_input_rect.y + 10, white);
    } else {
        draw_text(renderer, font_main, "nouveau-salon", modal_input_rect.x + 10, modal_input_rect.y + 10, dark_gray);
    }

    draw_text(renderer, font_main, "Salon Privé (Restreindre l'accès)", modal_bg_rect.x + 30, modal_bg_rect.y + 162, white);
    SDL_SetRenderDrawColor(renderer, layout->modal_is_private ? 0x23 : 0x80, layout->modal_is_private ? 0xA5 : 0x84, layout->modal_is_private ? 0x5A : 0x92, 255);
    SDL_RenderFillRect(renderer, &modal_toggle_rect);
    
    SDL_Rect pill = {modal_toggle_rect.x + (layout->modal_is_private ? 26 : 4), modal_toggle_rect.y + 3, 20, 20};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
    SDL_RenderFillRect(renderer, &pill);

    SDL_SetRenderDrawColor(renderer, 0x4E, 0x50, 0x58, 255); 
    SDL_RenderFillRect(renderer, &modal_btn_cancel);
    draw_text(renderer, font_main, "Annuler", modal_btn_cancel.x + 18, modal_btn_cancel.y + 10, white);
    
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_DISCORD_BLUE.r, VAR_COLOR_DISCORD_BLUE.g, VAR_COLOR_DISCORD_BLUE.b, 255); 
    SDL_RenderFillRect(renderer, &modal_btn_ok);
    draw_text(renderer, font_main, "Créer", modal_btn_ok.x + 32, modal_btn_ok.y + 10, white);
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = VAR_COLOR_TEXT_MUTED; 
    SDL_Color dark_gray = {79, 84, 92, 255};
    SDL_Color green = {35, 165, 90, 255};
    
    int mx, my; 
    SDL_GetMouseState(&mx, &my);

    // Active le mode Blend pour gérer les opacités transparentes des hovers
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Dessin des arrière-plans via les variables de thèmes centralisées
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_SERVERS.r, VAR_COLOR_BG_SERVERS.g, VAR_COLOR_BG_SERVERS.b, 255); 
    SDL_RenderFillRect(renderer, &layout->sidebar_servers);
    
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHANNELS.r, VAR_COLOR_BG_CHANNELS.g, VAR_COLOR_BG_CHANNELS.b, 255); 
    SDL_RenderFillRect(renderer, &layout->sidebar_channels);
    
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHAT.r, VAR_COLOR_BG_CHAT.g, VAR_COLOR_BG_CHAT.b, 255); 
    SDL_RenderFillRect(renderer, &layout->chat_area);
    
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_MEMBERS.r, VAR_COLOR_BG_MEMBERS.g, VAR_COLOR_BG_MEMBERS.b, 255); 
    SDL_RenderFillRect(renderer, &layout->sidebar_members);
    
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_INPUT.r, VAR_COLOR_BG_INPUT.g, VAR_COLOR_BG_INPUT.b, 255); 
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);

    // 1. Rendu graphique du bouton Microphone (avec Hover léger)
    if (layout->hover.hover_mic_button) {
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_LIGHT.r, VAR_COLOR_HOVER_LIGHT.g, VAR_COLOR_HOVER_LIGHT.b, VAR_COLOR_HOVER_LIGHT.a);
        SDL_RenderFillRect(renderer, &layout->btn_microphone);
    } else {
        SDL_SetRenderDrawColor(renderer, layout->is_mic_muted ? 0xED : 0x4E, layout->is_mic_muted ? 0x42 : 0x50, layout->is_mic_muted ? 0x45 : 0x58, 255);
        SDL_RenderFillRect(renderer, &layout->btn_microphone);
    }
    SDL_Texture *mic_tex = layout->is_mic_muted ? layout->tex_mic_off : layout->tex_mic_on;
    if (mic_tex) {
        SDL_RenderCopy(renderer, mic_tex, NULL, &layout->btn_microphone);
    } else {
        draw_text(renderer, font_sub, layout->is_mic_muted ? "MUT" : "MIC", layout->btn_microphone.x + 5, layout->btn_microphone.y + 10, white);
    }

    // 2. Rendu graphique du bouton Transfert de fichier (avec Hover léger)
    if (layout->hover.hover_file_button) {
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_LIGHT.r, VAR_COLOR_HOVER_LIGHT.g, VAR_COLOR_HOVER_LIGHT.b, VAR_COLOR_HOVER_LIGHT.a);
        SDL_RenderFillRect(renderer, &layout->btn_file_transfer);
    } else {
        SDL_SetRenderDrawColor(renderer, 0x4E, 0x50, 0x58, 255); 
        SDL_RenderFillRect(renderer, &layout->btn_file_transfer);
    }
    if (layout->tex_file) {
        SDL_RenderCopy(renderer, layout->tex_file, NULL, &layout->btn_file_transfer);
    } else {
        draw_text(renderer, font_sub, "+", layout->btn_file_transfer.x + 14, layout->btn_file_transfer.y + 8, white);
    }

    // 3. Dessin de la saisie utilisateur (Input Buffer)
    if (strlen(layout->input_buffer) > 0) {
        draw_text(renderer, font_main, layout->input_buffer, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, white);
    } else {
        Channel *ch = channel_model_get_active(); 
        char placeholder[128];
        snprintf(placeholder, sizeof(placeholder), "Envoyer un message dans #%s", ch ? ch->name : "général");
        draw_text(renderer, font_main, placeholder, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, gray);
    }

    // 4. Appel des sous-composants graphiques latéraux & messages
    channels_draw_sidebar(renderer, layout, font_title, font_main, font_sub, mx, my, white, gray, dark_gray);
    users_draw_sidebar(renderer, layout, font_main, font_sub, green, dark_gray);
    draw_chat_messages(renderer, layout, font_main, white);

    // 5. Affichage conditionnel de la Modale locale
    if (layout->show_create_modal) {
        draw_local_modal(renderer, layout, font_title, font_main, white, gray, dark_gray);
    }

    // 6. Rendu et coloration adaptative (Rouge au survol) du Menu Contextuel
    if (layout->menu_type > 0) {
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_MENU_BG.r, VAR_COLOR_MENU_BG.g, VAR_COLOR_MENU_BG.b, 255); 
        SDL_RenderFillRect(renderer, &layout->menu_rect);
        SDL_SetRenderDrawColor(renderer, 0x35, 0x37, 0x3C, 255); 
        SDL_RenderDrawRect(renderer, &layout->menu_rect);
        
        if (layout->hover.hover_menu_action) {
            // Fond rouge si l'action du menu est survolée
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_RED.r, VAR_COLOR_HOVER_RED.g, VAR_COLOR_HOVER_RED.b, 255);
            SDL_RenderFillRect(renderer, &layout->menu_rect);
            
            if (layout->menu_type == 1) draw_text(renderer, font_main, "Supprimer le Salon", layout->menu_rect.x + 15, layout->menu_rect.y + 12, white);
            else if (layout->menu_type == 2) draw_text(renderer, font_main, "Mentionner (@)", layout->menu_rect.x + 15, layout->menu_rect.y + 12, white);
            else if (layout->menu_type == 3) draw_text(renderer, font_main, "Supprimer le Message", layout->menu_rect.x + 15, layout->menu_rect.y + 12, white);
        } else {
            if (layout->menu_type == 1) draw_text(renderer, font_main, "Supprimer le Salon", layout->menu_rect.x + 15, layout->menu_rect.y + 12, VAR_COLOR_HOVER_RED);
            else if (layout->menu_type == 2) draw_text(renderer, font_main, "Mentionner (@)", layout->menu_rect.x + 15, layout->menu_rect.y + 12, white);
            else if (layout->menu_type == 3) draw_text(renderer, font_main, "Supprimer le Message", layout->menu_rect.x + 15, layout->menu_rect.y + 12, VAR_COLOR_HOVER_RED);
        }
    }
}

int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub) {
    (void)window; 
    ChatLayout layout = {0}; 
    update_chat_layout(&layout, 1200, 750);
    
    // Initialisation du controlleur et chargement des textures graphiques
    chat_controller_init(&layout, renderer);
    
    int running = 1, exit_status = 0; 
    SDL_Event event; 
    SDL_StartTextInput();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { 
                running = 0; 
                exit_status = 1; 
            }
            else if (event.type == SDL_MOUSEMOTION) {
                // Met à jour les états des hovers en temps réel à chaque micro-mouvement de souris
                chat_controller_update_hover(&layout, event.motion.x, event.motion.y);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int cx = event.button.x, cy = event.button.y;
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    chat_controller_handle_right_click(&layout, cx, cy);
                }
                else if (event.button.button == SDL_BUTTON_LEFT) {
                    if (layout.menu_type > 0) {
                        chat_controller_handle_menu_action(&layout, cx, cy);
                    } else {
                        int status = chat_controller_handle_left_click(&layout, cx, cy);
                        if (status == 2) { 
                            running = 0; 
                            exit_status = 2; 
                        }
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                chat_controller_handle_keydown(&layout, event.key.keysym.sym);
            }
            else if (event.type == SDL_TEXTINPUT) {
                chat_controller_handle_textinput(&layout, event.text.text);
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);
        
        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);
        
        SDL_RenderPresent(renderer);
    }
    
    // Libération propre des textures graphiques de la carte vidéo
    chat_controller_destroy(&layout);
    
    return exit_status;
}
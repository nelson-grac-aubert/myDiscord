#include "../include/ui_chat.h"
#include "channel.h"
#include "message.h"
#include "../include/ui_channels.h"
#include "../include/ui_users.h"
#include "../include/ui_login.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_SERVERS  0x0D, 0x0E, 0x12, 0xFF
#define COLOR_BG_CHANNELS 0x12, 0x14, 0x1C, 0xFF
#define COLOR_BG_CHAT     0x16, 0x19, 0x24, 0xFF
#define COLOR_BG_MEMBERS  0x12, 0x14, 0x1C, 0xFF
#define COLOR_BG_INPUT    0x1F, 0x23, 0x33, 0xFF
// Déclarations des rectangles internes pour la modale
SDL_Rect modal_bg_rect;
SDL_Rect modal_input_rect;
SDL_Rect modal_toggle_rect;
SDL_Rect modal_btn_confirm;
SDL_Rect modal_btn_cancel;

static void compute_modal_rects(ChatLayout *layout, int win_w, int win_h)
{
    int modal_w = 440;
    int modal_h = 280;
    modal_bg_rect = (SDL_Rect){(win_w - modal_w) / 2, (win_h - modal_h) / 2, modal_w, modal_h};
    modal_input_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 90, modal_bg_rect.w - 60, 40};
    modal_toggle_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 175, 50, 24};
    modal_btn_cancel = (SDL_Rect){modal_bg_rect.x + 210, modal_bg_rect.y + 220, 90, 36};
    modal_btn_confirm = (SDL_Rect){modal_bg_rect.x + 310, modal_bg_rect.y + 220, 100, 36};
    (void)layout;
}

void update_chat_layout(ChatLayout *layout, int win_w, int win_h)
{
    layout->window_w = win_w;
    layout->window_h = win_h;
    int servers_w = 0;
    int channels_w = 240;
    int members_w = 240;
    int chat_w = win_w - (servers_w + channels_w + members_w);
    if (chat_w < 350) {
        members_w = 0;
        chat_w = win_w - (servers_w + channels_w);
    }
    layout->sidebar_servers = (SDL_Rect){0, 0, servers_w, win_h};
    layout->sidebar_channels = (SDL_Rect){servers_w, 0, channels_w, win_h};
    layout->chat_area = (SDL_Rect){servers_w + channels_w, 0, chat_w, win_h};
    layout->sidebar_members = (SDL_Rect){win_w - members_w, 0, members_w, win_h};
    layout->chat_top_bar = (SDL_Rect){layout->chat_area.x, 0, layout->chat_area.w, 48};
    layout->chat_input_bar = (SDL_Rect){layout->chat_area.x + 16, win_h - 68, layout->chat_area.w - 32, 44};
    channels_update_layout(layout, win_h);
    compute_modal_rects(layout, win_w, win_h);
    if (layout->menu_type > 0)
        layout->menu_rect = (SDL_Rect){layout->menu_x, layout->menu_y, 130, 32};
}

static void draw_backgrounds(SDL_Renderer *renderer, ChatLayout *layout)
{
    SDL_SetRenderDrawColor(renderer, COLOR_BG_SERVERS);
    SDL_RenderFillRect(renderer, &layout->sidebar_servers);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHANNELS);
    SDL_RenderFillRect(renderer, &layout->sidebar_channels);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHAT);
    SDL_RenderFillRect(renderer, &layout->chat_area);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_MEMBERS);
    SDL_RenderFillRect(renderer, &layout->sidebar_members);
    SDL_SetRenderDrawColor(renderer, 0x0D, 0x0E, 0x12, 0xFF);
    SDL_RenderDrawLine(renderer, layout->chat_top_bar.x, layout->chat_top_bar.h, layout->chat_top_bar.x + layout->chat_top_bar.w, layout->chat_top_bar.h);
}

static void draw_input_bar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, SDL_Color white, SDL_Color dark_gray, SDL_Color blurple)
{
    SDL_SetRenderDrawColor(renderer, COLOR_BG_INPUT);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);
    if (layout->is_input_focused) {
        SDL_SetRenderDrawColor(renderer, blurple.r, blurple.g, blurple.b, blurple.a);
        SDL_RenderDrawRect(renderer, &layout->chat_input_bar);
    }
    Channel *active_ch = channel_model_get_active();
    if (strlen(layout->input_buffer) == 0) {
        char placeholder[64];
        snprintf(placeholder, sizeof(placeholder), "Envoyer un message dans #%s", active_ch ? active_ch->name : "salon");
        draw_text(renderer, font_main, placeholder, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 13, dark_gray);
        if (layout->is_input_focused && ((SDL_GetTicks() / 500) % 2)) {
            draw_text(renderer, font_main, "|", layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 13, white);
        }
    } else {
        draw_text(renderer, font_main, layout->input_buffer, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 13, white);
        if (layout->is_input_focused && ((SDL_GetTicks() / 500) % 2)) {
            int text_w = get_text_width(font_main, layout->input_buffer);
            draw_text(renderer, font_main, "|", layout->chat_input_bar.x + 15 + text_w, layout->chat_input_bar.y + 13, white);
        }
    }
}

static void draw_chat_messages(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, TTF_Font *font_sub, SDL_Color white, SDL_Color gray)
{
    (void)font_sub;
    (void)gray;
    Channel *active_ch = channel_model_get_active();
    if (!active_ch) return;
    Message msgs[MAX_MESSAGES];
    int msg_count = message_model_get_for_channel(active_ch->id, msgs, MAX_MESSAGES);
    int start_y = 70;
    for (int i = 0; i < msg_count; i++) {
        char full_msg[512];
        snprintf(full_msg, sizeof(full_msg), "%s : %s", msgs[i].username, msgs[i].text);
        draw_text(renderer, font_main, full_msg, layout->chat_area.x + 20, start_y, white);
        start_y += 30;
    }
}

static void draw_local_modal(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white, SDL_Color gray, SDL_Color dark_gray)
{
    (void)mx;
    (void)my;
    // Fond d'assombrissement global
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect full_screen = {0, 0, layout->window_w, layout->window_h};
    SDL_RenderFillRect(renderer, &full_screen);
    // Boîte modale
    SDL_SetRenderDrawColor(renderer, 0x31, 0x33, 0x38, 255);
    SDL_RenderFillRect(renderer, &modal_bg_rect);
    draw_text(renderer, font_title, "Créer un salon", modal_bg_rect.x + 30, modal_bg_rect.y + 25, white);
    draw_text(renderer, font_sub, "Les salons permettent de communiquer par écrit.", modal_bg_rect.x + 30, modal_bg_rect.y + 55, gray);
    // Champ Input Nom du Salon
    SDL_SetRenderDrawColor(renderer, 0x1E, 0x1F, 0x22, 255);
    SDL_RenderFillRect(renderer, &modal_input_rect);
    if (strlen(layout->modal_name_buffer) == 0) {
        draw_text(renderer, font_main, "nouveau-salon", modal_input_rect.x + 12, modal_input_rect.y + 11, dark_gray);
    } else {
        draw_text(renderer, font_main, layout->modal_name_buffer, modal_input_rect.x + 12, modal_input_rect.y + 11, white);
    }
    // Toggle privé / public (Vert si privé, Gris si public)
    draw_text(renderer, font_main, "Salon Privé ?", modal_bg_rect.x + 30, modal_bg_rect.y + 150, white);
    if (layout->modal_is_private) {
        SDL_SetRenderDrawColor(renderer, 0x23, 0xA5, 0x5A, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 0x80, 0x84, 0x92, 255);
    }
    SDL_RenderFillRect(renderer, &modal_toggle_rect);
    // Boutons de contrôle (Annuler / Confirmer)
    SDL_SetRenderDrawColor(renderer, 0x4E, 0x50, 0x58, 255);
    SDL_RenderFillRect(renderer, &modal_btn_cancel);
    SDL_SetRenderDrawColor(renderer, 0x58, 0x65, 0xF2, 255);
    SDL_RenderFillRect(renderer, &modal_btn_confirm);
    draw_text(renderer, font_main, "Annuler", modal_btn_cancel.x + 15, modal_btn_cancel.y + 8, white);
    draw_text(renderer, font_main, "Créer", modal_btn_confirm.x + 25, modal_btn_confirm.y + 8, white);
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub)
{
    SDL_Color white_color = {255, 255, 255, 255};
    SDL_Color gray_color = {148, 155, 164, 255};
    SDL_Color dark_gray = {80, 84, 92, 255};
    SDL_Color blurple = {0x58, 0x65, 0xF2, 255};
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    draw_backgrounds(renderer, layout);
    Channel *active_ch = channel_model_get_active();
    char title[128];
    snprintf(title, sizeof(title), "#  %s", active_ch ? active_ch->name : "aucun-salon");
    draw_text(renderer, font_title, title, layout->chat_top_bar.x + 20, 12, white_color);
    channels_draw_sidebar(renderer, layout, font_title, font_main, font_sub, mx, my, white_color, gray_color, dark_gray);
    users_draw_sidebar(renderer, layout, font_main, font_sub, (SDL_Color){35, 165, 90, 255}, dark_gray);
    draw_chat_messages(renderer, layout, font_main, font_sub, white_color, gray_color);
    draw_input_bar(renderer, layout, font_main, white_color, dark_gray, blurple);
    if (layout->show_create_modal) {
        draw_local_modal(renderer, layout, font_title, font_main, font_sub, mx, my, white_color, gray_color, dark_gray);
    }
}

// Les fonctions de clics ci-dessous gèrent la logique d'état 
int handle_left_click_normal(ChatLayout *layout, int cx, int cy)
{
    if (cx >= btn_logout.x && cx <= btn_logout.x + btn_logout.w && cy >= btn_logout.y && cy <= btn_logout.y + btn_logout.h) {
        return 2; // Signal d'état Déconnexion vers welcome loop
    }
    if (cx >= btn_add_channel.x && cx <= btn_add_channel.x + btn_add_channel.w && cy >= btn_add_channel.y && cy <= btn_add_channel.y + btn_add_channel.h) {
        layout->show_create_modal = 1;
        memset(layout->modal_name_buffer, 0, sizeof(layout->modal_name_buffer));
        layout->modal_is_private = 0;
        return 0;
    }
    if (cx >= layout->chat_input_bar.x && cx <= layout->chat_input_bar.x + layout->chat_input_bar.w && cy >= layout->chat_input_bar.y && cy <= layout->chat_input_bar.y + layout->chat_input_bar.h) {
        layout->is_input_focused = 1;
    } else {
        layout->is_input_focused = 0;
    }
    // Sélection d'un salon via clic
    if (cx >= layout->sidebar_channels.x && cx <= layout->sidebar_channels.x + layout->sidebar_channels.w) {
        int idx = (cy - 60) / 32;
        if (idx >= 0 && idx < channel_model_get_count()) {
            channel_model_set_active_index(idx);
        }
    }
    return 0;
}

void handle_left_click_modal(ChatLayout *layout, int cx, int cy)
{
    if (cx >= modal_btn_cancel.x && cx <= modal_btn_cancel.x + modal_btn_cancel.w && cy >= modal_btn_cancel.y && cy <= modal_btn_cancel.y + modal_btn_cancel.h) {
        layout->show_create_modal = 0;
    }
    else if (cx >= modal_btn_confirm.x && cx <= modal_btn_confirm.x + modal_btn_confirm.w && cy >= modal_btn_confirm.y && cy <= modal_btn_confirm.y + modal_btn_confirm.h) {
        if (strlen(layout->modal_name_buffer) > 0) {
            channel_model_add(channel_model_get_count() + 1, layout->modal_name_buffer, layout->modal_is_private);
        }
        layout->show_create_modal = 0;
    }
    else if (cx >= modal_toggle_rect.x && cx <= modal_toggle_rect.x + modal_toggle_rect.w && cy >= modal_toggle_rect.y && cy <= modal_toggle_rect.y + modal_toggle_rect.h) {
        layout->modal_is_private = !layout->modal_is_private;
    }
}

// Rendu allégé de la boucle globale de chat appelée par Welcome
int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub)
{
    (void)window; // Évite le warning sur window
    ChatLayout layout = {0};
    update_chat_layout(&layout, 1200, 750);
    SDL_Event event;
    int running = 1;
    int exit_status = 0; 
    SDL_StartTextInput();
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                exit_status = 0; // Quitter tout
            }
            else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                update_chat_layout(&layout, event.window.data1, event.window.data2);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int cx = event.button.x;
                int cy = event.button.y;
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (layout.show_create_modal) {
                        handle_left_click_modal(&layout, cx, cy);
                    } else {
                        int status = handle_left_click_normal(&layout, cx, cy);
                        if (status == 2) { // Clic déconnexion
                            running = 0;
                            exit_status = 2;
                        }
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                if (layout.show_create_modal) {
                    if (event.key.keysym.sym == SDL_SCANCODE_BACKSPACE && strlen(layout.modal_name_buffer) > 0) {
                        layout.modal_name_buffer[strlen(layout.modal_name_buffer) - 1] = '\0';
                    }
                } else if (layout.is_input_focused) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(layout.input_buffer) > 0) {
                        layout.input_buffer[strlen(layout.input_buffer) - 1] = '\0';
                    } else if (event.key.keysym.sym == SDLK_RETURN && strlen(layout.input_buffer) > 0) {
                        Channel *active_ch = channel_model_get_active();
                        if (active_ch) {
                            message_model_add(SDL_GetTicks(), active_ch->id, "Moi", layout.input_buffer);
                        }
                        memset(layout.input_buffer, 0, sizeof(layout.input_buffer));
                    }
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                if (layout.show_create_modal) {
                    if (strlen(layout.modal_name_buffer) + strlen(event.text.text) < 31) {
                        strcat(layout.modal_name_buffer, event.text.text);
                    }
                } else if (layout.is_input_focused) {
                    if (strlen(layout.input_buffer) + strlen(event.text.text) < MAX_MSG_LENGTH - 1) {
                        strcat(layout.input_buffer, event.text.text);
                    }
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);
        SDL_RenderPresent(renderer);
    }
    return exit_status;
}
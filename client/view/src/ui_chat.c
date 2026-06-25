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

// External declarations of rectangles for the shared creation modal
SDL_Rect modal_bg_rect, modal_input_rect, modal_toggle_rect, modal_btn_ok, modal_btn_cancel;

static void compute_modal_rects(int win_w, int win_h) {
    int modal_w = 440, modal_h = 300;
    modal_bg_rect = (SDL_Rect){(win_w - modal_w) / 2, (win_h - modal_h) / 2, modal_w, modal_h};
    modal_input_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 90, 380, 40};
    modal_toggle_rect = (SDL_Rect){modal_bg_rect.x + 340, modal_bg_rect.y + 160, 50, 26};
    modal_btn_cancel = (SDL_Rect){modal_bg_rect.x + 210, modal_bg_rect.y + 230, 90, 40};
    modal_btn_ok = (SDL_Rect){modal_bg_rect.x + 310, modal_bg_rect.y + 230, 100, 40};
}

// Implements ui_chat_handle_resize to match the header declaration
void ui_chat_handle_resize(ChatLayout *layout, int win_w, int win_h) {
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
    
    // Geometric positioning of icons inside/along the input bar
    layout->btn_file_transfer = (SDL_Rect){chat_x + 15, win_h - 58, 36, 36};
    layout->btn_microphone = (SDL_Rect){win_w - members_w - 50, win_h - 58, 36, 36};
    
    // FIX : Dimensionnement géométrique des nouveaux boutons pour activer la collision
    layout->btn_add_channel = (SDL_Rect){72 + 210, 14, 20, 20};     // Placé à droite du titre des salons
    layout->btn_logout = (SDL_Rect){72 + 15, win_h - 50, 36, 36};     // Placé en bas à gauche de la sidebar

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
        char full_msg[512];
        snprintf(full_msg, sizeof(full_msg), "[%s]: %s", active_msgs[i].username, active_msgs[i].text);
        
        SDL_Rect row_rect = {layout->chat_area.x + 10, start_y - 2, layout->chat_area.w - 20, MESSAGE_ITEM_HEIGHT};
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        int is_hovered = (mx >= row_rect.x && mx <= row_rect.x + row_rect.w && my >= row_rect.y && my <= row_rect.y + row_rect.h);

        if (is_hovered && !layout->show_create_modal) {
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_ITEM.r, VAR_COLOR_HOVER_ITEM.g, VAR_COLOR_HOVER_ITEM.b, VAR_COLOR_HOVER_ITEM.a);
            SDL_RenderFillRect(renderer, &row_rect);
        }

        draw_text(renderer, font_main, full_msg, layout->chat_area.x + 20, start_y, white);
        start_y += MESSAGE_ITEM_HEIGHT;
    }
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub) {
    SDL_Color color_white = VAR_COLOR_TEXT_WHITE;
    SDL_Color color_muted = VAR_COLOR_TEXT_MUTED;
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    // 1. Draw Layout Panels
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_SERVERS.r, VAR_COLOR_BG_SERVERS.g, VAR_COLOR_BG_SERVERS.b, VAR_COLOR_BG_SERVERS.a);
    SDL_RenderFillRect(renderer, &layout->sidebar_servers);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHANNELS.r, VAR_COLOR_BG_CHANNELS.g, VAR_COLOR_BG_CHANNELS.b, VAR_COLOR_BG_CHANNELS.a);
    SDL_RenderFillRect(renderer, &layout->sidebar_channels);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHAT.r, VAR_COLOR_BG_CHAT.g, VAR_COLOR_BG_CHAT.b, VAR_COLOR_BG_CHAT.a);
    SDL_RenderFillRect(renderer, &layout->chat_area);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_MEMBERS.r, VAR_COLOR_BG_MEMBERS.g, VAR_COLOR_BG_MEMBERS.b, VAR_COLOR_BG_MEMBERS.a);
    SDL_RenderFillRect(renderer, &layout->sidebar_members);

    // 2. Chat Top Bar
    SDL_SetRenderDrawColor(renderer, 0x24, 0x25, 0x28, 0xFF);
    SDL_RenderFillRect(renderer, &layout->chat_top_bar);

    Channel *active_ch = channel_model_get_active();
    if (active_ch) {
        char heading[64];
        snprintf(heading, sizeof(heading), "# %s", active_ch->name);
        draw_text(renderer, font_title, heading, layout->chat_top_bar.x + 20, 12, color_white);
    }

    // 3. Components Rendering
    channels_draw_sidebar(renderer, layout, font_title, font_main, font_sub, mx, my, color_white, color_muted, VAR_COLOR_BG_SERVERS);
    users_draw_sidebar(renderer, layout, font_main, font_sub, (SDL_Color){0x23, 0xA5, 0x5A, 0xFF}, color_muted);
    draw_chat_messages(renderer, layout, font_main, color_white);

    // 4. Chat Input Bar
    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_INPUT.r, VAR_COLOR_BG_INPUT.g, VAR_COLOR_BG_INPUT.b, VAR_COLOR_BG_INPUT.a);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);

    if (strlen(layout->input_buffer) == 0) {
        if (active_ch) {
            char placeholder[128];
            snprintf(placeholder, sizeof(placeholder), "Message #%s", active_ch->name);
            draw_text(renderer, font_main, placeholder, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, color_muted);
        }
    } else {
        draw_text(renderer, font_main, layout->input_buffer, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, color_white);
    }

    // 5. FIX : Remplacement des Textures Multimedia par la police de texte Émojis Windows
    if (font_emoji) {
        // --- RENDU DU BOUTON MICROPHONE ---
        if (layout->hover.hover_mic_button) {
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_LIGHT.r, VAR_COLOR_HOVER_LIGHT.g, VAR_COLOR_HOVER_LIGHT.b, VAR_COLOR_HOVER_LIGHT.a);
            SDL_RenderFillRect(renderer, &layout->btn_microphone);
        }
        if (layout->is_mic_muted) {
            draw_text(renderer, font_emoji, "🔇", layout->btn_microphone.x + 6, layout->btn_microphone.y + 6, color_white);
        } else {
            draw_text(renderer, font_emoji, "🎤", layout->btn_microphone.x + 6, layout->btn_microphone.y + 6, color_white);
        }

        // --- RENDU DU BOUTON ADDTION DE FICHIER ---
        if (layout->hover.hover_file_button) {
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_LIGHT.r, VAR_COLOR_HOVER_LIGHT.g, VAR_COLOR_HOVER_LIGHT.b, VAR_COLOR_HOVER_LIGHT.a);
            SDL_RenderFillRect(renderer, &layout->btn_file_transfer);
        }
        draw_text(renderer, font_emoji, "📁", layout->btn_file_transfer.x + 6, layout->btn_file_transfer.y + 6, color_white);

        // --- RENDU DU BOUTON "+" D'AJOUT DE SALON ---
        draw_text(renderer, font_emoji, "➕", layout->btn_add_channel.x, layout->btn_add_channel.y, color_muted);

        // --- RENDU DU BOUTON LOG OUT ---
        draw_text(renderer, font_emoji, "🚪", layout->btn_logout.x + 6, layout->btn_logout.y + 6, color_white);
    }

    // 6. Context Menu Rendering
    if (layout->menu_type > 0) {
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_MENU_BG.r, VAR_COLOR_MENU_BG.g, VAR_COLOR_MENU_BG.b, VAR_COLOR_MENU_BG.a);
        SDL_RenderFillRect(renderer, &layout->menu_rect);
        
        SDL_Rect item_rect = {layout->menu_rect.x + 5, layout->menu_rect.y + 5, layout->menu_rect.w - 10, 26};
        if (layout->hover.hover_menu_action) {
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_RED.r, VAR_COLOR_HOVER_RED.g, VAR_COLOR_HOVER_RED.b, VAR_COLOR_HOVER_RED.a);
            SDL_RenderFillRect(renderer, &item_rect);
        }
        draw_text(renderer, font_sub, "Delete", item_rect.x + 10, item_rect.y + 5, color_white);
    }

    // 7. Channel Creation Modal
    if (layout->show_create_modal) {
        // Dark translucent background layer
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, layout->window_w, layout->window_h};
        SDL_RenderFillRect(renderer, &overlay);

        // Modal Frame Box
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHANNELS.r, VAR_COLOR_BG_CHANNELS.g, VAR_COLOR_BG_CHANNELS.b, VAR_COLOR_BG_CHANNELS.a);
        SDL_RenderFillRect(renderer, &modal_bg_rect);

        draw_text(renderer, font_title, "Create a Channel", modal_bg_rect.x + 30, modal_bg_rect.y + 25, color_white);
        draw_text(renderer, font_sub, "CHANNEL NAME", modal_bg_rect.x + 30, modal_bg_rect.y + 70, color_muted);

        // Text input field box
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_SERVERS.r, VAR_COLOR_BG_SERVERS.g, VAR_COLOR_BG_SERVERS.b, VAR_COLOR_BG_SERVERS.a);
        SDL_RenderFillRect(renderer, &modal_input_rect);

        if (strlen(layout->modal_name_buffer) == 0) {
            draw_text(renderer, font_main, "new-channel", modal_input_rect.x + 12, modal_input_rect.y + 10, color_muted);
        } else {
            draw_text(renderer, font_main, layout->modal_name_buffer, modal_input_rect.x + 12, modal_input_rect.y + 10, color_white);
        }

        // Focus visual indicator line
        if (layout->modal_focused_field == 1) {
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_DISCORD_BLUE.r, VAR_COLOR_DISCORD_BLUE.g, VAR_COLOR_DISCORD_BLUE.b, VAR_COLOR_DISCORD_BLUE.a);
            SDL_RenderDrawRect(renderer, &modal_input_rect);
        }

        // Toggle Private Switch Label
        draw_text(renderer, font_main, "🔒 Private Channel", modal_bg_rect.x + 30, modal_bg_rect.y + 162, color_white);
        if (layout->modal_is_private) {
            SDL_SetRenderDrawColor(renderer, 0x23, 0xA5, 0x5A, 0xFF); // Green active switch
        } else {
            SDL_SetRenderDrawColor(renderer, 0x80, 0x84, 0x8E, 0xFF); // Gray inactive switch
        }
        SDL_RenderFillRect(renderer, &modal_toggle_rect);

        // Cancel Button Action
        SDL_SetRenderDrawColor(renderer, 0x4E, 0x50, 0x58, 0xFF);
        SDL_RenderFillRect(renderer, &modal_btn_cancel);
        draw_text(renderer, font_sub, "Cancel", modal_btn_cancel.x + 24, modal_btn_cancel.y + 11, color_white);

        // Confirm / Create Button Action
        SDL_SetRenderDrawColor(renderer, VAR_COLOR_DISCORD_BLUE.r, VAR_COLOR_DISCORD_BLUE.g, VAR_COLOR_DISCORD_BLUE.b, VAR_COLOR_DISCORD_BLUE.a);
        SDL_RenderFillRect(renderer, &modal_btn_ok);
        draw_text(renderer, font_sub, "Create", modal_btn_ok.x + 28, modal_btn_ok.y + 11, color_white);
    }
}

int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub) {
    ChatLayout layout;
    memset(&layout, 0, sizeof(ChatLayout));

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ui_chat_handle_resize(&layout, w, h);

    chat_controller_init(&layout, renderer);

    int running = 1;
    int exit_status = 1; // 1 = Exit application, 2 = Logged out (back to Auth)
    SDL_Event event;

    SDL_StartTextInput();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                exit_status = 1;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    ui_chat_handle_resize(&layout, event.window.data1, event.window.data2);
                }
            }
            else if (event.type == SDL_MOUSEMOTION) {
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

    chat_controller_destroy(&layout);
    return exit_status;
}
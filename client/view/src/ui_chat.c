#include "../include/ui_chat.h"
#include "../include/ui_chat_store.h"
#include "../include/ui_chat_components.h"
#include "../include/ui_channels.h"
#include "../include/ui_users.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_SERVERS   0x0D, 0x0E, 0x12, 0xFF 
#define COLOR_BG_CHANNELS  0x12, 0x14, 0x1C, 0xFF 
#define COLOR_BG_CHAT      0x16, 0x19, 0x24, 0xFF 
#define COLOR_BG_MEMBERS   0x12, 0x14, 0x1C, 0xFF 
#define COLOR_BG_INPUT     0x1F, 0x23, 0x33, 0xFF 

void update_chat_layout(ChatLayout *layout, int win_w, int win_h) {
    layout->window_w = win_w;
    layout->window_h = win_h;

    int servers_w  = 0;   
    int channels_w = 240;  
    int members_w  = 240;  
    int chat_w     = win_w - (servers_w + channels_w + members_w);

    if (chat_w < 350) {
        members_w = 0;
        chat_w = win_w - (servers_w + channels_w);
    }

    layout->sidebar_servers  = (SDL_Rect){0, 0, servers_w, win_h};
    layout->sidebar_channels = (SDL_Rect){servers_w, 0, channels_w, win_h};
    layout->chat_area        = (SDL_Rect){servers_w + channels_w, 0, chat_w, win_h};
    layout->sidebar_members  = (SDL_Rect){win_w - members_w, 0, members_w, win_h};

    layout->chat_top_bar   = (SDL_Rect){layout->chat_area.x, 0, layout->chat_area.w, 48};
    layout->chat_input_bar = (SDL_Rect){layout->chat_area.x + 16, win_h - 68, layout->chat_area.w - 32, 44};

    channels_update_layout(layout, win_h);

    int modal_w = 440;
    int modal_h = 280;
    modal_bg_rect = (SDL_Rect){ (win_w - modal_w) / 2, (win_h - modal_h) / 2, modal_w, modal_h };
    
    modal_input_rect   = (SDL_Rect){ modal_bg_rect.x + 30, modal_bg_rect.y + 90,  modal_bg_rect.w - 60, 40 };
    modal_toggle_rect  = (SDL_Rect){ modal_bg_rect.x + 30, modal_bg_rect.y + 175, 50, 24 };
    modal_btn_cancel   = (SDL_Rect){ modal_bg_rect.x + 210, modal_bg_rect.y + 220, 90, 36 };
    modal_btn_confirm  = (SDL_Rect){ modal_bg_rect.x + 310, modal_bg_rect.y + 220, 100, 36 };

    if (layout->menu_type > 0) {
        layout->menu_rect = (SDL_Rect){layout->menu_x, layout->menu_y, 130, 32};
    }
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub) {
    SDL_Color white_color   = {255, 255, 255, 255};
    SDL_Color gray_color    = {148, 155, 164, 255};
    SDL_Color dark_gray     = {94, 99, 106, 255};
    SDL_Color green_online  = {35, 165, 90, 255};
    SDL_Color blurple_color = {88, 101, 242, 255};
    SDL_Color red_delete    = {242, 63, 67, 255};

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_SetRenderDrawColor(renderer, COLOR_BG_SERVERS); SDL_RenderFillRect(renderer, &layout->sidebar_servers);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHANNELS); SDL_RenderFillRect(renderer, &layout->sidebar_channels);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHAT); SDL_RenderFillRect(renderer, &layout->chat_area);
    
    SDL_SetRenderDrawColor(renderer, 0x0D, 0x0E, 0x12, 0xFF);
    SDL_RenderDrawLine(renderer, layout->chat_top_bar.x, layout->chat_top_bar.h, layout->chat_top_bar.x + layout->chat_top_bar.w, layout->chat_top_bar.h);

    LocalChannel *active_ch = store_get_active_channel();

    SDL_SetRenderDrawColor(renderer, COLOR_BG_INPUT);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);

    if (active_ch) {
        if (strlen(layout->input_buffer) == 0) {
            char placeholder[64];
            snprintf(placeholder, sizeof(placeholder), "Envoyer un message dans %s %s", active_ch->is_private ? "🔒" : "#", active_ch->name);
            components_draw_text(renderer, font_main, placeholder, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, dark_gray);
        } else {
            components_draw_text(renderer, font_main, layout->input_buffer, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, white_color);
        }
    }

    if (layout->is_input_focused) {
        SDL_SetRenderDrawColor(renderer, blurple_color.r, blurple_color.g, blurple_color.b, blurple_color.a);
        SDL_RenderDrawRect(renderer, &layout->chat_input_bar);
        components_draw_cursor(renderer, font_main, layout->input_buffer, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, 18, white_color);
    }

    if (active_ch) {
        char title_buffer[64];
        snprintf(title_buffer, sizeof(title_buffer), "%s %s", active_ch->is_private ? "🔒" : "#", active_ch->name);
        components_draw_text(renderer, font_title, title_buffer, layout->chat_top_bar.x + 20, 13, white_color);
    }

    channels_draw_sidebar(renderer, layout, font_title, font_main, font_sub, mx, my, white_color, gray_color, dark_gray, red_delete);

    if (layout->sidebar_members.w > 0) {
        SDL_SetRenderDrawColor(renderer, COLOR_BG_MEMBERS); SDL_RenderFillRect(renderer, &layout->sidebar_members);
        users_draw_sidebar(renderer, layout, font_main, font_sub, green_online, dark_gray);
    }

    if (active_ch) {
        int chat_x = layout->chat_area.x + 20; int base_y = 70;
        for (int i = 0; i < active_ch->message_count; i++) {
            components_draw_text(renderer, font_sub, active_ch->messages[i].username, chat_x, base_y, green_online);
            components_draw_text(renderer, font_main, active_ch->messages[i].text, chat_x, base_y + 22, white_color);
            base_y += 55; 
        }
    }

    if (layout->show_create_modal) {
        components_draw_modal(renderer, layout, font_title, font_main, font_sub, mx, my, white_color, gray_color, dark_gray);
        if (layout->modal_focused_field == 1) {
            components_draw_cursor(renderer, font_main, layout->modal_name_buffer, modal_input_rect.x + 12, modal_input_rect.y + 8, 18, white_color);
        }
    }

    if (layout->menu_type > 0) {
        int h_menu = (mx >= layout->menu_rect.x && mx <= layout->menu_rect.x + layout->menu_rect.w && my >= layout->menu_rect.y && my <= layout->menu_rect.y + layout->menu_rect.h);
        SDL_SetRenderDrawColor(renderer, h_menu ? 242 : 25, h_menu ? 63 : 28, h_menu ? 67 : 38, 255);
        SDL_RenderFillRect(renderer, &layout->menu_rect);
        SDL_SetRenderDrawColor(renderer, 0x0D, 0x0E, 0x12, 255);
        SDL_RenderDrawRect(renderer, &layout->menu_rect);
        components_draw_text(renderer, font_sub, "Supprimer", layout->menu_rect.x + 35, layout->menu_rect.y + 8, white_color);
    }
}

// ➡️ CHANGEMENT : On passe de void à int pour renvoyer le statut de fermeture
int chat_ui_init_and_run(void) {
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 18);
    TTF_Font *font_main  = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    TTF_Font *font_sub   = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 13);
    if (!font_title || !font_main || !font_sub) return 0;

    SDL_Window *window = SDL_CreateWindow("myDiscord - Chat Hub", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1100, 700, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    ChatLayout layout; memset(&layout, 0, sizeof(ChatLayout));
    store_init(); 

    SDL_StartTextInput();
    int running = 1; 
    int exit_status = 0; // ➡️ 0 = Tout fermer (Clic sur la croix), 2 = Se déconnecter (Retour Login)
    SDL_Event event;

    while (running) {
        int win_w, win_h; SDL_GetWindowSize(window, &win_w, &win_h);
        update_chat_layout(&layout, win_w, win_h);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
                exit_status = 0; // On ferme définitivement l'application
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int cx = event.button.x; int cy = event.button.y;

                if (event.button.button == SDL_BUTTON_RIGHT) {
                    layout.menu_type = 0;

                    int channel_start_y = 55;
                    for (int i = 0; i < store_get_channel_count(); i++) {
                        if (cx >= layout.sidebar_channels.x && cx <= (layout.sidebar_channels.x + layout.sidebar_channels.w) && cy >= channel_start_y - 4 && cy <= channel_start_y + 24) {
                            layout.menu_type = 1;
                            layout.target_index = i;
                            layout.menu_x = cx; layout.menu_y = cy;
                            break;
                        }
                        channel_start_y += 32;
                    }

                    if (layout.menu_type == 0 && cx >= layout.chat_area.x && cx <= (layout.chat_area.x + layout.chat_area.w)) {
                        LocalChannel *active_ch = store_get_active_channel();
                        if (active_ch) {
                            int base_y = 70;
                            for (int i = 0; i < active_ch->message_count; i++) {
                                if (cy >= base_y && cy <= base_y + 45) {
                                    layout.menu_type = 2;
                                    layout.target_index = i;
                                    layout.menu_x = cx; layout.menu_y = cy;
                                    break;
                                }
                                base_y += 55;
                            }
                        }
                    }
                } 
                else if (event.button.button == SDL_BUTTON_LEFT) {
                    if (layout.menu_type > 0) {
                        if (cx >= layout.menu_rect.x && cx <= layout.menu_rect.x + layout.menu_rect.w && cy >= layout.menu_rect.y && cy <= layout.menu_rect.y + layout.menu_rect.h) {
                            if (layout.menu_type == 1) store_delete_channel_by_index(layout.target_index);
                            else if (layout.menu_type == 2) store_delete_message_by_index(layout.target_index);
                        }
                        layout.menu_type = 0;
                    }
                    else if (layout.show_create_modal) {
                        if (cx >= modal_input_rect.x && cx <= modal_input_rect.x + modal_input_rect.w && cy >= modal_input_rect.y && cy <= modal_input_rect.y + modal_input_rect.h)
                            layout.modal_focused_field = 1;
                        else layout.modal_focused_field = 0;

                        if (cx >= modal_toggle_rect.x && cx <= modal_toggle_rect.x + modal_toggle_rect.w && cy >= modal_toggle_rect.y && cy <= modal_toggle_rect.y + modal_toggle_rect.h)
                            layout.modal_is_private = !layout.modal_is_private;

                        if (cx >= modal_btn_cancel.x && cx <= modal_btn_cancel.x + modal_btn_cancel.w && cy >= modal_btn_cancel.y && cy <= modal_btn_cancel.y + modal_btn_cancel.h)
                            layout.show_create_modal = 0;

                        if (cx >= modal_btn_confirm.x && cx <= modal_btn_confirm.x + modal_btn_confirm.w && cy >= modal_btn_confirm.y && cy <= modal_btn_confirm.y + modal_btn_confirm.h) {
                            if (strlen(layout.modal_name_buffer) > 0) store_add_channel(layout.modal_name_buffer, layout.modal_is_private);
                            layout.show_create_modal = 0;
                        }
                    } else {
                        if (cx >= btn_options.x && cx <= btn_options.x + btn_options.w && cy >= btn_options.y && cy <= btn_options.y + btn_options.h) {
                            printf("[LOG] Ouverture des Options d'Utilisateur...\n");
                        }
                        
                        // ➡️ MODIFICATION ICI : Clic sur Déconnexion
                        if (cx >= btn_logout.x && cx <= btn_logout.x + btn_logout.w && cy >= btn_logout.y && cy <= btn_logout.y + btn_logout.h) {
                            printf("[LOG] Déconnexion demandée.\n");
                            running = 0; 
                            exit_status = 2; // 💡 Code 2 pour demander un retour au login
                        }

                        layout.is_input_focused = (cx >= layout.chat_input_bar.x && cx <= (layout.chat_input_bar.x + layout.chat_input_bar.w) && cy >= layout.chat_input_bar.y && cy <= (layout.chat_input_bar.y + layout.chat_input_bar.h));

                        if (cx >= btn_add_channel.x && cx <= (btn_add_channel.x + btn_add_channel.w) && cy >= btn_add_channel.y && cy <= (btn_add_channel.y + btn_add_channel.h)) {
                            layout.show_create_modal = 1; layout.modal_is_private = 0; layout.modal_focused_field = 1; layout.modal_name_buffer[0] = '\0';
                        }
                        
                        int channel_start_y = 55;
                        for (int i = 0; i < store_get_channel_count(); i++) {
                            if (cx >= layout.sidebar_channels.x && cx <= (layout.sidebar_channels.x + layout.sidebar_channels.w) && cy >= channel_start_y - 4 && cy <= channel_start_y + 22) {
                                store_set_active_index(i); break;
                            }
                            channel_start_y += 32;
                        }
                    }
                }
            }

            if (event.type == SDL_KEYDOWN) {
                if (layout.show_create_modal && layout.modal_focused_field == 1 && event.key.keysym.sym == SDLK_BACKSPACE) {
                    size_t len = strlen(layout.modal_name_buffer);
                    if (len > 0) layout.modal_name_buffer[len - 1] = '\0';
                } else if (layout.is_input_focused) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        size_t len = strlen(layout.input_buffer);
                        if (len > 0) layout.input_buffer[len - 1] = '\0';
                    } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if (strlen(layout.input_buffer) > 0) {
                            store_add_message_to_active("Moi (Local)", layout.input_buffer);
                            layout.input_buffer[0] = '\0';
                        }
                    }
                }
            }

            if (event.type == SDL_TEXTINPUT) {
                if (layout.show_create_modal && layout.modal_focused_field == 1 && strlen(layout.modal_name_buffer) + strlen(event.text.text) < 31)
                    strcat(layout.modal_name_buffer, event.text.text);
                else if (layout.is_input_focused && strlen(layout.input_buffer) + strlen(event.text.text) < MAX_MSG_LENGTH - 1)
                    strcat(layout.input_buffer, event.text.text);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); SDL_RenderClear(renderer);
        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);
        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    TTF_CloseFont(font_title); TTF_CloseFont(font_main); TTF_CloseFont(font_sub);
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);

    return exit_status; // ➡️ Renvoie 0 ou 2 au main.c
}
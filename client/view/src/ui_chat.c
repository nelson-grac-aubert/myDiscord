#include "../include/ui_chat.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_SERVERS   0x1E, 0x1F, 0x22, 0xFF 
#define COLOR_BG_CHANNELS  0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_BG_CHAT      0x31, 0x33, 0x38, 0xFF 
#define COLOR_BG_MEMBERS   0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_BG_INPUT     0x38, 0x3A, 0x40, 0xFF 

static void local_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    if (!font || !text || text[0] == '\0') return;
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dst_rect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void update_chat_layout(ChatLayout *layout, int win_w, int win_h) {
    layout->window_w = win_w;
    layout->window_h = win_h;

    int servers_w  = 72;   
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
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub) {
    
    SDL_Color white_color   = {255, 255, 255, 255};
    SDL_Color gray_color    = {148, 155, 164, 255};
    SDL_Color dark_gray     = {94, 99, 106, 255};
    SDL_Color green_online  = {35, 165, 90, 255};

    // 1. Dessin des grands panneaux d'arrière-plan
    SDL_SetRenderDrawColor(renderer, COLOR_BG_SERVERS);
    SDL_RenderFillRect(renderer, &layout->sidebar_servers);

    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHANNELS);
    SDL_RenderFillRect(renderer, &layout->sidebar_channels);

    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHAT);
    SDL_RenderFillRect(renderer, &layout->chat_area);

    if (layout->sidebar_members.w > 0) {
        SDL_SetRenderDrawColor(renderer, COLOR_BG_MEMBERS);
        SDL_RenderFillRect(renderer, &layout->sidebar_members);
    }

    // 2. Séparation fine sous la barre du haut
    SDL_SetRenderDrawColor(renderer, 0x1F, 0x20, 0x23, 0xFF);
    SDL_RenderDrawLine(renderer, layout->chat_top_bar.x, layout->chat_top_bar.h, 
                                 layout->chat_top_bar.x + layout->chat_top_bar.w, layout->chat_top_bar.h);

    // 3. Zone d'entrée de texte
    SDL_SetRenderDrawColor(renderer, COLOR_BG_INPUT);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);

    // Rendu dynamique du texte saisi ou du placeholder indicatif
    if (strlen(layout->input_buffer) == 0) {
        local_draw_text(renderer, font_main, "Envoyer un message dans # general", layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, dark_gray);
    } else {
        local_draw_text(renderer, font_main, layout->input_buffer, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, white_color);
    }

    // Bordure bleue claire si l'input a le focus
    if (layout->is_input_focused) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0xAA, 0xFF, 0xFF);
        SDL_RenderDrawRect(renderer, &layout->chat_input_bar);
    }

    // A. Titre du salon actuel
    local_draw_text(renderer, font_title, "# general", layout->chat_top_bar.x + 20, 13, white_color);

    // B. Contenu de la colonne des Salons
    local_draw_text(renderer, font_sub, "SALONS TEXTUELS", layout->sidebar_channels.x + 15, 20, dark_gray);
    local_draw_text(renderer, font_main, "#  general", layout->sidebar_channels.x + 15, 55, white_color);
    local_draw_text(renderer, font_main, "#  gaming-hub", layout->sidebar_channels.x + 15, 85, gray_color);
    local_draw_text(renderer, font_main, "#  off-topic", layout->sidebar_channels.x + 15, 115, gray_color);

    // C. Contenu de la colonne des Membres
    if (layout->sidebar_members.w > 0) {
        local_draw_text(renderer, font_sub, "EN LIGNE — 3", layout->sidebar_members.x + 20, 20, dark_gray);
        local_draw_text(renderer, font_main, "●  Nexus_One", layout->sidebar_members.x + 20, 55, green_online);
        local_draw_text(renderer, font_main, "●  SARA_X", layout->sidebar_members.x + 20, 85, green_online);
        local_draw_text(renderer, font_main, "●  AlexK", layout->sidebar_members.x + 20, 115, green_online);
    }

    // D. HISTORIQUE DYNAMIQUE DES MESSAGES EN MÉMOIRE RAM LOCAL
    int chat_x = layout->chat_area.x + 20;
    int base_y = 70;

    for (int i = 0; i < layout->message_count; i++) {
        // Pseudo
        local_draw_text(renderer, font_sub, layout->messages[i].username, chat_x, base_y, green_online);
        // Message texte
        local_draw_text(renderer, font_main, layout->messages[i].text, chat_x, base_y + 22, white_color);
        
        base_y += 55; // Espacement vertical automatique entre chaque message
    }
}

void chat_ui_init_and_run(void) {
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 18);
    TTF_Font *font_main  = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    TTF_Font *font_sub   = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 13);

    if (!font_title || !font_main || !font_sub) {
        printf("[CHAT ERROR] Impossible de charger les polices.\n");
        return;
    }

    SDL_Window *window = SDL_CreateWindow("myDiscord - Chat Hub", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          1100, 700, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Cursor *cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL_Cursor *cursor_ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    // Initialisation propre de la structure de layout et de message
    ChatLayout layout;
    memset(layout.input_buffer, 0, MAX_MSG_LENGTH);
    layout.is_input_focused = 0;
    layout.message_count = 0;

    // Simulation de quelques messages au chargement
    strcpy(layout.messages[0].username, "Nexus_One");
    strcpy(layout.messages[0].text, "Salut tout le monde ! Le serveur SDL avance bien ?");
    strcpy(layout.messages[1].username, "SARA_X");
    strcpy(layout.messages[1].text, "Carrément ! Le design responsive fonctionne parfaitement.");
    layout.message_count = 2;

    SDL_StartTextInput();
    int running = 1;
    SDL_Event event;

    while (running) {
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);
        update_chat_layout(&layout, win_w, win_h);

        // Curseur dynamique au survol de l'input texte
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        if (mx >= layout.chat_input_bar.x && mx <= (layout.chat_input_bar.x + layout.chat_input_bar.w) &&
            my >= layout.chat_input_bar.y && my <= (layout.chat_input_bar.y + layout.chat_input_bar.h)) {
            SDL_SetCursor(cursor_ibeam);
        } else {
            SDL_SetCursor(cursor_arrow);
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }

            // Gestion du Clic Souris pour activer la saisie
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int cx = event.button.x;
                int cy = event.button.y;

                if (cx >= layout.chat_input_bar.x && cx <= (layout.chat_input_bar.x + layout.chat_input_bar.w) &&
                    cy >= layout.chat_input_bar.y && cy <= (layout.chat_input_bar.y + layout.chat_input_bar.h)) {
                    layout.is_input_focused = 1;
                } else {
                    layout.is_input_focused = 0;
                }
            }

            // Gestion des touches du Clavier
            if (event.type == SDL_KEYDOWN && layout.is_input_focused) {
                // Effacer un caractère (Retour arrière)
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    size_t len = strlen(layout.input_buffer);
                    if (len > 0) layout.input_buffer[len - 1] = '\0';
                }
                // Envoyer le message (Touche Entrée)
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                    if (strlen(layout.input_buffer) > 0 && layout.message_count < MAX_MESSAGES) {
                        // Stockage local du message tapé
                        strcpy(layout.messages[layout.message_count].username, "Moi (Local)");
                        strcpy(layout.messages[layout.message_count].text, layout.input_buffer);
                        layout.message_count++;

                        // Remise à zéro sécurisée de la barre d'input
                        layout.input_buffer[0] = '\0';
                    }
                }
            }

            // Saisie textuelle standard (Lettres, chiffres, espace...)
            if (event.type == SDL_TEXTINPUT && layout.is_input_focused) {
                if (strlen(layout.input_buffer) + strlen(event.text.text) < MAX_MSG_LENGTH - 1) {
                    strcat(layout.input_buffer, event.text.text);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_FreeCursor(cursor_arrow);
    SDL_FreeCursor(cursor_ibeam);
    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
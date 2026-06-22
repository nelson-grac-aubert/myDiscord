#include "../include/ui_chat.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_SERVERS   0x1E, 0x1F, 0x22, 0xFF 
#define COLOR_BG_CHANNELS  0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_BG_CHAT      0x31, 0x33, 0x38, 0xFF 
#define COLOR_BG_MEMBERS   0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_BG_INPUT     0x38, 0x3A, 0x40, 0xFF 

// Fonction interne pour dessiner les textes dans le chat
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

    int servers_w  = 72;   // Largeur de la barre des serveurs Discord
    int channels_w = 240;  // Largeur de la liste des salons
    int members_w  = 240;  // Largeur de la liste des membres à droite
    int chat_w     = win_w - (servers_w + channels_w + members_w);

    // Si l'utilisateur réduit trop la fenêtre, on masque automatiquement la liste des membres
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

    // 1. Dessin des 4 grands panneaux d'arrière-plan
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

    // 2. Séparation fine sous la barre du haut (# general)
    SDL_SetRenderDrawColor(renderer, 0x1F, 0x20, 0x23, 0xFF);
    SDL_RenderDrawLine(renderer, layout->chat_top_bar.x, layout->chat_top_bar.h, 
                                 layout->chat_top_bar.x + layout->chat_top_bar.w, layout->chat_top_bar.h);

    // 3. Zone d'entrée de texte du message (en bas)
    SDL_SetRenderDrawColor(renderer, COLOR_BG_INPUT);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);
    local_draw_text(renderer, font_main, "Envoyer un message dans # general", layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, dark_gray);

    // ----------------------------------------------------------------
    // AJOUT DU CONTENU VISUEL (Salons, Membres, Messages)
    // ----------------------------------------------------------------

    // A. Titre du salon actuel dans la barre du haut
    local_draw_text(renderer, font_title, "# general", layout->chat_top_bar.x + 20, 13, white_color);

    // B. Contenu de la colonne des Salons (sidebar_channels)
    local_draw_text(renderer, font_sub, "SALONS TEXTUELS", layout->sidebar_channels.x + 15, 20, dark_gray);
    local_draw_text(renderer, font_main, "#  general", layout->sidebar_channels.x + 15, 55, white_color);
    local_draw_text(renderer, font_main, "#  gaming-hub", layout->sidebar_channels.x + 15, 85, gray_color);
    local_draw_text(renderer, font_main, "#  off-topic", layout->sidebar_channels.x + 15, 115, gray_color);

    // C. Contenu de la colonne des Membres (sidebar_members)
    if (layout->sidebar_members.w > 0) {
        local_draw_text(renderer, font_sub, "EN LIGNE — 3", layout->sidebar_members.x + 20, 20, dark_gray);
        local_draw_text(renderer, font_main, "●  Nexus_One", layout->sidebar_members.x + 20, 55, green_online);
        local_draw_text(renderer, font_main, "●  SARA_X", layout->sidebar_members.x + 20, 85, green_online);
        local_draw_text(renderer, font_main, "●  AlexK", layout->sidebar_members.x + 20, 115, green_online);
    }

    // D. Faux historique de messages (Zone centrale)
    int chat_x = layout->chat_area.x + 20;
    
    local_draw_text(renderer, font_sub, "Nexus_One", chat_x, 80, green_online);
    local_draw_text(renderer, font_main, "Salut tout le monde ! Le serveur SDL avance bien ?", chat_x, 102, white_color);

    local_draw_text(renderer, font_sub, "SARA_X", chat_x, 145, green_online);
    local_draw_text(renderer, font_main, "Carrément ! Le design responsive fonctionne parfaitement.", chat_x, 167, white_color);

    local_draw_text(renderer, font_sub, "AlexK", chat_x, 210, green_online);
    local_draw_text(renderer, font_main, "Essaye d'étirer la fenêtre pour voir la liste des membres se masquer !", chat_x, 232, white_color);
}

void chat_ui_init_and_run(void) {
    // Les tailles ont été réduites légèrement pour coller à la densité visuelle d'un chat
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

    ChatLayout layout;
    int running = 1;
    SDL_Event event;

    while (running) {
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);
        update_chat_layout(&layout, win_w, win_h);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
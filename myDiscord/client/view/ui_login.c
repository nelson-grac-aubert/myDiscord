#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

// Définition des couleurs Kinetic Obsidian
#define COLOR_BG_OBSIDIAN  0x11, 0x12, 0x14, 0xFF
#define COLOR_CARD_GRAY    0x1E, 0x1F, 0x22, 0xFF
#define COLOR_DISCORD_BLUE 0x58, 0x65, 0xF2, 0xFF

// Énumération pour gérer l'onglet actif
typedef enum {
    TAB_LOGIN,
    TAB_REGISTER
} AuthTab;

// Fonction utilitaire optimisée pour dessiner du texte en SDL2
void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    if (!font || !text || text[0] == '\0') return;

    // Création de la surface textuelle (Blit en mémoire)
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;

    // Conversion en texture (Prête pour la carte graphique)
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dst_rect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
        SDL_DestroyTexture(texture);
    }

    // Libération de la mémoire tampon
    SDL_FreeSurface(surface);
}

void ui_login_init_and_run(void) {
    // Initialisation
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return;
    }
    if (TTF_Init() == -1) {
        printf("Erreur SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return;
    }

    // Chargement global des polices pour éviter les accès disques répétés
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 26);
    TTF_Font *font_main  = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    TTF_Font *font_sub   = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 14);
    TTF_Font *font_label = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 11);

    if (!font_title || !font_main || !font_sub || !font_label) {
        printf("[SDL_ttf] Erreur chargement des fichiers de polices : %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_Window *window = SDL_CreateWindow(
        "myDiscord - SDL2 Version",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        900, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Définition des couleurs pour les polices
    SDL_Color white_color = {255, 255, 255, 255};
    SDL_Color gray_color  = {148, 155, 164, 255};

    AuthTab current_tab = TAB_LOGIN;
    SDL_Rect btn_submit = {0, 0, 0, 0};
    SDL_Rect tab_login_rect = {0, 0, 0, 0};
    SDL_Rect tab_reg_rect = {0, 0, 0, 0};

    int running = 1;
    SDL_Event event;

    while (running) {
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);

        // --- 1. CALCULS DES COORDONNÉES DYNAMIQUES D'ABORD ---
        // La carte centrale
        SDL_Rect card_rect;
        card_rect.w = 450;
        card_rect.h = 520;
        card_rect.x = (win_w - card_rect.w) / 2;
        card_rect.y = (win_h - card_rect.h) / 2;

        // Zones de clic virtuelles pour les onglets du haut
        tab_login_rect.x = card_rect.x + 60;
        tab_login_rect.y = card_rect.y + 20;
        tab_login_rect.w = 120;
        tab_login_rect.h = 30;

        tab_reg_rect.x = card_rect.x + 260;
        tab_reg_rect.y = card_rect.y + 20;
        tab_reg_rect.w = 120;
        tab_reg_rect.h = 30;

        // Les Inputs (Gris foncé)
        SDL_Rect email_input = { card_rect.x + 40, card_rect.y + 160, 370, 40 };
        SDL_Rect pass_input  = { card_rect.x + 40, card_rect.y + 260, 370, 40 };
        SDL_Rect user_input  = { card_rect.x + 40, card_rect.y + 360, 370, 40 }; // Pour l'inscription

        // Le bouton de validation
        btn_submit.w = 370;
        btn_submit.h = 45;
        btn_submit.x = card_rect.x + (card_rect.w - btn_submit.w) / 2;
        btn_submit.y = card_rect.y + card_rect.h - 80;


        // --- 2. GESTION DES ÉVÉNEMENTS APRÈS CALCUL ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouse_x = event.button.x;
                    int mouse_y = event.button.y;

                    // Clic sur l'onglet SE CONNECTER
                    if (mouse_x >= tab_login_rect.x && mouse_x <= (tab_login_rect.x + tab_login_rect.w) &&
                        mouse_y >= tab_login_rect.y && mouse_y <= (tab_login_rect.y + tab_login_rect.h)) {
                        current_tab = TAB_LOGIN;
                    }

                    // Clic sur l'onglet S'INSCRIRE
                    if (mouse_x >= tab_reg_rect.x && mouse_x <= (tab_reg_rect.x + tab_reg_rect.w) &&
                        mouse_y >= tab_reg_rect.y && mouse_y <= (tab_reg_rect.y + tab_reg_rect.h)) {
                        current_tab = TAB_REGISTER;
                    }

                    // Clic sur le bouton principal (Validation)
                    if (mouse_x >= btn_submit.x && mouse_x <= (btn_submit.x + btn_submit.w) &&
                        mouse_y >= btn_submit.y && mouse_y <= (btn_submit.y + btn_submit.h)) {
                        if (current_tab == TAB_LOGIN) {
                            printf("[SDL2] Soumission du formulaire de CONNEXION !\n");
                        } else {
                            printf("[SDL2] Soumission du formulaire d'INSCRIPTION !\n");
                        }
                        fflush(stdout); // Force l'écriture immédiate dans le terminal Windows
                    }
                }
            }
        }

        // --- 3. RENDU GRAPHIQUE ---
        // Fond d'écran
        SDL_SetRenderDrawColor(renderer, COLOR_BG_OBSIDIAN);
        SDL_RenderClear(renderer);

        // Dessin de la carte
        SDL_SetRenderDrawColor(renderer, COLOR_CARD_GRAY);
        SDL_RenderFillRect(renderer, &card_rect);

        // Dessin des bordures d'Inputs de saisie
        SDL_SetRenderDrawColor(renderer, COLOR_BG_OBSIDIAN);
        SDL_RenderFillRect(renderer, &email_input);
        SDL_RenderFillRect(renderer, &pass_input);
        if (current_tab == TAB_REGISTER) {
            SDL_RenderFillRect(renderer, &user_input);
        }

        // Dessin du bouton de validation
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
        SDL_RenderFillRect(renderer, &btn_submit);

        // --- 4. AFFICHAGE DES TEXTES ---
        // Onglets du haut (Bascule visuelle de surbrillance)
        draw_text(renderer, font_sub, "SE CONNECTER", tab_login_rect.x, tab_login_rect.y, (current_tab == TAB_LOGIN) ? white_color : gray_color);
        draw_text(renderer, font_sub, "S'INSCRIRE", tab_reg_rect.x, tab_reg_rect.y, (current_tab == TAB_REGISTER) ? white_color : gray_color);
        
        if (current_tab == TAB_LOGIN) {
            // Vue Connexion
            draw_text(renderer, font_title, "myDiscord", card_rect.x + 160, card_rect.y + 70, white_color);
            draw_text(renderer, font_main, "Ravi de vous revoir !", card_rect.x + 150, card_rect.y + 110, gray_color);

            draw_text(renderer, font_label, "EMAIL OU NUMÉRO DE TÉLÉPHONE", card_rect.x + 40, card_rect.y + 140, gray_color);
            draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 240, gray_color);

            draw_text(renderer, font_main, "Se connecter", btn_submit.x + 135, btn_submit.y + 12, white_color);
        } else {
            // Vue Inscription
            draw_text(renderer, font_title, "Créer un compte", card_rect.x + 130, card_rect.y + 70, white_color);
            
            draw_text(renderer, font_label, "ADRESSE EMAIL", card_rect.x + 40, card_rect.y + 140, gray_color);
            draw_text(renderer, font_label, "NOM D'UTILISATEUR", card_rect.x + 40, card_rect.y + 240, gray_color);
            draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 340, gray_color);

            draw_text(renderer, font_main, "Continuer", btn_submit.x + 150, btn_submit.y + 12, white_color);
        }

        SDL_RenderPresent(renderer);
    }

    // Nettoyage de la mémoire à la fermeture
    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    TTF_CloseFont(font_label);
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
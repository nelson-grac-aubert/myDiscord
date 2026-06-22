#include "../include/welcome.h"
#include "../include/ui_login.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_OBSIDIAN 0x11, 0x12, 0x14, 0xFF

void welcome_ui_init_and_run(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return;
    if (TTF_Init() == -1)
    {
        SDL_Quit();
        return;
    }

    // Le grand titre ("S'INSCRIRE" / "CONNEXION") : Gras et bien visible
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 24);
    // Le texte tapé par l'utilisateur et les placeholders : Épaisseur normale et lisible
    TTF_Font *font_main = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    // Les onglets du haut ("SE CONNECTER" / "S'INSCRIRE") et le bouton "Continuer" : SemiBold pour ressortir sur le fond
    TTF_Font *font_sub = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 14);
    // Les petites étiquettes ("ADRESSE EMAIL", etc.) : En gras (Bold ou SemiBold) car Discord écrit ses labels en petites capitales épaisses
    TTF_Font *font_label = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 11);

    if (!font_title || !font_main || !font_sub || !font_label)
    {
        TTF_Quit();
        SDL_Quit();
        return;
    }

    SDL_Window *window = SDL_CreateWindow("myDiscord", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 450, 520, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Cursor *cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL_Cursor *cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor *cursor_ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    UIState ui_state = {.current_tab = TAB_LOGIN, .current_focus = FOCUS_NONE};
    SDL_StartTextInput();

    int running = 1;
    SDL_Event event;

    while (running)
    {
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);

        // La carte commence en (0,0) et fait la taille exacte de la fenêtre
        SDL_Rect card_rect = {0, 0, 450, 520};
        // Tous les autres éléments restent parfaitement positionnés par rapport à card_rect !
        SDL_Rect btn_submit = {card_rect.x + (card_rect.w - 370) / 2, card_rect.y + card_rect.h - 60, 370, 45};
        SDL_Rect email_input = {card_rect.x + 40, card_rect.y + 180, 370, 40};
        SDL_Rect user_input = {card_rect.x + 40, card_rect.y + 280, 370, 40}; 
        SDL_Rect pass_input = {card_rect.x + 40, card_rect.y + 380, 370, 40}; 
        SDL_Rect tab_login_rect = {card_rect.x, card_rect.y, 225, 45};
        SDL_Rect tab_reg_rect = {card_rect.x + 225, card_rect.y, 225, 45};

        // --- GESTION DU CURSEUR ---
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        int is_hovering_button = (mouse_x >= btn_submit.x && mouse_x <= (btn_submit.x + btn_submit.w) && mouse_y >= btn_submit.y && mouse_y <= (btn_submit.y + btn_submit.h));

        if (is_hovering_button ||
            (mouse_x >= tab_login_rect.x && mouse_x <= (tab_login_rect.x + tab_login_rect.w) && mouse_y >= tab_login_rect.y && mouse_y <= (tab_login_rect.y + tab_login_rect.h)) ||
            (mouse_x >= tab_reg_rect.x && mouse_x <= (tab_reg_rect.x + tab_reg_rect.w) && mouse_y >= tab_reg_rect.y && mouse_y <= (tab_reg_rect.y + tab_reg_rect.h)))
        {
            SDL_SetCursor(cursor_hand);
        }
        else if ((mouse_x >= email_input.x && mouse_x <= (email_input.x + email_input.w) && mouse_y >= email_input.y && mouse_y <= (email_input.y + email_input.h)) ||
                 (mouse_x >= user_input.x && mouse_x <= (user_input.x + user_input.w) && mouse_y >= user_input.y && mouse_y <= (user_input.y + user_input.h)) ||
                 (ui_state.current_tab == TAB_REGISTER && mouse_x >= pass_input.x && mouse_x <= (pass_input.x + pass_input.w) && mouse_y >= pass_input.y && mouse_y <= (pass_input.y + pass_input.h)))
        {
            SDL_SetCursor(cursor_ibeam);
        }
        else
        {
            SDL_SetCursor(cursor_arrow);
        }

        // --- ENREGISTREMENT DES ÉVÉNEMENTS ---
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x;
                int my = event.button.y;

                // Clic sur les Onglets
                if (mx >= tab_login_rect.x && mx <= (tab_login_rect.x + tab_login_rect.w) && my >= tab_login_rect.y && my <= (tab_login_rect.y + tab_login_rect.h))
                {
                    ui_state.current_tab = TAB_LOGIN;
                    ui_state.current_focus = FOCUS_NONE;
                }
                else if (mx >= tab_reg_rect.x && mx <= (tab_reg_rect.x + tab_reg_rect.w) && my >= tab_reg_rect.y && my <= (tab_reg_rect.y + tab_reg_rect.h))
                {
                    ui_state.current_tab = TAB_REGISTER;
                    ui_state.current_focus = FOCUS_NONE;
                }
                // Champ du HAUT : Email
                else if (mx >= email_input.x && mx <= (email_input.x + email_input.w) && my >= email_input.y && my <= (email_input.y + email_input.h))
                {
                    ui_state.current_focus = FOCUS_EMAIL;
                }
                // Champ du MILIEU : Nom d'utilisateur (Inscription) OU Mot de passe (Connexion)
                else if (mx >= user_input.x && mx <= (user_input.x + user_input.w) && my >= user_input.y && my <= (user_input.y + user_input.h))
                {
                    if (ui_state.current_tab == TAB_REGISTER)
                    {
                        ui_state.current_focus = FOCUS_USERNAME;
                    }
                    else
                    {
                        ui_state.current_focus = FOCUS_PASSWORD;
                    }
                }
                // Champ du BAS : Mot de passe (seulement en inscription)
                else if (ui_state.current_tab == TAB_REGISTER && mx >= pass_input.x && mx <= (pass_input.x + pass_input.w) && my >= pass_input.y && my <= (pass_input.y + pass_input.h))
                {
                    ui_state.current_focus = FOCUS_PASSWORD;
                }
                // Bouton soumettre
                else if (is_hovering_button)
                {
                    if (ui_state.current_tab == TAB_LOGIN)
                        printf("[UI] Clic Se Connecter. Email: %s\n", ui_state.text_email);
                    else
                        printf("[UI] Clic Continuer (Inscription). User: %s\n", ui_state.text_username);
                    fflush(stdout);
                }
                else
                {
                    ui_state.current_focus = FOCUS_NONE;
                }
            }

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    char *buf = NULL;
                    if (ui_state.current_focus == FOCUS_EMAIL)
                        buf = ui_state.text_email;
                    else if (ui_state.current_focus == FOCUS_PASSWORD)
                        buf = ui_state.text_password;
                    else if (ui_state.current_focus == FOCUS_USERNAME)
                        buf = ui_state.text_username;
                    if (buf)
                    {
                        size_t len = strlen(buf);
                        if (len > 0)
                            buf[len - 1] = '\0';
                    }
                }
                if (event.key.keysym.sym == SDLK_TAB)
                {
                    if (ui_state.current_focus == FOCUS_EMAIL)
                    {
                        ui_state.current_focus = (ui_state.current_tab == TAB_REGISTER) ? FOCUS_USERNAME : FOCUS_PASSWORD;
                    }
                    else if (ui_state.current_focus == FOCUS_USERNAME)
                    {
                        ui_state.current_focus = FOCUS_PASSWORD;
                    }
                    else if (ui_state.current_focus == FOCUS_PASSWORD)
                    {
                        ui_state.current_focus = FOCUS_EMAIL;
                    }
                    else
                    {
                        ui_state.current_focus = FOCUS_EMAIL;
                    }
                }
            }

            if (event.type == SDL_TEXTINPUT)
            {
                char *buf = NULL;
                if (ui_state.current_focus == FOCUS_EMAIL)
                    buf = ui_state.text_email;
                else if (ui_state.current_focus == FOCUS_PASSWORD)
                    buf = ui_state.text_password;
                else if (ui_state.current_focus == FOCUS_USERNAME)
                    buf = ui_state.text_username;
                if (buf && (strlen(buf) + strlen(event.text.text) < 127))
                    strcat(buf, event.text.text);
            }
        }

        // --- RENDU GRAPHIQUE ---
        SDL_SetRenderDrawColor(renderer, COLOR_BG_OBSIDIAN);
        SDL_RenderClear(renderer);

        // Appel de l'interface dessinée dans login.c
        draw_login_interface(renderer, card_rect, &ui_state, font_title, font_main, font_sub, font_label, is_hovering_button);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_FreeCursor(cursor_arrow);
    SDL_FreeCursor(cursor_hand);
    SDL_FreeCursor(cursor_ibeam);
    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    TTF_CloseFont(font_label);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
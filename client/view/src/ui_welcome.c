#include "../include/ui_welcome.h"
#include "../include/ui_login.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_OBSIDIAN 0x11, 0x12, 0x14, 0xFF

// ➡️ CHANGEMENT : Retourne un int (1 = connecté/inscrit, 0 = fermeture par la croix)
int welcome_ui_init_and_run(void)
{
    // On retire SDL_Init et TTF_Init d'ici, car c'est le main.c qui va s'en occuper au démarrage global !

    // Utilisation des graisses pour correspondre à la hiérarchie de l'image
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 26);
    TTF_Font *font_main  = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    TTF_Font *font_sub   = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 14);
    TTF_Font *font_label = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 11);

    if (!font_title || !font_main || !font_sub || !font_label) {
        return 0;
    }

    // Fenêtre compacte sans fond noir résiduel, adaptée au contenu (hauteur 620)
    SDL_Window *window = SDL_CreateWindow("myDiscord - Welcome", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 450, 620, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_Cursor *cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL_Cursor *cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor *cursor_ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

    UIState ui_state = {.current_tab = TAB_LOGIN, .current_focus = FOCUS_NONE};
    SDL_StartTextInput();

    int running = 1;
    int connected = 0; // ➡️ AJOUT : passera à 1 si validation réussie
    SDL_Event event;

    while (running)
    {
        SDL_Rect card_rect = {0, 0, 450, 620};
        SDL_Rect btn_submit = { card_rect.x + (card_rect.w - 370) / 2, card_rect.y + card_rect.h - 65, 370, 45 };
        
        SDL_Rect input1 = { card_rect.x + 40, card_rect.y + 180, 370, 40 };
        SDL_Rect input2 = { card_rect.x + 40, card_rect.y + 270, 370, 40 };
        SDL_Rect input3 = { card_rect.x + 40, card_rect.y + 360, 370, 40 };
        SDL_Rect input4 = { card_rect.x + 40, card_rect.y + 450, 370, 40 };

        SDL_Rect tab_login_rect = {card_rect.x, card_rect.y, 225, 45};
        SDL_Rect tab_reg_rect = {card_rect.x + 225, card_rect.y, 225, 45};

        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        int is_hovering_button = (mouse_x >= btn_submit.x && mouse_x <= (btn_submit.x + btn_submit.w) && mouse_y >= btn_submit.y && mouse_y <= (btn_submit.y + btn_submit.h));

        if (is_hovering_button ||
            (mouse_x >= tab_login_rect.x && mouse_x <= (tab_login_rect.x + tab_login_rect.w) && mouse_y >= tab_login_rect.y && mouse_y <= (tab_login_rect.y + tab_login_rect.h)) ||
            (mouse_x >= tab_reg_rect.x && mouse_x <= (tab_reg_rect.x + tab_reg_rect.w) && mouse_y >= tab_reg_rect.y && mouse_y <= (tab_reg_rect.y + tab_reg_rect.h)))
        {
            SDL_SetCursor(cursor_hand);
        }
        else if ((mouse_x >= input1.x && mouse_x <= (input1.x + input1.w) && mouse_y >= input1.y && mouse_y <= (input1.y + input1.h)) ||
                 (mouse_x >= input2.x && mouse_x <= (input2.x + input2.w) && mouse_y >= input2.y && mouse_y <= (input2.y + input2.h)) ||
                 (ui_state.current_tab == TAB_REGISTER && mouse_x >= input3.x && mouse_x <= (input3.x + input3.w) && mouse_y >= input3.y && mouse_y <= (input3.y + input3.h)) ||
                 (ui_state.current_tab == TAB_REGISTER && mouse_x >= input4.x && mouse_x <= (input4.x + input4.w) && mouse_y >= input4.y && mouse_y <= (input4.y + input4.h)))
        {
            SDL_SetCursor(cursor_ibeam);
        }
        else { SDL_SetCursor(cursor_arrow); }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x; int my = event.button.y;

                if (mx >= tab_login_rect.x && mx <= (tab_login_rect.x + tab_login_rect.w) && my >= tab_login_rect.y && my <= (tab_login_rect.y + tab_login_rect.h)) {
                    ui_state.current_tab = TAB_LOGIN; ui_state.current_focus = FOCUS_NONE;
                }
                else if (mx >= tab_reg_rect.x && mx <= (tab_reg_rect.x + tab_reg_rect.w) && my >= tab_reg_rect.y && my <= (tab_reg_rect.y + tab_reg_rect.h)) {
                    ui_state.current_tab = TAB_REGISTER; ui_state.current_focus = FOCUS_NONE;
                }
                else if (mx >= input1.x && mx <= (input1.x + input1.w) && my >= input1.y && my <= (input1.y + input1.h)) {
                    ui_state.current_focus = FOCUS_EMAIL;
                }
                else if (mx >= input2.x && mx <= (input2.x + input2.w) && my >= input2.y && my <= (input2.y + input2.h)) {
                    ui_state.current_focus = (ui_state.current_tab == TAB_REGISTER) ? FOCUS_USERNAME : FOCUS_PASSWORD;
                }
                else if (ui_state.current_tab == TAB_REGISTER && mx >= input3.x && mx <= (input3.x + input3.w) && my >= input3.y && my <= (input3.y + input3.h)) {
                    ui_state.current_focus = FOCUS_PASSWORD;
                }
                else if (ui_state.current_tab == TAB_REGISTER && mx >= input4.x && mx <= (input4.x + input4.w) && my >= input4.y && my <= (input4.y + input4.h)) {
                    ui_state.current_focus = FOCUS_CONFIRM;
                }
                else if (is_hovering_button) {
                    if (ui_state.current_tab == TAB_LOGIN) printf("[UI] Connexion : %s\n", ui_state.text_email);
                    else printf("[UI] Inscription Pseudo : %s\n", ui_state.text_username);
                    fflush(stdout);

                    // ➡️ CORRECTION : Clic validé, on déclenche le passage au chat
                    connected = 1;
                    running = 0; 
                }
                else { ui_state.current_focus = FOCUS_NONE; }
            }

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    char *buf = NULL;
                    if (ui_state.current_focus == FOCUS_EMAIL) buf = ui_state.text_email;
                    else if (ui_state.current_focus == FOCUS_USERNAME) buf = ui_state.text_username;
                    else if (ui_state.current_focus == FOCUS_PASSWORD) buf = ui_state.text_password;
                    else if (ui_state.current_focus == FOCUS_CONFIRM) buf = ui_state.text_confirm;
                    if (buf) { size_t len = strlen(buf); if (len > 0) buf[len - 1] = '\0'; }
                }
                if (event.key.keysym.sym == SDLK_TAB)
                {
                    if (ui_state.current_tab == TAB_LOGIN) {
                        ui_state.current_focus = (ui_state.current_focus == FOCUS_EMAIL) ? FOCUS_PASSWORD : FOCUS_EMAIL;
                    } else {
                        if (ui_state.current_focus == FOCUS_EMAIL) ui_state.current_focus = FOCUS_USERNAME;
                        else if (ui_state.current_focus == FOCUS_USERNAME) ui_state.current_focus = FOCUS_PASSWORD;
                        else if (ui_state.current_focus == FOCUS_PASSWORD) ui_state.current_focus = FOCUS_CONFIRM;
                        else ui_state.current_focus = FOCUS_EMAIL;
                    }
                }
            }

            if (event.type == SDL_TEXTINPUT)
            {
                char *buf = NULL;
                if (ui_state.current_focus == FOCUS_EMAIL) buf = ui_state.text_email;
                else if (ui_state.current_focus == FOCUS_USERNAME) buf = ui_state.text_username;
                else if (ui_state.current_focus == FOCUS_PASSWORD) buf = ui_state.text_password;
                else if (ui_state.current_focus == FOCUS_CONFIRM) buf = ui_state.text_confirm;
                if (buf && (strlen(buf) + strlen(event.text.text) < 127)) strcat(buf, event.text.text);
            }
        }

        SDL_SetRenderDrawColor(renderer, COLOR_BG_OBSIDIAN);
        SDL_RenderClear(renderer);
        draw_login_interface(renderer, card_rect, &ui_state, font_title, font_main, font_sub, font_label, is_hovering_button);
        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();
    SDL_FreeCursor(cursor_arrow); SDL_FreeCursor(cursor_hand); SDL_FreeCursor(cursor_ibeam);
    TTF_CloseFont(font_title); TTF_CloseFont(font_main); TTF_CloseFont(font_sub); TTF_CloseFont(font_label);
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    
    // ➡️ CORRECTION CRUCIALE : On retire TTF_Quit() et SDL_Quit() d'ici pour laisser le Chat s'ouvrir !

    return connected; // Renvoie 1 si connecté, 0 si fermé
}
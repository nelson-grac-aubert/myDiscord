#include "ui_welcome.h"
#include "ui_login.h"
#include "ui_chat.h"
#include "variables.h"  
#include "channel.h"
#include "message.h"
#include "user.h"
#include "reaction.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    STATE_AUTH,
    STATE_CHAT,
    STATE_EXIT
} AppState;

int welcome_ui_init_and_run(void)
{
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 26);
    TTF_Font *font_main  = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    TTF_Font *font_sub   = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 14);
    TTF_Font *font_label = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 11);

    if (!font_title || !font_main || !font_sub || !font_label) {
        return 0;
    }

    // FIX : La fenêtre fait désormais la taille exacte du formulaire (370x560)
    SDL_Window *window = SDL_CreateWindow("myDiscord", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 370, 560, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window || !renderer) {
        return 0;
    }

    // Initialize local models/caches
    channel_model_init();
    message_model_init();
    user_model_init();
    reaction_model_init();

    AppState current_state = STATE_AUTH;
    UIState ui_state;
    memset(&ui_state, 0, sizeof(UIState));
    ui_state.current_tab = TAB_LOGIN;
    ui_state.current_focus = FOCUS_NONE;

    // FIX : La carte commence à (0,0) et remplit 100% de la fenêtre. Plus aucun espace noir !
    SDL_Rect card_rect = {0, 0, 370, 560};
    int running = 1;
    SDL_Event event;

    SDL_StartTextInput();

    while (running) 
    {
        if (current_state == STATE_EXIT) {
            running = 0;
            break;
        }

        if (current_state == STATE_AUTH) 
        {
            // Rendu de l'arrière-plan calqué sur le gris de la carte
            SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF); 
            SDL_RenderClear(renderer);
            draw_login_interface(renderer, card_rect, &ui_state, font_title, font_main, font_sub, font_label, 0);
            SDL_RenderPresent(renderer);

            while (current_state == STATE_AUTH && SDL_WaitEvent(&event)) 
            {
                int mx = 0, my = 0;
                SDL_GetMouseState(&mx, &my);

                // Bouton de validation
                SDL_Rect btn_rect = {card_rect.x + 40, card_rect.y + (ui_state.current_tab == TAB_LOGIN ? 300 : 495), 290, 44};
                int is_hovering_button = (mx >= btn_rect.x && mx <= btn_rect.x + btn_rect.w && my >= btn_rect.y && my <= btn_rect.y + btn_rect.h);

                if (event.type == SDL_QUIT) {
                    current_state = STATE_EXIT;
                    break;
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) 
                {
                    int cx = event.button.x;
                    int cy = event.button.y;

                    // Onglets Login / Register
                    SDL_Rect tab_l = {card_rect.x, card_rect.y, card_rect.w / 2, 45};
                    SDL_Rect tab_r = {card_rect.x + card_rect.w / 2, card_rect.y, card_rect.w / 2, 45};

                    if (cx >= tab_l.x && cx <= tab_l.x + tab_l.w && cy >= tab_l.y && cy <= tab_l.y + tab_l.h) {
                        ui_state.current_tab = TAB_LOGIN;
                        ui_state.current_focus = FOCUS_NONE;
                    }
                    else if (cx >= tab_r.x && cx <= tab_r.x + tab_r.w && cy >= tab_r.y && cy <= tab_r.y + tab_r.h) {
                        ui_state.current_tab = TAB_REGISTER;
                        ui_state.current_focus = FOCUS_NONE;
                    }

                    // Champs de saisie textuelle
                    SDL_Rect input1 = {card_rect.x + 40, card_rect.y + 135, 290, 40};
                    SDL_Rect input2 = {card_rect.x + 40, card_rect.y + 232, 290, 40};
                    SDL_Rect input3 = {card_rect.x + 40, card_rect.y + 330, 290, 40}; 
                    SDL_Rect input4 = {card_rect.x + 40, card_rect.y + 428, 290, 40}; 

                    if (ui_state.current_tab == TAB_LOGIN) {
                        if (cx >= input1.x && cx <= input1.x + input1.w && cy >= input1.y && cy <= input1.y + input1.h)
                            ui_state.current_focus = FOCUS_EMAIL;
                        else if (cx >= input2.x && cx <= input2.x + input2.w && cy >= input2.y && cy <= input2.y + input2.h)
                            ui_state.current_focus = FOCUS_PASSWORD;
                        else
                            ui_state.current_focus = FOCUS_NONE;
                    } else {
                        if (cx >= input1.x && cx <= input1.x + input1.w && cy >= input1.y && cy <= input1.y + input1.h)
                            ui_state.current_focus = FOCUS_EMAIL;
                        else if (cx >= input2.x && cx <= input2.x + input2.w && cy >= input2.y && cy <= input2.y + input2.h)
                            ui_state.current_focus = FOCUS_USERNAME;
                        else if (cx >= input3.x && cx <= input3.x + input3.w && cy >= input3.y && cy <= input3.y + input3.h)
                            ui_state.current_focus = FOCUS_PASSWORD;
                        else if (cx >= input4.x && cx <= input4.x + input4.w && cy >= input4.y && cy <= input4.y + input4.h)
                            ui_state.current_focus = FOCUS_CONFIRM;
                        else
                            ui_state.current_focus = FOCUS_NONE;
                    }

                    if (is_hovering_button) {
                        current_state = STATE_CHAT;
                    }
                }
                else if (event.type == SDL_KEYDOWN) 
                {
                    char *buf = NULL;
                    if (ui_state.current_focus == FOCUS_EMAIL) buf = ui_state.text_email;
                    else if (ui_state.current_focus == FOCUS_USERNAME) buf = ui_state.text_username;
                    else if (ui_state.current_focus == FOCUS_PASSWORD) buf = ui_state.text_password;
                    else if (ui_state.current_focus == FOCUS_CONFIRM) buf = ui_state.text_confirm;

                    if (event.key.keysym.sym == SDLK_BACKSPACE && buf) {
                        size_t len = strlen(buf);
                        if (len > 0) buf[len - 1] = '\0';
                    }
                    else if (event.key.keysym.sym == SDLK_TAB) {
                        if (ui_state.current_tab == TAB_LOGIN) {
                            ui_state.current_focus = (ui_state.current_focus == FOCUS_EMAIL) ? FOCUS_PASSWORD : FOCUS_EMAIL;
                        } else {
                            if (ui_state.current_focus == FOCUS_EMAIL) ui_state.current_focus = FOCUS_USERNAME;
                            else if (ui_state.current_focus == FOCUS_USERNAME) ui_state.current_focus = FOCUS_PASSWORD;
                            else if (ui_state.current_focus == FOCUS_PASSWORD) ui_state.current_focus = FOCUS_CONFIRM;
                            else ui_state.current_focus = FOCUS_EMAIL;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        current_state = STATE_CHAT;
                    }
                }
                else if (event.type == SDL_TEXTINPUT) 
                {
                    char *buf = NULL;
                    if (ui_state.current_focus == FOCUS_EMAIL) buf = ui_state.text_email;
                    else if (ui_state.current_focus == FOCUS_USERNAME) buf = ui_state.text_username;
                    else if (ui_state.current_focus == FOCUS_PASSWORD) buf = ui_state.text_password;
                    else if (ui_state.current_focus == FOCUS_CONFIRM) buf = ui_state.text_confirm;

                    if (buf && (strlen(buf) + strlen(event.text.text) < 127)) {
                        strcat(buf, event.text.text);
                    }
                }

                // Refresh graphique
                SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF);
                SDL_RenderClear(renderer);
                draw_login_interface(renderer, card_rect, &ui_state, font_title, font_main, font_sub, font_label, is_hovering_button);
                SDL_RenderPresent(renderer);
            }

            if (current_state == STATE_CHAT) {
                SDL_SetWindowSize(window, 1200, 750);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
        }
        else if (current_state == STATE_CHAT)
        {
            int chat_result = run_chat_loop(window, renderer, font_title, font_main, font_sub);

            if (chat_result == 2) {
                current_state = STATE_AUTH;
                // Retour propre au format du formulaire
                SDL_SetWindowSize(window, 370, 560);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            } else {
                current_state = STATE_EXIT;
            }
        }
    }

    SDL_StopTextInput();

    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    TTF_CloseFont(font_label);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 1;
}
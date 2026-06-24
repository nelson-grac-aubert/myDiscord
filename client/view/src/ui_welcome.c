#include "ui_welcome.h"
#include "ui_login.h"
#include "ui_chat.h"
#include "channel.h"
#include "message.h"
#include "user.h"
#include "reaction.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_OBSIDIAN 0x1F, 0x20, 0x23, 0xFF // Fond d'écran Discord (derrière la carte)

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

    SDL_Window *window = SDL_CreateWindow("myDiscord", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 450, 620, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    channel_model_init();
    message_model_init();
    user_model_init();
    reaction_model_init();

    AppState current_state = STATE_AUTH;
    SDL_Event event;

    UIState ui_state = {.current_tab = TAB_LOGIN, .current_focus = FOCUS_NONE};
    SDL_StartTextInput();

    while (current_state != STATE_EXIT)
    {
        if (current_state == STATE_AUTH)
        {
            int auth_running = 1;
            while (auth_running)
            {
                SDL_Rect card_rect = {0, 0, 450, 620};
                SDL_Rect btn_submit = { card_rect.x + (card_rect.w - 370) / 2, card_rect.y + card_rect.h - 70, 370, 45 };
                
                // Rectangles de détection des clics synchronisés avec ui_login.c
                SDL_Rect input1 = { card_rect.x + 40, card_rect.y + 195, 370, 40 };
                SDL_Rect input2 = { card_rect.x + 40, card_rect.y + 285, 370, 40 };
                SDL_Rect input3 = { card_rect.x + 40, card_rect.y + 375, 370, 40 };
                SDL_Rect input4 = { card_rect.x + 40, card_rect.y + 465, 370, 40 };

                int mx, my;
                SDL_GetMouseState(&mx, &my);
                int is_hovering_button = (mx >= btn_submit.x && mx <= btn_submit.x + btn_submit.w && my >= btn_submit.y && my <= btn_submit.y + btn_submit.h);

                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT) {
                        auth_running = 0;
                        current_state = STATE_EXIT;
                    }
                    else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                    {
                        int cx = event.button.x;
                        int cy = event.button.y;

                        if (is_hovering_button) {
                            auth_running = 0;
                            current_state = STATE_CHAT;
                        }
                        // Onglet "Se connecter"
                        else if (cx >= 0 && cx <= 225 && cy >= 0 && cy <= 50) {
                            ui_state.current_tab = TAB_LOGIN;
                            ui_state.current_focus = FOCUS_NONE;
                        }
                        // Onglet "S'inscrire"
                        else if (cx > 225 && cx <= 450 && cy >= 0 && cy <= 50) {
                            ui_state.current_tab = TAB_REGISTER;
                            ui_state.current_focus = FOCUS_NONE;
                        }
                        // Sélection des focus selon l'onglet actif
                        else if (cx >= input1.x && cx <= input1.x + input1.w && cy >= input1.y && cy <= input1.y + input1.h) {
                            ui_state.current_focus = FOCUS_EMAIL;
                        }
                        else if (cx >= input2.x && cx <= input2.x + input2.w && cy >= input2.y && cy <= input2.y + input2.h) {
                            ui_state.current_focus = (ui_state.current_tab == TAB_LOGIN) ? FOCUS_PASSWORD : FOCUS_USERNAME;
                        }
                        else if (ui_state.current_tab == TAB_REGISTER && cx >= input3.x && cx <= input3.x + input3.w && cy >= input3.y && cy <= input3.y + input3.h) {
                            ui_state.current_focus = FOCUS_PASSWORD;
                        }
                        else if (ui_state.current_tab == TAB_REGISTER && cx >= input4.x && cx <= input4.x + input4.w && cy >= input4.y && cy <= input4.y + input4.h) {
                            ui_state.current_focus = FOCUS_CONFIRM;
                        }
                        else {
                            ui_state.current_focus = FOCUS_NONE;
                        }
                    }
                    else if (event.type == SDL_KEYDOWN)
                    {
                        if (event.key.keysym.sym == SDLK_BACKSPACE) {
                            char *buf = NULL;
                            if (ui_state.current_focus == FOCUS_EMAIL) buf = ui_state.text_email;
                            else if (ui_state.current_focus == FOCUS_USERNAME) buf = ui_state.text_username;
                            else if (ui_state.current_focus == FOCUS_PASSWORD) buf = ui_state.text_password;
                            else if (ui_state.current_focus == FOCUS_CONFIRM) buf = ui_state.text_confirm;
                            
                            if (buf && strlen(buf) > 0) buf[strlen(buf) - 1] = '\0';
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
                }

                SDL_SetRenderDrawColor(renderer, COLOR_BG_OBSIDIAN);
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
                SDL_SetWindowSize(window, 450, 620);
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

    return 0;
}
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
#include <stdlib.h>

typedef enum
{
    STATE_AUTH,
    STATE_CHAT,
    STATE_EXIT
} AppState;

// Calcule précisément l'index du curseur dans la chaîne de texte en fonction des pixels cliqués
static int get_cursor_index_from_click(TTF_Font *font, const char *text, int click_x, int field_start_x, int is_password)
{
    int local_x = click_x - (field_start_x + 12); // Déduction du padding gauche de 12px
    if (local_x <= 0 || text == NULL || text[0] == '\0') 
        return 0;

    int len = (int)strlen(text);
    int last_w = 0;

    for (int i = 1; i <= len; i++)
    {
        int current_w = 0;
        char sub_str[128] = "";
        
        if (is_password) {
            for (int j = 0; j < i && j < 20; j++) strcat(sub_str, "*");
        } else {
            strncpy(sub_str, text, i);
            sub_str[i] = '\0';
        }

        current_w = get_text_width(font, sub_str);

        if (local_x < current_w)
        {
            if (local_x - last_w < current_w - local_x)
                return i - 1;
            return i;
        }
        last_w = current_w;
    }
    return len;
}

int welcome_ui_init_and_run(void)
{
    font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 26);
    font_main = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    font_sub = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 14);
    font_label = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 11);
    font_emoji = TTF_OpenFont("fonts/seguiemj.ttf", 18);

    if (!font_title || !font_main || !font_sub || !font_label || !font_emoji)
    {
        printf("[TTF ERROR] Échec du chargement d'une des polices : %s\n", TTF_GetError());
        return 0;
    }

    SDL_Window *window = SDL_CreateWindow("myDiscord", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 370, 560, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window || !renderer)
    {
        return 0;
    }

    channel_model_init();
    message_model_init();
    user_model_init();
    reaction_model_init();

    AppState current_state = STATE_AUTH;
    UIState ui_state;
    memset(&ui_state, 0, sizeof(UIState));
    ui_state.current_tab = TAB_LOGIN;
    ui_state.current_focus = FOCUS_NONE;

    SDL_Rect card_rect = {0, 0, 370, 560};
    int running = 1;
    SDL_Event event;

    SDL_StartTextInput();

    while (running)
    {
        if (current_state == STATE_EXIT)
        {
            running = 0;
            break;
        }

        if (current_state == STATE_AUTH)
        {
            SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF);
            SDL_RenderClear(renderer);
            draw_login_interface(renderer, card_rect, &ui_state, font_title, font_main, font_sub, font_label, 0);
            SDL_RenderPresent(renderer);

            while (current_state == STATE_AUTH && SDL_WaitEvent(&event))
            {
                int mx = 0, my = 0;
                SDL_GetMouseState(&mx, &my);

                SDL_Rect btn_rect = {card_rect.x + 40, card_rect.y + (ui_state.current_tab == TAB_LOGIN ? 300 : 495), 290, 44};
                int is_hovering_button = (mx >= btn_rect.x && mx <= btn_rect.x + btn_rect.w && my >= btn_rect.y && my <= btn_rect.y + btn_rect.h);

                if (event.type == SDL_QUIT)
                {
                    current_state = STATE_EXIT;
                    break;
                }
                else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
                {
                    int cx = event.button.x;
                    int cy = event.button.y;

                    SDL_Rect tab_l = {card_rect.x, card_rect.y, card_rect.w / 2, 45};
                    SDL_Rect tab_r = {card_rect.x + card_rect.w / 2, card_rect.y, card_rect.w / 2, 45};

                    if (cx >= tab_l.x && cx <= tab_l.x + tab_l.w && cy >= tab_l.y && cy <= tab_l.y + tab_l.h)
                    {
                        ui_state.current_tab = TAB_LOGIN;
                        ui_state.current_focus = FOCUS_NONE;
                        ui_state.cursor_email = 0;
                        ui_state.cursor_password = 0;
                        ui_state.text_email[0] = '\0';
                        ui_state.text_password[0] = '\0';
                    }
                    else if (cx >= tab_r.x && cx <= tab_r.x + tab_r.w && cy >= tab_r.y && cy <= tab_r.y + tab_r.h)
                    {
                        ui_state.current_tab = TAB_REGISTER;
                        ui_state.current_focus = FOCUS_NONE;
                        ui_state.cursor_email = 0;
                        ui_state.cursor_username = 0;
                        ui_state.cursor_password = 0;
                        ui_state.cursor_confirm = 0;
                        ui_state.text_email[0] = '\0';
                        ui_state.text_username[0] = '\0';
                        ui_state.text_password[0] = '\0';
                        ui_state.text_confirm[0] = '\0';
                    }

                    SDL_Rect input1 = {card_rect.x + 40, card_rect.y + 135, 290, 40};
                    SDL_Rect input2 = {card_rect.x + 40, card_rect.y + 232, 290, 40};
                    SDL_Rect input3 = {card_rect.x + 40, card_rect.y + 330, 290, 40};
                    SDL_Rect input4 = {card_rect.x + 40, card_rect.y + 428, 290, 40};

                    if (ui_state.current_tab == TAB_LOGIN)
                    {
                        if (cx >= input1.x && cx <= input1.x + input1.w && cy >= input1.y && cy <= input1.y + input1.h) {
                            ui_state.current_focus = FOCUS_EMAIL;
                            ui_state.cursor_email = get_cursor_index_from_click(font_main, ui_state.text_email, cx, input1.x, 0);
                        }
                        else if (cx >= input2.x && cx <= input2.x + input2.w && cy >= input2.y && cy <= input2.y + input2.h) {
                            ui_state.current_focus = FOCUS_PASSWORD;
                            ui_state.cursor_password = get_cursor_index_from_click(font_main, ui_state.text_password, cx, input2.x, 1);
                        }
                        else {
                            ui_state.current_focus = FOCUS_NONE;
                        }
                    }
                    else
                    {
                        if (cx >= input1.x && cx <= input1.x + input1.w && cy >= input1.y && cy <= input1.y + input1.h) {
                            ui_state.current_focus = FOCUS_EMAIL;
                            ui_state.cursor_email = get_cursor_index_from_click(font_main, ui_state.text_email, cx, input1.x, 0);
                        }
                        else if (cx >= input2.x && cx <= input2.x + input2.w && cy >= input2.y && cy <= input2.y + input2.h) {
                            ui_state.current_focus = FOCUS_USERNAME;
                            ui_state.cursor_username = get_cursor_index_from_click(font_main, ui_state.text_username, cx, input2.x, 0);
                        }
                        else if (cx >= input3.x && cx <= input3.x + input3.w && cy >= input3.y && cy <= input3.y + input3.h) {
                            ui_state.current_focus = FOCUS_PASSWORD;
                            ui_state.cursor_password = get_cursor_index_from_click(font_main, ui_state.text_password, cx, input3.x, 1);
                        }
                        else if (cx >= input4.x && cx <= input4.x + input4.w && cy >= input4.y && cy <= input4.y + input4.h) {
                            ui_state.current_focus = FOCUS_CONFIRM;
                            ui_state.cursor_confirm = get_cursor_index_from_click(font_main, ui_state.text_confirm, cx, input4.x, 1);
                        }
                        else {
                            ui_state.current_focus = FOCUS_NONE;
                        }
                    }

                    if (is_hovering_button)
                    {
                        if (ui_state.current_tab == TAB_LOGIN)
                        {
                            current_state = STATE_CHAT;
                        }
                        else if (ui_state.current_tab == TAB_REGISTER)
                        {
                            printf("[AUTH] Inscription validée. Redirection vers l'onglet de connexion.\n");
                            ui_state.current_tab = TAB_LOGIN;
                            ui_state.current_focus = FOCUS_NONE;
                        }
                    }
                }
                else if (event.type == SDL_KEYDOWN)
                {
                    char *buf = NULL;
                    int *cursor = NULL;
                    
                    if (ui_state.current_focus == FOCUS_EMAIL) { buf = ui_state.text_email; cursor = &ui_state.cursor_email; }
                    else if (ui_state.current_focus == FOCUS_USERNAME) { buf = ui_state.text_username; cursor = &ui_state.cursor_username; }
                    else if (ui_state.current_focus == FOCUS_PASSWORD) { buf = ui_state.text_password; cursor = &ui_state.cursor_password; }
                    else if (ui_state.current_focus == FOCUS_CONFIRM) { buf = ui_state.text_confirm; cursor = &ui_state.cursor_confirm; }

                    if (event.key.keysym.sym == SDLK_LEFT && cursor && *cursor > 0)
                    {
                        (*cursor)--;
                    }
                    else if (event.key.keysym.sym == SDLK_RIGHT && cursor && buf && *cursor < (int)strlen(buf))
                    {
                        (*cursor)++;
                    }
                    else if (event.key.keysym.sym == SDLK_BACKSPACE && buf && cursor && *cursor > 0)
                    {
                        size_t len = strlen(buf);
                        memmove(&buf[*cursor - 1], &buf[*cursor], len - *cursor + 1);
                        (*cursor)--;
                    }
                    else if (event.key.keysym.sym == SDLK_TAB)
                    {
                        if (ui_state.current_tab == TAB_LOGIN) {
                            ui_state.current_focus = (ui_state.current_focus == FOCUS_EMAIL) ? FOCUS_PASSWORD : FOCUS_EMAIL;
                        } else {
                            if (ui_state.current_focus == FOCUS_EMAIL) ui_state.current_focus = FOCUS_USERNAME;
                            else if (ui_state.current_focus == FOCUS_USERNAME) ui_state.current_focus = FOCUS_PASSWORD;
                            else if (ui_state.current_focus == FOCUS_PASSWORD) ui_state.current_focus = FOCUS_CONFIRM;
                            else ui_state.current_focus = FOCUS_EMAIL;
                        }
                        if (ui_state.current_focus == FOCUS_EMAIL) ui_state.cursor_email = (int)strlen(ui_state.text_email);
                        if (ui_state.current_focus == FOCUS_USERNAME) ui_state.cursor_username = (int)strlen(ui_state.text_username);
                        if (ui_state.current_focus == FOCUS_PASSWORD) ui_state.cursor_password = (int)strlen(ui_state.text_password);
                        if (ui_state.current_focus == FOCUS_CONFIRM) ui_state.cursor_confirm = (int)strlen(ui_state.text_confirm);
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
                    {
                        if (ui_state.current_tab == TAB_LOGIN) {
                            current_state = STATE_CHAT;
                        } else if (ui_state.current_tab == TAB_REGISTER) {
                            printf("[AUTH] Inscription validée via Entrée. Redirection vers l'onglet de connexion.\n");
                            ui_state.current_tab = TAB_LOGIN;
                            ui_state.current_focus = FOCUS_NONE;
                        }
                    }
                }
                else if (event.type == SDL_TEXTINPUT)
                {
                    char *buf = NULL;
                    int *cursor = NULL;
                    if (ui_state.current_focus == FOCUS_EMAIL) { buf = ui_state.text_email; cursor = &ui_state.cursor_email; }
                    else if (ui_state.current_focus == FOCUS_USERNAME) { buf = ui_state.text_username; cursor = &ui_state.cursor_username; }
                    else if (ui_state.current_focus == FOCUS_PASSWORD) { buf = ui_state.text_password; cursor = &ui_state.cursor_password; }
                    else if (ui_state.current_focus == FOCUS_CONFIRM) { buf = ui_state.text_confirm; cursor = &ui_state.cursor_confirm; }

                    if (buf && cursor)
                    {
                        size_t text_len = strlen(event.text.text);
                        size_t current_len = strlen(buf);
                        if (current_len + text_len < 127)
                        {
                            memmove(&buf[*cursor + text_len], &buf[*cursor], current_len - *cursor + 1);
                            memcpy(&buf[*cursor], event.text.text, text_len);
                            *cursor += text_len;
                        }
                    }
                }

                SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF);
                SDL_RenderClear(renderer);
                draw_login_interface(renderer, card_rect, &ui_state, font_title, font_main, font_sub, font_label, is_hovering_button);
                SDL_RenderPresent(renderer);
            }

            if (current_state == STATE_CHAT)
            {
                SDL_SetWindowSize(window, 1200, 750);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
        }
        else if (current_state == STATE_CHAT)
        {
            int chat_result = run_chat_loop(window, renderer, font_title, font_main, font_sub);

            if (chat_result == 2)
            {
                current_state = STATE_AUTH;
                SDL_SetWindowSize(window, 370, 560);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
            else
            {
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
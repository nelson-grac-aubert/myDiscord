#include "ui_welcome.h"
#include "ui_login.h"
#include "ui_chat.h"
#include "variables.h"
#include "channel.h"
#include "message.h"
#include "user.h"
#include "reaction.h"
#include "auth_controller.h"
#include <stdio.h>
#include <string.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 8080

typedef enum {
    STATE_AUTH,
    STATE_WAITING,
    STATE_CHAT,
    STATE_EXIT
} AppState;

static void render_auth(SDL_Renderer *renderer, SDL_Rect card_rect,
                        UIState *ui_state, int is_hovering_button,
                        const char *error_msg)
{
    SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF);
    SDL_RenderClear(renderer);
    draw_login_interface(renderer, card_rect, ui_state,
                         font_title, font_main, font_sub, font_label,
                         is_hovering_button);

    if (error_msg) {
        SDL_Color red = {0xF2, 0x3F, 0x43, 0xFF};
        draw_text(renderer, font_sub, error_msg,
                  card_rect.x + 40, card_rect.y + 510, red);
    }

    SDL_RenderPresent(renderer);
}

static SDL_Rect get_button_rect(SDL_Rect card_rect, UIState *ui_state)
{
    int y = (ui_state->current_tab == TAB_LOGIN) ? 300 : 495;
    return (SDL_Rect){card_rect.x + 40, card_rect.y + y, 290, 44};
}

static int is_hovering_btn(SDL_Rect btn, int mx, int my)
{
    return mx >= btn.x && mx <= btn.x + btn.w &&
           my >= btn.y && my <= btn.y + btn.h;
}

static void handle_field_click(UIState *ui_state, SDL_Rect card_rect, int cx, int cy)
{
    SDL_Rect input1 = {card_rect.x + 40, card_rect.y + 135, 290, 40};
    SDL_Rect input2 = {card_rect.x + 40, card_rect.y + 232, 290, 40};
    SDL_Rect input3 = {card_rect.x + 40, card_rect.y + 330, 290, 40};
    SDL_Rect input4 = {card_rect.x + 40, card_rect.y + 428, 290, 40};

    ui_state->current_focus = FOCUS_NONE;

    if (ui_state->current_tab == TAB_LOGIN) {
        if (cx >= input1.x && cx <= input1.x + input1.w &&
            cy >= input1.y && cy <= input1.y + input1.h)
            ui_state->current_focus = FOCUS_EMAIL;
        else if (cx >= input2.x && cx <= input2.x + input2.w &&
                 cy >= input2.y && cy <= input2.y + input2.h)
            ui_state->current_focus = FOCUS_PASSWORD;
    } else {
        if (cx >= input1.x && cx <= input1.x + input1.w &&
            cy >= input1.y && cy <= input1.y + input1.h)
            ui_state->current_focus = FOCUS_EMAIL;
        else if (cx >= input2.x && cx <= input2.x + input2.w &&
                 cy >= input2.y && cy <= input2.y + input2.h)
            ui_state->current_focus = FOCUS_USERNAME;
        else if (cx >= input3.x && cx <= input3.x + input3.w &&
                 cy >= input3.y && cy <= input3.y + input3.h)
            ui_state->current_focus = FOCUS_PASSWORD;
        else if (cx >= input4.x && cx <= input4.x + input4.w &&
                 cy >= input4.y && cy <= input4.y + input4.h)
            ui_state->current_focus = FOCUS_CONFIRM;
    }
}

static void handle_keydown(UIState *ui_state, SDL_Keycode sym)
{
    char *buf = NULL;
    if (ui_state->current_focus == FOCUS_EMAIL)    buf = ui_state->text_email;
    else if (ui_state->current_focus == FOCUS_USERNAME)  buf = ui_state->text_username;
    else if (ui_state->current_focus == FOCUS_PASSWORD)  buf = ui_state->text_password;
    else if (ui_state->current_focus == FOCUS_CONFIRM)   buf = ui_state->text_confirm;

    if (sym == SDLK_BACKSPACE && buf) {
        size_t len = strlen(buf);
        if (len > 0) buf[len - 1] = '\0';
    } else if (sym == SDLK_TAB) {
        if (ui_state->current_tab == TAB_LOGIN) {
            ui_state->current_focus = (ui_state->current_focus == FOCUS_EMAIL)
                                      ? FOCUS_PASSWORD : FOCUS_EMAIL;
        } else {
            if (ui_state->current_focus == FOCUS_EMAIL)         ui_state->current_focus = FOCUS_USERNAME;
            else if (ui_state->current_focus == FOCUS_USERNAME) ui_state->current_focus = FOCUS_PASSWORD;
            else if (ui_state->current_focus == FOCUS_PASSWORD) ui_state->current_focus = FOCUS_CONFIRM;
            else                                                 ui_state->current_focus = FOCUS_EMAIL;
        }
    }
}

static void handle_textinput(UIState *ui_state, const char *text)
{
    char *buf = NULL;
    if (ui_state->current_focus == FOCUS_EMAIL)         buf = ui_state->text_email;
    else if (ui_state->current_focus == FOCUS_USERNAME) buf = ui_state->text_username;
    else if (ui_state->current_focus == FOCUS_PASSWORD) buf = ui_state->text_password;
    else if (ui_state->current_focus == FOCUS_CONFIRM)  buf = ui_state->text_confirm;

    if (buf && strlen(buf) + strlen(text) < 127)
        strcat(buf, text);
}

static void submit_auth(UIState *ui_state, AppState *state)
{
    if (ui_state->current_tab == TAB_LOGIN) {
        auth_controller_login(ui_state);
    } else {
        if (strcmp(ui_state->text_password, ui_state->text_confirm) != 0) {
            printf("[auth] passwords do not match\n");
            return;
        }
        auth_controller_register(ui_state);
    }
    *state = STATE_WAITING;
}

int welcome_ui_init_and_run(void)
{
    font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf",    26);
    font_main  = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    font_sub   = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf",14);
    font_label = TTF_OpenFont("fonts/Urbanist-Bold.ttf",    11);
    font_emoji = TTF_OpenFont("fonts/seguiemj.ttf",         18);

    if (!font_title || !font_main || !font_sub || !font_label || !font_emoji) {
        printf("[TTF ERROR] %s\n", TTF_GetError());
        return 0;
    }

    if (auth_controller_connect(SERVER_IP, SERVER_PORT) != 0) {
        printf("[auth] could not connect to server\n");
        return 0;
    }

    SDL_Window   *window   = SDL_CreateWindow("myDiscord",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 370, 560, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                 SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window || !renderer)
        return 0;

    channel_model_init();
    message_model_init();
    user_model_init();
    reaction_model_init();

    AppState current_state = STATE_AUTH;
    UIState  ui_state;
    memset(&ui_state, 0, sizeof(UIState));
    ui_state.current_tab   = TAB_LOGIN;
    ui_state.current_focus = FOCUS_NONE;

    SDL_Rect card_rect = {0, 0, 370, 560};
    SDL_Event event;
    SDL_StartTextInput();

    while (current_state != STATE_EXIT) {

        if (current_state == STATE_WAITING) {
            AuthResult result = auth_controller_get_result();
            if (result == AUTH_RESULT_OK) {
                current_state = STATE_CHAT;
                SDL_SetWindowSize(window, 1200, 750);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            } else if (result == AUTH_RESULT_ERROR) {
                current_state = STATE_AUTH;
            }
        }

        if (current_state == STATE_AUTH) {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            SDL_Rect btn = get_button_rect(card_rect, &ui_state);
            int hovering = is_hovering_btn(btn, mx, my);
            const char *err = auth_controller_get_error();

            render_auth(renderer, card_rect, &ui_state, hovering, err);

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    current_state = STATE_EXIT;
                    break;
                }
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    event.button.button == SDL_BUTTON_LEFT) {
                    int cx = event.button.x, cy = event.button.y;

                    SDL_Rect tab_l = {card_rect.x,                   card_rect.y, card_rect.w / 2, 45};
                    SDL_Rect tab_r = {card_rect.x + card_rect.w / 2, card_rect.y, card_rect.w / 2, 45};

                    if (cx >= tab_l.x && cx <= tab_l.x + tab_l.w &&
                        cy >= tab_l.y && cy <= tab_l.y + tab_l.h) {
                        ui_state.current_tab   = TAB_LOGIN;
                        ui_state.current_focus = FOCUS_NONE;
                    } else if (cx >= tab_r.x && cx <= tab_r.x + tab_r.w &&
                               cy >= tab_r.y && cy <= tab_r.y + tab_r.h) {
                        ui_state.current_tab   = TAB_REGISTER;
                        ui_state.current_focus = FOCUS_NONE;
                    } else if (is_hovering_btn(btn, cx, cy)) {
                        submit_auth(&ui_state, &current_state);
                    } else {
                        handle_field_click(&ui_state, card_rect, cx, cy);
                    }
                }
                if (event.type == SDL_KEYDOWN)
                    handle_keydown(&ui_state, event.key.keysym.sym);
                if (event.type == SDL_TEXTINPUT)
                    handle_textinput(&ui_state, event.text.text);
            }
        }

        if (current_state == STATE_CHAT) {
            int chat_result = run_chat_loop(window, renderer,
                                            font_title, font_main, font_sub);
            if (chat_result == 2) {
                current_state = STATE_AUTH;
                SDL_SetWindowSize(window, 370, 560);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            } else {
                current_state = STATE_EXIT;
            }
        }
    }

    SDL_StopTextInput();
    auth_controller_disconnect();

    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    TTF_CloseFont(font_label);
    TTF_CloseFont(font_emoji);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 1;
}
#include "ui_login.h"
#include <string.h>

#define COLOR_CARD_GRAY 0x2B, 0x2D, 0x31, 0xFF // Original Discord card gray
#define COLOR_DISCORD_BLUE 0x58, 0x65, 0xF2, 0xFF
#define COLOR_DISCORD_BLUE_HOVER 0x47, 0x52, 0xC4, 0xFF
#define COLOR_TAB_ACTIVE_BG 0x2B, 0x2D, 0x31, 0xFF
#define COLOR_TAB_INACTIVE_BG 0x1E, 0x1F, 0x22, 0xFF
#define COLOR_INPUT_BG 0x1E, 0x1F, 0x22, 0xFF     // Dark input field background
#define COLOR_INPUT_BORDER 0x3F, 0x41, 0x47, 0xFF // Default border color

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
    if (!font || !text || text[0] == '\0')
        return;
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface)
        return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture)
    {
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

int get_text_width(TTF_Font *font, const char *text)
{
    if (!font || !text || text[0] == '\0')
        return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(font, text, &w, &h);
    return w;
}

// Factorized function to draw text input fields (Background, Border, Text/Placeholder, Blinking cursor)
static void draw_input_field(SDL_Renderer *renderer, TTF_Font *font_main, SDL_Rect rect, const char *text,
                             const char *placeholder, int is_focused, int show_cursor, int is_password,
                             SDL_Color white_color, SDL_Color placeholder_color)
{
    // 1. Background
    SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
    SDL_RenderFillRect(renderer, &rect);

    // 2. Focus border
    if (is_focused)
    {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
    }
    SDL_RenderDrawRect(renderer, &rect);

    // 3. Text content or placeholder
    if (strlen(text) == 0)
    {
        draw_text(renderer, font_main, placeholder, rect.x + 12, rect.y + 11, placeholder_color);
        if (is_focused && show_cursor)
            draw_text(renderer, font_main, "|", rect.x + 12, rect.y + 10, white_color);
    }
    else
    {
        if (is_password)
        {
            char stars[128] = "";
            size_t p_len = strlen(text);
            for (size_t i = 0; i < p_len && i < 20; i++)
                strcat(stars, "*");
            draw_text(renderer, font_main, stars, rect.x + 12, rect.y + 11, white_color);
            if (is_focused && show_cursor)
                draw_text(renderer, font_main, "|", rect.x + 12 + get_text_width(font_main, stars), rect.y + 10, white_color);
        }
        else
        {
            draw_text(renderer, font_main, text, rect.x + 12, rect.y + 11, white_color);
            if (is_focused && show_cursor)
                draw_text(renderer, font_main, "|", rect.x + 12 + get_text_width(font_main, text), rect.y + 10, white_color);
        }
    }
}

void draw_login_interface(SDL_Renderer *renderer, SDL_Rect card_rect, UIState *state,
                          TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_label,
                          int is_hovering_button)
{
    SDL_Color white_color = {255, 255, 255, 255};
    SDL_Color gray_color = {148, 155, 164, 255}; // Muted text color
    SDL_Color placeholder_color = {78, 80, 88, 255};

    Uint32 ticks = SDL_GetTicks();
    int show_cursor = (ticks / 500) % 2 == 0; // Cursor blinking frequency

    // --- 1. TOP TAB BAR ---
    SDL_Rect tab_l = {card_rect.x, card_rect.y, card_rect.w / 2, 45};
    SDL_Rect tab_r = {card_rect.x + card_rect.w / 2, card_rect.y, card_rect.w / 2, 45};

    if (state->current_tab == TAB_LOGIN)
    {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
    }
    SDL_RenderFillRect(renderer, &tab_l);

    if (state->current_tab == TAB_REGISTER)
    {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
    }
    SDL_RenderFillRect(renderer, &tab_r);

    int wl = get_text_width(font_sub, "Login");
    int wr = get_text_width(font_sub, "Register");
    draw_text(renderer, font_sub, "Login", tab_l.x + (tab_l.w - wl) / 2, tab_l.y + 13, (state->current_tab == TAB_LOGIN) ? white_color : gray_color);
    draw_text(renderer, font_sub, "Register", tab_r.x + (tab_r.w - wr) / 2, tab_r.y + 13, (state->current_tab == TAB_REGISTER) ? white_color : gray_color);

    // FIX : AJOUT DU TRAIT BLEU SOUS L'ONGLET ACTIF (De hauteur 3px tout en bas de l'onglet)
    SDL_Rect active_indicator = {
        (state->current_tab == TAB_LOGIN) ? tab_l.x : tab_r.x,
        card_rect.y + 42, // Positionné juste avant la fin des 45px de hauteur de l'onglet
        card_rect.w / 2,
        3 // Épaisseur du trait bleu
    };
    SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    SDL_RenderFillRect(renderer, &active_indicator);

    // --- 2. MAIN CARD CONTENT ---
    SDL_Rect content_rect = {card_rect.x, card_rect.y + 45, card_rect.w, card_rect.h - 45};
    SDL_SetRenderDrawColor(renderer, COLOR_CARD_GRAY);
    SDL_RenderFillRect(renderer, &content_rect);

    // Dynamic layout positions
    SDL_Rect input1 = {card_rect.x + 40, card_rect.y + 135, 290, 40};
    SDL_Rect input2 = {card_rect.x + 40, card_rect.y + 232, 290, 40};
    SDL_Rect input3 = {card_rect.x + 40, card_rect.y + 330, 290, 40};
    SDL_Rect input4 = {card_rect.x + 40, card_rect.y + 428, 290, 40};

    // --- 3. DYNAMIC BRANCH RENDERING ---
    if (state->current_tab == TAB_LOGIN)
    {
        draw_text(renderer, font_title, "Welcome back!", card_rect.x + 40, card_rect.y + 70, white_color);
        draw_text(renderer, font_main, "We're so excited to see you again!", card_rect.x + 40, card_rect.y + 102, gray_color);

        draw_text(renderer, font_label, "EMAIL ADDRESS", card_rect.x + 40, card_rect.y + 148, gray_color);
        draw_input_field(renderer, font_main, input1, state->text_email, "Enter your email",
                         (state->current_focus == FOCUS_EMAIL), show_cursor, 0, white_color, placeholder_color);

        draw_text(renderer, font_label, "PASSWORD", card_rect.x + 40, card_rect.y + 245, gray_color);
        draw_input_field(renderer, font_main, input2, state->text_password, "Enter your password",
                         (state->current_focus == FOCUS_PASSWORD), show_cursor, 1, white_color, placeholder_color);
    }
    else // TAB_REGISTER
    {
        draw_text(renderer, font_title, "Create an account", card_rect.x + 40, card_rect.y + 65, white_color);

        draw_text(renderer, font_label, "EMAIL ADDRESS", card_rect.x + 40, card_rect.y + 117, gray_color);
        draw_input_field(renderer, font_main, input1, state->text_email, "Enter your email",
                         (state->current_focus == FOCUS_EMAIL), show_cursor, 0, white_color, placeholder_color);

        draw_text(renderer, font_label, "USERNAME", card_rect.x + 40, card_rect.y + 214, gray_color);
        draw_input_field(renderer, font_main, input2, state->text_username, "Choose a username",
                         (state->current_focus == FOCUS_USERNAME), show_cursor, 0, white_color, placeholder_color);

        draw_text(renderer, font_label, "PASSWORD", card_rect.x + 40, card_rect.y + 312, gray_color);
        draw_input_field(renderer, font_main, input3, state->text_password, "Choose a password",
                         (state->current_focus == FOCUS_PASSWORD), show_cursor, 1, white_color, placeholder_color);

        draw_text(renderer, font_label, "CONFIRM PASSWORD", card_rect.x + 40, card_rect.y + 410, gray_color);
        draw_input_field(renderer, font_main, input4, state->text_confirm, "Confirm your password",
                         (state->current_focus == FOCUS_CONFIRM), show_cursor, 1, white_color, placeholder_color);
    }

    // --- 4. RENDER VALIDATION MAIN ACTION BUTTON ---
    SDL_Rect button_action_rect = {card_rect.x + 40, card_rect.y + (state->current_tab == TAB_LOGIN ? 300 : 495), 290, 44};

    if (is_hovering_button)
    {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE_HOVER);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    }
    SDL_RenderFillRect(renderer, &button_action_rect);

    const char *btn_text = (state->current_tab == TAB_LOGIN) ? "Log In" : "Continue";
    int btn_w = get_text_width(font_sub, btn_text);
    draw_text(renderer, font_sub, btn_text, button_action_rect.x + (button_action_rect.w - btn_w) / 2, button_action_rect.y + 13, white_color);
}
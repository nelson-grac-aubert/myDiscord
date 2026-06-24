#include "ui_login.h"
#include <string.h>

#define COLOR_CARD_GRAY          0x2B, 0x2D, 0x31, 0xFF // Original Discord card gray
#define COLOR_DISCORD_BLUE       0x58, 0x65, 0xF2, 0xFF
#define COLOR_DISCORD_BLUE_HOVER 0x47, 0x52, 0xC4, 0xFF 
#define COLOR_TAB_ACTIVE_BG      0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_TAB_INACTIVE_BG    0x1E, 0x1F, 0x22, 0xFF 
#define COLOR_INPUT_BG           0x1E, 0x1F, 0x22, 0xFF // Dark input field background
#define COLOR_INPUT_BORDER       0x3F, 0x41, 0x47, 0xFF // Default border color

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    if (!font || !text || text[0] == '\0') return;
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

int get_text_width(TTF_Font *font, const char *text) {
    if (!font || !text || text[0] == '\0') return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(font, text, &w, &h);
    return w;
}

void draw_login_interface(SDL_Renderer *renderer, SDL_Rect card_rect, UIState *state,
                          TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_label,
                          int is_hovering_button) 
{
    SDL_Color white_color       = {255, 255, 255, 255};
    SDL_Color gray_color        = {148, 155, 164, 255}; // Muted labels
    SDL_Color placeholder_color = {78, 80, 88, 255};

    Uint32 ticks = SDL_GetTicks();
    int show_cursor = (ticks / 500) % 2 == 0; // Blinking text input cursor blinking rate

    // --- 1. RENDER TOP TABS BAR ---
    SDL_Rect tab_l = {card_rect.x, card_rect.y, card_rect.w / 2, 45};
    SDL_Rect tab_r = {card_rect.x + card_rect.w / 2, card_rect.y, card_rect.w / 2, 45};

    if (state->current_tab == TAB_LOGIN) {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_l);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_r);
    } else {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_l);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_r);
    }

    // Text rendering centering alignment inside tab headers
    int wl = get_text_width(font_sub, "Login");
    int wr = get_text_width(font_sub, "Register");
    draw_text(renderer, font_sub, "Login", tab_l.x + (tab_l.w - wl) / 2, tab_l.y + 13, (state->current_tab == TAB_LOGIN) ? white_color : gray_color);
    draw_text(renderer, font_sub, "Register", tab_r.x + (tab_r.w - wr) / 2, tab_r.y + 13, (state->current_tab == TAB_REGISTER) ? white_color : gray_color);

    // --- 2. RENDER MAIN CONTEXT BODY CARD BACKGROUND ---
    SDL_Rect content_rect = {card_rect.x, card_rect.y + 45, card_rect.w, card_rect.h - 45};
    SDL_SetRenderDrawColor(renderer, COLOR_CARD_GRAY);
    SDL_RenderFillRect(renderer, &content_rect);

    // Shared geometry positions definitions for inputs fields row
    SDL_Rect input1 = {card_rect.x + 40, card_rect.y + 135, 290, 40};
    SDL_Rect input2 = {card_rect.x + 40, card_rect.y + 232, 290, 40};
    SDL_Rect input3 = {card_rect.x + 40, card_rect.y + 330, 290, 40};
    SDL_Rect input4 = {card_rect.x + 40, card_rect.y + 428, 290, 40};

    // --- 3. RENDERING LAYOUT BY SELECTED TAB ---
    if (state->current_tab == TAB_LOGIN) 
    {
        // Headers / Subtitles
        draw_text(renderer, font_title, "Welcome back!", card_rect.x + 40, card_rect.y + 70, white_color);
        draw_text(renderer, font_main, "We're so excited to see you again!", card_rect.x + 40, card_rect.y + 102, gray_color);

        // Field 1: Email Address field
        draw_text(renderer, font_label, "EMAIL ADDRESS", card_rect.x + 40, card_rect.y + 148, gray_color);
        SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
        SDL_RenderFillRect(renderer, &input1);
        if (state->current_focus == FOCUS_EMAIL) {
            SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
        } else {
            SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
        }
        SDL_RenderDrawRect(renderer, &input1);

        if (strlen(state->text_email) == 0) {
            draw_text(renderer, font_main, "Enter your email", input1.x + 12, input1.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12, input1.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_email, input1.x + 12, input1.y + 11, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12 + get_text_width(font_main, state->text_email), input1.y + 10, white_color);
        }

        // Field 2: Password field
        draw_text(renderer, font_label, "PASSWORD", card_rect.x + 40, card_rect.y + 245, gray_color);
        SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
        SDL_RenderFillRect(renderer, &input2);
        if (state->current_focus == FOCUS_PASSWORD) {
            SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
        } else {
            SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
        }
        SDL_RenderDrawRect(renderer, &input2);

        if (strlen(state->text_password) == 0) {
            draw_text(renderer, font_main, "Enter your password", input2.x + 12, input2.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12, input2.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t p_len = strlen(state->text_password);
            for(size_t i = 0; i < p_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input2.x + 12, input2.y + 11, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12 + get_text_width(font_main, stars), input2.y + 10, white_color);
        }
    } 
    else // TAB_REGISTER layout configuration
    {
        draw_text(renderer, font_title, "Create an account", card_rect.x + 40, card_rect.y + 65, white_color);

        // Inputs rendering processing loop logic maps
        SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
        SDL_RenderFillRect(renderer, &input1); SDL_RenderFillRect(renderer, &input2);
        SDL_RenderFillRect(renderer, &input3); SDL_RenderFillRect(renderer, &input4);

        SDL_SetRenderDrawColor(renderer, (state->current_focus == FOCUS_EMAIL) ? COLOR_DISCORD_BLUE : COLOR_INPUT_BORDER); SDL_RenderDrawRect(renderer, &input1);
        SDL_SetRenderDrawColor(renderer, (state->current_focus == FOCUS_USERNAME) ? COLOR_DISCORD_BLUE : COLOR_INPUT_BORDER); SDL_RenderDrawRect(renderer, &input2);
        SDL_SetRenderDrawColor(renderer, (state->current_focus == FOCUS_PASSWORD) ? COLOR_DISCORD_BLUE : COLOR_INPUT_BORDER); SDL_RenderDrawRect(renderer, &input3);
        SDL_SetRenderDrawColor(renderer, (state->current_focus == FOCUS_CONFIRM) ? COLOR_DISCORD_BLUE : COLOR_INPUT_BORDER); SDL_RenderDrawRect(renderer, &input4);

        // Email field UI mapping
        draw_text(renderer, font_label, "EMAIL ADDRESS", card_rect.x + 40, card_rect.y + 117, gray_color);
        if (strlen(state->text_email) == 0) {
            draw_text(renderer, font_main, "Enter your email address", input1.x + 12, input1.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12, input1.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_email, input1.x + 12, input1.y + 11, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12 + get_text_width(font_main, state->text_email), input1.y + 10, white_color);
        }

        // Username field UI mapping
        draw_text(renderer, font_label, "USERNAME", card_rect.x + 40, card_rect.y + 214, gray_color);
        if (strlen(state->text_username) == 0) {
            draw_text(renderer, font_main, "What should we call you?", input2.x + 12, input2.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12, input2.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_username, input2.x + 12, input2.y + 11, white_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12 + get_text_width(font_main, state->text_username), input2.y + 10, white_color);
        }

        // Password field UI mapping
        draw_text(renderer, font_label, "PASSWORD", card_rect.x + 40, card_rect.y + 312, gray_color);
        if (strlen(state->text_password) == 0) {
            draw_text(renderer, font_main, "Choose a password", input3.x + 12, input3.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input3.x + 12, input3.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t p_len = strlen(state->text_password);
            for(size_t i = 0; i < p_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input3.x + 12, input3.y + 11, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input3.x + 12 + get_text_width(font_main, stars), input3.y + 10, white_color);
        }

        // Confirm Password field UI mapping
        draw_text(renderer, font_label, "CONFIRM PASSWORD", card_rect.x + 40, card_rect.y + 410, gray_color);
        if (strlen(state->text_confirm) == 0) {
            draw_text(renderer, font_main, "Confirm your password", input4.x + 12, input4.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_CONFIRM && show_cursor) draw_text(renderer, font_main, "|", input4.x + 12, input4.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t c_len = strlen(state->text_confirm);
            for(size_t i = 0; i < c_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input4.x + 12, input4.y + 11, white_color);
            if (state->current_focus == FOCUS_CONFIRM && show_cursor) draw_text(renderer, font_main, "|", input4.x + 12 + get_text_width(font_main, stars), input4.y + 10, white_color);
        }
    }

    // --- 4. RENDER VALIDATION MAIN ACTION BUTTON ---
    SDL_Rect button_action_rect = {card_rect.x + 40, card_rect.y + (state->current_tab == TAB_LOGIN ? 330 : 495), 290, 44};
    if (is_hovering_button) {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE_HOVER);
    } else {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    }
    SDL_RenderFillRect(renderer, &button_action_rect);

    const char *btn_text = (state->current_tab == TAB_LOGIN) ? "Log In" : "Continue";
    int btn_w = get_text_width(font_sub, btn_text);
    draw_text(renderer, font_sub, btn_text, button_action_rect.x + (button_action_rect.w - btn_w) / 2, button_action_rect.y + 13, white_color);
}
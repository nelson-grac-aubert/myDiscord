#include "../include/ui_login.h"
#include <string.h>

#define COLOR_CARD_GRAY          0x1E, 0x1F, 0x22, 0xFF
#define COLOR_DISCORD_BLUE       0x58, 0x65, 0xF2, 0xFF
#define COLOR_DISCORD_BLUE_HOVER 0x47, 0x52, 0xC4, 0xFF 
#define COLOR_TAB_ACTIVE_BG      0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_TAB_INACTIVE_BG    0x1E, 0x1F, 0x22, 0xFF 
#define COLOR_INPUT_BG           0x11, 0x12, 0x14, 0xFF
#define COLOR_INPUT_BORDER       0x35, 0x37, 0x3C, 0xFF 

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
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

int get_text_width(TTF_Font *font, const char *text) {
    if (!font || !text || text[0] == '\0') return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(font, text, &w, &h);
    return w;
}

void draw_login_interface(SDL_Renderer *renderer, SDL_Rect card_rect, UIState *state,
                          TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_label,
                          int is_hovering_button) {
    
    SDL_Color white_color = {255, 255, 255, 255};
    SDL_Color gray_color  = {148, 155, 164, 255};
    SDL_Color placeholder_color = {80, 84, 92, 255};

    int tab_width = card_rect.w / 2;
    int tab_height = 45; 

    SDL_Rect tab_login_rect = {card_rect.x, card_rect.y, tab_width, tab_height};
    SDL_Rect tab_reg_rect   = {card_rect.x + tab_width, card_rect.y, tab_width, tab_height};

    // Définition des 4 rectangles physiques verticaux
    SDL_Rect input1 = { card_rect.x + 40, card_rect.y + 180, 370, 40 };
    SDL_Rect input2 = { card_rect.x + 40, card_rect.y + 270, 370, 40 };
    SDL_Rect input3 = { card_rect.x + 40, card_rect.y + 360, 370, 40 };
    SDL_Rect input4 = { card_rect.x + 40, card_rect.y + 450, 370, 40 }; // Uniquement Inscription

    SDL_Rect btn_submit = { card_rect.x + (card_rect.w - 370) / 2, card_rect.y + card_rect.h - 65, 370, 45 };

    // 1. Fond
    SDL_SetRenderDrawColor(renderer, COLOR_CARD_GRAY);
    SDL_RenderFillRect(renderer, &card_rect);

    // 2. Onglets avec ligne bleue sous l'onglet actif
    if (state->current_tab == TAB_LOGIN) {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);   SDL_RenderFillRect(renderer, &tab_login_rect);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG); SDL_RenderFillRect(renderer, &tab_reg_rect);
        SDL_Rect active_line = {tab_login_rect.x, tab_login_rect.y + tab_login_rect.h - 3, tab_login_rect.w, 3};
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);    SDL_RenderFillRect(renderer, &active_line);
    } else {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG); SDL_RenderFillRect(renderer, &tab_login_rect);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);   SDL_RenderFillRect(renderer, &tab_reg_rect);
        SDL_Rect active_line = {tab_reg_rect.x, tab_reg_rect.y + tab_reg_rect.h - 3, tab_reg_rect.w, 3};
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);    SDL_RenderFillRect(renderer, &active_line);
    }

    // 3. Rendu arrières-plans des Inputs
    SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
    SDL_RenderFillRect(renderer, &input1);
    SDL_RenderFillRect(renderer, &input2);
    if (state->current_tab == TAB_REGISTER) {
        SDL_RenderFillRect(renderer, &input3);
        SDL_RenderFillRect(renderer, &input4);
    }

    // 4. Bordures d'activation (Focus)
    SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
    if (state->current_focus == FOCUS_EMAIL) SDL_RenderDrawRect(renderer, &input1);
    else if (state->current_focus == FOCUS_USERNAME && state->current_tab == TAB_REGISTER) SDL_RenderDrawRect(renderer, &input2);
    else if (state->current_focus == FOCUS_PASSWORD) {
        SDL_RenderDrawRect(renderer, (state->current_tab == TAB_LOGIN) ? &input2 : &input3);
    }
    else if (state->current_focus == FOCUS_CONFIRM && state->current_tab == TAB_REGISTER) SDL_RenderDrawRect(renderer, &input4);

    // 5. Bouton
    if (is_hovering_button) {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE_HOVER);
    } else {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    }
    SDL_RenderFillRect(renderer, &btn_submit);

    // Textes génériques
    Uint32 ticks = SDL_GetTicks();
    int show_cursor = (ticks / 500) % 2;
    int text_padding_y = (tab_height - 14) / 2; 

    int tx_login_w = get_text_width(font_sub, "SE CONNECTER");
    int tx_reg_w   = get_text_width(font_sub, "S'INSCRIRE");
    draw_text(renderer, font_sub, "SE CONNECTER", tab_login_rect.x + (tab_login_rect.w - tx_login_w)/2, tab_login_rect.y + text_padding_y, (state->current_tab == TAB_LOGIN) ? white_color : gray_color);
    draw_text(renderer, font_sub, "S'INSCRIRE", tab_reg_rect.x + (tab_reg_rect.w - tx_reg_w)/2, tab_reg_rect.y + text_padding_y, (state->current_tab == TAB_REGISTER) ? white_color : gray_color);

    // --- MODE GAUCHE : SE CONNECTER ---
    if (state->current_tab == TAB_LOGIN) {
        int title_w = get_text_width(font_title, "Ravi de vous revoir !");
        draw_text(renderer, font_title, "Ravi de vous revoir !", card_rect.x + (card_rect.w - title_w)/2, card_rect.y + 80, white_color);
        
        // SECTION 1 : E-MAIL OU NOM D'UTILISATEUR
        draw_text(renderer, font_label, "E-MAIL OU NOM D'UTILISATEUR", card_rect.x + 40, card_rect.y + 160, gray_color);
        if (strlen(state->text_email) == 0) {
            draw_text(renderer, font_main, "nom@exemple.com", input1.x + 10, input1.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 10, input1.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_email, input1.x + 10, input1.y + 10, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 10 + get_text_width(font_main, state->text_email), input1.y + 10, white_color);
        }

        // SECTION 2 : MOT DE PASSE
        draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 250, gray_color);
        if (strlen(state->text_password) == 0) {
            draw_text(renderer, font_main, "Minimum 8 caractères", input2.x + 10, input2.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input2.x + 10, input2.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t p_len = strlen(state->text_password);
            for(size_t i = 0; i < p_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input2.x + 10, input2.y + 10, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input2.x + 10 + get_text_width(font_main, stars), input2.y + 10, white_color);
        }
        
        int btn_w = get_text_width(font_sub, "Se connecter");
        draw_text(renderer, font_sub, "Se connecter", btn_submit.x + (btn_submit.w - btn_w)/2, btn_submit.y + 13, white_color);
    } 
    // --- MODE DROITE : CRÉER UN COMPTE ---
    else {
        int title_w = get_text_width(font_title, "Créer un compte");
        draw_text(renderer, font_title, "Créer un compte", card_rect.x + (card_rect.w - title_w)/2, card_rect.y + 80, white_color);
        
        // SECTION 1 : ADRESSE E-MAIL
        draw_text(renderer, font_label, "ADRESSE E-MAIL", card_rect.x + 40, card_rect.y + 160, gray_color);
        if (strlen(state->text_email) == 0) {
            draw_text(renderer, font_main, "nom@exemple.com", input1.x + 10, input1.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 10, input1.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_email, input1.x + 10, input1.y + 10, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 10 + get_text_width(font_main, state->text_email), input1.y + 10, white_color);
        }

        // SECTION 2 : PSEUDO
        draw_text(renderer, font_label, "PSEUDO", card_rect.x + 40, card_rect.y + 250, gray_color);
        if (strlen(state->text_username) == 0) {
            draw_text(renderer, font_main, "Ton pseudo", input2.x + 10, input2.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor) draw_text(renderer, font_main, "|", input2.x + 10, input2.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_username, input2.x + 10, input2.y + 10, white_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor) draw_text(renderer, font_main, "|", input2.x + 10 + get_text_width(font_main, state->text_username), input2.y + 10, white_color);
        }

        // SECTION 3 : MOT DE PASSE
        draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 340, gray_color);
        if (strlen(state->text_password) == 0) {
            draw_text(renderer, font_main, "Minimum 8 caractères", input3.x + 10, input3.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input3.x + 10, input3.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t p_len = strlen(state->text_password);
            for(size_t i = 0; i < p_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input3.x + 10, input3.y + 10, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input3.x + 10 + get_text_width(font_main, stars), input3.y + 10, white_color);
        }

        // SECTION 4 : CONFIRMATION
        draw_text(renderer, font_label, "CONFIRMATION", card_rect.x + 40, card_rect.y + 430, gray_color);
        if (strlen(state->text_confirm) == 0) {
            draw_text(renderer, font_main, "Confirme ton mot de passe", input4.x + 10, input4.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_CONFIRM && show_cursor) draw_text(renderer, font_main, "|", input4.x + 10, input4.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t c_len = strlen(state->text_confirm);
            for(size_t i = 0; i < c_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input4.x + 10, input4.y + 10, white_color);
            if (state->current_focus == FOCUS_CONFIRM && show_cursor) draw_text(renderer, font_main, "|", input4.x + 10 + get_text_width(font_main, stars), input4.y + 10, white_color);
        }

        int btn_w = get_text_width(font_sub, "S'inscrire");
        draw_text(renderer, font_sub, "S'inscrire", btn_submit.x + (btn_submit.w - btn_w)/2, btn_submit.y + 13, white_color);
    }
}
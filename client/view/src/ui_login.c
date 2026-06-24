#include "ui_login.h"
#include <string.h>

#define COLOR_CARD_GRAY          0x2B, 0x2D, 0x31, 0xFF // Gris de carte Discord d'origine
#define COLOR_DISCORD_BLUE       0x58, 0x65, 0xF2, 0xFF
#define COLOR_DISCORD_BLUE_HOVER 0x47, 0x52, 0xC4, 0xFF 
#define COLOR_TAB_ACTIVE_BG      0x2B, 0x2D, 0x31, 0xFF 
#define COLOR_TAB_INACTIVE_BG    0x1E, 0x1F, 0x22, 0xFF 
#define COLOR_INPUT_BG           0x1E, 0x1F, 0x22, 0xFF // Fond sombre des inputs
#define COLOR_INPUT_BORDER       0x3F, 0x41, 0x47, 0xFF // Bordure par défaut

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
    SDL_Color placeholder_color = {128, 132, 142, 255};

    int tab_width = card_rect.w / 2;
    int tab_height = 50; 

    SDL_Rect tab_login_rect = {card_rect.x, card_rect.y, tab_width, tab_height};
    SDL_Rect tab_reg_rect   = {card_rect.x + tab_width, card_rect.y, tab_width, tab_height};

    SDL_Rect input1 = { card_rect.x + 40, card_rect.y + 195, 370, 40 };
    SDL_Rect input2 = { card_rect.x + 40, card_rect.y + 285, 370, 40 };
    SDL_Rect input3 = { card_rect.x + 40, card_rect.y + 375, 370, 40 };
    SDL_Rect input4 = { card_rect.x + 40, card_rect.y + 465, 370, 40 };
    SDL_Rect btn_submit = { card_rect.x + (card_rect.w - 370) / 2, card_rect.y + card_rect.h - 70, 370, 45 };

    int show_cursor = (SDL_GetTicks() / 500) % 2;

    // Fond global de la carte
    SDL_SetRenderDrawColor(renderer, COLOR_CARD_GRAY);
    SDL_RenderFillRect(renderer, &card_rect);

    // Dessin des onglets
    if (state->current_tab == TAB_LOGIN) {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_login_rect);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_reg_rect);
    } else {
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_login_rect);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_reg_rect);
    }

    // Séparateur horizontal sous les onglets si inactifs
    SDL_SetRenderDrawColor(renderer, 0x1E, 0x1F, 0x22, 255);
    SDL_RenderDrawLine(renderer, card_rect.x, card_rect.y + tab_height, card_rect.x + card_rect.w, card_rect.y + tab_height);

    // Dessin des textes d'onglets
    int tw1 = get_text_width(font_sub, "Se connecter");
    int tw2 = get_text_width(font_sub, "S'inscrire");
    draw_text(renderer, font_sub, "Se connecter", tab_login_rect.x + (tab_login_rect.w - tw1)/2, tab_login_rect.y + 16, state->current_tab == TAB_LOGIN ? white_color : gray_color);
    draw_text(renderer, font_sub, "S'inscrire", tab_reg_rect.x + (tab_reg_rect.w - tw2)/2, tab_reg_rect.y + 16, state->current_tab == TAB_REGISTER ? white_color : gray_color);

    // 🌟 TRAIT BLEU SOUS L'ONGLET ACTIF
    SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    if (state->current_tab == TAB_LOGIN) {
        SDL_Rect line_blue = {tab_login_rect.x + (tab_login_rect.w - tw1)/2, tab_login_rect.y + tab_height - 3, tw1, 3};
        SDL_RenderFillRect(renderer, &line_blue);
    } else {
        SDL_Rect line_blue = {tab_reg_rect.x + (tab_reg_rect.w - tw2)/2, tab_reg_rect.y + tab_height - 3, tw2, 3};
        SDL_RenderFillRect(renderer, &line_blue);
    }

    if (state->current_tab == TAB_LOGIN) {
        // --- VUE LOGIN ---
        int tw_title = get_text_width(font_title, "De retour !");
        draw_text(renderer, font_title, "De retour !", card_rect.x + (card_rect.w - tw_title)/2, card_rect.y + 90, white_color);
        int tw_sub = get_text_width(font_main, "Nous sommes ravis de te revoir !");
        draw_text(renderer, font_main, "Nous sommes ravis de te revoir !", card_rect.x + (card_rect.w - tw_sub)/2, card_rect.y + 125, gray_color);

        // --- INPUT EMAIL ---
        SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
        SDL_RenderFillRect(renderer, &input1);
        if (state->current_focus == FOCUS_EMAIL) {
            SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE); // Bordure Focus
        } else {
            SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
        }
        SDL_RenderDrawRect(renderer, &input1);

        draw_text(renderer, font_label, "ADRESSE E-MAIL", card_rect.x + 40, card_rect.y + 172, gray_color);
        if (strlen(state->text_email) == 0) {
            draw_text(renderer, font_main, "nom@exemple.com", input1.x + 12, input1.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12, input1.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_email, input1.x + 12, input1.y + 11, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12 + get_text_width(font_main, state->text_email), input1.y + 10, white_color);
        }

        // --- INPUT MOT DE PASSE ---
        SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
        SDL_RenderFillRect(renderer, &input2);
        if (state->current_focus == FOCUS_PASSWORD) {
            SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE); // Bordure Focus
        } else {
            SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
        }
        SDL_RenderDrawRect(renderer, &input2);

        draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 262, gray_color);
        if (strlen(state->text_password) == 0) {
            draw_text(renderer, font_main, "Ton mot de passe", input2.x + 12, input2.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12, input2.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t p_len = strlen(state->text_password);
            for(size_t i = 0; i < p_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input2.x + 12, input2.y + 11, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12 + get_text_width(font_main, stars), input2.y + 10, white_color);
        }
    } else {
        // --- VUE REGISTER ---
        int tw_title = get_text_width(font_title, "Créer un compte");
        draw_text(renderer, font_title, "Créer un compte", card_rect.x + (card_rect.w - tw_title)/2, card_rect.y + 80, white_color);

        // Rendu des 4 inputs avec gestion de focus individuelle
        SDL_Rect rects[4] = {input1, input2, input3, input4};
        FocusField fields[4] = {FOCUS_EMAIL, FOCUS_USERNAME, FOCUS_PASSWORD, FOCUS_CONFIRM};
        
        for (int i = 0; i < 4; i++) {
            SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
            SDL_RenderFillRect(renderer, &rects[i]);
            if (state->current_focus == fields[i]) {
                SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
            } else {
                SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
            }
            SDL_RenderDrawRect(renderer, &rects[i]);
        }

        // Email
        draw_text(renderer, font_label, "E-MAIL", card_rect.x + 40, card_rect.y + 172, gray_color);
        if (strlen(state->text_email) == 0) {
            draw_text(renderer, font_main, "nom@exemple.com", input1.x + 12, input1.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12, input1.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_email, input1.x + 12, input1.y + 11, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor) draw_text(renderer, font_main, "|", input1.x + 12 + get_text_width(font_main, state->text_email), input1.y + 10, white_color);
        }

        // Username
        draw_text(renderer, font_label, "NOM D'UTILISATEUR", card_rect.x + 40, card_rect.y + 262, gray_color);
        if (strlen(state->text_username) == 0) {
            draw_text(renderer, font_main, "Comment devrions-nous t'appeler ?", input2.x + 12, input2.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12, input2.y + 10, white_color);
        } else {
            draw_text(renderer, font_main, state->text_username, input2.x + 12, input2.y + 11, white_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor) draw_text(renderer, font_main, "|", input2.x + 12 + get_text_width(font_main, state->text_username), input2.y + 10, white_color);
        }

        // Password
        draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 352, gray_color);
        if (strlen(state->text_password) == 0) {
            draw_text(renderer, font_main, "Un mot de passe solide", input3.x + 12, input3.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input3.x + 12, input3.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t p_len = strlen(state->text_password);
            for(size_t i = 0; i < p_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input3.x + 12, input3.y + 11, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor) draw_text(renderer, font_main, "|", input3.x + 12 + get_text_width(font_main, stars), input3.y + 10, white_color);
        }

        // Confirm
        draw_text(renderer, font_label, "CONFIRMATION DU MOT DE PASSE", card_rect.x + 40, card_rect.y + 442, gray_color);
        if (strlen(state->text_confirm) == 0) {
            draw_text(renderer, font_main, "Confirme ton mot de passe", input4.x + 12, input4.y + 11, placeholder_color);
            if (state->current_focus == FOCUS_CONFIRM && show_cursor) draw_text(renderer, font_main, "|", input4.x + 12, input4.y + 10, white_color);
        } else {
            char stars[128] = ""; size_t c_len = strlen(state->text_confirm);
            for(size_t i = 0; i < c_len && i < 20; i++) strcat(stars, "*");
            draw_text(renderer, font_main, stars, input4.x + 12, input4.y + 11, white_color);
            if (state->current_focus == FOCUS_CONFIRM && show_cursor) draw_text(renderer, font_main, "|", input4.x + 12 + get_text_width(font_main, stars), input4.y + 10, white_color);
        }
    }

    // Bouton de validation
    if (is_hovering_button) {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE_HOVER);
    } else {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    }
    SDL_RenderFillRect(renderer, &btn_submit);

    const char *btn_label = (state->current_tab == TAB_LOGIN) ? "Connexion" : "Continuer";
    int tw_btn = get_text_width(font_sub, btn_label);
    draw_text(renderer, font_sub, btn_label, btn_submit.x + (btn_submit.w - tw_btn)/2, btn_submit.y + 13, white_color);
}
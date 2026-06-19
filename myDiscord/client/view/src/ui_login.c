#include "../include/ui_login.h"
#include <string.h>

// Couleurs thématiques Kinetic Obsidian
#define COLOR_CARD_GRAY 0x1E, 0x1F, 0x22, 0xFF
#define COLOR_DISCORD_BLUE 0x58, 0x65, 0xF2, 0xFF
#define COLOR_DISCORD_BLUE_HOVER 0x47, 0x52, 0xC4, 0xFF
#define COLOR_TAB_ACTIVE_BG 0x2B, 0x2D, 0x31, 0xFF
#define COLOR_TAB_INACTIVE_BG 0x1E, 0x1F, 0x22, 0xFF
#define COLOR_INPUT_BG 0x11, 0x12, 0x14, 0xFF
#define COLOR_INPUT_BORDER 0x35, 0x37, 0x3C, 0xFF

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
        SDL_Rect dst_rect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
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

void draw_login_interface(SDL_Renderer *renderer, SDL_Rect card_rect, UIState *state,
                          TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_label,
                          int is_hovering_button)
{

    SDL_Color white_color = {255, 255, 255, 255};
    SDL_Color gray_color = {148, 155, 164, 255};
    SDL_Color placeholder_color = {80, 84, 92, 255};

    // --- positions des composants (Réordonnées logiquement) ---
    int tab_width = card_rect.w / 2;
    int tab_height = 45;

    SDL_Rect tab_login_rect = {card_rect.x, card_rect.y, tab_width, tab_height};
    SDL_Rect tab_reg_rect = {card_rect.x + tab_width, card_rect.y, tab_width, tab_height};

    SDL_Rect email_input = {card_rect.x + 40, card_rect.y + 180, 370, 40};
    SDL_Rect user_input = {card_rect.x + 40, card_rect.y + 280, 370, 40}; // Milieu
    SDL_Rect pass_input = {card_rect.x + 40, card_rect.y + 380, 370, 40}; // Bas

    SDL_Rect btn_submit = {card_rect.x + (card_rect.w - 370) / 2, card_rect.y + card_rect.h - 60, 370, 45};

    // 1. Fond de la carte
    SDL_SetRenderDrawColor(renderer, COLOR_CARD_GRAY);
    SDL_RenderFillRect(renderer, &card_rect);

    // 2. Rendu des Onglets (Style moderne avec ligne bleue)
    if (state->current_tab == TAB_LOGIN)
    {
        // Onglet Connexion Actif
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_login_rect);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_reg_rect);

        // Petite ligne bleue sous l'onglet Connexion
        SDL_Rect active_line = {tab_login_rect.x, tab_login_rect.y + tab_login_rect.h - 3, tab_login_rect.w, 3};
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
        SDL_RenderFillRect(renderer, &active_line);
    }
    else
    {
        // Onglet Inscription Actif
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_INACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_login_rect);
        SDL_SetRenderDrawColor(renderer, COLOR_TAB_ACTIVE_BG);
        SDL_RenderFillRect(renderer, &tab_reg_rect);

        // Petite ligne bleue sous l'onglet Inscription
        SDL_Rect active_line = {tab_reg_rect.x, tab_reg_rect.y + tab_reg_rect.h - 3, tab_reg_rect.w, 3};
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
        SDL_RenderFillRect(renderer, &active_line);
    }

    // 3. Rendu des arrières-plans des Inputs
    SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BG);
    SDL_RenderFillRect(renderer, &email_input);

    if (state->current_tab == TAB_LOGIN)
    {
        SDL_RenderFillRect(renderer, &user_input); // En mode login, le second champ physique (milieu) sert au mot de passe
    }
    else
    {
        SDL_RenderFillRect(renderer, &user_input); // Milieu (Username)
        SDL_RenderFillRect(renderer, &pass_input); // Bas (Password)
    }

    // 4. Bordures des Inputs (si focus actif)
    SDL_SetRenderDrawColor(renderer, COLOR_INPUT_BORDER);
    if (state->current_focus == FOCUS_EMAIL)
    {
        SDL_RenderDrawRect(renderer, &email_input);
    }
    else if (state->current_focus == FOCUS_USERNAME && state->current_tab == TAB_REGISTER)
    {
        SDL_RenderDrawRect(renderer, &user_input);
    }
    else if (state->current_focus == FOCUS_PASSWORD)
    {
        if (state->current_tab == TAB_LOGIN)
        {
            SDL_RenderDrawRect(renderer, &user_input); // En Login, le Password est au milieu
        }
        else
        {
            SDL_RenderDrawRect(renderer, &pass_input); // En Register, le Password est en bas
        }
    }

    // 5. Rendu du bouton de validation
    if (is_hovering_button)
    {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE_HOVER);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, COLOR_DISCORD_BLUE);
    }
    SDL_RenderFillRect(renderer, &btn_submit);

    // 6. Affichage des Textes
    Uint32 ticks = SDL_GetTicks();
    int show_cursor = (ticks / 500) % 2;
    int text_padding_y = (tab_height - 14) / 2;

    int tx_login_w = get_text_width(font_sub, "SE CONNECTER");
    int tx_reg_w = get_text_width(font_sub, "S'INSCRIRE");
    draw_text(renderer, font_sub, "SE CONNECTER", tab_login_rect.x + (tab_login_rect.w - tx_login_w) / 2, tab_login_rect.y + text_padding_y, (state->current_tab == TAB_LOGIN) ? white_color : gray_color);
    draw_text(renderer, font_sub, "S'INSCRIRE", tab_reg_rect.x + (tab_reg_rect.w - tx_reg_w) / 2, tab_reg_rect.y + text_padding_y, (state->current_tab == TAB_REGISTER) ? white_color : gray_color);

    if (state->current_tab == TAB_LOGIN)
    {
        draw_text(renderer, font_title, "CONNEXION", card_rect.x + 160, card_rect.y + 80, white_color);
        draw_text(renderer, font_label, "EMAIL OU NUMÉRO DE TÉLÉPHONE", card_rect.x + 40, card_rect.y + 160, gray_color);

        if (strlen(state->text_email) == 0)
        {
            draw_text(renderer, font_main, "nom@exemple.com", email_input.x + 10, email_input.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor)
                draw_text(renderer, font_main, "|", email_input.x + 10, email_input.y + 10, white_color);
        }
        else
        {
            draw_text(renderer, font_main, state->text_email, email_input.x + 10, email_input.y + 10, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor)
                draw_text(renderer, font_main, "|", email_input.x + 10 + get_text_width(font_main, state->text_email), email_input.y + 10, white_color);
        }

        draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 260, gray_color);
        if (strlen(state->text_password) == 0)
        {
            draw_text(renderer, font_main, "Minimum 8 caractères", user_input.x + 10, user_input.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor)
                draw_text(renderer, font_main, "|", user_input.x + 10, user_input.y + 10, white_color);
        }
        else
        {
            char stars[128] = "";
            size_t p_len = strlen(state->text_password);
            for (size_t i = 0; i < p_len && i < 20; i++)
                strcat(stars, "*");
            draw_text(renderer, font_main, stars, user_input.x + 10, user_input.y + 10, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor)
                draw_text(renderer, font_main, "|", user_input.x + 10 + get_text_width(font_main, stars), user_input.y + 10, white_color);
        }
        draw_text(renderer, font_main, "Se connecter", btn_submit.x + 135, btn_submit.y + 12, white_color);
    }
    else
    {
        draw_text(renderer, font_title, "S'INSCRIRE", card_rect.x + 160, card_rect.y + 80, white_color);

        // 1. ADRESSE EMAIL (Haut)
        draw_text(renderer, font_label, "ADRESSE EMAIL", card_rect.x + 40, card_rect.y + 160, gray_color);
        if (strlen(state->text_email) == 0)
        {
            draw_text(renderer, font_main, "nom@exemple.com", email_input.x + 10, email_input.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor)
                draw_text(renderer, font_main, "|", email_input.x + 10, email_input.y + 10, white_color);
        }
        else
        {
            draw_text(renderer, font_main, state->text_email, email_input.x + 10, email_input.y + 10, white_color);
            if (state->current_focus == FOCUS_EMAIL && show_cursor)
                draw_text(renderer, font_main, "|", email_input.x + 10 + get_text_width(font_main, state->text_email), email_input.y + 10, white_color);
        }

        // 2. NOM D'UTILISATEUR (Milieu)
        draw_text(renderer, font_label, "NOM D'UTILISATEUR", card_rect.x + 40, card_rect.y + 260, gray_color);
        if (strlen(state->text_username) == 0)
        {
            draw_text(renderer, font_main, "Comment voulez-vous qu'on vous appelle ?", user_input.x + 10, user_input.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor)
                draw_text(renderer, font_main, "|", user_input.x + 10, user_input.y + 10, white_color);
        }
        else
        {
            draw_text(renderer, font_main, state->text_username, user_input.x + 10, user_input.y + 10, white_color);
            if (state->current_focus == FOCUS_USERNAME && show_cursor)
                draw_text(renderer, font_main, "|", user_input.x + 10 + get_text_width(font_main, state->text_username), user_input.y + 10, white_color);
        }

        // 3. MOT DE PASSE (Bas)
        draw_text(renderer, font_label, "MOT DE PASSE", card_rect.x + 40, card_rect.y + 360, gray_color);
        if (strlen(state->text_password) == 0)
        {
            draw_text(renderer, font_main, "Minimum 8 caractères", pass_input.x + 10, pass_input.y + 10, placeholder_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor)
                draw_text(renderer, font_main, "|", pass_input.x + 10, pass_input.y + 10, white_color);
        }
        else
        {
            char stars[128] = "";
            size_t p_len = strlen(state->text_password);
            for (size_t i = 0; i < p_len && i < 20; i++)
                strcat(stars, "*");
            draw_text(renderer, font_main, stars, pass_input.x + 10, pass_input.y + 10, white_color);
            if (state->current_focus == FOCUS_PASSWORD && show_cursor)
                draw_text(renderer, font_main, "|", pass_input.x + 10 + get_text_width(font_main, stars), pass_input.y + 10, white_color);
        }
        draw_text(renderer, font_main, "Continuer", btn_submit.x + 150, btn_submit.y + 12, white_color);
    }
}
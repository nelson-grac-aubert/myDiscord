#include "../include/ui_channels.h"
#include "channel.h"
#include "../include/ui_login.h" // pour draw_text
#include <stdio.h>

SDL_Rect btn_logout;
SDL_Rect btn_add_channel;

void channels_clear_textures(void) {
    // Plus rien à nettoyer !
}

void channels_update_layout(ChatLayout *layout, int win_h) {
    int btn_w = layout->sidebar_channels.w - 32;
    int btn_h = 36;
    btn_logout = (SDL_Rect){layout->sidebar_channels.x + 16, win_h - 52, btn_w, btn_h};
    btn_add_channel = (SDL_Rect){layout->sidebar_channels.x + layout->sidebar_channels.w - 35, 18, 20, 20};
}

void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray) {
    (void)dark_gray; // 💡 Dis au compilateur que c'est normal si elle n'est pas utilisée ici !
    // --- 1. EN-TÊTE DE LA SIDEBAR ---
    draw_text(renderer, font_title, "Salons textuels", layout->sidebar_channels.x + 15, 15, white_color);

    int is_add_hovered = (mx >= btn_add_channel.x && mx <= btn_add_channel.x + btn_add_channel.w && my >= btn_add_channel.y && my <= btn_add_channel.y + btn_add_channel.w);
    draw_text(renderer, font_title, "+", btn_add_channel.x, btn_add_channel.y - 3, is_add_hovered ? white_color : gray_color);

    // --- 2. LISTE DES SALONS DEPUIS LE MODÈLE ---
    int channel_y = 60;
    int ch_count = channel_model_get_count();

    for (int i = 0; i < ch_count; i++) {
        Channel *ch = channel_model_get_by_index(i);
        if (!ch) continue;

        char ch_display[64];
        snprintf(ch_display, sizeof(ch_display), "%s  %s", ch->is_private ? "🔒" : "#", ch->name);

        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, channel_y - 4, layout->sidebar_channels.w - 16, 28};
        int is_hovered = (mx >= item_rect.x && mx <= item_rect.x + item_rect.w && my >= item_rect.y && my <= item_rect.y + item_rect.h);

        if (i == channel_model_get_active_index()) {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0xFF); 
            SDL_RenderFillRect(renderer, &item_rect);
            draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, white_color);
        } else if (is_hovered && !layout->show_create_modal) {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0x80); 
            SDL_RenderFillRect(renderer, &item_rect);
            draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, white_color);
        } else {
            draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, gray_color);
        }
        channel_y += 32;
    }

    // --- 3. RENDU DU BOUTON DÉCONNEXION ---
    int h_log = (mx >= btn_logout.x && mx <= btn_logout.x + btn_logout.w && my >= btn_logout.y && my <= btn_logout.y + btn_logout.h);
    if (h_log) {
        SDL_SetRenderDrawColor(renderer, 0xDA, 0x37, 0x3C, 0xFF);
    } else {
        SDL_SetRenderDrawColor(renderer, 0xA9, 0x1D, 0x22, 0xFF);
    }
    SDL_RenderFillRect(renderer, &btn_logout);
    
    int text_w = 0;
    TTF_SizeUTF8(font_sub, "Déconnexion", &text_w, NULL);
    draw_text(renderer, font_sub, "Déconnexion", btn_logout.x + (btn_logout.w - text_w) / 2, btn_logout.y + 8, white_color);
}
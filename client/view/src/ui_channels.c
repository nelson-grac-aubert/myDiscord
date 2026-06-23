#include "../include/ui_channels.h"
#include "../include/ui_chat_store.h"
#include "../include/ui_chat_components.h"
#include <stdio.h>

// Seul le bouton déconnexion est conservé
SDL_Rect btn_logout;

// Conservée vide pour éviter de casser les appels dans ui_chat.c
void channels_clear_textures(void) {
    // Plus rien à nettoyer !
}

void channels_update_layout(ChatLayout *layout, int win_h) {
    // On donne une vraie forme de bouton rectangulaire large en bas de la barre
    int btn_w = layout->sidebar_channels.w - 32; // Prend presque toute la largeur
    int btn_h = 36;                             // Hauteur confortable pour cliquer
    
    btn_logout = (SDL_Rect){layout->sidebar_channels.x + 16, win_h - 52, btn_w, btn_h};

    // Positionnement du bouton '+' pour créer un salon
    btn_add_channel = (SDL_Rect){layout->sidebar_channels.x + layout->sidebar_channels.w - 35, 18, 20, 20};
}

void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray) {
    
    // --- 1. DESSIN DU TITRE ET DU BOUTON '+' ---
    components_draw_text(renderer, font_sub, "SALONS TEXTUELS", layout->sidebar_channels.x + 15, 20, dark_gray);
    
    int hover_add = (mx >= btn_add_channel.x && mx <= btn_add_channel.x + btn_add_channel.w && my >= btn_add_channel.y && my <= btn_add_channel.y + btn_add_channel.h);
    if (hover_add && !layout->show_create_modal) {
        components_draw_text(renderer, font_title, "+", btn_add_channel.x, btn_add_channel.y - 2, white_color);
    } else {
        components_draw_text(renderer, font_title, "+", btn_add_channel.x, btn_add_channel.y - 2, gray_color);
    }

    // --- 2. RENDU DE LA LISTE DES SALONS ---
    int channel_y = 55;
    for (int i = 0; i < store_get_channel_count(); i++) {
        LocalChannel *ch = store_get_channel(i);
        if (!ch) continue;

        char ch_display[64];
        snprintf(ch_display, sizeof(ch_display), "%s  %s", ch->is_private ? "🔒" : "#", ch->name);

        // Zone récursive de sélection identique à celle de ui_chat.c
        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, channel_y - 4, layout->sidebar_channels.w - 16, 28};
        int is_hovered = (mx >= item_rect.x && mx <= item_rect.x + item_rect.w && my >= item_rect.y && my <= item_rect.y + item_rect.h);

        if (i == store_get_active_index()) {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0xFF); 
            SDL_RenderFillRect(renderer, &item_rect);
            components_draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, white_color);
        } else if (is_hovered && !layout->show_create_modal) {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0x80); 
            SDL_RenderFillRect(renderer, &item_rect);
            components_draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, white_color);
        } else {
            components_draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, gray_color);
        }
        channel_y += 32;
    }

    // --- 3. RENDU DU BOUTON DÉCONNEXION (RECTANGLE ROUGE + TEXTE) ---
    int h_log = (mx >= btn_logout.x && mx <= btn_logout.x + btn_logout.w && my >= btn_logout.y && my <= btn_logout.y + btn_logout.h);
    
    if (h_log && !layout->show_create_modal) {
        SDL_SetRenderDrawColor(renderer, 242, 63, 67, 255); 
    } else {
        SDL_SetRenderDrawColor(renderer, 166, 38, 41, 255); 
    }
    
    SDL_RenderFillRect(renderer, &btn_logout);
    components_draw_text(renderer, font_sub, "Déconnexion", btn_logout.x + 58, btn_logout.y + 10, white_color);
}
#include "../include/ui_channels.h"
#include "../include/ui_chat_store.h"
#include "../include/ui_chat_components.h"
#include <SDL2/SDL_image.h> // 💡 Nécessaire pour IMG_LoadTexture

SDL_Rect btn_options;
SDL_Rect btn_logout;

// Variables statiques pour conserver les textures des icônes localement
static SDL_Texture *texture_settings = NULL;
static SDL_Texture *texture_logout = NULL;

void channels_update_layout(ChatLayout *layout, int win_h) {
    // Positionnement des boutons tout en bas de la barre des salons
    btn_options = (SDL_Rect){layout->sidebar_channels.x + 16, win_h - 56, 32, 32};
    btn_logout  = (SDL_Rect){layout->sidebar_channels.x + 64, win_h - 56, 32, 32};

    // ➡️ FIX : On positionne le bouton '+' tout à droite de la barre latérale des salons, centré avec le titre
    btn_add_channel = (SDL_Rect){layout->sidebar_channels.x + layout->sidebar_channels.w - 35, 18, 20, 20};
}

void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray, SDL_Color red_delete) {
    
    // --- 1. CHARGEMENT UNIQUE DES TEXTURES (si pas encore fait) ---
    if (texture_settings == NULL) {
        texture_settings = IMG_LoadTexture(renderer, "assets/bouton/btn-setting.png");
    }
    if (texture_logout == NULL) {
        texture_logout = IMG_LoadTexture(renderer, "assets/bouton/btn-disconnect.png");
    }

    // --- 2. DESSIN DU TITRE ET DU BOUTON '+' ---
    components_draw_text(renderer, font_sub, "SALONS TEXTUELS", layout->sidebar_channels.x + 15, 20, dark_gray);
    
    // Utilise les coordonnées corrigées calculées dans channels_update_layout
    int hover_add = (mx >= btn_add_channel.x && mx <= btn_add_channel.x + btn_add_channel.w && my >= btn_add_channel.y && my <= btn_add_channel.y + btn_add_channel.h);
    if (hover_add && !layout->show_create_modal) {
        components_draw_text(renderer, font_title, "+", btn_add_channel.x, btn_add_channel.y - 2, white_color);
    } else {
        components_draw_text(renderer, font_title, "+", btn_add_channel.x, btn_add_channel.y - 2, gray_color);
    }

    // --- 3. RENDU DE LA LISTE DES SALONS ---
    int channel_y = 55;
    for (int i = 0; i < store_get_channel_count(); i++) {
        LocalChannel *ch = store_get_channel(i);
        if (!ch) continue;

        char ch_display[64];
        snprintf(ch_display, sizeof(ch_display), "%s  %s", ch->is_private ? "🔒" : "#", ch->name);

        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, channel_y - 4, layout->sidebar_channels.w - 16, 28};
        int is_hovered = (mx >= item_rect.x && mx <= item_rect.x + item_rect.w && my >= item_rect.y && my <= item_rect.y + item_rect.h);

        if (i == store_get_active_index()) {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0xFF); SDL_RenderFillRect(renderer, &item_rect);
            components_draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, white_color);
        } else if (is_hovered && !layout->show_create_modal) {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0x80); SDL_RenderFillRect(renderer, &item_rect);
            components_draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, white_color);
        } else {
            components_draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, gray_color);
        }
        channel_y += 32;
    }

    // --- 4. COLORATION ET RENDU DES ICÔNES OPTIONS & DÉCONNEXION ---
    
    // Bouton Option / Settings
    if (texture_settings) {
        int h_opt = (mx >= btn_options.x && mx <= btn_options.x + btn_options.w && my >= btn_options.y && my <= btn_options.y + btn_options.h);
        if (h_opt && !layout->show_create_modal) {
            SDL_SetTextureColorMod(texture_settings, white_color.r, white_color.g, white_color.b); // Blanc au survol
        } else {
            SDL_SetTextureColorMod(texture_settings, gray_color.r, gray_color.g, gray_color.b);     // Gris par défaut
        }
        SDL_RenderCopy(renderer, texture_settings, NULL, &btn_options);
    }

    // Bouton Déconnexion / Logout
    if (texture_logout) {
        int h_log = (mx >= btn_logout.x && mx <= btn_logout.x + btn_logout.w && my >= btn_logout.y && my <= btn_logout.y + btn_logout.h);
        if (h_log && !layout->show_create_modal) {
            SDL_SetTextureColorMod(texture_logout, red_delete.r, red_delete.g, red_delete.b);     // Rouge pastel au survol
        } else {
            SDL_SetTextureColorMod(texture_logout, gray_color.r, gray_color.g, gray_color.b);     // Gris par défaut
        }
        SDL_RenderCopy(renderer, texture_logout, NULL, &btn_logout);
    }
}
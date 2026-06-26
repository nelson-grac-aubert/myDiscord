#include "../include/ui_channels.h"
#include "channel.h"
#include "../include/ui_login.h" // For draw_text
#include <stdio.h>

void channels_clear_textures(void)
{
    // Rien à libérer
}

void channels_update_layout(ChatLayout *layout, int win_h)
{
    (void)win_h;
    // CORRECTIF CENTRAL : On écrit directement dans la structure partagée lue par le contrôleur
    layout->btn_add_channel = (SDL_Rect){layout->sidebar_channels.x + layout->sidebar_channels.w - 35, 14, 24, 24};
}
void channels_draw_sidebar(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_emoji, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray)
{
    (void)dark_gray;
    (void)font_sub;

    // --- 1. TITRE DE LA SIDEBAR ---
    draw_text(renderer, font_title, "Text Channels", layout->sidebar_channels.x + 15, 15, white_color);

    // --- 2. LISTE DES SALONS ---
    int channel_y = 60;
    int ch_count = channel_model_get_count();

    for (int i = 0; i < ch_count; i++)
    {
        Channel *ch = channel_model_get_by_index(i);
        if (!ch)
            continue;

        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, channel_y - 4, layout->sidebar_channels.w - 16, 28};
        int is_hovered = (mx >= item_rect.x && mx <= item_rect.x + item_rect.w && my >= item_rect.y && my <= item_rect.y + item_rect.h);

        // 1. On détermine la couleur du texte (blanc si actif/survolé, gris sinon)
        SDL_Color current_color = gray_color;

        if (i == channel_model_get_active_index())
        {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0xFF);
            SDL_RenderFillRect(renderer, &item_rect);
            current_color = white_color;
        }
        else if (is_hovered && !layout->show_create_modal)
        {
            SDL_SetRenderDrawColor(renderer, 0x1F, 0x23, 0x33, 0x80);
            SDL_RenderFillRect(renderer, &item_rect);
            current_color = white_color;
        }

        // 2. CORRECTION AFFICHAGE : Dessin séparé pour le Cadenas et le Texte
        if (ch->is_private)
        {
            // On dessine le cadenas avec la police emoji
            draw_text(renderer, font_emoji, "🔒", layout->sidebar_channels.x + 15, channel_y, current_color);
            // On dessine le nom du salon décalé de 30 pixels vers la droite avec la police principale
            draw_text(renderer, font_main, ch->name, layout->sidebar_channels.x + 45, channel_y, current_color);
        }
        else
        {
            // Salon normal : on garde le fonctionnement avec le dièse "#"
            char ch_display[64];
            snprintf(ch_display, sizeof(ch_display), "#  %s", ch->name);
            draw_text(renderer, font_main, ch_display, layout->sidebar_channels.x + 15, channel_y, current_color);
        }

        channel_y += 32;
    }
}
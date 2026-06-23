#include "../include/ui_chat_components.h"
#include "../include/ui_chat_store.h"

#define COLOR_BG_SERVERS   0x0D, 0x0E, 0x12, 0xFF 

SDL_Rect btn_add_channel;
SDL_Rect modal_bg_rect;
SDL_Rect modal_input_rect;
SDL_Rect modal_toggle_rect;
SDL_Rect modal_btn_confirm;
SDL_Rect modal_btn_cancel;

void components_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
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

void components_draw_cursor(SDL_Renderer *renderer, TTF_Font *font, const char *text, int base_x, int base_y, int height, SDL_Color color) {
    int text_w = 0;
    if (font && text && text[0] != '\0') {
        TTF_SizeUTF8(font, text, &text_w, NULL);
    }
    if ((SDL_GetTicks() / 500) % 2 == 0) {
        SDL_Rect cursor_rect = { base_x + text_w, base_y, 2, height };
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &cursor_rect);
    }
}

void components_draw_modal(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray) {
    SDL_Rect overlay = {0, 0, layout->window_w, layout->window_h};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_RenderFillRect(renderer, &overlay);

    SDL_SetRenderDrawColor(renderer, 0x16, 0x19, 0x24, 0xFF);
    SDL_RenderFillRect(renderer, &modal_bg_rect);
    SDL_SetRenderDrawColor(renderer, 0x0D, 0x0E, 0x12, 0xFF);
    SDL_RenderDrawRect(renderer, &modal_bg_rect);

    components_draw_text(renderer, font_title, "Créer un salon", modal_bg_rect.x + 30, modal_bg_rect.y + 22, white_color);
    components_draw_text(renderer, font_sub, "NOM DU SALON", modal_input_rect.x, modal_input_rect.y - 20, gray_color);

    SDL_SetRenderDrawColor(renderer, COLOR_BG_SERVERS);
    SDL_RenderFillRect(renderer, &modal_input_rect);
    if (layout->modal_focused_field == 1) {
        SDL_SetRenderDrawColor(renderer, 88, 101, 242, 255);
        SDL_RenderDrawRect(renderer, &modal_input_rect);
    }

    if (strlen(layout->modal_name_buffer) == 0) {
        components_draw_text(renderer, font_main, "nouveau-salon", modal_input_rect.x + 12, modal_input_rect.y + 8, dark_gray);
    } else {
        components_draw_text(renderer, font_main, layout->modal_name_buffer, modal_input_rect.x + 12, modal_input_rect.y + 8, white_color);
    }

    components_draw_text(renderer, font_sub, "SALON PRIVÉ (🔒)", modal_toggle_rect.x, modal_toggle_rect.y - 20, gray_color);
    if (layout->modal_is_private) {
        SDL_SetRenderDrawColor(renderer, 35, 165, 90, 255);
        SDL_RenderFillRect(renderer, &modal_toggle_rect);
        SDL_Rect switch_pill = {modal_toggle_rect.x + 28, modal_toggle_rect.y + 3, 18, 18};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &switch_pill);
    } else {
        SDL_SetRenderDrawColor(renderer, 128, 132, 142, 255);
        SDL_RenderFillRect(renderer, &modal_toggle_rect);
        SDL_Rect switch_pill = {modal_toggle_rect.x + 4, modal_toggle_rect.y + 3, 18, 18};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &switch_pill);
    }

    int h_cancel = (mx >= modal_btn_cancel.x && mx <= modal_btn_cancel.x + modal_btn_cancel.w && my >= modal_btn_cancel.y && my <= modal_btn_cancel.y + modal_btn_cancel.h);
    components_draw_text(renderer, font_main, "Annuler", modal_btn_cancel.x + 18, modal_btn_cancel.y + 8, h_cancel ? white_color : gray_color);

    int h_confirm = (mx >= modal_btn_confirm.x && mx <= modal_btn_confirm.x + modal_btn_confirm.w && my >= modal_btn_confirm.y && my <= modal_btn_confirm.y + modal_btn_confirm.h);
    SDL_SetRenderDrawColor(renderer, h_confirm ? 0x47 : 0x58, h_confirm ? 0x52 : 0x65, h_confirm ? 0xC4 : 0xF2, 255);
    SDL_RenderFillRect(renderer, &modal_btn_confirm);
    components_draw_text(renderer, font_main, "Créer", modal_btn_confirm.x + 28, modal_btn_confirm.y + 8, white_color);
}
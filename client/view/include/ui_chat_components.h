#ifndef UI_CHAT_COMPONENTS_H
#define UI_CHAT_COMPONENTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "ui_chat.h"

extern SDL_Rect btn_add_channel;
extern SDL_Rect btn_delete_channel;
extern SDL_Rect modal_bg_rect;
extern SDL_Rect modal_input_rect;
extern SDL_Rect modal_toggle_rect;
extern SDL_Rect modal_btn_confirm;
extern SDL_Rect modal_btn_cancel;

void components_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);
void components_draw_cursor(SDL_Renderer *renderer, TTF_Font *font, const char *text, int base_x, int base_y, int height, SDL_Color color);
void components_draw_modal(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, int mx, int my, SDL_Color white_color, SDL_Color gray_color, SDL_Color dark_gray);

#endif /* UI_CHAT_COMPONENTS_H */
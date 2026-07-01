#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef enum {
    TAB_LOGIN,
    TAB_REGISTER
} AuthTab;

typedef enum {
    FOCUS_NONE,
    FOCUS_EMAIL,
    FOCUS_USERNAME,
    FOCUS_PASSWORD,
    FOCUS_CONFIRM
} FocusField;

typedef struct {
    AuthTab current_tab;
    FocusField current_focus;
    char text_email[128];
    char text_username[128];
    char text_password[128];
    char text_confirm[128];
} UIState;

// Global text rendering helper shared across all active views
void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);
int get_text_width(TTF_Font *font, const char *text);

// Word-wraps text to wrap_width pixels. measure_text_wrapped_height returns
// the rendered block height without drawing (0 if empty); draw_text_wrapped
// draws it and also returns that height.
int measure_text_wrapped_height(TTF_Font *font, const char *text, Uint32 wrap_width);
int draw_text_wrapped(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y,
                      SDL_Color color, Uint32 wrap_width);

// Renders the Login / Register multi-tab card frame
void draw_login_interface(SDL_Renderer *renderer, SDL_Rect card_rect, UIState *state,
                          TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_label,
                          int is_hovering_button);

#endif /* UI_LOGIN_H */
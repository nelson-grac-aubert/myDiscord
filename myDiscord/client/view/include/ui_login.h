#ifndef LOGIN_H
#define LOGIN_H

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
    FOCUS_CONFIRM      // <-- Ajout du champ de confirmation
} ActiveFocus;

typedef struct {
    AuthTab current_tab;
    ActiveFocus current_focus;
    char text_email[128];
    char text_username[128];
    char text_password[128];
    char text_confirm[128];   // <-- Ajout du texte de confirmation
} UIState;

void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);
int get_text_width(TTF_Font *font, const char *text);

void draw_login_interface(SDL_Renderer *renderer, SDL_Rect card_rect, UIState *state,
                          TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_label,
                          int is_hovering_button);

#endif /* LOGIN_H */
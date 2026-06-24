#ifndef CHANNEL_CONTROLLER_H
#define CHANNEL_CONTROLLER_H

#include <SDL2/SDL.h>
#include "ui_chat.h"

// Gère les clics gauches liés aux salons et à la modale
void channel_control_left_click(ChatLayout *layout, int cx, int cy);

// Gère le clic droit sur la barre des salons (menu contextuel)
void channel_control_right_click(ChatLayout *layout, int cx, int cy);

// Gère les touches claviers (ex: Backspace pour effacer le texte du salon)
void channel_control_keydown(ChatLayout *layout, SDL_Keycode key);

// Gère l'ajout de caractères dans le nom du salon
void channel_control_textinput(ChatLayout *layout, const char *text);

#endif /* CHANNEL_CONTROLLER_H */
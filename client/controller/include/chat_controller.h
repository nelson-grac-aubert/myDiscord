#ifndef CHAT_CONTROLLER_H
#define CHAT_CONTROLLER_H

#include <SDL2/SDL.h>
#include "ui_chat.h"

// Gère tous les clics gauches (salons, modale, barre d'envoi, bouton déconnexion)
// Retourne 0 par défaut, ou 2 si l'utilisateur a cliqué sur "Déconnexion"
int chat_control_left_click(ChatLayout *layout, int cx, int cy);

// Gère tous les clics droits (ouvre le menu contextuel sur un salon ou un message)
void chat_control_right_click(ChatLayout *layout, int cx, int cy);

// Gère le clic gauche à l'intérieur d'un menu contextuel déjà ouvert
void chat_control_menu_click(ChatLayout *layout, int cx, int cy);

// Gère les touches spéciales comme Backspace ou Entrée
void chat_control_keydown(ChatLayout *layout, SDL_Keycode key);

// Gère la saisie de texte caractère par caractère (dans l'input bar ou la modale)
void chat_control_textinput(ChatLayout *layout, const char *text);

#endif /* CHAT_CONTROLLER_H */
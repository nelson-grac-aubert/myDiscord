#ifndef CHAT_CONTROLLER_H
#define CHAT_CONTROLLER_H

#include "ui_chat.h"
#include <SDL2/SDL.h>

// Initialise l'état initial et charge les textures PNG avec le renderer
void chat_controller_init(ChatLayout *layout, SDL_Renderer *renderer);

// Libère proprement les textures de boutons chargées en mémoire vidéo
void chat_controller_destroy(ChatLayout *layout);

// Met à jour les états de survol (hover) à chaque mouvement de la souris
void chat_controller_update_hover(ChatLayout *layout, int mx, int my);

// Gère les clics gauches et retourne 2 si on demande une déconnexion globale
int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy);

// Gère les clics droits pour ouvrir les menus contextuels (salons, membres, messages)
void chat_controller_handle_right_click(ChatLayout *layout, int cx, int cy);

// Gère les actions sur les éléments du menu contextuel (suppression, mention)
void chat_controller_handle_menu_action(ChatLayout *layout, int cx, int cy);

// Gère les entrées clavier (Backspace, Entrée pour envoyer)
void chat_controller_handle_keydown(ChatLayout *layout, SDL_Keycode sym);

// Gère l'ajout de texte caractère par caractère dans les buffers actifs
void chat_controller_handle_textinput(ChatLayout *layout, const char *text);

// Getters pour les états du contrôleur (ex: microphone)
int chat_controller_is_mic_muted(void);

#endif /* CHAT_CONTROLLER_H */
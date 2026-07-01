#ifndef CHAT_CONTROLLER_H
#define CHAT_CONTROLLER_H

#include "ui_chat.h"
#include <SDL2/SDL.h>

// Initializes the controller state and loads PNG textures using the renderer
void chat_controller_init(ChatLayout *layout, SDL_Renderer *renderer);

// Properly frees loaded button textures from video memory
void chat_controller_destroy(ChatLayout *layout);

// Updates hover states on every mouse movement
void chat_controller_update_hover(ChatLayout *layout, int mx, int my);

// Handles left clicks and returns 2 if a global logout is requested
int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy);

// Handles right clicks to open context menus (channels, members, messages)
void chat_controller_handle_right_click(ChatLayout *layout, int cx, int cy);

// Handles actions on context menu items (deletion, mention)
void chat_controller_handle_menu_action(ChatLayout *layout, int cx, int cy);

// Handles keyboard inputs (Backspace, Enter to send)
void chat_controller_handle_keydown(ChatLayout *layout, SDL_Keycode sym);

// Handles character-by-character text input into active buffers
void chat_controller_handle_textinput(ChatLayout *layout, const char *text);

// Handles mouse wheel scrolling of the message list
void chat_controller_handle_mousewheel(ChatLayout *layout, int wheel_y);

// Getters for controller states (e.g., microphone)
int chat_controller_is_mic_muted(void);

#endif /* CHAT_CONTROLLER_H */
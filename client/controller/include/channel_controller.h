#ifndef CHANNEL_CONTROLLER_H
#define CHANNEL_CONTROLLER_H

#include <SDL2/SDL.h>
#include "ui_chat.h"

// Handles left clicks related to channels and the channel creation modal
void channel_controller_handle_left_click(ChatLayout *layout, int cx, int cy);

// Handles right clicks on the channels sidebar (context menu)
void channel_controller_handle_right_click(ChatLayout *layout, int cx, int cy);

// Handles keyboard inputs (e.g., Backspace to erase channel name text)
void channel_controller_handle_keydown(ChatLayout *layout, SDL_Keycode key);

// Handles character-by-character text input for the channel name
void channel_controller_handle_textinput(ChatLayout *layout, const char *text);

#endif /* CHANNEL_CONTROLLER_H */
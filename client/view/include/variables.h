#ifndef VARIABLES_H
#define VARIABLES_H

#include <SDL2/SDL.h>

// --- FIXED DIMENSIONS & CONFIGURATIONS ---
#define SIDEBAR_CHANNELS_Y 55
#define CHANNEL_ITEM_HEIGHT 32
#define CHAT_MESSAGES_START_Y 70
#define MESSAGE_ITEM_HEIGHT 30
#define ICON_BUTTON_SIZE 28

// --- THEME COLOR DECLARATIONS ---
extern const SDL_Color VAR_COLOR_BG_SERVERS;
extern const SDL_Color VAR_COLOR_BG_CHANNELS;
extern const SDL_Color VAR_COLOR_BG_CHAT;
extern const SDL_Color VAR_COLOR_BG_MEMBERS;
extern const SDL_Color VAR_COLOR_BG_INPUT;
extern const SDL_Color VAR_COLOR_MENU_BG;
extern const SDL_Color VAR_COLOR_DISCORD_BLUE;
extern const SDL_Color VAR_COLOR_TEXT_WHITE;
extern const SDL_Color VAR_COLOR_TEXT_MUTED;

// --- HOVER COLORS AND OPACITIES ---
extern const SDL_Color VAR_COLOR_HOVER_LIGHT; // Button hover (transparent white)
extern const SDL_Color VAR_COLOR_HOVER_RED;   // Deletion hover (bright red)
extern const SDL_Color VAR_COLOR_HOVER_ITEM;  // Channel/message row hover (light gray)

// --- REAL-TIME HOVER TRACKING STRUCTURE ---
typedef struct {
    int hover_channel_index;  // Index of the hovered channel (-1 if none)
    int hover_message_index;  // Index of the hovered message (-1 if none)
    int hover_mic_button;     // 1 if mouse is over the microphone icon
    int hover_file_button;    // 1 if mouse is over the file upload icon
    int hover_menu_action;    // 1 if mouse is over an open context menu item
} ChatHoverState;

#endif /* VARIABLES_H */
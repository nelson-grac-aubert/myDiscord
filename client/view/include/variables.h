#ifndef VARIABLES_H
#define VARIABLES_H
#include <SDL2/SDL_ttf.h>
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
extern const SDL_Color VAR_COLOR_BG_OBSIDIAN;

// --- HOVER COLORS AND OPACITIES ---
extern const SDL_Color VAR_COLOR_HOVER_LIGHT; 
extern const SDL_Color VAR_COLOR_HOVER_RED;   
extern const SDL_Color VAR_COLOR_HOVER_ITEM;  

// --- FONT DECLARATIONS ---
extern TTF_Font *font_title;
extern TTF_Font *font_main;
extern TTF_Font *font_sub;
extern TTF_Font *font_label;
extern TTF_Font *font_emoji;

// --- AUDIO DECLARATIONS ---
extern SDL_AudioDeviceID mic_device;
extern int g_is_mic_muted;             // <- Ajout pour lier la variable d'état du micro
int init_hardware_microphone();        // <- Uniformisation du nom ici

// --- REAL-TIME HOVER TRACKING STRUCTURE ---
typedef struct {
    int hover_channel_index;  
    int hover_message_index;  
    int hover_mic_button;     
    int hover_file_button;    
    int hover_menu_action;    
} ChatHoverState;

#endif /* VARIABLES_H */
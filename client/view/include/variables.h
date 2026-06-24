#ifndef VARIABLES_H
#define VARIABLES_H

#include <SDL2/SDL.h>

// --- DIMENSIONS & CONFIGURATIONS FIXES ---
#define SIDEBAR_CHANNELS_Y 55
#define CHANNEL_ITEM_HEIGHT 32
#define CHAT_MESSAGES_START_Y 70
#define MESSAGE_ITEM_HEIGHT 30
#define ICON_BUTTON_SIZE 28

// --- DECLARATION DES COULEURS DU THEME ---
extern const SDL_Color VAR_COLOR_BG_SERVERS;
extern const SDL_Color VAR_COLOR_BG_CHANNELS;
extern const SDL_Color VAR_COLOR_BG_CHAT;
extern const SDL_Color VAR_COLOR_BG_MEMBERS;
extern const SDL_Color VAR_COLOR_BG_INPUT;
extern const SDL_Color VAR_COLOR_MENU_BG;
extern const SDL_Color VAR_COLOR_DISCORD_BLUE;
extern const SDL_Color VAR_COLOR_TEXT_WHITE;
extern const SDL_Color VAR_COLOR_TEXT_MUTED;

// --- COULEURS ET OPACITES DES SURVOLS (HOVER) ---
extern const SDL_Color VAR_COLOR_HOVER_LIGHT; // Survol bouton (blanc transparent)
extern const SDL_Color VAR_COLOR_HOVER_RED;   // Survol suppression (rouge vif)
extern const SDL_Color VAR_COLOR_HOVER_ITEM;  // Survol ligne salon/message (gris léger)

// --- STRUCTURE DE SUIVI DES HOVERS EN TEMPS RÉEL ---
typedef struct {
    int hover_channel_index;    // Index du salon survolé (-1 si aucun)
    int hover_message_index;    // Index du message survolé (-1 si aucun)
    int hover_mic_button;       // 1 si bouton micro survolé, 0 sinon
    int hover_file_button;      // 1 si bouton transfert survolé, 0 sinon
    int hover_menu_action;      // 1 si option du menu contextuel survolée, 0 sinon
} ChatHoverState;

#endif /* VARIABLES_H */
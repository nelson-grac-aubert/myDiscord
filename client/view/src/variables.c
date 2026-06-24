#include "variables.h"
#include <SDL2/SDL_ttf.h>

// --- THEME COLOR DECLARATIONS ---
const SDL_Color VAR_COLOR_BG_SERVERS   = {0x1E, 0x1F, 0x22, 0xFF};
const SDL_Color VAR_COLOR_BG_CHANNELS  = {0x2B, 0x2D, 0x31, 0xFF};
const SDL_Color VAR_COLOR_BG_CHAT      = {0x31, 0x33, 0x38, 0xFF};
const SDL_Color VAR_COLOR_BG_MEMBERS   = {0x2B, 0x2D, 0x31, 0xFF};
const SDL_Color VAR_COLOR_BG_INPUT     = {0x38, 0x3A, 0x40, 0xFF};
const SDL_Color VAR_COLOR_MENU_BG      = {0x11, 0x12, 0x14, 0xFF};
const SDL_Color VAR_COLOR_DISCORD_BLUE = {0x58, 0x65, 0xF2, 0xFF};
const SDL_Color VAR_COLOR_TEXT_WHITE   = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color VAR_COLOR_TEXT_MUTED   = {0x94, 0x9B, 0xA4, 0xFF};
const SDL_Color VAR_COLOR_BG_OBSIDIAN  = {0x1F, 0x20, 0x23, 0xFF};

// Hover parameters
const SDL_Color VAR_COLOR_HOVER_LIGHT  = {0xFF, 0xFF, 0xFF, 0x22}; // Semi-transparent white overlay
const SDL_Color VAR_COLOR_HOVER_RED    = {0xF2, 0x3F, 0x43, 0xFF}; // Discord red
const SDL_Color VAR_COLOR_HOVER_ITEM   = {0x35, 0x37, 0x3C, 0xFF}; // Light gray channel hover highlight

// Vos initialisations existantes...
TTF_Font *font_title = NULL;
TTF_Font *font_main  = NULL;
TTF_Font *font_sub   = NULL;
TTF_Font *font_label = NULL;

// FIX : Initialisation globale
TTF_Font *font_emoji = NULL;
#include "variables.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

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
const SDL_Color VAR_COLOR_HOVER_LIGHT  = {0xFF, 0xFF, 0xFF, 0x22}; 
const SDL_Color VAR_COLOR_HOVER_RED    = {0xF2, 0x3F, 0x43, 0xFF}; 
const SDL_Color VAR_COLOR_HOVER_ITEM   = {0x35, 0x37, 0x3C, 0xFF}; 

// Polices
TTF_Font *font_title = NULL;
TTF_Font *font_main  = NULL;
TTF_Font *font_sub   = NULL;
TTF_Font *font_label = NULL;
TTF_Font *font_emoji = NULL;

// --- AUDIO DECLARATIONS ---
SDL_AudioDeviceID mic_device = 0;
int g_is_mic_muted = 0; // Défini par défaut à faux (micro actif au lancement)

int init_hardware_microphone() { // <- Changement du nom pour correspondre au .h
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    
    desired.freq = 44100;          
    desired.format = AUDIO_S16SYS; 
    desired.channels = 1;          
    desired.samples = 2048;        
    desired.callback = NULL;       

    // Ouverture du périphérique (1 = capture)
    mic_device = SDL_OpenAudioDevice(NULL, 1, &desired, &obtained, 0);

    if (mic_device == 0) {
        printf("⚠️ Impossible d'ouvrir le micro matériel : %s\n", SDL_GetError());
        return 0; 
    }

    // Gestion de la pause matérielle initiale
    if (!g_is_mic_muted) {
        SDL_PauseAudioDevice(mic_device, 0); 
    } else {
        SDL_PauseAudioDevice(mic_device, 1); 
    }

    printf("🎙️ Micro matériel initialisé avec succès ! ID : %d\n", mic_device);
    return 1; 
}
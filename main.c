#include "ui_welcome.h"
#include "variables.h" // Inclus pour s'assurer que les prototypes audio sont connus
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // 1. Initialisation unique de SDL2 (Vidéo + Audio) et SDL_ttf
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { // <- FIX : Ajout de SDL_INIT_AUDIO
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialisation du micro matériel
    init_hardware_microphone(); 

    // 2. On délègue tout au gestionnaire d'état de l'application
    int status = welcome_ui_init_and_run();

    // 3. Quitter proprement et fermer le périphérique audio s'il a été ouvert
    if (mic_device > 0) {
        SDL_CloseAudioDevice(mic_device);
    }

    TTF_Quit();
    SDL_Quit();
    
    return status;
}
#include "ui_welcome.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // 1. Initialisation unique de SDL2 et SDL_ttf
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // 2. On délègue tout au gestionnaire d'état de l'application
    // Cette fonction gère le login, le chat, la déconnexion et le nettoyage interne.
    int status = welcome_ui_init_and_run();

    // 3. Quitter proprement une fois que welcome_ui_init_and_run se termine
    TTF_Quit();
    SDL_Quit();
    
    return status;
}
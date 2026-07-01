#include "ui_welcome.h"
#include "variables.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "[SDL] init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "[TTF] init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    init_hardware_microphone();

    int status = welcome_ui_init_and_run();

    if (mic_device > 0)
        SDL_CloseAudioDevice(mic_device);

    TTF_Quit();
    SDL_Quit();

    return status;
}
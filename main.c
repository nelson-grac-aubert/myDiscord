#include "client/view/include/ui_welcome.h"
#include "client/view/include/ui_chat.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // Global init
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }
    if (TTF_Init() == -1) {
        SDL_Quit();
        return 1;
    }

    // Start GUI, return 1 if connection successful, 0 when closed with window red cross
    int user_logged_in = welcome_ui_init_and_run();

    // If user is successfully logged in, go to chat windows
    if (user_logged_in) {
        chat_ui_init_and_run();
    }

    // Close SDL and TTF properly on program termination
    TTF_Quit();
    SDL_Quit();

    return 0;
}
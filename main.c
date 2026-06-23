#include "client/view/include/ui_welcome.h"
#include "client/view/include/ui_chat.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;
    if (TTF_Init() == -1) {
        SDL_Quit();
        return 1;
    }

    int app_running = 1;

    while (app_running) {
        int user_logged_in = welcome_ui_init_and_run();

        if (!user_logged_in) {
            app_running = 0;
            break;
        }

        int chat_status = chat_ui_init_and_run();

        if (chat_status == 2)
            printf("[MAIN] Returning to login screen.\n");
        else
            app_running = 0;
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}
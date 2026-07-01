#include "ui_welcome.h"
#include "variables.h"
#include "data/include/db_init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

PGconn *g_db_conn         = NULL;
int     g_current_user_id = 1;

int main(int argc, char **argv)
{
    const char *host   = argc > 1 ? argv[1] : "localhost";
    const char *port   = argc > 2 ? argv[2] : "5432";
    const char *dbname = argc > 3 ? argv[3] : "mydiscord";
    const char *user   = argc > 4 ? argv[4] : "postgres";
    const char *pass   = argc > 5 ? argv[5] : "";

    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo),
             "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, dbname, user, pass);

    g_db_conn = PQconnectdb(conninfo);
    if (PQstatus(g_db_conn) != CONNECTION_OK) {
        fprintf(stderr, "[DB] connection failed: %s\n", PQerrorMessage(g_db_conn));
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }
    printf("[DB] connected to %s\n", dbname);

    if (initializeData(g_db_conn) != 0) {
        fprintf(stderr, "[DB] initializeData failed\n");
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "[SDL] init failed: %s\n", SDL_GetError());
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "[TTF] init failed: %s\n", TTF_GetError());
        SDL_Quit();
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }

    init_hardware_microphone();

    /* Socket connection is handled by auth_controller_connect in ui_welcome */
    int status = welcome_ui_init_and_run();

    if (mic_device > 0)
        SDL_CloseAudioDevice(mic_device);

    TTF_Quit();
    SDL_Quit();
    PQfinish(g_db_conn);
    g_db_conn = NULL;

    return status;
}
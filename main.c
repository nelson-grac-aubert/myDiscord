#include "ui_welcome.h"
#include "variables.h"
#include "client_socket.h"
#include "data/include/db_init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Variables globales ─────────────────────────────────────────────── */
PGconn       *g_db_conn         = NULL;
int           g_current_user_id = 1;
ClientSocket  g_client_socket;        /* ← utilisé par chat_controller  */

int main(int argc, char **argv)
{
    /* ── 1. Connexion PostgreSQL ─────────────────────────────────────── */
    const char *host   = argc > 1 ? argv[1] : "localhost";
    const char *port   = argc > 2 ? argv[2] : "5432";
    const char *dbname = argc > 3 ? argv[3] : "mydiscord";
    const char *user   = argc > 4 ? argv[4] : "postgres";
    const char *pass   = argc > 5 ? argv[5] : "";

    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo),
             "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, dbname, user, pass);

    printf("[DB] Connexion : %s\n", conninfo);

    g_db_conn = PQconnectdb(conninfo);
    if (PQstatus(g_db_conn) != CONNECTION_OK) {
        fprintf(stderr, "[ERREUR] Connexion DB echouee : %s\n",
                PQerrorMessage(g_db_conn));
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }
    printf("[DB] Connecte a %s\n\n", dbname);

    /* ── 2. Initialisation schema + données ─────────────────────────── */
    if (initializeData(g_db_conn) != 0) {
        fprintf(stderr, "[ERREUR] initializeData echouee\n");
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }

    /* ── 3. Connexion socket au serveur ──────────────────────────────── */
    if (client_socket_connect(&g_client_socket, "127.0.0.1", 8080, NULL) != 0) {
        fprintf(stderr, "[ERREUR] Connexion serveur echouee\n");
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }
    client_socket_start_listener(&g_client_socket);

    /* ── 4. Initialisation SDL2 ──────────────────────────────────────── */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "[ERREUR] SDL_Init : %s\n", SDL_GetError());
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "[ERREUR] TTF_Init : %s\n", TTF_GetError());
        SDL_Quit();
        PQfinish(g_db_conn);
        return EXIT_FAILURE;
    }

    /* ── 5. Micro ────────────────────────────────────────────────────── */
    init_hardware_microphone();

    /* ── 6. Boucle principale ────────────────────────────────────────── */
    int status = welcome_ui_init_and_run();

    /* ── 7. Nettoyage ────────────────────────────────────────────────── */
    if (mic_device > 0)
        SDL_CloseAudioDevice(mic_device);

    TTF_Quit();
    SDL_Quit();

    client_socket_disconnect(&g_client_socket);

    PQfinish(g_db_conn);
    g_db_conn = NULL;

    return status;
}
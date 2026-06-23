#include "client/view/include/ui_welcome.h"
#include "client/view/include/ui_chat.h" 
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    // 1. Initialisation globale (Une seule fois au démarrage de l'application)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }
    if (TTF_Init() == -1) {
        SDL_Quit();
        return 1;
    }

    int app_running = 1;

    // ➡️ Machine à états : Tant que app_running est vrai, on peut naviguer entre les écrans
    while (app_running) {
        
        // 2. Lance l'interface d'accueil/login
        int user_logged_in = welcome_ui_init_and_run();

        // Si l'utilisateur a fermé la fenêtre d'accueil avec la croix
        if (!user_logged_in) {
            app_running = 0; // On casse la boucle pour quitter l'application
            break;
        }

        // 3. L'utilisateur s'est connecté -> On bascule sur la fenêtre de Chat !
        int chat_status = chat_ui_init_and_run();

        if (chat_status == 2) {
            // L'utilisateur a cliqué sur Déconnexion !
            // On affiche un log et on laisse app_running à 1. La boucle while recommence,
            // ce qui va ré-ouvrir automatiquement l'écran welcome_ui_init_and_run().
            printf("[MAIN] Redirection vers l'écran de login...\n");
        } else {
            // L'utilisateur a fermé le chat avec la croix (chat_status == 0)
            app_running = 0; // On quitte définitivement
        }
    }

    // 4. Fermeture propre et définitive de la SDL et TTF à la toute fin du programme
    TTF_Quit();
    SDL_Quit();

    return 0;
}
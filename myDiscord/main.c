#include "client/view/include/ui_welcome.h"
#include "client/view/include/ui_chat.h" // ➡️ AJOUT : Pour pouvoir appeler l'interface du chat
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

    // 2. Lance l'interface d'accueil et récupère le résultat de la connexion
    // welcome_ui_init_and_run renvoie désormais 1 si connecté, 0 si fermé avec la croix
    int user_logged_in = welcome_ui_init_and_run();

    // 3. Si l'utilisateur s'est connecté avec succès, on bascule sur la fenêtre de Chat !
    if (user_logged_in) {
        chat_ui_init_and_run();
    }

    // 4. Fermeture propre et définitive de la SDL et TTF à la toute fin du programme
    TTF_Quit();
    SDL_Quit();

    return 0;
}
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> // Inclure le header pour le texte
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // 1. Initialiser SDL2 et SDL_ttf
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // 2. Créer la fenêtre et le Renderer
    SDL_Window* window = SDL_CreateWindow("SDL2 Texte Centre", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 3. Charger la police (Taille 48)
    TTF_Font* font = TTF_OpenFont("Urbanist-Regular.ttf", 48);
    if (!font) {
        printf("Erreur chargement police: %s\n", TTF_GetError());
        // Si erreur, on nettoie et on quitte
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 4. Créer la surface et la texture de texte
    SDL_Color whiteColor = {255, 255, 255, 255}; // Couleur du texte
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, "Hello World!", whiteColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Déterminer la taille du texte généré
    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface); // On n'a plus besoin de la surface, la texture suffit

    // 5. Calculer la position pour centrer le texte
    SDL_Rect textRect;
    textRect.x = (SCREEN_WIDTH - textWidth) / 2;
    textRect.y = (SCREEN_HEIGHT - textHeight) / 2;
    textRect.w = textWidth;
    textRect.h = textHeight;

    // 6. Boucle principale
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Effacer l'écran en bleu
        SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
        SDL_RenderClear(renderer);

        // Dessiner le texte à sa position centrée
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Afficher le rendu
        SDL_RenderPresent(renderer);
    }

    // 7. Nettoyage
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
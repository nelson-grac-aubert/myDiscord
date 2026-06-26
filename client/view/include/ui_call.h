#ifndef UI_CALL_H
#define UI_CALL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * @brief Ouvre l'interface de la fenêtre d'appel vocal de la maquette.
 * Intercepte la boucle principale jusqu'à ce que l'utilisateur raccroche.
 */
void ouvrir_fenetre_appel(SDL_Renderer *renderer, 
                          TTF_Font *font_title, 
                          TTF_Font *font_main, 
                          TTF_Font *font_sub, 
                          TTF_Font *font_emoji, 
                          int win_w, 
                          int win_h);

#endif /* UI_CALL_H */
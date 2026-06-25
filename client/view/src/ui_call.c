#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "variables.h"
#include "ui_call.h"
#include <stdio.h>

// Fonction externe pour dessiner du texte (déjà présente dans ton projet)
extern void draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);

void ouvrir_fenetre_appel(SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub, TTF_Font *font_emoji, int win_w, int win_h)
{
    int dans_l_appel = 1;
    SDL_Event e;

    // --- CONFIGURATION DU LAYOUT (PROPORTIONNEL) ---
    int sidebar_w = 240;
    SDL_Rect area_main = {0, 0, win_w - sidebar_w, win_h - 80};
    SDL_Rect area_participants = {win_w - sidebar_w, 0, sidebar_w, win_h};
    SDL_Rect area_controls = {0, win_h - 80, win_w - sidebar_w, 80};

    // Boutons de contrôle (Centrés dans la barre du bas)
    int btn_y = area_controls.y + (area_controls.h - 44) / 2;
    int center_x = area_controls.w / 2;

    SDL_Rect btn_mute = {center_x - 60, btn_y, 44, 44};
    SDL_Rect btn_hangup = {center_x + 16, btn_y, 44, 44};

    // Liste fictive de participants pour la maquette
    const char *participants[] = {"Moi (Propriétaire)", "Jean_Dupont", "Marie_Curie", "Lucas_Dev"};
    int nb_participants = 4;

    // On désactive la saisie de texte du chat pendant l'appel
    SDL_StopTextInput();

    while (dans_l_appel)
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                dans_l_appel = 0;
                // On peut aussi quitter proprement l'application globale si nécessaire
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    // Clic sur RACCROCHER (Ferme cette fenêtre et retourne au chat)
                    if (mx >= btn_hangup.x && mx <= btn_hangup.x + btn_hangup.w &&
                        my >= btn_hangup.y && my <= btn_hangup.y + btn_hangup.h)
                    {
                        dans_l_appel = 0;
                    }

                    // Clic sur MUTE (Active/Coupe le micro matériel)
                    if (mx >= btn_mute.x && mx <= btn_mute.x + btn_mute.w &&
                        my >= btn_mute.y && my <= btn_mute.y + btn_mute.h)
                    {
                        g_is_mic_muted = !g_is_mic_muted;
                        if (mic_device > 0)
                        {
                            SDL_PauseAudioDevice(mic_device, g_is_mic_muted);
                        }
                    }
                }
            }
        }

        // --- RENDU GRAPHIQUE (Couleurs Discord) ---

        // 1. Zone principale (Salon vocal)
        SDL_SetRenderDrawColor(renderer, 0x1E, 0x1F, 0x22, 0xFF); // Sombre profond
        SDL_RenderFillRect(renderer, &area_main);

        if (font_emoji)
        {
            draw_text(renderer, font_emoji, "🔊", center_x - 20, (area_main.h / 2) - 40, VAR_COLOR_TEXT_WHITE);
        }
        draw_text(renderer, font_title, "Salon Vocal Connecté", center_x - 100, (area_main.h / 2) + 10, VAR_COLOR_TEXT_WHITE);
        draw_text(renderer, font_sub, "Qualité audio : 44.1 kHz Standard", center_x - 110, (area_main.h / 2) + 40, VAR_COLOR_TEXT_MUTED);

        // 2. Sidebar Participants (À Droite)
        SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF); // Gris moyen
        SDL_RenderFillRect(renderer, &area_participants);

        draw_text(renderer, font_title, "PARTICIPANTS", area_participants.x + 20, 20, VAR_COLOR_TEXT_MUTED);

        for (int i = 0; i < nb_participants; i++)
        {
            int item_y = 60 + (i * 35);
            // Petit indicateur vert pour montrer qu'ils sont en ligne
            SDL_Rect dot = {area_participants.x + 20, item_y + 6, 8, 8};
            SDL_SetRenderDrawColor(renderer, 0x23, 0xA5, 0x5A, 0xFF); // Vert Discord
            SDL_RenderFillRect(renderer, &dot);

            draw_text(renderer, font_main, participants[i], area_participants.x + 38, item_y, VAR_COLOR_TEXT_WHITE);

            // Si c'est "Moi" et que je suis muté, on affiche un petit micro coupé à côté
            if (i == 0 && g_is_mic_muted && font_emoji)
            {
                draw_text(renderer, font_emoji, "🔇", area_participants.x + sidebar_w - 40, item_y - 2, VAR_COLOR_TEXT_MUTED);
            }
        }

        // 3. Barre de Contrôles (En Bas)
        SDL_SetRenderDrawColor(renderer, 0x11, 0x12, 0x14, 0xFF); // Noir pur
        SDL_RenderFillRect(renderer, &area_controls);

        // --- DESSIN DU BOUTON MUTE ---
        int hover_mute = (mx >= btn_mute.x && mx <= btn_mute.x + btn_mute.w && my >= btn_mute.y && my <= btn_mute.y + btn_mute.h);
        if (g_is_mic_muted)
        {
            SDL_SetRenderDrawColor(renderer, 0xF2, 0x3F, 0x43, 0xFF); // Rouge si coupé
        }
        else if (hover_mute)
        {
            SDL_SetRenderDrawColor(renderer, 0x40, 0x42, 0x49, 0xFF); // Gris clair au survol
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0x31, 0x33, 0x38, 0xFF); // Gris de base
        }
        SDL_RenderFillRect(renderer, &btn_mute);
        if (font_emoji)
        {
            draw_text(renderer, font_emoji, g_is_mic_muted ? "🔇" : "🎤", btn_mute.x + 10, btn_mute.y + 8, VAR_COLOR_TEXT_WHITE);
        }

        // --- DESSIN DU BOUTON RACCROCHER ---
        int hover_hangup = (mx >= btn_hangup.x && mx <= btn_hangup.x + btn_hangup.w && my >= btn_hangup.y && my <= btn_hangup.y + btn_hangup.h);

        // 1. Activer la gestion de la transparence (Blend Mode)
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        if (hover_hangup)
        {
            // Rouge semi-transparent au survol (Alpha à 0x66, soit environ 40% d'opacité)
            SDL_SetRenderDrawColor(renderer, 0xF2, 0x3F, 0x43, 0x66);
            SDL_RenderFillRect(renderer, &btn_hangup);
        }
        else
        {
            // Totalement transparent quand on ne le survole pas (Alpha à 0x00)
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderFillRect(renderer, &btn_hangup);
        }

        // 2. Désactiver le Blend Mode pour le reste des dessins standards
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        if (font_emoji)
        {
            // La croix blanche par-dessus
            draw_text(renderer, font_emoji, "❌", btn_hangup.x + 10, btn_hangup.y + 8, VAR_COLOR_TEXT_WHITE);
        }

        SDL_RenderPresent(renderer);
    }

    // À la sortie de l'appel, on réactive la saisie clavier pour le chat
    SDL_StartTextInput();
}
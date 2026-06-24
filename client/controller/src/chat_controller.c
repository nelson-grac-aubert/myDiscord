#include "chat_controller.h"
#include "variables.h"
#include "channel.h"
#include "message.h"
#include "ui_channels.h"
#include <SDL2/SDL_image.h> // Nécessaire pour charger tes images .png de boutons
#include <string.h>
#include <stdio.h>

void chat_controller_init(ChatLayout *layout, SDL_Renderer *renderer) {
    layout->menu_type = 0;
    layout->show_create_modal = 0;
    layout->input_buffer[0] = '\0';
    layout->modal_name_buffer[0] = '\0';
    layout->is_input_focused = 1; // Focus par défaut sur la barre de message principale
    layout->menu_x = -1;
    layout->menu_y = -1;
    layout->is_mic_muted = 0;
    // Initialisation des états de survol (hover) par défaut
    layout->hover.hover_channel_index = -1;
    layout->hover.hover_message_index = -1;
    layout->hover.hover_mic_button = 0;
    layout->hover.hover_file_button = 0;
    layout->hover.hover_menu_action = 0;
    // Chargement des textures d'images depuis tes dossiers assets
    layout->tex_mic_on = IMG_LoadTexture(renderer, "assets/bouton/microphone_on.png");
    layout->tex_mic_off = IMG_LoadTexture(renderer, "assets/bouton/microphone_off.png");
    layout->tex_file = IMG_LoadTexture(renderer, "assets/bouton/file_transfer.png");
    if (!layout->tex_mic_on || !layout->tex_mic_off || !layout->tex_file) {
        printf("[ATTENTION] Une ou plusieurs icônes introuvables dans assets/bouton/ : %s\n", IMG_GetError());
    }
}

void chat_controller_destroy(ChatLayout *layout) {
    if (layout->tex_mic_on) SDL_DestroyTexture(layout->tex_mic_on);
    if (layout->tex_mic_off) SDL_DestroyTexture(layout->tex_mic_off);
    if (layout->tex_file) SDL_DestroyTexture(layout->tex_file);
}

void chat_controller_update_hover(ChatLayout *layout, int mx, int my) {
    // 1. Survol des boutons multimédias
    layout->hover.hover_file_button = (mx >= layout->btn_file_transfer.x && mx <= layout->btn_file_transfer.x + layout->btn_file_transfer.w &&
                                       my >= layout->btn_file_transfer.y && my <= layout->btn_file_transfer.y + layout->btn_file_transfer.h);
    layout->hover.hover_mic_button = (mx >= layout->btn_microphone.x && mx <= layout->btn_microphone.x + layout->btn_microphone.w &&
                                      my >= layout->btn_microphone.y && my <= layout->btn_microphone.y + layout->btn_microphone.h);
    // 2. Survol de la zone d'action du menu contextuel (clic droit)
    if (layout->menu_type > 0) {
        layout->hover.hover_menu_action = (mx >= layout->menu_rect.x && mx <= layout->menu_rect.x + layout->menu_rect.w &&
                                           my >= layout->menu_rect.y && my <= layout->menu_rect.y + layout->menu_rect.h);
    } else {
        layout->hover.hover_menu_action = 0;
    }
    // 3. Survol de la liste des salons dans la sidebar
    layout->hover.hover_channel_index = -1;
    if (mx >= layout->sidebar_channels.x && mx <= layout->sidebar_channels.x + layout->sidebar_channels.w && my > SIDEBAR_CHANNELS_Y) {
        int ch_y = SIDEBAR_CHANNELS_Y;
        for (int i = 0; i < channel_model_get_count(); i++) {
            if (my >= ch_y && my < ch_y + CHANNEL_ITEM_HEIGHT) {
                layout->hover.hover_channel_index = i;
                break;
            }
            ch_y += CHANNEL_ITEM_HEIGHT;
        }
    }
}

int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy) {
    // 1. Déconnexion via le bouton dans la sidebar channel
    if (cx >= btn_logout.x && cx <= btn_logout.x + btn_logout.w && 
        cy >= btn_logout.y && cy <= btn_logout.y + btn_logout.h) {
        printf("[LOG] Déconnexion demandée.\n");
        channels_clear_textures();
        return 2; 
    }
    // 2. Bouton '+' d'ajout de salon
    if (cx >= btn_add_channel.x && cx <= btn_add_channel.x + btn_add_channel.w && 
        cy >= btn_add_channel.y && cy <= btn_add_channel.y + btn_add_channel.h) {
        layout->show_create_modal = 1;
        layout->modal_name_buffer[0] = '\0';
        layout->modal_is_private = 0;
        return 0;
    }
    // 3. Clic sur le Micro 🎤 (utilise notre état hover calculé)
    if (layout->hover.hover_mic_button) {
        layout->is_mic_muted = !layout->is_mic_muted;
        return 0;
    }
    // 4. Clic sur le Transfert de fichier 📁
    if (layout->hover.hover_file_button) {
        printf("[FILE] Demande de transfert de fichier initiée !\n");
        return 0;
    }
    // 5. Modale de création active
    if (layout->show_create_modal) {
        extern SDL_Rect modal_toggle_rect, modal_btn_cancel, modal_btn_ok;
        if (cx >= modal_toggle_rect.x && cx <= modal_toggle_rect.x + modal_toggle_rect.w &&
            cy >= modal_toggle_rect.y && cy <= modal_toggle_rect.y + modal_toggle_rect.h) {
            layout->modal_is_private = !layout->modal_is_private;
        }
        else if (cx >= modal_btn_cancel.x && cx <= modal_btn_cancel.x + modal_btn_cancel.w &&
                 cy >= modal_btn_cancel.y && cy <= modal_btn_cancel.y + modal_btn_cancel.h) {
            layout->show_create_modal = 0;
        }
        else if (cx >= modal_btn_ok.x && cx <= modal_btn_ok.x + modal_btn_ok.w &&
                 cy >= modal_btn_ok.y && cy <= modal_btn_ok.y + modal_btn_ok.h) {
            if (strlen(layout->modal_name_buffer) > 0) {
                int new_id = channel_model_get_count() + 1;
                channel_model_add(new_id, layout->modal_name_buffer, layout->modal_is_private);
            }
            layout->show_create_modal = 0;
        }
        return 0;
    }
    // 6. Clic dans la zone des salons (Sidebar Gauche) optimisé avec le hover pré-calculé !
    if (layout->hover.hover_channel_index != -1) {
        channel_model_set_active_index(layout->hover.hover_channel_index);
        return 0;
    }
    // 7. Gestion du focus sur la barre de saisie de messages de la zone principale
    if (cx >= layout->chat_input_bar.x && cx <= layout->chat_input_bar.x + layout->chat_input_bar.w &&
        cy >= layout->chat_input_bar.y && cy <= layout->chat_input_bar.y + layout->chat_input_bar.h) {
        layout->is_input_focused = 1;
    } else {
        layout->is_input_focused = 0;
    }
    return 0;
}

void chat_controller_handle_right_click(ChatLayout *layout, int cx, int cy) {
    // Menu contextuel sur un Salon (simplifié grâce à notre tracker hover)
    if (layout->hover.hover_channel_index != -1) {
        layout->menu_type = 1; // 1 = Menu Salon
        layout->menu_x = cx;
        layout->menu_y = cy;
        layout->menu_rect = (SDL_Rect){cx, cy, 180, MESSAGE_ITEM_HEIGHT + 14};
        return;
    }
    // Menu contextuel sur un Message
    else if (cx >= layout->chat_area.x && cx <= layout->chat_area.x + layout->chat_area.w && 
             cy > CHAT_MESSAGES_START_Y && cy < layout->chat_input_bar.y) {
        layout->menu_type = 3; // 3 = Menu Message
        layout->menu_x = cx;
        layout->menu_y = cy;
        layout->menu_rect = (SDL_Rect){cx, cy, 180, MESSAGE_ITEM_HEIGHT + 14};
    }
    // Menu contextuel sur un Membre
    else if (cx >= layout->sidebar_members.x && cy > 50) {
        layout->menu_type = 2; // 2 = Menu Membre
        layout->menu_x = cx;
        layout->menu_y = cy;
        layout->menu_rect = (SDL_Rect){cx, cy, 150, MESSAGE_ITEM_HEIGHT + 14};
    }
}

void chat_controller_handle_menu_action(ChatLayout *layout, int cx, int cy) {
    (void)cx; // Supprime l'avertissement "paramètre inutilisé"
    (void)cy; // Supprime l'avertissement "paramètre inutilisé"
    // On n'agit que si le clic gauche a réellement eu lieu dans la case du menu contextuel survolé
    if (layout->hover.hover_menu_action) 
    {
        if (layout->menu_type == 1 && layout->menu_y > SIDEBAR_CHANNELS_Y) {
            int index = (layout->menu_y - SIDEBAR_CHANNELS_Y) / CHANNEL_ITEM_HEIGHT;
            if (index >= 0 && index < channel_model_get_count()) {
                channel_model_delete_by_index(index);
            }
        } 
        else if (layout->menu_type == 2) {
            snprintf(layout->input_buffer, sizeof(layout->input_buffer), "@Membres ");
        }
        else if (layout->menu_type == 3 && layout->menu_y > CHAT_MESSAGES_START_Y) {
            Channel *active_ch = channel_model_get_active();
            if (active_ch) {
                int index = (layout->menu_y - CHAT_MESSAGES_START_Y) / MESSAGE_ITEM_HEIGHT;
                message_model_delete_by_index_in_channel(active_ch->id, index);
            }
        }
    }
    layout->menu_type = 0; // Ferme le menu dans tous les cas après un clic
}

void chat_controller_handle_keydown(ChatLayout *layout, SDL_Keycode sym) {
    // 1. Touche Backspace (Effacer)
    if (sym == SDLK_BACKSPACE) {
        if (layout->show_create_modal) {
            size_t len = strlen(layout->modal_name_buffer);
            if (len > 0) layout->modal_name_buffer[len - 1] = '\0';
        }
        else if (layout->is_input_focused) {
            size_t len = strlen(layout->input_buffer);
            if (len > 0) layout->input_buffer[len - 1] = '\0';
        }
    }
    // 2. Touche Entrée (Envoyer message)
    else if ((sym == SDLK_RETURN || sym == SDLK_KP_ENTER) && !layout->show_create_modal) {
        if (layout->is_input_focused && strlen(layout->input_buffer) > 0) {
            Channel *active_ch = channel_model_get_active();
            int ch_id = active_ch ? active_ch->id : 1;
            message_model_add(0, ch_id, "Moi (Local)", layout->input_buffer);
            layout->input_buffer[0] = '\0';
        }
    }
}
void chat_controller_handle_textinput(ChatLayout *layout, const char *text) {
    if (layout->show_create_modal) {
        if (strlen(layout->modal_name_buffer) + strlen(text) < 31) {
            strcat(layout->modal_name_buffer, text);
        }
    }
    else if (layout->is_input_focused) {
        if (strlen(layout->input_buffer) + strlen(text) < (MAX_MSG_LENGTH - 1)) {
            strcat(layout->input_buffer, text);
        }
    }
}
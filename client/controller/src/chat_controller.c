#include "../include/chat_controller.h"
#include "../include/ui_chat_store.h"
#include "../include/ui_chat_components.h"
#include "../include/ui_channels.h"
#include <string.h>
#include <stdio.h>

int chat_control_left_click(ChatLayout *layout, int cx, int cy)
{
    // 1. Bouton Déconnexion globale
    if (cx >= btn_logout.x && cx <= btn_logout.x + btn_logout.w &&
        cy >= btn_logout.y && cy <= btn_logout.y + btn_logout.h)
    {
        printf("[LOG] Déconnexion demandée.\n");
        channels_clear_textures();
        return 2; // Statut de déconnexion
    }

    // 2. Gestion de la modale de création si elle est active
    if (layout->show_create_modal)
    {
        if (cx >= modal_input_rect.x && cx <= modal_input_rect.x + modal_input_rect.w &&
            cy >= modal_input_rect.y && cy <= modal_input_rect.y + modal_input_rect.h)
            layout->modal_focused_field = 1;
        else
            layout->modal_focused_field = 0;

        if (cx >= modal_toggle_rect.x && cx <= modal_toggle_rect.x + modal_toggle_rect.w &&
            cy >= modal_toggle_rect.y && cy <= modal_toggle_rect.y + modal_toggle_rect.h)
            layout->modal_is_private = !layout->modal_is_private;
            
        if (cx >= modal_btn_cancel.x && cx <= modal_btn_cancel.x + modal_btn_cancel.w &&
            cy >= modal_btn_cancel.y && cy <= modal_btn_cancel.y + modal_btn_cancel.h)
            layout->show_create_modal = 0;
            
        if (cx >= modal_btn_confirm.x && cx <= modal_btn_confirm.x + modal_btn_confirm.w &&
            cy >= modal_btn_confirm.y && cy <= modal_btn_confirm.y + modal_btn_confirm.h)
        {
            if (strlen(layout->modal_name_buffer) > 0)
                store_add_channel(layout->modal_name_buffer, layout->modal_is_private);
            layout->show_create_modal = 0;
        }
        return 0;
    }

    // 3. Bouton '+' pour ouvrir la modale
    if (cx >= btn_add_channel.x && cx <= btn_add_channel.x + btn_add_channel.w &&
        cy >= btn_add_channel.y && cy <= btn_add_channel.y + btn_add_channel.h)
    {
        layout->show_create_modal = 1;
        layout->modal_is_private = 0;
        layout->modal_focused_field = 1;
        layout->modal_name_buffer[0] = '\0';
        return 0;
    }

    // 4. Focus de la barre d'input de message
    layout->is_input_focused = (cx >= layout->chat_input_bar.x &&
                                cx <= layout->chat_input_bar.x + layout->chat_input_bar.w &&
                                cy >= layout->chat_input_bar.y &&
                                cy <= layout->chat_input_bar.y + layout->chat_input_bar.h);

    // 5. Sélection d'un salon dans la liste
    int ch_y = 55;
    for (int i = 0; i < store_get_channel_count(); i++)
    {
        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, ch_y - 4, layout->sidebar_channels.w - 16, 28};
        if (cx >= item_rect.x && cx <= item_rect.x + item_rect.w &&
            cy >= item_rect.y && cy <= item_rect.y + item_rect.h)
        {
            store_set_active_index(i);
            break;
        }
        ch_y += 32;
    }

    return 0;
}

void chat_control_right_click(ChatLayout *layout, int cx, int cy)
{
    layout->menu_type = 0; // Reset par défaut

    // Clic droit sur la zone des salons (Sidebar)
    int ch_y = 55;
    for (int i = 0; i < store_get_channel_count(); i++)
    {
        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, ch_y - 4, layout->sidebar_channels.w - 16, 28};
        if (cx >= item_rect.x && cx <= item_rect.x + item_rect.w &&
            cy >= item_rect.y && cy <= item_rect.y + item_rect.h)
        {
            layout->menu_type = 1; // 1 = Menu Salon
            layout->target_index = i;
            layout->menu_x = cx;
            layout->menu_y = cy;
            return;
        }
        ch_y += 32;
    }

    // Clic droit sur la zone centrale des messages (Chat Area)
    if (cx >= layout->chat_area.x && cx <= layout->chat_area.x + layout->chat_area.w)
    {
        LocalChannel *ch = store_get_active_channel();
        if (!ch) return;

        int base_y = 70;
        for (int i = 0; i < ch->message_count; i++)
        {
            if (cy >= base_y && cy <= base_y + 45)
            {
                layout->menu_type = 2; // 2 = Menu Message
                layout->target_index = i;
                layout->menu_x = cx;
                layout->menu_y = cy;
                return;
            }
            base_y += 55;
        }
    }
}

void chat_control_menu_click(ChatLayout *layout, int cx, int cy)
{
    if (cx >= layout->menu_rect.x && cx <= layout->menu_rect.x + layout->menu_rect.w &&
        cy >= layout->menu_rect.y && cy <= layout->menu_rect.y + layout->menu_rect.h)
    {
        if (layout->menu_type == 1)
            store_delete_channel_by_index(layout->target_index);
        else if (layout->menu_type == 2)
            store_delete_message_by_index(layout->target_index);
    }
    layout->menu_type = 0; // Ferme le menu après action ou clic à côté
}

void chat_control_keydown(ChatLayout *layout, SDL_Keycode key)
{
    // Effacer du texte dans la modale
    if (layout->show_create_modal && layout->modal_focused_field == 1 && key == SDLK_BACKSPACE)
    {
        size_t len = strlen(layout->modal_name_buffer);
        if (len > 0)
            layout->modal_name_buffer[len - 1] = '\0';
        return;
    }

    // Gestion de l'input bar de messages
    if (!layout->is_input_focused) return;

    if (key == SDLK_BACKSPACE)
    {
        size_t len = strlen(layout->input_buffer);
        if (len > 0)
            layout->input_buffer[len - 1] = '\0';
    }
    else if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
    {
        if (strlen(layout->input_buffer) > 0)
        {
            store_add_message_to_active("Moi (Local)", layout->input_buffer);
            layout->input_buffer[0] = '\0';
        }
    }
}

void chat_control_textinput(ChatLayout *layout, const char *text)
{
    // Écriture dans la modale
    if (layout->show_create_modal && layout->modal_focused_field == 1)
    {
        if (strlen(layout->modal_name_buffer) + strlen(text) < 31)
            strcat(layout->modal_name_buffer, text);
        return;
    }

    // Écriture dans la zone de chat principale
    if (layout->is_input_focused &&
        strlen(layout->input_buffer) + strlen(text) < MAX_MSG_LENGTH - 1)
    {
        strcat(layout->input_buffer, text);
    }
}
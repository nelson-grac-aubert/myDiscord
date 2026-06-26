#include "chat_controller.h"
#include "channel.h"
#include "message.h"
#include "ui_call.h"
#include "ui_channels.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

static SDL_Renderer *current_renderer = NULL;

extern SDL_Rect modal_bg_rect, modal_input_rect, modal_toggle_rect, modal_btn_ok, modal_btn_cancel;

static char* open_file_explorer(void);

void chat_controller_init(ChatLayout *layout, SDL_Renderer *renderer)
{
    current_renderer = renderer;
    layout->menu_type = 0;
    layout->show_create_modal = 0;
    layout->input_buffer[0] = '\0';
    layout->modal_buffer[0] = '\0';
    layout->modal_name_buffer[0] = '\0';
    layout->is_mic_muted = 0;
    layout->modal_is_private = 0;
    layout->modal_focused_field = 0;
    layout->hover_channel_delete_index = -1;
    layout->hover_message_delete_index = -1;
    layout->show_context_menu = 0;
    g_is_mic_muted = 0;
}

void chat_controller_destroy(ChatLayout *layout)
{
    (void)layout;
}

void chat_controller_update_hover(ChatLayout *layout, int mx, int my)
{
    (void)layout;
    (void)mx;
    (void)my;
}

int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy)
{
    // 1. MODALE DE CRÉATION DE SALON
    if (layout->show_create_modal)
    {
        if (cx >= modal_input_rect.x && cx <= modal_input_rect.x + modal_input_rect.w &&
            cy >= modal_input_rect.y && cy <= modal_input_rect.y + modal_input_rect.h)
        {
            layout->modal_focused_field = 1;
            return 0;
        }
        else
        {
            layout->modal_focused_field = 0;
        }

        if (cx >= modal_toggle_rect.x && cx <= modal_toggle_rect.x + modal_toggle_rect.w &&
            cy >= modal_toggle_rect.y && cy <= modal_toggle_rect.y + modal_toggle_rect.h)
        {
            layout->modal_is_private = !layout->modal_is_private;
            return 0;
        }

        if (cx >= modal_btn_cancel.x && cx <= modal_btn_cancel.x + modal_btn_cancel.w &&
            cy >= modal_btn_cancel.y && cy <= modal_btn_cancel.y + modal_btn_cancel.h)
        {
            layout->show_create_modal = 0;
            layout->modal_buffer[0] = '\0';
            return 0;
        }

        if (cx >= modal_btn_ok.x && cx <= modal_btn_ok.x + modal_btn_ok.w &&
            cy >= modal_btn_ok.y && cy <= modal_btn_ok.y + modal_btn_ok.h)
        {
            if (strlen(layout->modal_buffer) > 0)
            {
                int next_id = channel_model_get_count() + 1;
                channel_model_add(next_id, layout->modal_buffer, layout->modal_is_private);
                layout->show_create_modal = 0;
                layout->modal_buffer[0] = '\0';
            }
            return 0;
        }
        return 0;
    }

    // 2. BOUTON LOGOUT
    if (cx >= layout->btn_logout.x && cx <= layout->btn_logout.x + layout->btn_logout.w &&
        cy >= layout->btn_logout.y && cy <= layout->btn_logout.y + layout->btn_logout.h)
    {
        return 2;
    }

    // 3. BOUTON AJOUT SALON "➕"
    if (cx >= layout->btn_add_channel.x && cx <= layout->btn_add_channel.x + layout->btn_add_channel.w &&
        cy >= layout->btn_add_channel.y && cy <= layout->btn_add_channel.y + layout->btn_add_channel.h)
    {
        layout->show_create_modal = 1;
        layout->modal_focused_field = 1;
        layout->modal_buffer[0] = '\0';
        layout->modal_is_private = 0;
        return 0;
    }

    // 4. BOUTON APPEL 📞
    if (cx >= layout->btn_call.x && cx <= layout->btn_call.x + layout->btn_call.w &&
        cy >= layout->btn_call.y && cy <= layout->btn_call.y + layout->btn_call.h)
    {
        // 1. Ouvrir l'interface de l'appel avec le renderer statique et les polices globales
        ouvrir_fenetre_appel(current_renderer, font_title, font_main, font_sub, font_emoji, layout->window_w, layout->window_h);

        printf("📞 Clic sur le bouton Appel : Ouverture de la fenêtre d'appel...\n");

        return 1; // On retourne 1 pour indiquer que l'événement de clic a été géré
    }

    // 5. BOUTON TRANSFERT DE FICHIER
    if (cx >= layout->btn_file_transfer.x && cx <= layout->btn_file_transfer.x + layout->btn_file_transfer.w &&
        cy >= layout->btn_file_transfer.y && cy <= layout->btn_file_transfer.y + layout->btn_file_transfer.h)
    {
        char* file_path = open_file_explorer();
        
        if (file_path != NULL && strlen(file_path) > 0)
        {
            printf("📁 Fichier sélectionné pour l'envoi : %s\n", file_path);
            
            Channel *active = channel_model_get_active();
            if (active)
            {
                // On ajoute le chemin du fichier dans la liste des messages !
                // Pour l'instant, on l'envoie comme texte pour vérifier que le chemin est bien transmis.
                message_model_add(0, active->id, "Me", file_path); 
            }
        }
        
        return 1;
    }

    // FIX : BOUTON ENVOYER LE MESSAGE (Calcul des coordonnées en direct à la place de la structure manquante)
    int send_x = layout->chat_input_bar.x + layout->chat_input_bar.w - 45;
    int send_y = layout->window_h - 56;
    if (cx >= send_x && cx <= send_x + 36 &&
        cy >= send_y && cy <= send_y + 36)
    {
        if (strlen(layout->input_buffer) > 0)
        {
            Channel *active = channel_model_get_active();
            if (active)
            {
                message_model_add(0, active->id, "Me", layout->input_buffer);
                layout->input_buffer[0] = '\0';
            }
        }
        return 0;
    }

    // SUPPRESSION DE SALON
    if (layout->hover_channel_delete_index != -1)
    {
        if (cx >= layout->btn_delete_channel_rect.x && cx <= layout->btn_delete_channel_rect.x + layout->btn_delete_channel_rect.w &&
            cy >= layout->btn_delete_channel_rect.y && cy <= layout->btn_delete_channel_rect.y + layout->btn_delete_channel_rect.h)
        {
            channel_model_delete_by_index(layout->hover_channel_delete_index);
            layout->hover_channel_delete_index = -1;
            return 0;
        }
    }

    // SÉLECTION D'UN SALON
    if (cx >= layout->sidebar_channels.x && cx <= layout->sidebar_channels.x + layout->sidebar_channels.w)
    {
        int channel_y = 60;
        int ch_count = channel_model_get_count();
        for (int i = 0; i < ch_count; i++)
        {
            SDL_Rect item_rect = {layout->sidebar_channels.x + 8, channel_y - 4, layout->sidebar_channels.w - 16, 28};
            if (cx >= item_rect.x && cx <= item_rect.x + item_rect.w &&
                cy >= item_rect.y && cy <= item_rect.y + item_rect.h)
            {
                channel_model_set_active_index(i);
                return 0;
            }
            channel_y += 32;
        }
    }

    // SUPPRESSION DE MESSAGE
    if (layout->hover_message_delete_index != -1)
    {
        if (cx >= layout->btn_delete_message_rect.x && cx <= layout->btn_delete_message_rect.x + layout->btn_delete_message_rect.w &&
            cy >= layout->btn_delete_message_rect.y && cy <= layout->btn_delete_message_rect.y + layout->btn_delete_message_rect.h)
        {
            Channel *active = channel_model_get_active();
            if (active)
            {
                message_model_delete_by_index_in_channel(active->id, layout->hover_message_delete_index);
            }
            layout->hover_message_delete_index = -1;
            return 0;
        }
    }

    // FOCUS INPUT CHAT
    if (cx >= layout->chat_input_bar.x && cx <= layout->chat_input_bar.x + layout->chat_input_bar.w &&
        cy >= layout->chat_input_bar.y && cy <= layout->chat_input_bar.y + layout->chat_input_bar.h)
    {
        layout->is_input_focused = 1;
    }
    else
    {
        layout->is_input_focused = 0;
    }
    return 0;
}

void chat_controller_handle_keydown(ChatLayout *layout, SDL_Keycode sym)
{
    if (layout->show_create_modal && layout->modal_focused_field)
    {
        if (sym == SDLK_BACKSPACE)
        {
            size_t len = strlen(layout->modal_buffer);
            if (len > 0)
                layout->modal_buffer[len - 1] = '\0';
        }
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
        {
            if (strlen(layout->modal_buffer) > 0)
            {
                int next_id = channel_model_get_count() + 1;
                channel_model_add(next_id, layout->modal_buffer, layout->modal_is_private);
                layout->show_create_modal = 0;
                layout->modal_buffer[0] = '\0';
            }
        }
        return;
    }

    if (sym == SDLK_BACKSPACE && layout->is_input_focused)
    {
        size_t len = strlen(layout->input_buffer);
        if (len > 0)
            layout->input_buffer[len - 1] = '\0';
    }
    else if ((sym == SDLK_RETURN || sym == SDLK_KP_ENTER) && layout->is_input_focused && strlen(layout->input_buffer) > 0)
    {
        Channel *active = channel_model_get_active();
        if (active)
        {
            message_model_add(0, active->id, "Me", layout->input_buffer);
            layout->input_buffer[0] = '\0';
        }
    }
}

void chat_controller_handle_textinput(ChatLayout *layout, const char *text)
{
    if (layout->show_create_modal && layout->modal_focused_field)
    {
        if (strlen(layout->modal_buffer) + strlen(text) < sizeof(layout->modal_buffer) - 1)
        {
            strcat(layout->modal_buffer, text);
        }
        return;
    }

    if (layout->is_input_focused)
    {
        if (strlen(layout->input_buffer) + strlen(text) < MAX_MSG_LENGTH - 1)
        {
            strcat(layout->input_buffer, text);
        }
    }
}

int chat_controller_is_mic_muted(void) { return g_is_mic_muted; }

static char* open_file_explorer(void)
{
#ifdef _WIN32
    OPENFILENAMEW ofn;           // Version Wide (Unicode)
    static wchar_t szFileW[260]; // Buffer en caractères larges
    static char szFileUTF8[512]; // Buffer final converti en UTF-8
    
    memset(szFileW, 0, sizeof(szFileW));
    memset(szFileUTF8, 0, sizeof(szFileUTF8));
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFileW;
    ofn.nMaxFile = 260;
    // Les filtres doivent aussi être en Wide string (L"...")
    ofn.lpstrFilter = L"Images (*.png;*.jpg;*.jpeg)\0*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        // Conversion propre du chemin Wide (UTF-16 Windows) vers du bon vieil UTF-8 compatible partout
        WideCharToMultiByte(CP_UTF8, 0, szFileW, -1, szFileUTF8, sizeof(szFileUTF8), NULL, NULL);
        printf("[FILE EXPLORER] Selected file (UTF-8): %s\n", szFileUTF8);
        return szFileUTF8; 
    }
    return NULL;
#else
    printf("[FILE EXPLORER] Native explorer only supported on Windows.\n");
    return NULL;
#endif
}

void chat_controller_handle_right_click(ChatLayout *layout, int cx, int cy)
{
    (void)layout;
    (void)cx;
    (void)cy;
}
void chat_controller_handle_menu_action(ChatLayout *layout, int cx, int cy)
{
    (void)layout;
    (void)cx;
    (void)cy;
}
#include "chat_controller.h"
#include "channel.h"
#include "message.h"
#include "ui_channels.h"
#include <string.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

static int is_mic_muted = 0;
static int selected_item_id = -1; 

// Forward declaration for the compiler
static void open_file_explorer(void);

void chat_controller_init(ChatLayout *layout, SDL_Renderer *renderer) {
    layout->menu_type = 0;
    layout->show_create_modal = 0;
    layout->input_buffer[0] = '\0';
    layout->modal_name_buffer[0] = '\0';
    layout->is_mic_muted = 0;

    // Loading PNG textures
    layout->tex_mic_on = IMG_LoadTexture(renderer, "assets/bouton/microphone_on.png");
    layout->tex_mic_off = IMG_LoadTexture(renderer, "assets/bouton/btn-microphone.png");
    layout->tex_file = IMG_LoadTexture(renderer, "assets/bouton/btn-ajoutfichier.png");
    
    if (!layout->tex_mic_on || !layout->tex_mic_off || !layout->tex_file) {
        printf("[WARNING] One or more icons missing in assets/bouton/: %s\n", IMG_GetError());
    }
}

void chat_controller_destroy(ChatLayout *layout) {
    if (layout->tex_mic_on) SDL_DestroyTexture(layout->tex_mic_on);
    if (layout->tex_mic_off) SDL_DestroyTexture(layout->tex_mic_off);
    if (layout->tex_file) SDL_DestroyTexture(layout->tex_file);
}

void chat_controller_update_hover(ChatLayout *layout, int mx, int my) {
    // SECURITY: If context menu is open, block background hovers
    if (layout->menu_type > 0) {
        layout->hover.hover_channel_index = -1;
        layout->hover.hover_message_index = -1;
        layout->hover.hover_mic_button = 0;
        layout->hover.hover_file_button = 0;

        if (mx >= layout->menu_rect.x && mx <= layout->menu_rect.x + layout->menu_rect.w &&
            my >= layout->menu_rect.y && my <= layout->menu_rect.y + layout->menu_rect.h) {
            layout->hover.hover_menu_action = 1;
        } else {
            layout->hover.hover_menu_action = 0;
        }
        return; 
    }

    // NORMAL BEHAVIOR (Menu closed)
    // 1. Microphone button hover
    layout->hover.hover_mic_button = (mx >= layout->btn_microphone.x && mx <= layout->btn_microphone.x + layout->btn_microphone.w &&
                                      my >= layout->btn_microphone.y && my <= layout->btn_microphone.y + layout->btn_microphone.h);

    // 2. File transfer button hover
    layout->hover.hover_file_button = (mx >= layout->btn_file_transfer.x && mx <= layout->btn_file_transfer.x + layout->btn_file_transfer.w &&
                                       my >= layout->btn_file_transfer.y && my <= layout->btn_file_transfer.y + layout->btn_file_transfer.h);

    // 3. Channels sidebar hover
    if (mx >= layout->sidebar_channels.x && mx <= layout->sidebar_channels.x + layout->sidebar_channels.w && my > SIDEBAR_CHANNELS_Y) {
        int ch_y = SIDEBAR_CHANNELS_Y;
        int detected_idx = -1;
        int total_ch = channel_model_get_count(); 

        for (int i = 0; i < total_ch; i++) {
            if (my >= ch_y - 4 && my < (ch_y - 4) + CHANNEL_ITEM_HEIGHT) {
                detected_idx = i;
                break;
            }
            ch_y += CHANNEL_ITEM_HEIGHT;
        }
        layout->hover.hover_channel_index = detected_idx;
    } else {
        layout->hover.hover_channel_index = -1;
    }

    // 4. Chat messages area hover
    if (mx >= layout->chat_area.x && mx <= layout->chat_area.x + layout->chat_area.w &&
        my >= CHAT_MESSAGES_START_Y && my < layout->chat_input_bar.y) {
        
        int start_y = CHAT_MESSAGES_START_Y;
        int detected_msg_idx = -1;
        Channel *active_ch = channel_model_get_active();
        
        if (active_ch) {
            Message dummy[MAX_MESSAGES];
            int msg_count = message_model_get_for_channel(active_ch->id, dummy, MAX_MESSAGES);
            for (int i = 0; i < msg_count; i++) {
                if (my >= start_y && my < start_y + MESSAGE_ITEM_HEIGHT) {
                    detected_msg_idx = i;
                    break;
                }
                start_y += MESSAGE_ITEM_HEIGHT;
            }
        }
        layout->hover.hover_message_index = detected_msg_idx;
    } else {
        layout->hover.hover_message_index = -1;
    }
}

static void open_file_explorer(void) {
#ifdef _WIN32
    OPENFILENAME ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileName(&ofn) == TRUE) {
        printf("[FILE] Selected file: %s\n", ofn.lpstrFile);
    }
#endif
}

int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy) {
    // Close context menu if open and clicked outside
    if (layout->menu_type > 0) {
        if (layout->hover.hover_menu_action) {
            chat_controller_handle_menu_action(layout, cx, cy);
            layout->menu_type = 0;
            return 0;
        } else {
            layout->menu_type = 0;
            return 0; 
        }
    }

    // 1. Logout button
    if (cx >= btn_logout.x && cx <= btn_logout.x + btn_logout.w && 
        cy >= btn_logout.y && cy <= btn_logout.y + btn_logout.h) {
        printf("[LOG] Logout requested.\n");
        channels_clear_textures();
        return 2; 
    }

    // 2. Add channel button '+'
    if (cx >= btn_add_channel.x && cx <= btn_add_channel.x + btn_add_channel.w && 
        cy >= btn_add_channel.y && cy <= btn_add_channel.y + btn_add_channel.h) {
        layout->show_create_modal = 1;
        layout->modal_name_buffer[0] = '\0';
        layout->modal_is_private = 0;
        return 0;
    }

    // 3. Microphone Toggle click
    if (layout->hover.hover_mic_button) {
        layout->is_mic_muted = !layout->is_mic_muted;
        return 0;
    }

    // 4. File transfer click
    if (layout->hover.hover_file_button) {
        printf("[FILE] File transfer requested.\n");
        open_file_explorer(); 
        return 0;
    }

    // 5. Creation Modal interaction
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

    // 6. Channel selection click
    if (layout->hover.hover_channel_index != -1) {
        channel_model_set_active_index(layout->hover.hover_channel_index);
        return 0;
    }

    // 7. Saisie focus input bar
    if (cx >= layout->chat_input_bar.x && cx <= layout->chat_input_bar.x + layout->chat_input_bar.w &&
        cy >= layout->chat_input_bar.y && cy <= layout->chat_input_bar.y + layout->chat_input_bar.h) {
        layout->is_input_focused = 1;
    } else {
        layout->is_input_focused = 0;
    }
    return 0;
}

void chat_controller_handle_right_click(ChatLayout *layout, int cx, int cy) {
    // Context menu on Channel
    if (layout->hover.hover_channel_index != -1) {
        layout->menu_type = 1; // 1 = Channel Menu
        layout->menu_x = cx;
        layout->menu_y = cy;
        layout->menu_rect = (SDL_Rect){cx, cy, 180, MESSAGE_ITEM_HEIGHT + 14};
        return;
    }
    // Context menu on Message
    else if (cx >= layout->chat_area.x && cx <= layout->chat_area.x + layout->chat_area.w && 
             cy > CHAT_MESSAGES_START_Y && cy < layout->chat_input_bar.y) {
        layout->menu_type = 3; // 3 = Message Menu
        layout->menu_x = cx;
        layout->menu_y = cy;
        layout->menu_rect = (SDL_Rect){cx, cy, 180, MESSAGE_ITEM_HEIGHT + 14};
    }
    // Context menu on Member
    else if (cx >= layout->sidebar_members.x && cy > 50) {
        layout->menu_type = 2; // 2 = Member Menu
        layout->menu_x = cx;
        layout->menu_y = cy;
        layout->menu_rect = (SDL_Rect){cx, cy, 150, MESSAGE_ITEM_HEIGHT + 14};
    }
}
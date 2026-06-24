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

// Global internal tracking for microphone state accessor
static int g_is_mic_muted = 0;

// Forward declaration for the compiler
static void open_file_explorer(void);

void chat_controller_init(ChatLayout *layout, SDL_Renderer *renderer) {
    layout->menu_type = 0;
    layout->show_create_modal = 0;
    layout->input_buffer[0] = '\0';
    layout->modal_name_buffer[0] = '\0';
    layout->is_mic_muted = 0;
    g_is_mic_muted = 0;

    // Loading PNG textures
    layout->tex_mic_on = IMG_LoadTexture(renderer, "assets/bouton/microphone_on.png");
    layout->tex_mic_off = IMG_LoadTexture(renderer, "assets/bouton/btn-microphone.png");
    layout->tex_file = IMG_LoadTexture(renderer, "assets/bouton/btn-ajoutfichier.png");
    
    if (!layout->tex_mic_on || !layout->tex_mic_off || !layout->tex_file) {
        printf("[WARNING] One or more icons missing in assets/bouton/: %s\n", IMG_GetError());
    }
}

void chat_controller_destroy(ChatLayout *layout) {
    // Safely free up video memory loaded textures during destruction
    if (layout->tex_mic_on) {
        SDL_DestroyTexture(layout->tex_mic_on);
        layout->tex_mic_on = NULL;
    }
    if (layout->tex_mic_off) {
        SDL_DestroyTexture(layout->tex_mic_off);
        layout->tex_mic_off = NULL;
    }
    if (layout->tex_file) {
        SDL_DestroyTexture(layout->tex_file);
        layout->tex_file = NULL;
    }
    printf("[CONTROLLER] Textures destroyed successfully.\n");
}

void chat_controller_update_hover(ChatLayout *layout, int mx, int my) {
    // Updates layout UI hover metrics based on live cursor motion coordinates
    // Add custom hit detection boundary updates for zones here if required
    (void)layout;
    (void)mx;
    (void)my;
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
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        printf("[FILE EXPLORER] Selected file: %s\n", ofn.lpstrFile);
    } else {
        printf("[FILE EXPLORER] Canceled or error occurred.\n");
    }
#else
    printf("[FILE EXPLORER] File explorer only supported on Windows OS via Win32 API.\n");
#endif
}

int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy) {
    // 1. Click on Microphone Button
    if (cx >= layout->btn_microphone.x && cx <= layout->btn_microphone.x + layout->btn_microphone.w &&
        cy >= layout->btn_microphone.y && cy <= layout->btn_microphone.y + layout->btn_microphone.h) {
        layout->is_mic_muted = !layout->is_mic_muted;
        g_is_mic_muted = layout->is_mic_muted;
        printf("[CONTROLLER] Microphone state changed: %s\n", layout->is_mic_muted ? "MUTED" : "UNMUTED");
        return 0;
    }

    // 2. Click on File Upload Button
    if (cx >= layout->btn_file_transfer.x && cx <= layout->btn_file_transfer.x + layout->btn_file_transfer.w &&
        cy >= layout->btn_file_transfer.y && cy <= layout->btn_file_transfer.y + layout->btn_file_transfer.h) {
        printf("[CONTROLLER] File transfer button triggered. Opening explorer...\n");
        open_file_explorer();
        return 0;
    }

    // 3. Channel Selection Click Handling
    if (layout->hover.hover_channel_index != -1) {
        channel_model_set_active_index(layout->hover.hover_channel_index);
        return 0;
    }

    // 4. Input Bar Focus Handling
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
        return;
    }
}

void chat_controller_handle_menu_action(ChatLayout *layout, int cx, int cy) {
    // Handles context menu action selections
    (void)cx; (void)cy;
    layout->menu_type = 0; // Closes context menu
}

void chat_controller_handle_keydown(ChatLayout *layout, SDL_Keycode sym) {
    // 1. Handle Backspace to remove text characters from input buffer
    if (sym == SDLK_BACKSPACE) {
        if (layout->is_input_focused) {
            size_t len = strlen(layout->input_buffer);
            if (len > 0) {
                layout->input_buffer[len - 1] = '\0';
            }
        }
    }
    // 2. Handle Return / Enter to send the message
    else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
        if (layout->is_input_focused && strlen(layout->input_buffer) > 0) {
            // Clear buffer after processing
            layout->input_buffer[0] = '\0';
        }
    }
}

void chat_controller_handle_textinput(ChatLayout *layout, const char *text) {
    // Appends character keys to buffer when chat input box is active
    if (layout->is_input_focused) {
        size_t current_len = strlen(layout->input_buffer);
        size_t append_len = strlen(text);
        
        if (current_len + append_len < MAX_MSG_LENGTH) {
            strcat(layout->input_buffer, text);
        }
    }
}

int chat_controller_is_mic_muted(void) {
    return g_is_mic_muted;
}
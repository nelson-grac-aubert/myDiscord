#include "../include/channel_controller.h"
#include "channel.h"
#include "ui_chat.h"
#include <string.h>

void channel_controller_handle_left_click(ChatLayout *layout, int cx, int cy)
{
    if (layout->show_create_modal) {
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

        if (cx >= modal_btn_ok.x && cx <= modal_btn_ok.x + modal_btn_ok.w &&
            cy >= modal_btn_ok.y && cy <= modal_btn_ok.y + modal_btn_ok.h) {
            if (strlen(layout->modal_name_buffer) > 0)
                channel_model_add(0, layout->modal_name_buffer, layout->modal_is_private);
            layout->show_create_modal = 0;
        }
        return;
    }

   // Click on the '+' button to open the channel creation modal
    if (cx >= layout->btn_add_channel.x && cx <= layout->btn_add_channel.x + layout->btn_add_channel.w &&
        cy >= layout->btn_add_channel.y && cy <= layout->btn_add_channel.y + layout->btn_add_channel.h)
    {
        layout->show_create_modal = 1;
        layout->modal_is_private = 0;
        layout->modal_focused_field = 1;
        layout->modal_name_buffer[0] = '\0';
        return 0; // On renvoie 0 pour indiquer que le clic a été traité sans changement de vue
    }

    int ch_y = 55;
    for (int i = 0; i < channel_model_get_count(); i++) {
        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, ch_y - 4,
                              layout->sidebar_channels.w - 16, 28};
        if (cx >= item_rect.x && cx <= item_rect.x + item_rect.w &&
            cy >= item_rect.y && cy <= item_rect.y + item_rect.h) {
            channel_model_set_active_index(i);
            break;
        }
        ch_y += 32;
    }
}

void channel_controller_handle_right_click(ChatLayout *layout, int cx, int cy)
{
    int ch_y = 55;
    for (int i = 0; i < channel_model_get_count(); i++) {
        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, ch_y - 4,
                              layout->sidebar_channels.w - 16, 28};
        if (cx >= item_rect.x && cx <= item_rect.x + item_rect.w &&
            cy >= item_rect.y && cy <= item_rect.y + item_rect.h) {
            layout->menu_type    = 1;
            layout->target_index = i;
            layout->menu_x = cx;
            layout->menu_y = cy;
            return;
        }
        ch_y += 32;
    }
}

void channel_controller_handle_keydown(ChatLayout *layout, SDL_Keycode key)
{
    if (layout->show_create_modal && layout->modal_focused_field == 1 &&
        key == SDLK_BACKSPACE) {
        size_t len = strlen(layout->modal_name_buffer);
        if (len > 0)
            layout->modal_name_buffer[len - 1] = '\0';
    }
}

void channel_controller_handle_textinput(ChatLayout *layout, const char *text)
{
    if (layout->show_create_modal && layout->modal_focused_field == 1) {
        if (strlen(layout->modal_name_buffer) + strlen(text) < 31)
            strcat(layout->modal_name_buffer, text);
    }
}
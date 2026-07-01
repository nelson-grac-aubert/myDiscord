#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "message.h"
#include "variables.h"

#define MAX_MEMBER_ROWS 64

typedef struct
{
    int window_w;
    int window_h;

    SDL_Rect sidebar_servers;
    SDL_Rect sidebar_channels;
    SDL_Rect chat_area;
    SDL_Rect sidebar_members;

    SDL_Rect chat_top_bar;
    SDL_Rect chat_input_bar;

    // Geometry bounds for multimedia buttons
    SDL_Rect btn_call;
    SDL_Rect btn_file_transfer;
    SDL_Rect btn_logout;
    SDL_Rect btn_add_channel;

    int is_input_focused;
    char input_buffer[MAX_MSG_LENGTH];

    int show_create_modal;
    char modal_name_buffer[32];
    int modal_is_private;
    int modal_focused_field;
    int target_index;

    int menu_type;
    int menu_x;
    int menu_y;
    SDL_Rect menu_rect;

    int is_mic_muted;
    SDL_Texture *tex_mic_on;
    SDL_Texture *tex_mic_off;
    SDL_Texture *tex_file;

    ChatHoverState hover;
    int hover_channel_delete_index;
    int hover_message_delete_index;
    SDL_Rect btn_delete_channel_rect;
    SDL_Rect btn_delete_message_rect;
    int show_context_menu;
    int context_menu_x;
    int context_menu_y;
    char modal_buffer[256];

    // Pixels scrolled up from the bottom (0 = pinned to the latest message)
    int chat_scroll_offset;

    // Right-click context menu on a member of the online-users panel
    int show_user_context_menu;
    int context_menu_target_user_id;
    int context_menu_is_unban; // 0 = show "Ban User", 1 = show "Unban User"
    SDL_Rect btn_ban_user_rect;

    // Populated each frame by users_draw_sidebar (one entry per rendered
    // row, online or offline) and consumed by
    // chat_controller_handle_right_click, so hit-testing always matches
    // exactly what was drawn instead of duplicating the layout math
    SDL_Rect member_row_rect[MAX_MEMBER_ROWS];
    int member_row_user_id[MAX_MEMBER_ROWS];
    int member_row_is_banned[MAX_MEMBER_ROWS];
    int member_row_count;

    // Right-click context menu on a chat message (add/remove reaction)
    int show_reaction_menu;
    int reaction_menu_message_id;
    int reaction_menu_is_remove; // 0 = show 3 emoji add-buttons, 1 = show "Remove Reaction"
    SDL_Rect btn_reaction_rects[3];
    SDL_Rect btn_reaction_remove_rect;

    // Populated each frame by draw_chat_messages (one visible row per
    // message) and consumed by chat_controller_handle_right_click, same
    // rationale as member_row_*
    SDL_Rect message_row_rect[MAX_MESSAGES];
    int message_row_id[MAX_MESSAGES];
    int message_row_count;
} ChatLayout;

extern SDL_Rect modal_input_rect;
extern SDL_Rect modal_toggle_rect;
extern SDL_Rect modal_btn_cancel;
extern SDL_Rect modal_btn_ok;

void ui_chat_handle_resize(ChatLayout *layout, int win_w, int win_h);

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer,
                  TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub);

#endif /* UI_CHAT_H */
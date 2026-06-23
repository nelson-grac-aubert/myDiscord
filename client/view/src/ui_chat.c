#include "../include/ui_chat.h"
#include "../include/ui_chat_store.h"
#include "../include/ui_chat_components.h"
#include "../include/ui_channels.h"
#include "../include/ui_users.h"
#include <stdio.h>
#include <string.h>

#define COLOR_BG_SERVERS 0x0D, 0x0E, 0x12, 0xFF
#define COLOR_BG_CHANNELS 0x12, 0x14, 0x1C, 0xFF
#define COLOR_BG_CHAT 0x16, 0x19, 0x24, 0xFF
#define COLOR_BG_MEMBERS 0x12, 0x14, 0x1C, 0xFF
#define COLOR_BG_INPUT 0x1F, 0x23, 0x33, 0xFF

static void compute_modal_rects(ChatLayout *layout, int win_w, int win_h)
{
    int modal_w = 440;
    int modal_h = 280;
    modal_bg_rect = (SDL_Rect){(win_w - modal_w) / 2, (win_h - modal_h) / 2, modal_w, modal_h};
    modal_input_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 90, modal_bg_rect.w - 60, 40};
    modal_toggle_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 175, 50, 24};
    modal_btn_cancel = (SDL_Rect){modal_bg_rect.x + 210, modal_bg_rect.y + 220, 90, 36};
    modal_btn_confirm = (SDL_Rect){modal_bg_rect.x + 310, modal_bg_rect.y + 220, 100, 36};
    (void)layout;
}

void update_chat_layout(ChatLayout *layout, int win_w, int win_h)
{
    layout->window_w = win_w;
    layout->window_h = win_h;
    int servers_w = 0;
    int channels_w = 240;
    int members_w = 240;
    int chat_w = win_w - (servers_w + channels_w + members_w);
    if (chat_w < 350)
    {
        members_w = 0;
        chat_w = win_w - (servers_w + channels_w);
    }
    layout->sidebar_servers = (SDL_Rect){0, 0, servers_w, win_h};
    layout->sidebar_channels = (SDL_Rect){servers_w, 0, channels_w, win_h};
    layout->chat_area = (SDL_Rect){servers_w + channels_w, 0, chat_w, win_h};
    layout->sidebar_members = (SDL_Rect){win_w - members_w, 0, members_w, win_h};
    layout->chat_top_bar = (SDL_Rect){layout->chat_area.x, 0, layout->chat_area.w, 48};
    layout->chat_input_bar = (SDL_Rect){layout->chat_area.x + 16, win_h - 68, layout->chat_area.w - 32, 44};
    channels_update_layout(layout, win_h);
    compute_modal_rects(layout, win_w, win_h);
    if (layout->menu_type > 0)
        layout->menu_rect = (SDL_Rect){layout->menu_x, layout->menu_y, 130, 32};
}

static void draw_backgrounds(SDL_Renderer *renderer, ChatLayout *layout)
{
    SDL_SetRenderDrawColor(renderer, COLOR_BG_SERVERS);
    SDL_RenderFillRect(renderer, &layout->sidebar_servers);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHANNELS);
    SDL_RenderFillRect(renderer, &layout->sidebar_channels);
    SDL_SetRenderDrawColor(renderer, COLOR_BG_CHAT);
    SDL_RenderFillRect(renderer, &layout->chat_area);
    SDL_SetRenderDrawColor(renderer, 0x0D, 0x0E, 0x12, 0xFF);
    SDL_RenderDrawLine(renderer,
                       layout->chat_top_bar.x, layout->chat_top_bar.h,
                       layout->chat_top_bar.x + layout->chat_top_bar.w, layout->chat_top_bar.h);
}

static void draw_input_bar(SDL_Renderer *renderer, ChatLayout *layout,
                           TTF_Font *font_main,
                           SDL_Color white, SDL_Color dark_gray, SDL_Color blurple)
{
    SDL_SetRenderDrawColor(renderer, COLOR_BG_INPUT);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);
    LocalChannel *ch = store_get_active_channel();
    if (!ch)
        return;

    if (strlen(layout->input_buffer) == 0)
    {
        char placeholder[64];
        snprintf(placeholder, sizeof(placeholder), "Envoyer un message dans %s %s",
                 ch->is_private ? "🔒" : "#", ch->name);
        components_draw_text(renderer, font_main, placeholder,
                             layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, dark_gray);
    }
    else
    {
        components_draw_text(renderer, font_main, layout->input_buffer,
                             layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, white);
    }
    if (layout->is_input_focused)
    {
        SDL_SetRenderDrawColor(renderer, blurple.r, blurple.g, blurple.b, blurple.a);
        SDL_RenderDrawRect(renderer, &layout->chat_input_bar);
        components_draw_cursor(renderer, font_main, layout->input_buffer,
                               layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12,
                               18, white);
    }
}

static void draw_channel_title(SDL_Renderer *renderer, ChatLayout *layout,
                               TTF_Font *font_title, SDL_Color white)
{
    LocalChannel *ch = store_get_active_channel();
    if (!ch)
        return;
    char title[64];
    snprintf(title, sizeof(title), "%s %s", ch->is_private ? "🔒" : "#", ch->name);
    components_draw_text(renderer, font_title, title,
                         layout->chat_top_bar.x + 20, 13, white);
}

static void draw_messages(SDL_Renderer *renderer, TTF_Font *font_main, TTF_Font *font_sub,
                          ChatLayout *layout, SDL_Color white, SDL_Color green)
{
    LocalChannel *ch = store_get_active_channel();
    if (!ch)
        return;
    int x = layout->chat_area.x + 20;
    int base_y = 70;
    for (int i = 0; i < ch->message_count; i++)
    {
        components_draw_text(renderer, font_sub, ch->messages[i].username, x, base_y, green);
        components_draw_text(renderer, font_main, ch->messages[i].text, x, base_y + 22, white);
        base_y += 55;
    }
}

static void draw_context_menu(SDL_Renderer *renderer, ChatLayout *layout,
                              TTF_Font *font_sub, SDL_Color white, int mx, int my)
{
    if (layout->menu_type == 0)
        return;
    int hovered = (mx >= layout->menu_rect.x && mx <= layout->menu_rect.x + layout->menu_rect.w &&
                   my >= layout->menu_rect.y && my <= layout->menu_rect.y + layout->menu_rect.h);
    SDL_SetRenderDrawColor(renderer, hovered ? 242 : 25, hovered ? 63 : 28, hovered ? 67 : 38, 255);
    SDL_RenderFillRect(renderer, &layout->menu_rect);
    SDL_SetRenderDrawColor(renderer, 0x0D, 0x0E, 0x12, 255);
    SDL_RenderDrawRect(renderer, &layout->menu_rect);
    components_draw_text(renderer, font_sub, "Supprimer",
                         layout->menu_rect.x + 35, layout->menu_rect.y + 8, white);
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout,
                         TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub)
{
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {148, 155, 164, 255};
    SDL_Color dark_gray = {94, 99, 106, 255};
    SDL_Color green = {35, 165, 90, 255};
    SDL_Color blurple = {88, 101, 242, 255};
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    draw_backgrounds(renderer, layout);
    draw_input_bar(renderer, layout, font_main, white, dark_gray, blurple);
    draw_channel_title(renderer, layout, font_title, white);
    
    channels_draw_sidebar(renderer, layout, font_title, font_main, font_sub,
                          mx, my, white, gray, dark_gray);
                          
    if (layout->sidebar_members.w > 0)
    {
        SDL_SetRenderDrawColor(renderer, COLOR_BG_MEMBERS);
        SDL_RenderFillRect(renderer, &layout->sidebar_members);
        users_draw_sidebar(renderer, layout, font_main, font_sub, green, dark_gray);
    }
    draw_messages(renderer, font_main, font_sub, layout, white, green);
    if (layout->show_create_modal)
    {
        components_draw_modal(renderer, layout, font_title, font_main, font_sub,
                              mx, my, white, gray, dark_gray);
        if (layout->modal_focused_field == 1)
            components_draw_cursor(renderer, font_main, layout->modal_name_buffer,
                                   modal_input_rect.x + 12, modal_input_rect.y + 8, 18, white);
    }
    draw_context_menu(renderer, layout, font_sub, white, mx, my);
}

static void handle_right_click(ChatLayout *layout, int cx, int cy)
{
    layout->menu_type = 0;
    int ch_y = 55;
    for (int i = 0; i < store_get_channel_count(); i++)
    {
        // ➡️ CORRECTION : Alignement parfait de la zone de clic droit avec item_rect (28px de haut)
        SDL_Rect item_rect = {layout->sidebar_channels.x + 8, ch_y - 4, layout->sidebar_channels.w - 16, 28};

        if (cx >= item_rect.x && cx <= item_rect.x + item_rect.w &&
            cy >= item_rect.y && cy <= item_rect.y + item_rect.h)
        {
            layout->menu_type = 1;
            layout->target_index = i;
            layout->menu_x = cx;
            layout->menu_y = cy;
            return;
        }
        ch_y += 32;
    }
    if (cx >= layout->chat_area.x &&
        cx <= layout->chat_area.x + layout->chat_area.w)
    {
        LocalChannel *ch = store_get_active_channel();
        if (!ch)
            return;
        int base_y = 70;
        for (int i = 0; i < ch->message_count; i++)
        {
            if (cy >= base_y && cy <= base_y + 45)
            {
                layout->menu_type = 2;
                layout->target_index = i;
                layout->menu_x = cx;
                layout->menu_y = cy;
                return;
            }
            base_y += 55;
        }
    }
}

static void handle_left_click_menu(ChatLayout *layout, int cx, int cy)
{
    if (cx >= layout->menu_rect.x && cx <= layout->menu_rect.x + layout->menu_rect.w &&
        cy >= layout->menu_rect.y && cy <= layout->menu_rect.y + layout->menu_rect.h)
    {
        if (layout->menu_type == 1)
            store_delete_channel_by_index(layout->target_index);
        else if (layout->menu_type == 2)
            store_delete_message_by_index(layout->target_index);
    }
    layout->menu_type = 0;
}

static void handle_left_click_modal(ChatLayout *layout, int cx, int cy)
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
}

static int handle_left_click_normal(ChatLayout *layout, int cx, int cy)
{
    if (cx >= btn_logout.x && cx <= btn_logout.x + btn_logout.w &&
        cy >= btn_logout.y && cy <= btn_logout.y + btn_logout.h)
    {
        printf("[LOG] Deconnexion.\n");
        channels_clear_textures();
        return 2; 
    }
    layout->is_input_focused = (cx >= layout->chat_input_bar.x &&
                                cx <= layout->chat_input_bar.x + layout->chat_input_bar.w &&
                                cy >= layout->chat_input_bar.y &&
                                cy <= layout->chat_input_bar.y + layout->chat_input_bar.h);
    if (cx >= btn_add_channel.x && cx <= btn_add_channel.x + btn_add_channel.w &&
        cy >= btn_add_channel.y && cy <= btn_add_channel.y + btn_add_channel.h)
    {
        layout->show_create_modal = 1;
        layout->modal_is_private = 0;
        layout->modal_focused_field = 1;
        layout->modal_name_buffer[0] = '\0';
    }
    int ch_y = 55;
    for (int i = 0; i < store_get_channel_count(); i++)
    {
        // ➡️ CORRECTION : Remplacement de la détection brute par la boîte item_rect exacte (28px de haut)
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

static void handle_keydown(ChatLayout *layout, SDL_Keycode key)
{
    if (layout->show_create_modal && layout->modal_focused_field == 1 &&
        key == SDLK_BACKSPACE)
    {
        size_t len = strlen(layout->modal_name_buffer);
        if (len > 0)
            layout->modal_name_buffer[len - 1] = '\0';
        return;
    }
    if (!layout->is_input_focused)
        return;
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

static void handle_textinput(ChatLayout *layout, const char *text)
{
    if (layout->show_create_modal && layout->modal_focused_field == 1)
    {
        if (strlen(layout->modal_name_buffer) + strlen(text) < 31)
            strcat(layout->modal_name_buffer, text);
        return;
    }
    if (layout->is_input_focused &&
        strlen(layout->input_buffer) + strlen(text) < MAX_MSG_LENGTH - 1)
        strcat(layout->input_buffer, text);
}

int chat_ui_init_and_run(void)
{
    TTF_Font *font_title = TTF_OpenFont("fonts/Urbanist-Bold.ttf", 18);
    TTF_Font *font_main = TTF_OpenFont("fonts/Urbanist-Regular.ttf", 15);
    TTF_Font *font_sub = TTF_OpenFont("fonts/Urbanist-SemiBold.ttf", 13);
    if (!font_title || !font_main || !font_sub)
        return 0;
    SDL_Window *window = SDL_CreateWindow("myDiscord - Chat",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1100, 700,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    ChatLayout layout;
    memset(&layout, 0, sizeof(ChatLayout));
    store_init();
    SDL_StartTextInput();
    int running = 1;
    int exit_status = 0;
    SDL_Event event;
    while (running)
    {
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);
        
        update_chat_layout(&layout, win_w, win_h);
        
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
                exit_status = 0;
                channels_clear_textures();
            }
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int cx = event.button.x;
                int cy = event.button.y;
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    handle_right_click(&layout, cx, cy);
                }
                else if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (layout.menu_type > 0)
                    {
                        handle_left_click_menu(&layout, cx, cy);
                    }
                    else if (layout.show_create_modal)
                    {
                        handle_left_click_modal(&layout, cx, cy);
                    }
                    else
                    {
                        int status = handle_left_click_normal(&layout, cx, cy);
                        if (status == 2)
                        {
                            running = 0;
                            exit_status = 2;
                        }
                    }
                }
            }

            if (event.type == SDL_KEYDOWN)
                handle_keydown(&layout, event.key.keysym.sym);
            if (event.type == SDL_TEXTINPUT)
                handle_textinput(&layout, event.text.text);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);
        SDL_RenderPresent(renderer);
    }
    SDL_StopTextInput();
    TTF_CloseFont(font_title);
    TTF_CloseFont(font_main);
    TTF_CloseFont(font_sub);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return exit_status;
}
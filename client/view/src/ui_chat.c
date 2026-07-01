#include "ui_chat.h"
#include "variables.h"
#include "channel.h"
#include "message.h"
#include "ui_channels.h"
#include "ui_users.h"
#include "ui_login.h"
#include "chat_controller.h"
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>

SDL_Rect modal_bg_rect, modal_input_rect, modal_toggle_rect, modal_btn_ok, modal_btn_cancel;

static void compute_modal_rects(int win_w, int win_h)
{
    int modal_w = 440, modal_h = 300;
    modal_bg_rect = (SDL_Rect){(win_w - modal_w) / 2, (win_h - modal_h) / 2, modal_w, modal_h};
    modal_input_rect = (SDL_Rect){modal_bg_rect.x + 30, modal_bg_rect.y + 90, 380, 40};
    modal_toggle_rect = (SDL_Rect){modal_bg_rect.x + 340, modal_bg_rect.y + 160, 50, 26};
    modal_btn_cancel = (SDL_Rect){modal_bg_rect.x + 210, modal_bg_rect.y + 230, 90, 40};
    modal_btn_ok = (SDL_Rect){modal_bg_rect.x + 310, modal_bg_rect.y + 230, 100, 40};
}

void ui_chat_handle_resize(ChatLayout *layout, int win_w, int win_h)
{
    layout->window_w = win_w;
    layout->window_h = win_h;
    layout->sidebar_servers = (SDL_Rect){0, 0, 72, win_h};
    layout->sidebar_channels = (SDL_Rect){72, 0, 240, win_h};
    int chat_x = 72 + 240;
    int members_w = 240;
    layout->sidebar_members = (SDL_Rect){win_w - members_w, 0, members_w, win_h};
    layout->chat_area = (SDL_Rect){chat_x, 0, win_w - chat_x - members_w, win_h};
    layout->chat_top_bar = (SDL_Rect){chat_x, 0, layout->chat_area.w, 48};

    // Saisie raccourcie légèrement pour laisser de la place au micro à droite
    layout->chat_input_bar = (SDL_Rect){chat_x + 60, win_h - 60, layout->chat_area.w - 280, 44};

    layout->btn_file_transfer = (SDL_Rect){chat_x + 15, win_h - 56, 36, 36};
    layout->btn_add_channel = (SDL_Rect){72 + 205, 14, 24, 24};

    // Micro placé juste à droite de la barre de message
    layout->btn_call = (SDL_Rect){layout->chat_input_bar.x + layout->chat_input_bar.w + 10, win_h - 56, 36, 36};
    layout->btn_logout = (SDL_Rect){win_w - 180, win_h - 55, 160, 40};

    channels_update_layout(layout, win_h);
    compute_modal_rects(win_w, win_h);
}

// Petite fonction utilitaire pour vérifier l'extension
static int is_image_path(const char *text) {
    size_t len = strlen(text);
    if (len < 5) return 0;
    return (strcasecmp(text + len - 4, ".png") == 0 ||
            strcasecmp(text + len - 4, ".jpg") == 0 ||
            strcasecmp(text + len - 5, ".jpeg") == 0);
}

static void draw_chat_messages(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_main, TTF_Font *font_emoji, SDL_Color white)
{
    Channel *active_ch = channel_model_get_active();
    if (!active_ch)
        return;

    Message active_msgs[MAX_MESSAGES];
    int msg_count = message_model_get_for_channel(active_ch->id, active_msgs, MAX_MESSAGES);

    /* Leave room on the right for the scrollbar so wrapped text never
       overlaps it (or spills into the members panel) */
    int text_wrap_width = layout->chat_area.w - 40;

    static char full_msgs[MAX_MESSAGES][560];
    int item_heights[MAX_MESSAGES];
    int total_height = 0;
    for (int i = 0; i < msg_count; i++) {
        if (is_image_path(active_msgs[i].text)) {
            item_heights[i] = 120;
        } else {
            snprintf(full_msgs[i], sizeof(full_msgs[i]), "%s  %s: %s",
                     active_msgs[i].timestamp, active_msgs[i].username, active_msgs[i].text);
            int h = measure_text_wrapped_height(font_main, full_msgs[i], text_wrap_width);
            item_heights[i] = (h > MESSAGE_ITEM_HEIGHT ? h : MESSAGE_ITEM_HEIGHT) + 8;
        }
        total_height += item_heights[i];
    }

    /* Viewport = area between the channel top bar and the (dynamically
       sized) input bar. Messages are laid out bottom-anchored so that a
       scroll offset of 0 always shows the latest message, growing the
       offset reveals older ones - like any normal chat scrollback. */
    int viewport_top = CHAT_MESSAGES_START_Y;
    int viewport_bottom = layout->chat_input_bar.y - 10;
    int viewport_height = viewport_bottom - viewport_top;
    if (viewport_height < 0) viewport_height = 0;

    int max_scroll = total_height - viewport_height;
    if (max_scroll < 0) max_scroll = 0;
    if (layout->chat_scroll_offset > max_scroll) layout->chat_scroll_offset = max_scroll;
    if (layout->chat_scroll_offset < 0) layout->chat_scroll_offset = 0;

    int start_y = viewport_bottom - total_height + layout->chat_scroll_offset;

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    layout->hover_message_delete_index = -1;

    // SÉCURITÉ : Activer le Blend Mode pour gérer la transparence des survols
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Rect clip = {layout->chat_area.x, viewport_top, layout->chat_area.w, viewport_height};
    SDL_RenderSetClipRect(renderer, &clip);

    for (int i = 0; i < msg_count; i++)
    {
        int current_item_height = item_heights[i];

        /* Skip rows fully outside the visible viewport, but keep advancing
           start_y so later rows still land at the right position */
        if (start_y + current_item_height < viewport_top || start_y > viewport_bottom)
        {
            start_y += current_item_height;
            continue;
        }

        int is_img = is_image_path(active_msgs[i].text);

        SDL_Rect row_rect = {layout->chat_area.x + 10, start_y - 2, layout->chat_area.w - 20, current_item_height};
        int is_hovered = (mx >= row_rect.x && mx <= row_rect.x + row_rect.w &&
                          my >= row_rect.y && my <= row_rect.y + row_rect.h &&
                          my >= viewport_top && my <= viewport_bottom);

        if (is_hovered && !layout->show_create_modal)
        {
            SDL_SetRenderDrawColor(renderer, VAR_COLOR_HOVER_ITEM.r, VAR_COLOR_HOVER_ITEM.g, VAR_COLOR_HOVER_ITEM.b, VAR_COLOR_HOVER_ITEM.a);
            SDL_RenderFillRect(renderer, &row_rect);

            layout->hover_message_delete_index = i;
            layout->btn_delete_message_rect = (SDL_Rect){row_rect.x + row_rect.w - 35, start_y, 24, 24};
        }

        if (is_img)
        {
            char user_header[128];
            snprintf(user_header, sizeof(user_header), "%s  %s: ",
                     active_msgs[i].timestamp, active_msgs[i].username);
            draw_text(renderer, font_main, user_header, layout->chat_area.x + 20, start_y, white);
            
            SDL_Texture *img_tex = NULL;
            SDL_RWops *rw = SDL_RWFromFile(active_msgs[i].text, "rb");
            if (rw)
            {
                img_tex = IMG_LoadTexture_RW(renderer, rw, 1); 
            }

            if (img_tex)
            {
                SDL_Rect img_target = { layout->chat_area.x + 30, start_y + 22, 140, 80 };
                SDL_RenderCopy(renderer, img_tex, NULL, &img_target);
                SDL_DestroyTexture(img_tex);
            }
            else
            {
                draw_text(renderer, font_main, "<Image introuvable ou corrompue>", layout->chat_area.x + 30, start_y + 22, (SDL_Color){200, 50, 50, 255});
            }            
        }
        else
        {
            draw_text_wrapped(renderer, font_main, full_msgs[i],
                              layout->chat_area.x + 20, start_y, white, text_wrap_width);
        }

        if (is_hovered && !layout->show_create_modal && font_emoji)
        {
            int mx_poub, my_poub;
            SDL_GetMouseState(&mx_poub, &my_poub);
            if (mx_poub >= layout->btn_delete_message_rect.x && mx_poub <= layout->btn_delete_message_rect.x + layout->btn_delete_message_rect.w &&
                my_poub >= layout->btn_delete_message_rect.y && my_poub <= layout->btn_delete_message_rect.y + layout->btn_delete_message_rect.h)
            {
                SDL_SetRenderDrawColor(renderer, 80, 20, 20, 255);
                SDL_RenderFillRect(renderer, &layout->btn_delete_message_rect);
            }
            draw_text(renderer, font_emoji, "🗑️", layout->btn_delete_message_rect.x, layout->btn_delete_message_rect.y, (SDL_Color){220, 50, 50, 255});
        }

        start_y += current_item_height;
    }

    SDL_RenderSetClipRect(renderer, NULL);

    // Scrollbar : uniquement visible si le contenu dépasse la zone visible
    if (max_scroll > 0)
    {
        int track_w = 6;
        int track_x = layout->chat_area.x + layout->chat_area.w - track_w - 4;

        int thumb_h = viewport_height * viewport_height / total_height;
        if (thumb_h < 20) thumb_h = 20;
        if (thumb_h > viewport_height) thumb_h = viewport_height;

        float scroll_ratio = (float)layout->chat_scroll_offset / (float)max_scroll;
        int thumb_y = viewport_top + viewport_height - thumb_h -
                     (int)(scroll_ratio * (viewport_height - thumb_h));

        SDL_Rect track_rect = {track_x, viewport_top, track_w, viewport_height};
        SDL_SetRenderDrawColor(renderer, 0x1A, 0x1B, 0x1E, 255);
        SDL_RenderFillRect(renderer, &track_rect);

        SDL_Rect thumb_rect = {track_x, thumb_y, track_w, thumb_h};
        SDL_SetRenderDrawColor(renderer, 0x5A, 0x5C, 0x62, 255);
        SDL_RenderFillRect(renderer, &thumb_rect);
    }

    // SÉCURITÉ : Désactiver le blend mode et restaurer une couleur neutre (ex: Noir opaque)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void draw_chat_interface(SDL_Renderer *renderer, ChatLayout *layout, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub)
{
    SDL_Color color_white = VAR_COLOR_TEXT_WHITE;
    SDL_Color color_muted = VAR_COLOR_TEXT_MUTED;
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_SERVERS.r, VAR_COLOR_BG_SERVERS.g, VAR_COLOR_BG_SERVERS.b, VAR_COLOR_BG_SERVERS.a);
    SDL_RenderFillRect(renderer, &layout->sidebar_servers);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHANNELS.r, VAR_COLOR_BG_CHANNELS.g, VAR_COLOR_BG_CHANNELS.b, VAR_COLOR_BG_CHANNELS.a);
    SDL_RenderFillRect(renderer, &layout->sidebar_channels);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_CHAT.r, VAR_COLOR_BG_CHAT.g, VAR_COLOR_BG_CHAT.b, VAR_COLOR_BG_CHAT.a);
    SDL_RenderFillRect(renderer, &layout->chat_area);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_MEMBERS.r, VAR_COLOR_BG_MEMBERS.g, VAR_COLOR_BG_MEMBERS.b, VAR_COLOR_BG_MEMBERS.a);
    SDL_RenderFillRect(renderer, &layout->sidebar_members);

    SDL_SetRenderDrawColor(renderer, 0x24, 0x25, 0x28, 0xFF);
    SDL_RenderFillRect(renderer, &layout->chat_top_bar);

    Channel *active_ch = channel_model_get_active();
    if (active_ch)
    {
        if (active_ch->is_private)
        {
            // 1. On dessine le cadenas avec la police emoji
            if (font_emoji)
            {
                draw_text(renderer, font_emoji, "🔒", layout->chat_top_bar.x + 20, 12, color_white);
            }
            // 2. On dessine le nom du salon avec la police de titre, décalé pour ne pas chevaucher le cadenas
            draw_text(renderer, font_title, active_ch->name, layout->chat_top_bar.x + 55, 12, color_white);
        }
        else
        {
            // Salon public normal : on garde le fonctionnement d'origine avec le dièse "#"
            char heading[64];
            snprintf(heading, sizeof(heading), "# %s", active_ch->name);
            draw_text(renderer, font_title, heading, layout->chat_top_bar.x + 20, 12, color_white);
        }
    }

    channels_draw_sidebar(renderer, layout, font_title, font_main, font_sub, font_emoji, mx, my, color_white, color_muted, VAR_COLOR_BG_SERVERS);
    users_draw_sidebar(renderer, layout, font_main, font_sub, (SDL_Color){0x23, 0xA5, 0x5A, 0xFF}, color_muted);

    /// GESTION DU SURVOL / DESSIN CADENAS OU POUBELLE SUR LES SALONS
    layout->hover_channel_delete_index = -1;
    if (!layout->show_create_modal && mx >= layout->sidebar_channels.x && mx <= layout->sidebar_channels.x + layout->sidebar_channels.w)
    {
        int channel_y = 60;
        int ch_count = channel_model_get_count();
        for (int i = 0; i < ch_count; i++)
        {
            SDL_Rect item_rect = {layout->sidebar_channels.x + 8, channel_y - 4, layout->sidebar_channels.w - 16, 28};
            if (mx >= item_rect.x && mx <= item_rect.x + item_rect.w && my >= item_rect.y && my <= item_rect.y + item_rect.h)
            {
                layout->hover_channel_delete_index = i;
                layout->btn_delete_channel_rect = (SDL_Rect){item_rect.x + item_rect.w - 24, channel_y - 2, 20, 20};
                // --- EFFET DE SURVOL DYNAMIQUE SUR LA POUBELLE ---
                int mx_poub, my_poub;
                SDL_GetMouseState(&mx_poub, &my_poub);
                // On vérifie si la souris est précisément au-dessus de la corbeille
                int is_bin_hovered = (mx_poub >= layout->btn_delete_channel_rect.x && mx_poub <= layout->btn_delete_channel_rect.x + layout->btn_delete_channel_rect.w &&
                                      my_poub >= layout->btn_delete_channel_rect.y && my_poub <= layout->btn_delete_channel_rect.y + layout->btn_delete_channel_rect.h);

                SDL_Color trash_color = (SDL_Color){160, 40, 40, 255}; // Couleur rouge plus sombre par défaut

                if (is_bin_hovered)
                {
                    // On affiche le fond rouge foncé uniquement si on pointe la corbeille
                    SDL_SetRenderDrawColor(renderer, 80, 20, 20, 255);
                    SDL_RenderFillRect(renderer, &layout->btn_delete_channel_rect);

                    // On passe la corbeille en rouge vif
                    trash_color = (SDL_Color){255, 60, 60, 255};
                }

                if (font_emoji)
                {
                    draw_text(renderer, font_emoji, "🗑️", layout->btn_delete_channel_rect.x, layout->btn_delete_channel_rect.y, trash_color);
                }
                break;
            }
            channel_y += 32;
        }
    }

    /* Grow the input bar upward to fit the message being typed, anchoring
       its bottom edge in place so the send/call/file buttons (positioned
       from window_h directly) don't move. Computed before draw_chat_messages
       so the message viewport shrinks to make room for it. */
    {
        int wrap_width = layout->chat_input_bar.w - 30;
        int text_h = measure_text_wrapped_height(font_main, layout->input_buffer, wrap_width);
        int line_h = TTF_FontLineSkip(font_main);
        int padding_v = 24;
        int min_h = 44;
        int max_h = padding_v + line_h * 6;

        int new_h = (text_h > 0 ? text_h : line_h) + padding_v;
        if (new_h < min_h) new_h = min_h;
        if (new_h > max_h) new_h = max_h;

        int anchor_bottom = layout->window_h - 16;
        layout->chat_input_bar.h = new_h;
        layout->chat_input_bar.y = anchor_bottom - new_h;
    }

    draw_chat_messages(renderer, layout, font_main, font_emoji, color_white);

    SDL_SetRenderDrawColor(renderer, VAR_COLOR_BG_INPUT.r, VAR_COLOR_BG_INPUT.g, VAR_COLOR_BG_INPUT.b, VAR_COLOR_BG_INPUT.a);
    SDL_RenderFillRect(renderer, &layout->chat_input_bar);

    if (layout->is_input_focused)
    {
        SDL_SetRenderDrawColor(renderer, 0x58, 0x65, 0xF2, 0xFF);
        SDL_RenderDrawRect(renderer, &layout->chat_input_bar);
        SDL_Rect interior = {layout->chat_input_bar.x + 1, layout->chat_input_bar.y + 1, layout->chat_input_bar.w - 2, layout->chat_input_bar.h - 2};
        SDL_RenderDrawRect(renderer, &interior);
    }

    int text_width = 0;
    int wrapped_h = 0;
    if (strlen(layout->input_buffer) == 0)
    {
        if (active_ch)
        {
            char placeholder[128];
            snprintf(placeholder, sizeof(placeholder), "Message %s%s", active_ch->is_private ? "🔒" : "#", active_ch->name);
            draw_text(renderer, font_main, placeholder, layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12, color_muted);
        }
    }
    else
    {
        wrapped_h = draw_text_wrapped(renderer, font_main, layout->input_buffer,
                                      layout->chat_input_bar.x + 15, layout->chat_input_bar.y + 12,
                                      color_white, layout->chat_input_bar.w - 30);
        TTF_SizeText(font_main, layout->input_buffer, &text_width, NULL);
    }

    // Blinking caret only makes sense while the text still fits on one line;
    // once it wraps, just leave the wrapped block visible without a caret.
    int single_line_h = TTF_FontLineSkip(font_main);
    if (layout->is_input_focused && wrapped_h <= single_line_h + 2 && (SDL_GetTicks() / 500) % 2 == 0)
    {
        int cursor_x = layout->chat_input_bar.x + 15 + text_width;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, cursor_x, layout->chat_input_bar.y + 12, cursor_x, layout->chat_input_bar.y + 32);
    }

    // 5. GESTION DU RECTANGLE DE SURVOL SUR LES EMOJIS
    if (font_emoji)
    {
        // Couleur du fond au survol (Gris clair transparent Discord style)
        SDL_Color hover_bg_color = {0x35, 0x37, 0x3C, 255};

        // Émoji : Ajouter Salon ➕
        int is_add_hovered = (mx >= layout->btn_add_channel.x && mx <= layout->btn_add_channel.x + layout->btn_add_channel.w &&
                              my >= layout->btn_add_channel.y && my <= layout->btn_add_channel.y + layout->btn_add_channel.h);
        if (is_add_hovered && !layout->show_create_modal)
        {
            SDL_SetRenderDrawColor(renderer, hover_bg_color.r, hover_bg_color.g, hover_bg_color.b, hover_bg_color.a);
            SDL_RenderFillRect(renderer, &layout->btn_add_channel);
        }
        draw_text(renderer, font_emoji, "➕", layout->btn_add_channel.x, layout->btn_add_channel.y, is_add_hovered ? color_white : color_muted);

        // Émoji : Transfert de Fichier ➕ (dans la barre)
        int is_file_hovered = (mx >= layout->btn_file_transfer.x && mx <= layout->btn_file_transfer.x + layout->btn_file_transfer.w &&
                               my >= layout->btn_file_transfer.y && my <= layout->btn_file_transfer.y + layout->btn_file_transfer.h);
        if (is_file_hovered && !layout->show_create_modal)
        {
            SDL_SetRenderDrawColor(renderer, hover_bg_color.r, hover_bg_color.g, hover_bg_color.b, hover_bg_color.a);
            SDL_RenderFillRect(renderer, &layout->btn_file_transfer);
        }
        draw_text(renderer, font_emoji, "➕", layout->btn_file_transfer.x + 6, layout->btn_file_transfer.y + 6, is_file_hovered ? color_white : color_muted);

        // Émoji : Envoyer le message ➡️
        int send_x = layout->chat_input_bar.x + layout->chat_input_bar.w - 45;
        int send_y = layout->window_h - 56;
        SDL_Rect btn_send_rect = {send_x, send_y, 36, 36};
        int is_send_hovered = (mx >= send_x && mx <= send_x + 36 && my >= send_y && my <= send_y + 36);
        if (is_send_hovered && !layout->show_create_modal)
        {
            SDL_SetRenderDrawColor(renderer, hover_bg_color.r, hover_bg_color.g, hover_bg_color.b, hover_bg_color.a);
            SDL_RenderFillRect(renderer, &btn_send_rect);
        }
        draw_text(renderer, font_emoji, "➡️", send_x + 6, send_y + 6, is_send_hovered ? color_white : color_muted);

        // Émoji : Bouton Appel 📞
        int is_call_hovered = (mx >= layout->btn_call.x && mx <= layout->btn_call.x + layout->btn_call.w &&
                               my >= layout->btn_call.y && my <= layout->btn_call.y + layout->btn_call.h);
        if (is_call_hovered && !layout->show_create_modal)
        {
            SDL_SetRenderDrawColor(renderer, hover_bg_color.r, hover_bg_color.g, hover_bg_color.b, hover_bg_color.a);
            SDL_RenderFillRect(renderer, &layout->btn_call);
        }

        // Au survol, le bouton devient blanc, sinon il reste grisé
        SDL_Color call_color = is_call_hovered ? color_white : color_muted;

        // On affiche l'icône d'appel
        draw_text(renderer, font_emoji, "📞", layout->btn_call.x + 6, layout->btn_call.y + 6, call_color);

        // Bouton : Log Out 
        int is_logout_hovered = (mx >= layout->btn_logout.x && mx <= layout->btn_logout.x + layout->btn_logout.w &&
                                 my >= layout->btn_logout.y && my <= layout->btn_logout.y + layout->btn_logout.h);
        if (is_logout_hovered)
        {
            SDL_SetRenderDrawColor(renderer, 240, 71, 71, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 170, 30, 30, 255);
        }
        SDL_RenderFillRect(renderer, &layout->btn_logout);
        draw_text(renderer, font_sub, "Log Out", layout->btn_logout.x + 50, layout->btn_logout.y + 10, color_white);
    }

    // 6. Modale de création
    if (layout->show_create_modal)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, NULL);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_SetRenderDrawColor(renderer, 0x31, 0x33, 0x38, 0xFF);
        SDL_RenderFillRect(renderer, &modal_bg_rect);

        draw_text(renderer, font_title, "Create a Channel", modal_bg_rect.x + 30, modal_bg_rect.y + 25, color_white);
        draw_text(renderer, font_sub, "CHANNEL NAME", modal_bg_rect.x + 30, modal_bg_rect.y + 70, color_muted);

        SDL_SetRenderDrawColor(renderer, 0x1E, 0x1F, 0x22, 0xFF);
        SDL_RenderFillRect(renderer, &modal_input_rect);

        if (layout->modal_focused_field)
        {
            SDL_SetRenderDrawColor(renderer, 0x58, 0x65, 0xF2, 0xFF);
            SDL_RenderDrawRect(renderer, &modal_input_rect);
            SDL_Rect interior_modal = {modal_input_rect.x + 1, modal_input_rect.y + 1, modal_input_rect.w - 2, modal_input_rect.h - 2};
            SDL_RenderDrawRect(renderer, &interior_modal);
        }

        int modal_text_width = 0;
        if (strlen(layout->modal_buffer) == 0)
        {
            draw_text(renderer, font_main, "new-channel", modal_input_rect.x + 12, modal_input_rect.y + 10, color_muted);
        }
        else
        {
            draw_text(renderer, font_main, layout->modal_buffer, modal_input_rect.x + 12, modal_input_rect.y + 10, color_white);
            TTF_SizeText(font_main, layout->modal_buffer, &modal_text_width, NULL);
        }

        if (layout->modal_focused_field && (SDL_GetTicks() / 500) % 2 == 0)
        {
            int m_cursor_x = modal_input_rect.x + 12 + modal_text_width;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, m_cursor_x, modal_input_rect.y + 10, m_cursor_x, modal_input_rect.y + 30);
        }

        // On dessine le cadenas d'un côté avec la police emoji, et le texte à côté avec la police principale
        draw_text(renderer, font_emoji, "🔒", modal_bg_rect.x + 30, modal_bg_rect.y + 162, color_white);
        draw_text(renderer, font_main, "Private Channel", modal_bg_rect.x + 60, modal_bg_rect.y + 162, color_white); // décalé à +60 pour laisser la place au cadenas

        if (layout->modal_is_private)
        {
            SDL_SetRenderDrawColor(renderer, 0x23, 0xA5, 0x5A, 0xFF);
            SDL_RenderFillRect(renderer, &modal_toggle_rect);
            SDL_Rect knob = {modal_toggle_rect.x + 26, modal_toggle_rect.y + 3, 20, 20};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &knob);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0x80, 0x84, 0x8E, 0xFF);
            SDL_RenderFillRect(renderer, &modal_toggle_rect);
            SDL_Rect knob = {modal_toggle_rect.x + 4, modal_toggle_rect.y + 3, 20, 20};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &knob);
        }

        SDL_Rect footer_rect = {modal_bg_rect.x, modal_bg_rect.y + 215, modal_bg_rect.w, 85};
        SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF);
        SDL_RenderFillRect(renderer, &footer_rect);

        draw_text(renderer, font_main, "Cancel", modal_btn_cancel.x + 20, modal_btn_cancel.y + 10, color_white);

        SDL_SetRenderDrawColor(renderer, 0x58, 0x65, 0xF2, 0xFF);
        SDL_RenderFillRect(renderer, &modal_btn_ok);
        draw_text(renderer, font_main, "Create", modal_btn_ok.x + 25, modal_btn_ok.y + 10, color_white);
    }

    // 7. Menu contextuel "Ban User" / "Unban User" (panneau des membres)
    if (layout->show_user_context_menu)
    {
        layout->btn_ban_user_rect = (SDL_Rect){layout->context_menu_x, layout->context_menu_y, 140, 32};

        SDL_SetRenderDrawColor(renderer, 0x2B, 0x2D, 0x31, 0xFF);
        SDL_RenderFillRect(renderer, &layout->btn_ban_user_rect);
        SDL_SetRenderDrawColor(renderer, 0x3F, 0x41, 0x47, 0xFF);
        SDL_RenderDrawRect(renderer, &layout->btn_ban_user_rect);

        if (layout->context_menu_is_unban)
        {
            SDL_Color unban_color = {0x3B, 0xA5, 0x5C, 0xFF};
            draw_text(renderer, font_main, "Unban User", layout->btn_ban_user_rect.x + 14,
                     layout->btn_ban_user_rect.y + 8, unban_color);
        }
        else
        {
            SDL_Color ban_color = {0xF0, 0x47, 0x47, 0xFF};
            draw_text(renderer, font_main, "Ban User", layout->btn_ban_user_rect.x + 14,
                     layout->btn_ban_user_rect.y + 8, ban_color);
        }
    }
}

int run_chat_loop(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font_title, TTF_Font *font_main, TTF_Font *font_sub)
{
    ChatLayout layout;
    memset(&layout, 0, sizeof(ChatLayout));

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ui_chat_handle_resize(&layout, w, h);

    chat_controller_init(&layout, renderer);

    int running = 1;
    int exit_status = 1;
    SDL_Event event;

    SDL_StartTextInput();

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
                exit_status = 1;
            }
            else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    ui_chat_handle_resize(&layout, event.window.data1, event.window.data2);
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                chat_controller_update_hover(&layout, event.motion.x, event.motion.y);

                int mx = event.motion.x, my = event.motion.y;
                int over_input_field = 0;

                if (mx >= layout.chat_input_bar.x && mx <= layout.chat_input_bar.x + layout.chat_input_bar.w &&
                    my >= layout.chat_input_bar.y && my <= layout.chat_input_bar.y + layout.chat_input_bar.h)
                {
                    over_input_field = 1;
                }
                if (layout.show_create_modal &&
                    mx >= modal_input_rect.x && mx <= modal_input_rect.x + modal_input_rect.w &&
                    my >= modal_input_rect.y && my <= modal_input_rect.y + modal_input_rect.h)
                {
                    over_input_field = 1;
                }

                SDL_Cursor *cursor;
                if (over_input_field)
                {
                    cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
                }
                else
                {
                    cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
                }
                SDL_SetCursor(cursor);
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int cx = event.button.x, cy = event.button.y;

                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int status = chat_controller_handle_left_click(&layout, cx, cy);
                    if (status == 2)
                    {
                        running = 0;
                        exit_status = 2;
                    }
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    chat_controller_handle_right_click(&layout, cx, cy);
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                chat_controller_handle_keydown(&layout, event.key.keysym.sym);
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                chat_controller_handle_textinput(&layout, event.text.text);
            }
            else if (event.type == SDL_MOUSEWHEEL)
            {
                chat_controller_handle_mousewheel(&layout, event.wheel.y);
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_chat_interface(renderer, &layout, font_title, font_main, font_sub);
        SDL_RenderPresent(renderer);
    }

    chat_controller_destroy(&layout);
    return exit_status;
}
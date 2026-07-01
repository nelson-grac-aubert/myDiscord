#include "chat_controller.h"
#include "auth_controller.h"
#include "channel.h"
#include "message.h"
#include "user.h"
#include "ui_call.h"
#include "ui_channels.h"
#include "client_socket.h"
#include "packet.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

static SDL_Renderer *current_renderer = NULL;
static int g_first_channel_joined = 0;

/* Tracks a CHANNEL_CREATE request awaiting its SERVER_OK reply, so the
   channel can be added locally with the server-assigned id instead of a
   client-guessed one */
static int  g_pending_create = 0;
static char g_pending_create_name[MAX_NAME_LENGTH];
static int  g_pending_create_is_private = 0;

/* Tracks a CHANNEL_DELETE request awaiting its SERVER_OK reply, so the
   channel is only removed locally once the server actually deleted it
   (e.g. rejects deletion by a non-creator) */
static int g_pending_delete = 0;
static int g_pending_delete_id = -1;

extern SDL_Rect modal_bg_rect, modal_input_rect, modal_toggle_rect,
               modal_btn_ok, modal_btn_cancel;

static char *open_file_explorer(void);

static void local_time_hhmm(char *out, size_t out_size)
{
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    strftime(out, out_size, "%H:%M", tm_now);
}

static void request_channel_join_and_history(int channel_id)
{
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", channel_id);

    Packet join;
    packet_build(&join, CHANNEL_JOIN, 1, id_str);
    client_socket_send(&g_client_socket, &join);

    char lim_str[8];
    snprintf(lim_str, sizeof(lim_str), "50");
    Packet hist;
    packet_build(&hist, MSG_HISTORY, 2, id_str, lim_str);
    client_socket_send(&g_client_socket, &hist);
}

static void request_user_list(void)
{
    Packet pkt;
    packet_build(&pkt, USER_LIST, 0);
    client_socket_send(&g_client_socket, &pkt);
}

static void on_server_push(const Packet *pkt)
{
    /* A pending CHANNEL_CREATE is acked/rejected by the next SERVER_OK/ERROR */
    if (g_pending_create) {
        if (pkt->type == SERVER_OK) {
            int id = pkt->field_count > 0 ? atoi(pkt->fields[0]) : -1;
            if (id > 0)
                channel_model_add(id, g_pending_create_name, g_pending_create_is_private);
            g_pending_create = 0;
            return;
        } else if (pkt->type == SERVER_ERROR) {
            g_pending_create = 0;
            return;
        }
    }

    /* A pending CHANNEL_DELETE is acked/rejected by the next SERVER_OK/ERROR;
       only remove the channel locally once the server confirms it (a
       non-creator's delete request is rejected server-side) */
    if (g_pending_delete) {
        if (pkt->type == SERVER_OK) {
            channel_model_delete_by_id(g_pending_delete_id);
            g_pending_delete = 0;
            return;
        } else if (pkt->type == SERVER_ERROR) {
            g_pending_delete = 0;
            return;
        }
    }

    /* Ignore other server acknowledgments */
    if (pkt->type == SERVER_OK || pkt->type == SERVER_ERROR)
        return;

    if (pkt->field_count < 1)
        return;

    const char *payload = pkt->fields[0];

    /* CHANNEL_LIST push: "CHAN:id|name|is_private" */
    if (strncmp(payload, "CHAN:", 5) == 0) {
        const char *data = payload + 5;
        char buf[100];
        strncpy(buf, data, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        char *p1 = strtok(buf, "|");
        char *p2 = strtok(NULL, "|");
        char *p3 = strtok(NULL, "|");

        if (p1 && p2 && p3) {
            int id = atoi(p1);
            int is_private = (p3[0] == '1' || p3[0] == 't');
            int found = 0;
            for (int i = 0; i < channel_model_get_count(); i++) {
                Channel *ch = channel_model_get_by_index(i);
                if (ch && ch->id == id) { found = 1; break; }
            }
            if (!found) {
                channel_model_add(id, p2, is_private);

                /* Auto-join the first channel received */
                if (!g_first_channel_joined) {
                    g_first_channel_joined = 1;
                    request_channel_join_and_history(id);
                }
            }
        }
        return;
    }

    /* Live channel roster push: "USERS:email1;email2;..." - a full
       snapshot of who's currently online in that channel, so it always
       replaces whatever roster we had rather than appending to it */
    if (strncmp(payload, "USERS:", 6) == 0) {
        const char *data = payload + 6;
        char buf[PACKET_FIELD_SIZE];
        strncpy(buf, data, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        user_model_init();

        int next_id = 1;
        char *tok = strtok(buf, ";");
        while (tok != NULL) {
            if (strlen(tok) > 0)
                user_model_add(next_id++, tok, 1);
            tok = strtok(NULL, ";");
        }
        return;
    }

    /* Message deletion push: "DELMSG:message_id" - broadcast to everyone in
       the channel (including whoever asked for the delete), so the message
       only disappears once the server actually confirmed it */
    if (strncmp(payload, "DELMSG:", 7) == 0) {
        message_model_delete_by_id(atoi(payload + 7));
        return;
    }

    /* Regular message push: "channel_id|message_id|HH:MM|username|content" */
    char raw[PACKET_FIELD_SIZE];
    strncpy(raw, payload, sizeof(raw) - 1);
    raw[sizeof(raw) - 1] = '\0';

    char *sep1 = strchr(raw, '|');
    if (!sep1)
        return;
    *sep1 = '\0';
    int channel_id = atoi(raw);
    char *rest = sep1 + 1;

    char *sep2 = strchr(rest, '|');
    if (!sep2)
        return;
    *sep2 = '\0';
    int message_id = atoi(rest);
    rest = sep2 + 1;

    char *sep3 = strchr(rest, '|');
    if (!sep3)
        return;
    *sep3 = '\0';
    const char *timestamp = rest;
    rest = sep3 + 1;

    char *sep4 = strchr(rest, '|');
    if (sep4) {
        *sep4 = '\0';
        message_model_add(message_id, channel_id, rest, sep4 + 1, timestamp);
    } else {
        message_model_add(message_id, channel_id, "unknown", rest, timestamp);
    }
}

static void send_message(ChatLayout *layout, Channel *active)
{
    if (!active || strlen(layout->input_buffer) == 0)
        return;

    char channel_id_str[16];
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", active->id);

    Packet pkt;
    if (packet_build(&pkt, MSG_SEND, 2, channel_id_str, layout->input_buffer) != 0)
        return;

    if (client_socket_send(&g_client_socket, &pkt) != 0) {
        fprintf(stderr, "[chat] send failed\n");
        return;
    }

    layout->input_buffer[0] = '\0';
}

static void request_channel_list(void)
{
    Packet pkt;
    packet_build(&pkt, CHANNEL_LIST, 0);
    client_socket_send(&g_client_socket, &pkt);
}

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
    g_first_channel_joined = 0;
    g_pending_create = 0;
    g_pending_delete = 0;

    auth_controller_set_chat_callback(on_server_push);

    /* Request channel list; first received channel will be auto-joined */
    request_channel_list();

    /* Online-users roster is server-wide, independent of channel, so it's
       requested once here rather than on every channel switch */
    request_user_list();
}

void chat_controller_destroy(ChatLayout *layout) { (void)layout; }
void chat_controller_update_hover(ChatLayout *layout, int mx, int my)
{ (void)layout; (void)mx; (void)my; }

int chat_controller_handle_left_click(ChatLayout *layout, int cx, int cy)
{
    /* 1. Modal de création de salon */
    if (layout->show_create_modal) {
        if (cx >= modal_input_rect.x && cx <= modal_input_rect.x + modal_input_rect.w &&
            cy >= modal_input_rect.y && cy <= modal_input_rect.y + modal_input_rect.h)
        { layout->modal_focused_field = 1; return 0; }
        else
            layout->modal_focused_field = 0;

        if (cx >= modal_toggle_rect.x && cx <= modal_toggle_rect.x + modal_toggle_rect.w &&
            cy >= modal_toggle_rect.y && cy <= modal_toggle_rect.y + modal_toggle_rect.h)
        { layout->modal_is_private = !layout->modal_is_private; return 0; }

        if (cx >= modal_btn_cancel.x && cx <= modal_btn_cancel.x + modal_btn_cancel.w &&
            cy >= modal_btn_cancel.y && cy <= modal_btn_cancel.y + modal_btn_cancel.h)
        { layout->show_create_modal = 0; layout->modal_buffer[0] = '\0'; return 0; }

        if (cx >= modal_btn_ok.x && cx <= modal_btn_ok.x + modal_btn_ok.w &&
            cy >= modal_btn_ok.y && cy <= modal_btn_ok.y + modal_btn_ok.h) {
            if (strlen(layout->modal_buffer) > 0) {
                char priv_str[2];
                snprintf(priv_str, sizeof(priv_str), "%d", layout->modal_is_private);
                Packet pkt;
                packet_build(&pkt, CHANNEL_CREATE, 2, layout->modal_buffer, priv_str);
                client_socket_send(&g_client_socket, &pkt);

                strncpy(g_pending_create_name, layout->modal_buffer, MAX_NAME_LENGTH - 1);
                g_pending_create_name[MAX_NAME_LENGTH - 1] = '\0';
                g_pending_create_is_private = layout->modal_is_private;
                g_pending_create = 1;

                layout->show_create_modal = 0;
                layout->modal_buffer[0] = '\0';
            }
            return 0;
        }
        return 0;
    }

    /* 2. Bouton logout */
    if (cx >= layout->btn_logout.x && cx <= layout->btn_logout.x + layout->btn_logout.w &&
        cy >= layout->btn_logout.y && cy <= layout->btn_logout.y + layout->btn_logout.h)
        return 2;

    /* 3. Bouton ajout salon */
    if (cx >= layout->btn_add_channel.x && cx <= layout->btn_add_channel.x + layout->btn_add_channel.w &&
        cy >= layout->btn_add_channel.y && cy <= layout->btn_add_channel.y + layout->btn_add_channel.h) {
        layout->show_create_modal = 1;
        layout->modal_focused_field = 1;
        layout->modal_buffer[0] = '\0';
        layout->modal_is_private = 0;
        return 0;
    }

    /* 4. Bouton appel */
    if (cx >= layout->btn_call.x && cx <= layout->btn_call.x + layout->btn_call.w &&
        cy >= layout->btn_call.y && cy <= layout->btn_call.y + layout->btn_call.h) {
        ouvrir_fenetre_appel(current_renderer, font_title, font_main, font_sub, font_emoji,
                             layout->window_w, layout->window_h);
        return 1;
    }

    /* 5. Bouton transfert de fichier */
    if (cx >= layout->btn_file_transfer.x && cx <= layout->btn_file_transfer.x + layout->btn_file_transfer.w &&
        cy >= layout->btn_file_transfer.y && cy <= layout->btn_file_transfer.y + layout->btn_file_transfer.h) {
        char *file_path = open_file_explorer();
        if (file_path && strlen(file_path) > 0) {
            Channel *active = channel_model_get_active();
            if (active) {
                char ts[8];
                local_time_hhmm(ts, sizeof(ts));
                message_model_add(0, active->id, "Me", file_path, ts);
            }
        }
        return 1;
    }

    /* 6. Bouton envoyer */
    int send_x = layout->chat_input_bar.x + layout->chat_input_bar.w - 45;
    int send_y = layout->window_h - 56;
    if (cx >= send_x && cx <= send_x + 36 && cy >= send_y && cy <= send_y + 36) {
        Channel *active = channel_model_get_active();
        send_message(layout, active);
        return 0;
    }

    /* 7. Suppression de salon */
    if (layout->hover_channel_delete_index != -1 &&
        cx >= layout->btn_delete_channel_rect.x &&
        cx <= layout->btn_delete_channel_rect.x + layout->btn_delete_channel_rect.w &&
        cy >= layout->btn_delete_channel_rect.y &&
        cy <= layout->btn_delete_channel_rect.y + layout->btn_delete_channel_rect.h) {
        Channel *target = channel_model_get_by_index(layout->hover_channel_delete_index);
        if (target) {
            char id_str[16];
            snprintf(id_str, sizeof(id_str), "%d", target->id);

            Packet pkt;
            packet_build(&pkt, CHANNEL_DELETE, 1, id_str);
            client_socket_send(&g_client_socket, &pkt);

            g_pending_delete_id = target->id;
            g_pending_delete = 1;
        }
        layout->hover_channel_delete_index = -1;
        return 0;
    }

    /* 8. Sélection d'un salon */
    if (cx >= layout->sidebar_channels.x &&
        cx <= layout->sidebar_channels.x + layout->sidebar_channels.w) {
        int channel_y = 60;
        for (int i = 0; i < channel_model_get_count(); i++) {
            SDL_Rect r = {layout->sidebar_channels.x + 8, channel_y - 4,
                          layout->sidebar_channels.w - 16, 28};
            if (cx >= r.x && cx <= r.x + r.w && cy >= r.y && cy <= r.y + r.h) {
                channel_model_set_active_index(i);
                Channel *ch = channel_model_get_by_index(i);
                if (ch)
                    request_channel_join_and_history(ch->id);
                return 0;
            }
            channel_y += 32;
        }
    }

    /* 9. Suppression de message */
    if (layout->hover_message_delete_index != -1 &&
        cx >= layout->btn_delete_message_rect.x &&
        cx <= layout->btn_delete_message_rect.x + layout->btn_delete_message_rect.w &&
        cy >= layout->btn_delete_message_rect.y &&
        cy <= layout->btn_delete_message_rect.y + layout->btn_delete_message_rect.h) {
        Channel *active = channel_model_get_active();
        if (active) {
            int msg_id = message_model_get_id_by_index_in_channel(active->id,
                layout->hover_message_delete_index);
            /* Only ask the server to delete; the message only disappears
               once the DELMSG broadcast confirms it (e.g. rejects deleting
               someone else's message) */
            if (msg_id > 0) {
                char id_str[16];
                snprintf(id_str, sizeof(id_str), "%d", msg_id);
                Packet pkt;
                packet_build(&pkt, MSG_DELETE, 1, id_str);
                client_socket_send(&g_client_socket, &pkt);
            }
        }
        layout->hover_message_delete_index = -1;
        return 0;
    }

    /* 10. Focus input */
    layout->is_input_focused =
        (cx >= layout->chat_input_bar.x &&
         cx <= layout->chat_input_bar.x + layout->chat_input_bar.w &&
         cy >= layout->chat_input_bar.y &&
         cy <= layout->chat_input_bar.y + layout->chat_input_bar.h) ? 1 : 0;
    return 0;
}

void chat_controller_handle_keydown(ChatLayout *layout, SDL_Keycode sym)
{
    if (layout->show_create_modal && layout->modal_focused_field) {
        if (sym == SDLK_BACKSPACE) {
            size_t len = strlen(layout->modal_buffer);
            if (len > 0) layout->modal_buffer[len - 1] = '\0';
        } else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
            if (strlen(layout->modal_buffer) > 0) {
                char priv_str[2];
                snprintf(priv_str, sizeof(priv_str), "%d", layout->modal_is_private);
                Packet pkt;
                packet_build(&pkt, CHANNEL_CREATE, 2, layout->modal_buffer, priv_str);
                client_socket_send(&g_client_socket, &pkt);

                strncpy(g_pending_create_name, layout->modal_buffer, MAX_NAME_LENGTH - 1);
                g_pending_create_name[MAX_NAME_LENGTH - 1] = '\0';
                g_pending_create_is_private = layout->modal_is_private;
                g_pending_create = 1;

                layout->show_create_modal = 0;
                layout->modal_buffer[0] = '\0';
            }
        }
        return;
    }

    if (sym == SDLK_BACKSPACE && layout->is_input_focused) {
        size_t len = strlen(layout->input_buffer);
        if (len > 0) layout->input_buffer[len - 1] = '\0';
        return;
    }

    if ((sym == SDLK_RETURN || sym == SDLK_KP_ENTER) &&
        layout->is_input_focused && strlen(layout->input_buffer) > 0) {
        Channel *active = channel_model_get_active();
        send_message(layout, active);
    }
}

void chat_controller_handle_textinput(ChatLayout *layout, const char *text)
{
    if (layout->show_create_modal && layout->modal_focused_field) {
        if (strlen(layout->modal_buffer) + strlen(text) < sizeof(layout->modal_buffer) - 1)
            strcat(layout->modal_buffer, text);
        return;
    }
    if (layout->is_input_focused) {
        if (strlen(layout->input_buffer) + strlen(text) < MAX_MSG_LENGTH - 1)
            strcat(layout->input_buffer, text);
    }
}

int chat_controller_is_mic_muted(void) { return g_is_mic_muted; }

void chat_controller_handle_right_click(ChatLayout *layout, int cx, int cy)
{ (void)layout; (void)cx; (void)cy; }

void chat_controller_handle_menu_action(ChatLayout *layout, int cx, int cy)
{ (void)layout; (void)cx; (void)cy; }

static char *open_file_explorer(void)
{
#ifdef _WIN32
    OPENFILENAMEW ofn;
    static wchar_t szFileW[260];
    static char szFileUTF8[512];
    memset(szFileW, 0, sizeof(szFileW));
    memset(szFileUTF8, 0, sizeof(szFileUTF8));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.lpstrFile    = szFileW;
    ofn.nMaxFile     = 260;
    ofn.lpstrFilter  = L"Images\0*.png;*.jpg;*.jpeg\0All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&ofn) == TRUE) {
        WideCharToMultiByte(CP_UTF8, 0, szFileW, -1, szFileUTF8, sizeof(szFileUTF8), NULL, NULL);
        return szFileUTF8;
    }
    return NULL;
#else
    return NULL;
#endif
}
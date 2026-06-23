#include "../include/ui_chat_store.h"
#include <string.h>

static LocalChannel local_channels[MAX_CHANNELS];
static int local_channel_count = 0;
static int local_active_channel_index = 0;

void store_init(void) {
    local_channel_count = 0;
    local_active_channel_index = 0;

    store_add_channel("général", 0);
    store_add_channel("gaming", 0);
    store_add_channel("privé-staff", 1);

    store_add_message_to_active("Nexus_One", "Salut tout le monde ! Bienvenue sur myDiscord !");
    store_add_message_to_active("SARA_X", "Hey ! Stylé le nouveau thème noir/bleu nuit.");
}

int store_get_channel_count(void) { return local_channel_count; }
int store_get_active_index(void) { return local_active_channel_index; }
void store_set_active_index(int index) {
    if (index >= 0 && index < local_channel_count) {
        local_active_channel_index = index;
    }
}

LocalChannel* store_get_active_channel(void) {
    if (local_channel_count == 0) return NULL;
    return &local_channels[local_active_channel_index];
}

LocalChannel* store_get_channel(int index) {
    if (index < 0 || index >= local_channel_count) return NULL;
    return &local_channels[index];
}

int store_add_channel(const char* name, int is_private) {
    if (local_channel_count >= MAX_CHANNELS) return 0;
    LocalChannel *ch = &local_channels[local_channel_count];
    strncpy(ch->name, name, MAX_NAME_LENGTH - 1);
    ch->name[MAX_NAME_LENGTH - 1] = '\0';
    ch->message_count = 0;
    ch->is_private = is_private;
    local_channel_count++;
    return 1;
}

void store_delete_last_channel(void) {
    if (local_channel_count > 1) {
        local_channel_count--;
        if (local_active_channel_index >= local_channel_count) {
            local_active_channel_index = local_channel_count - 1;
        }
    }
}

void store_delete_channel_by_index(int index) {
    if (local_channel_count <= 1 || index < 0 || index >= local_channel_count) return;
    for (int i = index; i < local_channel_count - 1; i++) {
        local_channels[i] = local_channels[i + 1];
    }
    local_channel_count--;
    if (local_active_channel_index >= local_channel_count) {
        local_active_channel_index = local_channel_count - 1;
    }
}

void store_add_message_to_active(const char* username, const char* text) {
    LocalChannel *ch = store_get_active_channel();
    if (!ch) return;
    if (ch->message_count >= MAX_MESSAGES) {
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            ch->messages[i] = ch->messages[i + 1];
        }
        ch->message_count = MAX_MESSAGES - 1;
    }
    ChatMessage *msg = &ch->messages[ch->message_count];
    strncpy(msg->username, username, 31);
    msg->username[31] = '\0';
    strncpy(msg->text, text, MAX_MSG_LENGTH - 1);
    msg->text[MAX_MSG_LENGTH - 1] = '\0';
    ch->message_count++;
}

void store_delete_last_message_from_active(void) {
    LocalChannel *ch = store_get_active_channel();
    if (ch && ch->message_count > 0) {
        ch->message_count--;
    }
}

void store_delete_message_by_index(int msg_index) {
    LocalChannel *ch = store_get_active_channel();
    if (!ch || msg_index < 0 || msg_index >= ch->message_count) return;
    for (int i = msg_index; i < ch->message_count - 1; i++) {
        ch->messages[i] = ch->messages[i + 1];
    }
    ch->message_count--;
}
#include "../include/message.h"
#include <string.h>

static Message global_messages[MAX_MESSAGES];
static int global_message_count = 0;

void message_model_init(void) {
    global_message_count = 0;
}

int message_model_get_for_channel(int channel_id, Message* out_messages, int max_out) {
    int count = 0;
    for (int i = 0; i < global_message_count; i++) {
        if (global_messages[i].channel_id == channel_id) {
            if (count < max_out) {
                out_messages[count] = global_messages[i];
                count++;
            }
        }
    }
    return count;
}

void message_model_add(int id, int channel_id, const char* username, const char* text) {
    /* Real server-assigned ids (id > 0) can re-arrive via a later MSG_HISTORY
       fetch (e.g. re-joining a channel already viewed this session); skip
       re-adding a message we already have. Locally-originated placeholder
       entries (id == 0, e.g. file transfers) are never deduped this way. */
    if (id > 0) {
        for (int i = 0; i < global_message_count; i++) {
            if (global_messages[i].id == id)
                return;
        }
    }

    if (global_message_count >= MAX_MESSAGES) {
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            global_messages[i] = global_messages[i + 1];
        }
        global_message_count = MAX_MESSAGES - 1;
    }

    Message *msg = &global_messages[global_message_count];
    msg->id = id;
    msg->channel_id = channel_id;
    strncpy(msg->username, username, 31);
    msg->username[31] = '\0';
    strncpy(msg->text, text, MAX_MSG_LENGTH - 1);
    msg->text[MAX_MSG_LENGTH - 1] = '\0';

    global_message_count++;
}

void message_model_delete_by_index_in_channel(int channel_id, int index_in_channel) {
    int current_match = 0;
    int target_global_index = -1;

    for (int i = 0; i < global_message_count; i++) {
        if (global_messages[i].channel_id == channel_id) {
            if (current_match == index_in_channel) {
                target_global_index = i;
                break;
            }
            current_match++;
        }
    }

    if (target_global_index != -1) {
        for (int i = target_global_index; i < global_message_count - 1; i++) {
            global_messages[i] = global_messages[i + 1];
        }
        global_message_count--;
    }
}
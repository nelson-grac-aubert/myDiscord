#include "../include/channel.h"
#include <string.h>

static Channel local_channels[MAX_CHANNELS];
static int local_channel_count = 0;
static int local_active_channel_index = 0;

void channel_model_init(void) {
    local_channel_count = 0;
    local_active_channel_index = 0;

    // Salons par défaut (ID arbitraires pour le moment, correspondants à la future DB)
    channel_model_add(1, "général", 0);
    channel_model_add(2, "gaming", 0);
    channel_model_add(3, "privé-staff", 1);
}

int channel_model_get_count(void) {
    return local_channel_count;
}

int channel_model_get_active_index(void) {
    return local_active_channel_index;
}

void channel_model_set_active_index(int index) {
    if (index >= 0 && index < local_channel_count) {
        local_active_channel_index = index;
    }
}

Channel* channel_model_get_active(void) {
    if (local_channel_count == 0) return NULL;
    return &local_channels[local_active_channel_index];
}

Channel* channel_model_get_by_index(int index) {
    if (index >= 0 && index < local_channel_count) {
        return &local_channels[index];
    }
    return NULL;
}

int channel_model_add(int id, const char* name, int is_private) {
    if (local_channel_count >= MAX_CHANNELS) return -1;

    Channel *ch = &local_channels[local_channel_count];
    ch->id = id;
    strncpy(ch->name, name, MAX_NAME_LENGTH - 1);
    ch->name[MAX_NAME_LENGTH - 1] = '\0';
    ch->is_private = is_private;

    local_channel_count++;
    return 0;
}

void channel_model_delete_by_index(int index) {
    if (local_channel_count <= 1 || index < 0 || index >= local_channel_count) return;

    for (int i = index; i < local_channel_count - 1; i++) {
        local_channels[i] = local_channels[i + 1];
    }
    local_channel_count--;

    if (local_active_channel_index >= local_channel_count) {
        local_active_channel_index = local_channel_count - 1;
    }
}
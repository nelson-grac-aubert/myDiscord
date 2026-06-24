#ifndef MESSAGE_MODEL_H
#define MESSAGE_MODEL_H

#define MAX_MSG_LENGTH 256
#define MAX_MESSAGES   100

typedef struct {
    int id;
    int channel_id;
    char username[32];
    char text[MAX_MSG_LENGTH];
} Message;

// Initializes the local message cache with sample data
void message_model_init(void);

// Retrieves all messages bound to a specific channel (via channel_id)
int message_model_get_for_channel(int channel_id, Message* out_messages, int max_out);

// Logical actions on messages
void message_model_add(int id, int channel_id, const char* username, const char* text);
void message_model_delete_by_index_in_channel(int channel_id, int index_in_channel);

#endif /* MESSAGE_MODEL_H */
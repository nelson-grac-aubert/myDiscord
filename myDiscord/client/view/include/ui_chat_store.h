#ifndef UI_CHAT_STORE_H
#define UI_CHAT_STORE_H

#define MAX_MSG_LENGTH 256
#define MAX_MESSAGES 50
#define MAX_CHANNELS 10
#define MAX_NAME_LENGTH 32

typedef struct {
    char username[32];
    char text[MAX_MSG_LENGTH];
} ChatMessage;

typedef struct {
    char name[MAX_NAME_LENGTH];
    ChatMessage messages[MAX_MESSAGES];
    int message_count;
    int is_private; // 0 = Public, 1 = Privé
} LocalChannel;

void store_init(void);
int store_get_channel_count(void);
int store_get_active_index(void);
void store_set_active_index(int index);
LocalChannel* store_get_active_channel(void);
LocalChannel* store_get_channel(int index);

int store_add_channel(const char* name, int is_private);
void store_delete_last_channel(void);
void store_add_message_to_active(const char* username, const char* text);
void store_delete_last_message_from_active(void);

// ➡️ AJOUTS POUR LES SUPPRESSIONS PAR CLIC DROIT
void store_delete_channel_by_index(int index);
void store_delete_message_by_index(int msg_index);

#endif /* UI_CHAT_STORE_H */
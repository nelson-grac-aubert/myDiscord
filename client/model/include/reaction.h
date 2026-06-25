#ifndef REACTION_MODEL_H
#define REACTION_MODEL_H

#define MAX_REACTIONS 200

typedef struct {
    int message_id;
    int user_id;
    char emoji[8]; // Contains the UTF-8 emoji string (e.g., "👍", "🔥")
} Reaction;

// Initializes the local reaction cache
void reaction_model_init(void);

// Retrieves all reactions attached to a specific message ID
int reaction_model_get_for_message(int message_id, Reaction* out_reactions, int max_out);

// Adds or removes a reaction
void reaction_model_add(int message_id, int user_id, const char* emoji);
void reaction_model_remove(int message_id, int user_id, const char* emoji);

#endif /* REACTION_MODEL_H */
#ifndef REACTION_MODEL_H
#define REACTION_MODEL_H

#define MAX_REACTIONS 200

typedef struct {
    int message_id;
    int user_id;
    char emoji[8]; // Contient la chaîne UTF-8 de l'emoji (ex: "👍", "🔥")
} Reaction;

void reaction_model_init(void);

// Récupère toutes les réactions rattachées à un ID de message
int reaction_model_get_for_message(int message_id, Reaction* out_reactions, int max_out);

// Ajoute ou retire une réaction
void reaction_model_add(int message_id, int user_id, const char* emoji);
void reaction_model_remove(int message_id, int user_id, const char* emoji);

#endif /* REACTION_MODEL_H */
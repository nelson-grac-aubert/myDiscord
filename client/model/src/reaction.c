#include "../include/reaction.h"
#include <string.h>

static Reaction global_reactions[MAX_REACTIONS];
static int global_reaction_count = 0;

void reaction_model_init(void) {
    global_reaction_count = 0;
}

int reaction_model_get_for_message(int message_id, Reaction* out_reactions, int max_out) {
    int count = 0;
    for (int i = 0; i < global_reaction_count; i++) {
        if (global_reactions[i].message_id == message_id) {
            if (count < max_out) {
                out_reactions[count] = global_reactions[i];
                count++;
            }
        }
    }
    return count;
}

void reaction_model_add(int message_id, int user_id, const char* emoji) {
    // Évite les doublons exacts (un utilisateur réagit une seule fois avec le même emoji)
    for (int i = 0; i < global_reaction_count; i++) {
        if (global_reactions[i].message_id == message_id &&
            global_reactions[i].user_id == user_id &&
            strcmp(global_reactions[i].emoji, emoji) == 0) {
            return; 
        }
    }

    if (global_reaction_count >= MAX_REACTIONS) return;

    Reaction *r = &global_reactions[global_reaction_count];
    r->message_id = message_id;
    r->user_id = user_id;
    strncpy(r->emoji, emoji, 7);
    r->emoji[7] = '\0';

    global_reaction_count++;
}

void reaction_model_remove(int message_id, int user_id, const char* emoji) {
    for (int i = 0; i < global_reaction_count; i++) {
        if (global_reactions[i].message_id == message_id &&
            global_reactions[i].user_id == user_id &&
            strcmp(global_reactions[i].emoji, emoji) == 0) {
            
            // Décale le reste du tableau pour combler le vide
            for (int j = i; j < global_reaction_count - 1; j++) {
                global_reactions[j] = global_reactions[j + 1];
            }
            global_reaction_count--;
            return;
        }
    }
}
#ifndef CHANNEL_MODEL_H
#define CHANNEL_MODEL_H

#define MAX_CHANNELS    10
#define MAX_NAME_LENGTH 32

typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    int is_private;
} Channel;

// Initialise le modèle et configure les salons de test
void channel_model_init(void);

// Getters d'état pour les salons
int channel_model_get_count(void);
int channel_model_get_active_index(void);
void channel_model_set_active_index(int index);

// Récupération des pointeurs de données
Channel* channel_model_get_active(void);
Channel* channel_model_get_by_index(int index);

// Actions de manipulation
int channel_model_add(int id, const char* name, int is_private);
void channel_model_delete_by_index(int index);

#endif /* CHANNEL_MODEL_H */
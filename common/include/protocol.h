#ifndef PROTOCOL_H
#define PROTOCOL_H

// Les types de paquets requis par ton sujet et ton architecture
typedef enum {
    MSG_LOGIN,
    MSG_LOGIN_SUCCESS,
    MSG_LOGIN_FAILURE,
    MSG_REGISTER,
    MSG_REGISTER_SUCCESS,
    MSG_SEND,              // Envoi de message textuel / fichier
    MSG_RECEIVE,           // Réception temps réel (Broadcast)
    MSG_HISTORY,           // Demande d'historique
    MSG_HISTORY_RESPONSE,
    MSG_NEW_CHANNEL,
    MSG_DELETE_CHANNEL,
    MSG_REACTION,
    MSG_LOGOUT
} PacketType;

// La structure unique qui va transiter dans le tunnel de ton ClientSocket
typedef struct {
    PacketType type;
    int user_id;
    int channel_id;
    int message_id;
    int is_private;        // Pour les canaux (0: public, 1: privé)
    char username[50];
    char email[100];
    char extra_fields[64]; // Utile pour le statut ('En ligne', etc.) ou les emojis
    char payload[1024];    // Le texte du message, le hash du mot de passe, ou l'URL d'un fichier
} Packet;

#endif
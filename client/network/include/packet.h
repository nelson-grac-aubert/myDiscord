/* includeguard : will not be included twice */
#ifndef PACKET_H
#define PACKET_H

/* Maximum size of a raw packet string, max number of fields, max field size */
#define PACKET_MAX_SIZE 2048
#define PACKET_MAX_FIELDS 8
#define PACKET_FIELD_SIZE 512

/* | and \n are used to structure the packet
They cannot be in a string */
#define ESCAPE_PIPE     '\x01'
#define ESCAPE_NEWLINE  '\x02'

/* Packet types */
typedef enum {

    /* authentication */
    AUTH_REGISTER,  /* email | password | username */
    AUTH_LOGIN,     /* email | password */
    AUTH_LOGOUT,
 
    /* messaging */
    MSG_SEND,       /* channel_id | content */
    MSG_HISTORY,    /* channel_id */
    MSG_REACTION,   /* message_id | emoji */
    MSG_DELETE,     /* message_id */

    /* channels */
    CHANNEL_JOIN,   /* channel_id */
    CHANNEL_LEAVE,  /* channel_id */
    CHANNEL_CREATE, /* name | is_private (0 or 1) */
    CHANNEL_DELETE, /* channel_id */
    CHANNEL_LIST, 
 
    /* users */
    USER_BAN,       /* user_id */
    USER_LIST,      /* channel_id */
 
    /* responses */
    SERVER_OK,      /* message_text */
    SERVER_ERROR,   /* error_message */
    SERVER_PUSH,    /* channel_id | author | ts | content */
 
    PACKET_TYPE_COUNT /* used for bounds checking */

} PacketType;

/* Packet structure */
typedef struct {
    PacketType  type;
    int field_count;
    char fields[PACKET_MAX_FIELDS][PACKET_FIELD_SIZE];
} Packet;

/* FUNCTIONS */

/*
 * Build a ready-to-send Packet.
 * type       : the packet type (AUTH_LOGIN, MSG_SEND, etc.)
 * field_count: how many fields follow
 * ...        : field strings (const char *), one per field
 * Returns 0 on success, -1 on failure.
 */
int packet_build(Packet *out, PacketType type, int field_count, ...);

/*
 * Serialize a Packet into a raw string: "TYPE|field0|field1\n"
 * out      : destination buffer
 * out_size : size of the destination buffer
 * Returns 0 on success, -1 on failure.
 */
int packet_serialize(const Packet *pkt, char *out, int out_size);

/*
 * Deserialize a raw string into a Packet.
 * raw : the string received from the socket (must end with '\n')
 * Returns 0 on success, -1 on failure.
 */
int packet_deserialize(const char *raw, Packet *out);

/*
 * Escape '|' and '\n' inside a user-provided field value.
 * Must be called before inserting free text into a Packet field.
 * Returns 0 on success, -1 if dst is too small.
 */
int packet_encode(const char *src, char *dst, int dst_size);

/*
 * Reverse of packet_escape : restore original characters after parsing.
 * Returns 0 on success, -1 if dst is too small.
 */
int packet_decode(const char *src, char *dst, int dst_size);

/*
 * Convert a PacketType enum value to its string label.
 * e.g. MSG_SEND → "MSG_SEND"
 * Returns NULL if the type is out of range.
 */
const char *packet_type_to_str(PacketType type);

/*
 * Convert a string label to its PacketType.
 * e.g. "MSG_SEND" → MSG_SEND
 * Returns 0 on success, -1 if the label is unknown.
 */
int packet_str_to_type(const char *str, PacketType *out);

/* includeguard : will not be included twice */
#endif
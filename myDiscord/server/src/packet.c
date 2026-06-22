#include "../include/packet.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/* Maps each PacketType to its wire label */
static const char *TYPE_LABELS[PACKET_TYPE_COUNT] = {
    "AUTH_REGISTER", "AUTH_LOGIN", "AUTH_LOGOUT",
    "MSG_SEND", "MSG_HISTORY", "MSG_REACTION",
    "CHANNEL_JOIN", "CHANNEL_LEAVE", "CHANNEL_CREATE", "CHANNEL_DELETE", "CHANNEL_LIST",
    "USER_BAN", "USER_LIST",
    "SERVER_OK", "SERVER_ERROR", "SERVER_PUSH"
};

const char *packet_type_to_str(PacketType type)
{
    /* Reject anything outside the valid enum range */
    if (type < 0 || type >= PACKET_TYPE_COUNT)
        return NULL;
    return TYPE_LABELS[type];
}

int packet_str_to_type(const char *str, PacketType *out)
{
    for (int i = 0; i < PACKET_TYPE_COUNT; i++) {
        if (strcmp(str, TYPE_LABELS[i]) == 0) {
            *out = (PacketType)i;
            return 0;
        }
    }
    return -1;
}

int packet_sanitize_field(const char *src, char *dst, int dst_size)
{
    int j = 0;

    for (int i = 0; src[i] != '\0'; i++) {

        /* Always leave room for the null terminator */
        if (j >= dst_size - 1)
            return -1;

        if (src[i] == '|')
            dst[j++] = ESCAPE_PIPE; /* replace field separator */
        else if (src[i] == '\n')
            dst[j++] = ESCAPE_NEWLINE; /* replace packet terminator */
        else
            dst[j++] = src[i];
    }

    dst[j] = '\0';
    return 0;
}

int packet_restore_field(const char *src, char *dst, int dst_size)
{
    int j = 0;

    for (int i = 0; src[i] != '\0'; i++) {

        if (j >= dst_size - 1)
            return -1;

        if (src[i] == ESCAPE_PIPE)
            dst[j++] = '|'; /* restore original pipe */
        else if (src[i] == ESCAPE_NEWLINE)
            dst[j++] = '\n'; /* restore original newline */
        else
            dst[j++] = src[i];
    }

    dst[j] = '\0';
    return 0;
}

int packet_build(Packet *out, PacketType type, int field_count, ...)
{
    /* Validate type and field count before touching anything */
    if (type < 0 || type >= PACKET_TYPE_COUNT)
        return -1;
    if (field_count < 0 || field_count > PACKET_MAX_FIELDS)
        return -1;

    out->type        = type;
    out->field_count = field_count;

    /* Read each field from the variadic arguments */
    va_list args;
    va_start(args, field_count);

    for (int i = 0; i < field_count; i++) {
        const char *field = va_arg(args, const char *);
        strncpy(out->fields[i], field, PACKET_FIELD_SIZE - 1);
        out->fields[i][PACKET_FIELD_SIZE - 1] = '\0'; /* guarantee termination */
    }

    va_end(args);
    return 0;
}

int packet_serialize(const Packet *pkt, char *out, int out_size)
{
    const char *label = packet_type_to_str(pkt->type);
    if (label == NULL)
        return -1;

    /* Start with the type label */
    int written = snprintf(out, out_size, "%s", label);
    if (written < 0 || written >= out_size)
        return -1;

    /* Append each field, sanitized, separated by '|' */
    for (int i = 0; i < pkt->field_count; i++) {
        char sanitized[PACKET_FIELD_SIZE];

        if (packet_sanitize_field(pkt->fields[i], sanitized, sizeof(sanitized)) != 0)
            return -1;

        int added = snprintf(out + written, out_size - written, "|%s", sanitized);
        if (added < 0 || added >= out_size - written)
            return -1;

        written += added;
    }

    /* Terminate the packet with a newline */
    if (written >= out_size - 1)
        return -1;

    out[written++] = '\n';
    out[written]   = '\0';
    return 0;
}

int packet_deserialize(const char *raw, Packet *out)
{
    /* Work on a local copy : strtok modifies the string in place */
    char copy[PACKET_MAX_SIZE];
    strncpy(copy, raw, PACKET_MAX_SIZE - 1);
    copy[PACKET_MAX_SIZE - 1] = '\0';

    /* Strip the trailing newline */
    int len = strlen(copy);
    if (len > 0 && copy[len - 1] == '\n')
        copy[len - 1] = '\0';

    memset(out, 0, sizeof(Packet));

    /* First token is the type label */
    char *token = strtok(copy, "|");
    if (token == NULL)
        return -1;

    if (packet_str_to_type(token, &out->type) != 0)
        return -1; /* unknown type : reject the packet */

    /* Remaining tokens are fields */
    while ((token = strtok(NULL, "|")) != NULL) {

        if (out->field_count >= PACKET_MAX_FIELDS)
            return -1; /* too many fields : malformed packet */

        /* Restore the field to its original value */
        if (packet_restore_field(token, out->fields[out->field_count],
                            PACKET_FIELD_SIZE) != 0)
            return -1;

        out->field_count++;
    }

    return 0;
}
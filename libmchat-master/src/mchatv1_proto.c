/*!
 * \file mchatv1_proto.c
 * \author Sean Tracy
 * \date 17 March 2017
 * \version 0.0.1
 * \brief MChatv1 Protocol Constants
 *
 * \details
 * This file contains the externs declared in mchatv1_proto.h.
 * Most of these are just string arrays for formatting/parsing
 * purposes.
 */
#include <string.h>
#include <glib.h>
#include "mchatv1_proto.h"


const char *mchatv1_message_type_strings[] = { \
    MCHATV1_MESSAGE_TYPES_MAP(MAP_MACRO_MESSAGE_TYPE_STRING) \
};


const char *mchatv1_header_type_strings[] = { \
    MCHATV1_HEADER_TYPES_MAP(MAP_MACRO_HEADER_TYPE_STRING) \
};


const int *mchatv1_message_type_required_headers[] = { \
    MCHATV1_MESSAGE_TYPES_MAP(MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS) \
};


const int mchatv1_message_type_required_headers_len[] = { \
    MCHATV1_MESSAGE_TYPES_MAP(MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_LEN) \
};

const char* mchatv1_protocol_line_string = "%s MCHAT/%u.%u\r\n";

const char *mchatv1_header_line_string = "%s: %s\r\n";

const char *mchatv1_not_connected_channel_string = "<Not Connected>";

int mchatv1_find_message_type(char *ptr, unsigned int len)
{
    // Skip the None type message
    for (int i = 1; i < MCHATV1_MESSAGE_TYPES_COUNT; i++)
    {
        const char *j = mchatv1_message_type_strings[i];
        int jlen = strlen(j);
        if (jlen == len && g_ascii_strncasecmp(ptr, j, jlen) == 0)
            return i;
    }
    return -1;
}


int mchatv1_find_header_type(char *ptr, unsigned int len)
{
    for(int i = 0; i < MCHATV1_HEADER_TYPES_COUNT; i++)
    {
        const char *j = mchatv1_header_type_strings[i];
        int jlen = strlen(j);
        if (jlen == len && g_ascii_strncasecmp(ptr, j, jlen) == 0)
            return i;
    }
    return -1;
}

int mchatv1_message_type_has_body(enum mchatv1_type type)
{
    int rh_len = mchatv1_message_type_required_headers_len[type];
    const int *rh = mchatv1_message_type_required_headers[type];
    for (int i = 0; i < rh_len; i++)
    {
        if (rh[i] == MCHATV1_HEADER_TYPE_LENGTH)
            return 1;
    }
    return 0;
}

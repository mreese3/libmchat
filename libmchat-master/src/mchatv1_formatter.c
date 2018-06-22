#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "mchatv1.h"
#include "mchatv1_macro_hell.h"
#include "mchatv1_proto.h"
#include "mchatv1_structs.h"
#include "mchatv1_formatter.h"

/*****************************************************************************
 * 								Header Formatters							 *
 *****************************************************************************/

/*!
 * \name Header Formatters
 * @{
 */

/*
Formatter template - Copy and paste
int _format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}
*/


int nickname_format(struct mchat_thread *thread_info, char *dst)
{
    int offset = 0;
    HEADER_FORMAT_START(MCHATV1_HEADER_TYPE_NICKNAME, offset, dst);
    memcpy(dst + offset, thread_info->mchat->nickname, thread_info->mchat->nickname_size);
    offset += thread_info->mchat->nickname_size;
    FORMAT_CRLF(offset, dst);
    return offset;
}


int length_format(struct mchat_thread *thread_info, char *dst)
{
    int offset = 0;
    if (thread_info->buffer_flag)
    {
        HEADER_FORMAT_START(MCHATV1_HEADER_TYPE_LENGTH, offset, dst);
        offset += g_snprintf(dst + offset, 6, "%u", strlen(thread_info->buffer->body));
        FORMAT_CRLF(offset, dst);
    }
    return offset;
}


int filename_format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}


int filesum_format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}


int chunk_format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}


int chunkcount_format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}


int chunksum_format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}


int channel_format(struct mchat_thread *thread_info, char *dst)
{
    int offset = 0;
    HEADER_FORMAT_START(MCHATV1_HEADER_TYPE_CHANNEL, offset, dst);
    int len;
    /* We may need to use a mutex for mchat structures here -Sean */
    if (thread_info->mchat->is_connected && thread_info->mchat->current_channel != NULL)
    {
        len = strlen(thread_info->mchat->current_channel->channel_name);
        memcpy(dst + offset, thread_info->mchat->current_channel->channel_name, len);
    }
    else
    {
        len = strlen(mchatv1_not_connected_channel_string);
        memcpy(dst + offset, mchatv1_not_connected_channel_string, len);
    }
    offset += len;
    FORMAT_CRLF(offset, dst);
    return offset;
}


int presence_format(struct mchat_thread *thread_info, char *dst)
{
    return 0;
}


int address_format(struct mchat_thread *thread_info, char *dst)
{
    int offset = 0;
    if (thread_info->mchat->is_connected && thread_info->mchat->current_channel != NULL)
    {
        HEADER_FORMAT_START(MCHATV1_HEADER_TYPE_ADDRESS, offset, dst);
        guchar *addr = g_inet_address_to_string(thread_info->mchat->current_channel->channel_address);
        int addr_len = strlen(addr);
        memcpy(dst + offset, addr, addr_len);
        g_free(addr);
        offset += addr_len;
        FORMAT_CRLF(offset, dst);
    }
    return offset;
}


int port_format(struct mchat_thread *thread_info, char *dst)
{
    int offset = 0;
    if (thread_info->mchat->is_connected && thread_info->mchat->current_channel != NULL)
    {
        HEADER_FORMAT_START(MCHATV1_HEADER_TYPE_PORT, offset, dst);
        offset += g_snprintf(dst + offset, 5, "%u", thread_info->mchat->current_channel->channel_portno);
        FORMAT_CRLF(offset, dst);
    }
    return offset;
}

//! @}


/*****************************************************************************
 * 							Header Formatters - End							 *
 *****************************************************************************/

/*!
 * \brief Function map for header formatting
 */
int (*mchatv1_header_formatters[])(struct mchat_thread *, char *) = { \
        MCHATV1_HEADER_TYPES_MAP(MAP_MACRO_HEADER_TYPE_FORMAT_FUNCTION) \
};


int mchatv1_format(mchat_thread *thread_info, char *dest, enum mchatv1_type type)
{
    const char *typestring = mchatv1_message_type_strings[type];
    const int *req_hdrs = mchatv1_message_type_required_headers[type];
    const int rh_len = mchatv1_message_type_required_headers_len[type];

    int offset = g_sprintf(dest, mchatv1_protocol_line_string, typestring,
                         MCHAT_PROTOCOL_VERSION_MAJOR, MCHAT_PROTOCOL_VERSION_MINOR);

    for (int i = 0; i < rh_len; i++)
        offset += mchatv1_header_formatters[req_hdrs[i]](thread_info, dest + offset);
    FORMAT_CRLF(offset, dest);
    if (mchatv1_message_type_has_body(type) && thread_info->buffer_flag)
    {
        int bodylen = strlen(thread_info->buffer->body);
        memcpy(dest + offset, thread_info->buffer->body, bodylen);
        offset += bodylen;
    }
    return offset;
}

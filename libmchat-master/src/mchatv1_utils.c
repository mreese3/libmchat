/*!
 * \file mchatv1_utils.c
 * \author Sean Tracy
 * \date 27 March 2017
 * \version 0.0.1
 * \brief Miscellaneous internal utility functions for libmchat
 *
 * \details
 * \todo This documentation
 */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include "mchatv1.h"
#include "mchatv1_structs.h"
#include "mchatv1_utils.h"


mchat_channel *channel_query_by_name(GPtrArray *array, char *channel_name)
{
    int channel_length = strlen(channel_name);
    for (int i = 0; i < array->len; i++)
    {
        mchat_channel *c = g_ptr_array_index(array, i);
        if (strlen(c->channel_name) == channel_length)
        {
            if (strncmp(channel_name, c->channel_name, channel_length) == 0)
                return c;
        }
    }
    return NULL;
}


mchat_channel *channel_query_by_id(GPtrArray *array, unsigned int channel_id)
{
    for (int i = 0; i < array->len; i++)
    {
        mchat_channel *c = g_ptr_array_index(array, i);
        if (c->channel_id == channel_id)
            return c;
    }
    return NULL;
}


int peerlist_query(mchat_t *mchat, guint32 address)
{
    for (int i = 0; i < mchat->peerlist->len; i++)
    {
        mchat_peer *p = &g_array_index(mchat->peerlist, mchat_peer, i);
        if (p->source_address == address)
            return i;
    }
    return -1;
}


int peerlist_expire(mchat_t *mchat)
{
    gint64 now = g_get_real_time();
    g_mutex_lock(&mchat->peerlist_mutex);
    if (mchat->peerlist->len)
    {
        for (int i = 0; i < mchat->peerlist->len; i++)
        {
            mchat_peer *p = &g_array_index(mchat->peerlist, mchat_peer, i);
            if (now - p->last_seen > MCHAT_PROTOCOL_DEFAULT_EXPIRE_INTERVAL * G_TIME_SPAN_SECOND)
                g_array_remove_index_fast(mchat->peerlist, i);
        }
    }
    g_mutex_unlock(&mchat->peerlist_mutex);
}


int peerlist_update_peer(mchat_t *mchat, mchat_parser parsed_message, guint32 address)
{
    g_mutex_lock(&mchat->peerlist_mutex);
    int index = peerlist_query(mchat, address);

    if (index == -1)
    {
        mchat_peer p;
        memset(&p, 0, sizeof(p));
        p.nickname_len = parsed_message.header_len[MCHATV1_HEADER_TYPE_NICKNAME];
        memcpy(p.nickname, parsed_message.header_offset[MCHATV1_HEADER_TYPE_NICKNAME], p.nickname_len);
        p.channel_len = parsed_message.header_len[MCHATV1_HEADER_TYPE_CHANNEL];
        memcpy(p.channel, parsed_message.header_offset[MCHATV1_HEADER_TYPE_CHANNEL], p.channel_len);
        p.last_seen = g_get_real_time();
        p.source_address = address;
        g_array_append_val(mchat->peerlist, p);
    }
    else
    {
        mchat_peer *p = &g_array_index(mchat->peerlist, mchat_peer, index);
        p->nickname_len = parsed_message.header_len[MCHATV1_HEADER_TYPE_NICKNAME];
        memcpy(p->nickname, parsed_message.header_offset[MCHATV1_HEADER_TYPE_NICKNAME], p->nickname_len);
        p->channel_len = parsed_message.header_len[MCHATV1_HEADER_TYPE_CHANNEL];
        memcpy(p->channel, parsed_message.header_offset[MCHATV1_HEADER_TYPE_CHANNEL], p->channel_len);
        p->last_seen = g_get_real_time();
    }
    g_mutex_unlock(&mchat->peerlist_mutex);

    return 0;
}


unsigned int mchat_channel_hash_struct(mchat_channel *chan)
{
    guint32 hash = MCHAT_CHANNEL_HASH_FNV_OFFSET;

    // First, hash the channel_name
    for (int i = 0; i < strlen(chan->channel_name); i++)
        hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ chan->channel_name[i];

    // Next, the address
    guchar *addr = g_inet_address_to_string(chan->channel_address);
    for (int i = 0; i < strlen(addr); i++)
         hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ addr[i];
    g_free(addr);

    // Finally, add the port number
    guchar *port_bytes = (guchar*)&chan->channel_portno;
    hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ port_bytes[0];
    hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ port_bytes[1];

    return hash;
}


unsigned int mchat_channel_hash_params(guchar *name, guchar *addr, guint16 portno)
{
    guint32 hash = MCHAT_CHANNEL_HASH_FNV_OFFSET;

    for (int i = 0; i < strlen(name); i++)
        hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ name[i];

    for (int i = 0; i < strlen(addr); i++)
        hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ addr[i];

    guchar *port_bytes = (guchar*)&portno;
    hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ port_bytes[0];
    hash = (hash * MCHAT_CHANNEL_HASH_FNV_PRIME) ^ port_bytes[1];

    return hash;
}


void mchat_channel_destroy(gpointer data)
{
    mchat_channel *c = (mchat_channel*)data;
    g_object_unref(c->channel_address);
    g_free(c);
}

mchat_channel *mchat_channel_copy(mchat_channel *src)
{
    mchat_channel *dst = g_malloc(sizeof(mchat_channel));
    memset(dst, 0, sizeof(mchat_channel));
    memcpy(dst->channel_name, src->channel_name, strlen(src->channel_name));
    guchar *addr = g_inet_address_to_string(src->channel_address);
    dst->channel_address = g_inet_address_new_from_string(addr);
    g_free(addr);
    dst->channel_portno = src->channel_portno;
    dst->channel_id = src->channel_id;

    return dst;
}

int mchat_channel_update(mchat_t *mchat, mchat_parser *parsed_message)
{
    char chan_name[MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE];
    char chan_addr[40];
    memcpy(chan_name,
           parsed_message->header_offset[MCHATV1_HEADER_TYPE_CHANNEL],
           parsed_message->header_len[MCHATV1_HEADER_TYPE_CHANNEL]);
    memcpy(chan_addr,
           parsed_message->header_offset[MCHATV1_HEADER_TYPE_ADDRESS],
           parsed_message->header_len[MCHATV1_HEADER_TYPE_ADDRESS]);
    chan_name[parsed_message->header_len[MCHATV1_HEADER_TYPE_CHANNEL]] = '\0';
    chan_addr[parsed_message->header_len[MCHATV1_HEADER_TYPE_ADDRESS]] = '\0';
    guint16 portno = strtol(parsed_message->header_offset[MCHATV1_HEADER_TYPE_PORT],
                                   NULL, 10);
    guint32 id = mchat_channel_hash_params(chan_name, chan_addr, portno);
    g_mutex_lock(&mchat->channels_mutex);
    mchat_channel *c = channel_query_by_id(mchat->cdsc_channels, id);
    if (c == NULL)
    {
        c = g_malloc(sizeof(mchat_channel));
        memset(c, 0, sizeof(mchat_channel));
        c->channel_address = g_inet_address_new_from_string(chan_addr);
        c->channel_id = id;
        c->channel_portno = portno;
        memcpy(c->channel_name, chan_name,
               parsed_message->header_len[MCHATV1_HEADER_TYPE_CHANNEL]);
    }
    c->last_seen = g_get_real_time();
    g_mutex_unlock(&mchat->channels_mutex);
    return 0;
}


int mchat_channel_expire(mchat_t *mchat)
{
    g_mutex_lock(&mchat->channels_mutex);
    for (int i = 0; i < mchat->cdsc_channels->len; i++)
    {
        mchat_channel *c = g_ptr_array_index(mchat->cdsc_channels, i);
        if (g_get_real_time() - c->last_seen > MCHAT_PROTOCOL_DEFAULT_CDSC_EXPIRE * G_TIME_SPAN_SECOND)
        {
            g_ptr_array_remove_fast(mchat->cdsc_channels, c);
            i--;	/* g_ptr_array_remove_fast moves the last element
                      of the array into the space removed, so we need to
                      inspect that index again. */
        }
    }
    g_mutex_unlock(&mchat->channels_mutex);
    return 0;
}

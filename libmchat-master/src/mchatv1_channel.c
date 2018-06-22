/*!
 * \file mchatv1_channel.c
 * \author Sean Tracy
 * \date 5 April 2017
 * \version 0.0.1
 * \brief MChat API channel-related functions
 *
 * \details
 * This file defines the MChat channel API.
 */

#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include "mchatv1.h"
#include "mchatv1_utils.h"
#include "mchatv1_structs.h"


int mchatv1_add_channel(mchat_t *mchat, char *channel, char *channel_address, unsigned short channel_portno)
{
    if (channel_address == NULL && channel_portno == 0)
    {
        /* This is a channal that is (hopefully) in the cdsc_channels list */
        g_mutex_lock(&mchat->channels_mutex);
        mchat_channel *c = channel_query_by_name(mchat->cdsc_channels, channel);
        if (c == NULL)
        {
            g_mutex_unlock(&mchat->channels_mutex);
            return -1;
        }
        mchat_channel *newc = mchat_channel_copy(c);
        g_ptr_array_add(mchat->added_channels, newc);
        g_mutex_unlock(&mchat->channels_mutex);
    }
    else
    {
        /* This is a new channel that we need to completely define */
        int hash = mchat_channel_hash_params(channel, channel_address, channel_portno);
        mchat_channel *c = g_malloc(sizeof(mchat_channel));
        memset(c, 0, sizeof(mchat_channel));
        memcpy(c->channel_name, channel, strlen(channel));
        c->channel_portno = channel_portno;
        c->channel_address = g_inet_address_new_from_string(channel_address);
        c->channel_id = hash;
        g_ptr_array_add(mchat->added_channels, c);
    }
    return 0;
}


int mchatv1_del_channel(mchat_t *mchat, char *channel)
{
    if (g_ascii_strncasecmp(channel, MCHAT_PROTOCOL_DEFAULT_CHANNEL_NAME, strlen(MCHAT_PROTOCOL_DEFAULT_CHANNEL_NAME)) == 0)
        return -1;

    mchat_channel *c = channel_query_by_name(mchat->added_channels, channel);
    if (c != NULL)
    {
        if (mchat->is_connected && mchat->current_channel == c)
            return -1;
        g_ptr_array_remove(mchat->added_channels, c);
    }
    else
        return -1;

    return 0;
}


int mchatv1_get_channel(mchat_t *mchat, char *buf, unsigned int buf_size)
{
    if (!mchat->is_connected)
        return -1;

    int tocopy;
    int clen = strlen(mchat->current_channel->channel_name);
    if (clen > buf_size)
        tocopy = buf_size;
    else
        tocopy = clen;
    memcpy(buf, mchat->current_channel->channel_name, tocopy);
    buf[tocopy] = '\0';
    return tocopy;
}


int mchatv1_get_channel_count(mchat_t *mchat)
{
    return mchat->added_channels->len;
}


int mchatv1_get_added_channels(mchat_t *mchat, mchat_chanlist_t **channel_list)
{
    /*! \todo This function needs to be implemented */
    return 0;
}


int mchatv1_get_cdsc_channels(mchat_t *mchat, mchat_chanlist_t **channel_list)
{
    /*! \todo This function needs to be implemented */
    return 0;
}


int mchatv1_chanlist_size(mchat_chanlist_t *channel_list)
{
    return channel_list->length;
}


int mchatv1_chanlist_get_channel_name(mchat_chanlist_t *channel, unsigned int index, char *buf, unsigned int buf_size)
{
    /*! \todo This function needs to be implemented */
    return 0;
}


int mchatv1_chanlist_get_channel_address(mchat_chanlist_t *channel, unsigned int index, char *buf, unsigned int buf_size)
{
    /*! \todo This function needs to be implemented */
    return 0;
}


int mchatv1_chanlist_get_channel_port(mchat_chanlist_t *channel, unsigned int index, unsigned short *port)
{
    /*! \todo This function needs to be implemented */
    return 0;
}


int mchatv1_chanlist_get_channel_time(mchat_chanlist_t *channel, unsigned int index, long *time)
{
    /*! \todo This function needs to be implemented */
    return 0;
}


int mchatv1_chanlist_get_channel_info(mchat_chanlist_t *channel, unsigned int index,
                                      char *name_buf, unsigned int name_buf_size,
                                      char *addr_buf, unsigned int addr_buf_size,
                                      unsigned short *portno, long *time)
{
    /*! \todo This function needs to be implemented */
    return 0;
}

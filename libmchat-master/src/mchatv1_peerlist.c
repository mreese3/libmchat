/*!
 * \file mchatv1_peerlist.c
 * \author Sean Tracy
 * \date 5 April 2017
 * \version 0.0.1
 * \brief MChat API peerlist-related functions
 *
 * \details
 * This file defines the MChat peerlist API.
 */

#include <glib.h>
#include <string.h>
#include "mchatv1.h"
#include "mchatv1_structs.h"

int mchatv1_peers_available(mchat_t *mchat)
{
    if (mchat->peerlist->len > 0)
        return 1;
    else
        return 0;
}


int mchatv1_get_peerlist(mchat_t *mchat, mchat_peerlist_t **peerlist)
{

    mchat_peerlist_t *l = g_malloc(sizeof(mchat_peerlist_t));
    memset(l, 0, sizeof(mchat_peerlist_t));
    int ret = 0;
    g_mutex_lock(&mchat->peerlist_mutex);
    if (mchat->peerlist->len)
    {
        ret = 1;
        l->length = mchat->peerlist->len;
        l->list = g_malloc(sizeof(mchat_peer) * l->length);
        memset(l->list, 0, l->length * sizeof(mchat_peer));
        for (int i = 0; i < l->length; i++)
            memcpy(&l->list[i], &g_array_index(mchat->peerlist, mchat_peer, i), sizeof(mchat_peer));
    }
    g_mutex_unlock(&mchat->peerlist_mutex);
    *peerlist = l;
    return ret;
}


int mchatv1_peerlist_get_size(mchat_peerlist_t *peerlist)
{
    return peerlist->length;
}


int mchatv1_peer_get_name(mchat_peerlist_t *peerlist, unsigned int index, char *buf, unsigned int buf_size)
{
    if (index >= peerlist->length)
        return -1;

    mchat_peer *p = &peerlist->list[index];
    int tocopy = 0;
    if (buf_size >= p->nickname_len)
        tocopy = p->nickname_len;
    else
        tocopy = buf_size;
    memcpy(buf, p->nickname, tocopy);
    buf[tocopy] = '\0';
    return tocopy;
}


int mchatv1_peer_get_channel(mchat_peerlist_t *peerlist, unsigned int index, char *buf, unsigned int buf_size)
{
    if (index >= peerlist->length)
        return -1;

    mchat_peer *p = &peerlist->list[index];
    int tocopy = 0;
    if (buf_size >= p->channel_len)
        tocopy = p->channel_len;
    else
        tocopy = buf_size;
    memcpy(buf, p->channel, tocopy);
    buf[tocopy] = '\0';
    return tocopy;
}


int mchatv1_peer_get_timestamp(mchat_peerlist_t *peerlist, unsigned int index, long *t)
{
    if (index >= peerlist->length)
        return -1;
    *t = peerlist->list[index].last_seen;
    return 0;
}


int mchatv1_peer_get_peer(mchat_peerlist_t *peerlist, unsigned int index,
                          char *nick_buf, char *channel_buf,
                          unsigned int nick_buf_size,
                          unsigned int channel_buf_size, long *t)
{
    if (index >= peerlist->length)
        return -1;

    mchat_peer *p = &peerlist->list[index];

    if (nick_buf_size < p->nickname_len)
        return -2;
    if (channel_buf_size < p->channel_len)
        return -3;

    memcpy(nick_buf, p->nickname, p->nickname_len);
    memcpy(channel_buf, p->channel, p->channel_len);
    nick_buf[p->nickname_len] = '\0';
    channel_buf[p->channel_len] = '\0';
    *t = p->last_seen;
    return 0;
}


int mchatv1_peer_get_source_address(mchat_peerlist_t *peerlist,
                                    unsigned int index,
                                    char *addr_buf, unsigned int size)
{
    if (index >= peerlist->length)
        return -1;

    mchat_peer *p = &peerlist->list[index];
    guchar bytes[4];
    memcpy(bytes, &p->source_address, 4);
    return g_snprintf(addr_buf, size, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
}


int mchatv1_peerlist_destroy(mchat_peerlist_t **peerlist)
{
    if ((*peerlist)->list != NULL)
        g_free((*peerlist)->list);
    g_free(*peerlist);
    return 0;
}

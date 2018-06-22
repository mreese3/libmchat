/*!
 * \file mchatv1.c
 * \author Sean Tracy
 * \date 3 March 2017
 * \version 0.0.1
 * \brief MChat core API functions
 *
 * \details
 * This file defines the core API functions, such as ::mchatv1_init() and ::mchatv1_connect().
 *
 * \todo Implement message api and add more api functions for messages
 * \todo Implement error handling and error strings
 * \todo Go through each function and struct to check for consistent name scheme
 * \todo Implement ping messages into mchatv1_thread_send()
 */
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include "mchatv1.h"
#include "mchatv1_structs.h"
#include "mchatv1_threads.h"
#include "mchatv1_utils.h"


/****************************************************************************
 ************************    Public API Functions    ************************
 ****************************************************************************/
mchat_t *mchatv1_init(char *cfg_filename)
{
    /*!
     * \todo This function needs to use configuration system eventually
     */
    mchat_t *mchat = g_malloc(sizeof(mchat_t));

    memset(mchat, 0, sizeof(mchat_t));

    // Initialize child structs
    mchat->nickname = g_malloc(sizeof(gchar) * MCHAT_LIMIT_MAX_NICKNAME_SIZE);
    memset(mchat->nickname, 0, sizeof(gchar) * MCHAT_LIMIT_MAX_NICKNAME_SIZE);

    // Initialize the channels array
    mchat->added_channels = g_ptr_array_new();
    g_ptr_array_set_free_func(mchat->added_channels, mchat_channel_destroy);
    mchat->cdsc_channels = g_ptr_array_new();
    g_ptr_array_set_free_func(mchat->cdsc_channels, mchat_channel_destroy);

    // Create the default #mchat channel
    mchatv1_add_channel(mchat, MCHAT_PROTOCOL_DEFAULT_CHANNEL_NAME, MCHAT_PROTOCOL_DEFAULT_CHANNEL_ADDRESS,
                      MCHAT_PROTOCOL_DEFAULT_CHANNEL_PORT);

    // Create a random nickname if one has not been set by configuration
    if (!mchat->nickname_size)
    {
        g_snprintf(mchat->nickname, 16, "NoNick%u", g_random_int());
        mchat->nickname_size = strlen(mchat->nickname);
    }
    mchat->peerlist = g_array_new(FALSE, FALSE, sizeof(mchat_peer));
    g_mutex_init(&mchat->peerlist_mutex);
    g_mutex_init(&mchat->channels_mutex);

    // Now init the common channel
    GSocket *tsock, *rsock;
    GInetAddress *tinet, *rinet;
    GSocketAddress *taddr, *raddr;

    tinet = g_inet_address_new_from_string(MCHAT_PROTOCOL_COMMON_CHANNEL_ADDRESS);
    taddr = g_inet_socket_address_new(tinet, MCHAT_PROTOCOL_COMMON_CHANNEL_PORT);
    tsock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                         G_SOCKET_PROTOCOL_UDP, NULL);

    rinet = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
    raddr = g_inet_socket_address_new(rinet, MCHAT_PROTOCOL_COMMON_CHANNEL_PORT);
    rsock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                         G_SOCKET_PROTOCOL_UDP, NULL);

    g_socket_set_multicast_loopback(tsock, FALSE);
    g_socket_set_multicast_loopback(rsock, FALSE);

    g_socket_join_multicast_group(rsock, tinet, FALSE, NULL, NULL);
    g_socket_bind(rsock, raddr, TRUE, NULL);

    g_object_unref(tinet);
    g_object_unref(rinet);

    mchatv1_thread_init(mchat, &mchat->comm_send_thread, "Comm Send",
                        taddr, tsock, mchatv1_thread_comm_send, NULL);
    mchatv1_thread_init(mchat, &mchat->comm_recv_thread, "Comm Recv",
                        raddr, rsock, mchatv1_thread_comm_recv, NULL);
    return mchat;
}


int mchatv1_destroy(mchat_t **mchat)
{
    // make sure we are disconnected
    mchatv1_disconnect(*mchat);

    mchatv1_thread_destroy(&(*mchat)->comm_send_thread);
    mchatv1_thread_destroy(&(*mchat)->comm_recv_thread);
    // Free our glib data structures
    g_ptr_array_free((*mchat)->added_channels, TRUE);
    g_ptr_array_free((*mchat)->cdsc_channels, TRUE);
    g_array_unref((*mchat)->peerlist);
    g_mutex_clear(&(*mchat)->peerlist_mutex);
    g_mutex_clear(&(*mchat)->channels_mutex);
    // Free nickname buffer
    g_free((*mchat)->nickname);
    g_free(*mchat);
    return 0;
}


int mchatv1_get_stealth_mode(mchat_t *mchat)
{
    return mchat->stealth_mode;
}


int mchatv1_set_stealth_mode(mchat_t *mchat, int stealth)
{
    if (stealth != 0)
        mchat->stealth_mode = 1;
    else
        mchat->stealth_mode = 0;

    return mchat->stealth_mode;
}


int mchatv1_connect(mchat_t *mchat, char *channel)
{
    if (channel == NULL)
        channel = "#mchat";

    GSocket *tsock, *rsock;
    GInetAddress *rinet;
    GSocketAddress *taddr, *raddr;

    mchat_channel *c = channel_query_by_name(mchat->added_channels, channel);
    if (c == NULL)
        return -1;

    mchat->current_channel = c;
    taddr = g_inet_socket_address_new(c->channel_address, c->channel_portno);
    tsock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                         G_SOCKET_PROTOCOL_UDP, NULL);

    rinet = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
    raddr = g_inet_socket_address_new(rinet, c->channel_portno);
    rsock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                         G_SOCKET_PROTOCOL_UDP, NULL);

    g_socket_set_multicast_loopback(tsock, FALSE);
    g_socket_set_multicast_loopback(rsock, FALSE);
    g_socket_join_multicast_group(rsock, c->channel_address, FALSE, NULL, NULL);

    g_socket_bind(rsock, raddr, TRUE, NULL);
    g_object_unref(rinet);

    mchatv1_thread_init(mchat, &mchat->text_send_thread, "Text Send",
                        taddr, tsock, mchatv1_thread_text_send, NULL);
    mchatv1_thread_init(mchat, &mchat->text_recv_thread, "Text Recv",
                        raddr, rsock, mchatv1_thread_text_recv, NULL);

    mchat->is_connected = 1;
    return 0;
}


int mchatv1_disconnect(mchat_t *mchat)
{
    // Don't disconnect if we never connected in the first place
    if (!mchat->is_connected)
        return -1;

    mchatv1_thread_destroy(&mchat->text_recv_thread);
    mchatv1_thread_destroy(&mchat->text_send_thread);
    /* Make sure comm_send or comm_recv is not using channel info */
    g_mutex_lock(&mchat->channels_mutex);
    mchat->is_connected = 0;
    mchat->current_channel = NULL;
    g_mutex_unlock(&mchat->channels_mutex);

    return 0;
}


int mchatv1_is_connected(mchat_t *mchat)
{
    return mchat->is_connected;
}


int mchatv1_send_message(mchat_t *mchat, char *message)
{
    if (!mchat->is_connected)
        return -1;

    if (mchat->text_send_thread->run_flag == 0)
        return -1;

    int len = strlen(message);
    if (len > MCHAT_LIMIT_MAX_MESSAGE_SIZE || len == 0)
        return -1;

    g_mutex_lock(&mchat->text_send_thread->mutex);
    if (mchat->text_send_thread->buffer_flag)
        g_cond_wait(&mchat->text_send_thread->cond, &mchat->text_send_thread->mutex);

    memset(mchat->text_send_thread->buffer->body, 0, MCHAT_LIMIT_MAX_MESSAGE_SIZE);
    memset(mchat->text_send_thread->buffer->nickname, 0, MCHAT_LIMIT_MAX_NICKNAME_SIZE);
    memcpy(mchat->text_send_thread->buffer->body, message, len);
    memcpy(mchat->text_send_thread->buffer->nickname, mchat->nickname, mchat->nickname_size);
    mchat->text_send_thread->buffer_flag = 1;

    g_cond_broadcast(&mchat->text_send_thread->cond);
    g_mutex_unlock(&mchat->text_send_thread->mutex);
    return 0;
}


int mchatv1_recv_message(mchat_t *mchat, mchat_message_t **message)
{
    //! \todo Make the return values of this function more meaningful
    //! \todo Update function description to reflect return values
    if (!mchat->is_connected)
        return -1;

    if (mchat->text_recv_thread->run_flag == 0)
        return -1;

    g_mutex_lock(&mchat->text_recv_thread->mutex);

    if (!mchat->text_recv_thread->buffer_flag)
    {
        g_mutex_unlock(&mchat->text_recv_thread->mutex);
        return 0;
    }

    mchat_message_t *m = g_malloc(sizeof(mchat_message_t));

    char *body = g_malloc(MCHAT_LIMIT_MAX_MESSAGE_SIZE);
    char *nickname = g_malloc(MCHAT_LIMIT_MAX_NICKNAME_SIZE);
    memcpy(m, mchat->text_recv_thread->buffer, sizeof(mchat_message_t));
    memset(body, 0, MCHAT_LIMIT_MAX_MESSAGE_SIZE);
    memset(nickname, 0, MCHAT_LIMIT_MAX_NICKNAME_SIZE);
    m->body = body;
    m->nickname = nickname;
    memcpy(m->body, mchat->text_recv_thread->buffer->body, m->body_len);
    memcpy(m->nickname, mchat->text_recv_thread->buffer->nickname, m->nickname_len);
    mchat->text_recv_thread->buffer_flag = 0;
    g_cond_broadcast(&mchat->text_recv_thread->cond);
    g_mutex_unlock(&mchat->text_recv_thread->mutex);

    *message = m;
    return 1;
}


int mchatv1_set_nickname(mchat_t *mchat, char *new_nickname, unsigned int len)
{
    int nickname_len = strlen(new_nickname);
    if (len > MCHAT_LIMIT_MAX_NICKNAME_SIZE || nickname_len < len)
        return -1;

    memset(mchat->nickname, 0, MCHAT_LIMIT_MAX_NICKNAME_SIZE);
    memcpy(mchat->nickname, new_nickname, len);
    mchat->nickname_size = len;
    return 0;
}


int mchatv1_get_nickname(mchat_t *mchat, char *buf, unsigned int buf_size)
{
    if (mchat->nickname_size > buf_size)
        return -1;

    memcpy(buf, mchat->nickname, mchat->nickname_size);
    buf[mchat->nickname_size] = '\0';
    return mchat->nickname_size;
}


int mchatv1_set_presence(mchat_t *mchat, unsigned int presence)
{
    //! \todo This Needs to be written (Low priority)
    return 0;
}


int mchatv1_get_presence(mchat_t *mchat)
{
    //! \todo This Needs to be written (Low priority)
    return 0;
}


/*****************************************************************************
 ***************************	MChat Message API	**************************
 *****************************************************************************/
int mchatv1_message_get_body_size(mchat_message_t *packet)
{
    return packet->body_len;
}

int mchatv1_message_get_body(mchat_message_t *packet, char *buf, unsigned int len)
{
    int tocopy = 0;
    if (len < packet->body_len)
        tocopy = len - 1;
    else
        tocopy = packet->body_len;
    memcpy(buf, packet->body, tocopy);
    buf[tocopy] = '\0';
    return tocopy + 1;
}

int mchatv1_message_get_nickname(mchat_message_t *packet, char *buf, unsigned int len)
{
    int tocopy = 0;
    if (len < packet->nickname_len)
        tocopy = len - 1;
    else
        tocopy = packet->nickname_len;
    memcpy(buf, packet->nickname, tocopy);
    buf[tocopy] = '\0';
    return tocopy + 1;
}

int mchatv1_message_get_timestamp(mchat_message_t *packet, long *t)
{
    *t = packet->timestamp;
    return 0;
}

int mchatv1_message_destroy(mchat_message_t **packet)
{
    g_free((*packet)->body);
    g_free((*packet)->nickname);
    g_free(*packet);
    return 0;
}

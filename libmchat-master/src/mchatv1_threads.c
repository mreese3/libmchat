/*!
 * \file mchatv1_threads.c
 * \author Sean Tracy
 * \date 24 March 2017
 * \version 0.0.1
 * \brief MChat Thread functions
 *
 * \details
 * This file contains all the functions related to threading for mchat
 */

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include  <gio/gio.h>
#include "mchatv1.h"
#include "mchatv1_proto.h"
#include "mchatv1_formatter.h"
#include "mchatv1_parser.h"
#include "mchatv1_structs.h"
#include "mchatv1_threads.h"
#include "mchatv1_utils.h"


const char *mchatv1_thread_error_strings[] =
{
    ERROR_STRING_ARRAY(MCHATV1_THREAD_ERRORS_MAP)
};

int mchatv1_thread_init(mchat_t 		*mchat,
                        mchat_thread 	**tptr,
                        gchar 			*thread_name,
                        GSocketAddress 	*addr,
                        GSocket			*sock,
                        gpointer		(*thread_func)(gpointer),
                        mchat_fileio	*fiocfg)
{
    mchat_thread *t = g_malloc(sizeof(mchat_thread));
    if (t == NULL)
        return -1;

    memset(t, 0, sizeof(mchat_thread));
    t->sock = sock;
    t->addr = addr;
    t->cancel = g_cancellable_new();
    g_mutex_init(&t->mutex);
    g_cond_init(&t->cond);
    if ((t->buffer = g_malloc(sizeof(mchat_message_t))) == NULL)
        return -1;
    memset(t->buffer, 0, sizeof(mchat_message_t));
    if (fiocfg != NULL)
        t->fiocfg = fiocfg;

    t->mchat = mchat;

    g_mutex_lock(&t->mutex);
    t->run_flag = 1;
    t->thread_id = g_thread_new(thread_name, thread_func, (gpointer)t);
    *tptr = t;
    return 0;
}


int mchatv1_thread_destroy(mchat_thread **tptr)
{
    mchat_thread *t = *tptr;
    t->run_flag = 0;
    g_cond_broadcast(&t->cond);
    g_cancellable_cancel(t->cancel);
    g_thread_join(t->thread_id);

    g_socket_close(t->sock, NULL);

    g_cond_clear(&t->cond);
    g_mutex_clear(&t->mutex);
    g_object_unref(t->sock);
    g_object_unref(t->addr);
    g_object_unref(t->cancel);
    g_free(t->buffer);
    if (t->fiocfg)
        g_free(t->fiocfg);
    g_free(*tptr);
    return 0;
}


gpointer mchatv1_thread_text_send(gpointer args)
{
    struct mchat_thread *t = (struct mchat_thread *)args;
    // Allocate our message buffers
    t->buffer->body = g_malloc(sizeof(gchar) * MCHAT_LIMIT_MAX_MESSAGE_SIZE);
    t->buffer->nickname = g_malloc(sizeof(gchar) * MCHAT_LIMIT_MAX_NICKNAME_SIZE);

    g_mutex_unlock(&t->mutex);
    // Let's be safe and make our raw packet buffer larger than a UDP packet can be
    gchar send_buffer[1 << 16];
    gssize send_len;

    // Send out 3 pings to announce to others that we have connected
    if (!t->mchat->stealth_mode)
    {
        send_len = mchatv1_format(t, send_buffer, MCHATV1_MESSAGE_TYPE_PING);
        if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
        {
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
            t->run_flag = 0;
        }
        if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
        {
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
            t->run_flag = 0;
        }
        if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
        {
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
            t->run_flag = 0;
        }
    }

    while (t->run_flag)
    {
        send_len = 0;
        gint64 timeout = g_get_monotonic_time() +
                (MCHAT_PROTOCOL_DEFAULT_KEEPALIVE_TIMER * G_TIME_SPAN_SECOND);
        // Make sure we wake up to send a ping if we don't have a message
        g_mutex_lock(&t->mutex);
        if (!t->buffer_flag)
            g_cond_wait_until(&t->cond, &t->mutex, timeout);

        if (t->buffer_flag)
        {
            send_len = mchatv1_format(t, send_buffer, MCHATV1_MESSAGE_TYPE_TEXT);
            t->buffer_flag = 0;
            g_cond_broadcast(&t->cond);
        }
        g_mutex_unlock(&t->mutex);

        // If we have a message, send it. If not, send a keepalive ping
        if (send_len)
        {
            if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
            {
                t->run_flag = 0;
                t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
                break;
            }
        }
        else
        {
            if (!t->mchat->stealth_mode)
            {
                send_len = mchatv1_format(t, send_buffer, MCHATV1_MESSAGE_TYPE_PING);
                if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
                {
                    t->run_flag = 0;
                    t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
                    break;
                }
            }
        }
    }
    g_free(t->buffer->body);
    g_free(t->buffer->nickname);
    return NULL;
}


gpointer mchatv1_thread_text_recv(gpointer args)
{
    struct mchat_thread *t = (struct mchat_thread *)args;
    // Allocate our message buffers
    t->buffer->body = g_malloc(sizeof(gchar) * MCHAT_LIMIT_MAX_MESSAGE_SIZE);
    t->buffer->nickname = g_malloc(sizeof(gchar) * MCHAT_LIMIT_MAX_NICKNAME_SIZE);

    // Mutex is locked until our buffer is allocated
    g_mutex_unlock(&t->mutex);
    // Let's be safe and make our buffer larger than a UDP packet can be
    gchar recv_buffer[1 << 16];
    gssize recv_len;
    gint64 recv_time;
    mchat_parser parser;
    GSocketAddress *saddr;
    GInetAddress *sinet;
    guint32 sbytes;

    while (t->run_flag)
    {
        recv_len = 0;
        memset(recv_buffer, 0, 1 << 16);
        memset(&parser, 0, sizeof(mchat_parser));
        if ((recv_len = g_socket_receive_from(t->sock, &saddr, recv_buffer, 1 << 16, t->cancel, NULL)) == -1)
        {
            t->run_flag = 0;
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
            break;
        }

        sinet = g_inet_socket_address_get_address((GInetSocketAddress*)saddr);
        const gchar *bytes = g_inet_address_to_bytes(sinet);
        memcpy(&sbytes, bytes, 4);
        g_object_unref(saddr);

        recv_time = g_get_real_time();
        if (mchatv1_parse_and_validate(&parser, recv_buffer, recv_len) == 0)
        {
            switch (parser.packet_type)
            {
                case MCHATV1_MESSAGE_TYPE_TEXT:
                {
                    g_mutex_lock(&t->mutex);
                    if (t->buffer_flag)
                        g_cond_wait(&t->cond, &t->mutex);

                    if (!t->buffer_flag)
                    {
                        mchatv1_parser_to_message(&parser, t->buffer);
                        t->buffer->timestamp = recv_time;
                        t->buffer->source_address = sbytes;
                        t->buffer_flag = 1;
                    }
                    g_mutex_unlock(&t->mutex);
                    peerlist_update_peer(t->mchat, parser, sbytes);
                    break;
                }
                case MCHATV1_MESSAGE_TYPE_PING:
                {
                    peerlist_update_peer(t->mchat, parser, sbytes);
                    break;
                }
            }
        }
    }
    g_free(t->buffer->body);
    g_free(t->buffer->nickname);
    return NULL;
}


gpointer mchatv1_thread_comm_send(gpointer args)
{
    mchat_thread *t = (mchat_thread *)args;

    /* Unlock the mutex, but as the thread-specific buffer is not used,
     * this is just so when the mutex is cleared, we don't crash.
     */
    g_mutex_unlock(&t->mutex);

    gchar send_buffer[1 << 16];
    guint32 send_len;

    /* Send out 3 pings to common channel on start */
    if (!t->mchat->stealth_mode)
    {
        send_len = mchatv1_format(t, send_buffer, MCHATV1_MESSAGE_TYPE_PING);
        if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
        {
            t->run_flag = 0;
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
        }
        if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
        {
            t->run_flag = 0;
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
        }
        if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
        {
            t->run_flag = 0;
            t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
        }
    }

    GTimer *ping_timer = g_timer_new();
    GTimer *cdsc_timer = g_timer_new();
    while (t->run_flag)
    {
        g_usleep(100000);	/* sleep for 0.1 seconds */
        send_len = 0;
        if (!t->mchat->stealth_mode)
        {
            if (g_timer_elapsed(ping_timer, NULL) >= MCHAT_PROTOCOL_DEFAULT_KEEPALIVE_TIMER)
            {
                g_timer_start(ping_timer);
                g_mutex_lock(&t->mchat->channels_mutex);
                send_len = mchatv1_format(t, send_buffer, MCHATV1_MESSAGE_TYPE_PING);
                g_mutex_unlock(&t->mchat->channels_mutex);
                if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
                {
                    t->run_flag = 0;
                    t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
                    break;
                }
            }
            if ((t->mchat->is_connected && t->mchat->current_channel != g_ptr_array_index(t->mchat->added_channels, 0)) &&
                    g_timer_elapsed(cdsc_timer, NULL) >= MCHAT_PROTOCOL_DEFAULT_CDSC_TIMER)
            {
                g_timer_start(cdsc_timer);
                g_mutex_lock(&t->mchat->channels_mutex);
                send_len = mchatv1_format(t, send_buffer, MCHATV1_MESSAGE_TYPE_CDSC);
                g_mutex_unlock(&t->mchat->channels_mutex);
                if (g_socket_send_to(t->sock, t->addr, send_buffer, send_len, t->cancel, NULL) != send_len)
                {
                    t->run_flag = 0;
                    t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
                    break;
                }
            }
        }
    }
    g_timer_destroy(ping_timer);
    g_timer_destroy(cdsc_timer);
    return NULL;
}


gpointer mchatv1_thread_comm_recv(gpointer args)
{
    mchat_thread *t = (mchat_thread *)args;

    g_socket_set_blocking(t->sock, FALSE);
    /* Unlock the mutex, but as the thread-specific buffer is not used,
     * this is just so when the mutex is cleared, we don't crash.
     */
    g_mutex_unlock(&t->mutex);

    gchar recv_buffer[1 << 16];
    gint64 recv_len;
    gint64 recv_time;
    mchat_parser parser;
    GSocketAddress *saddr;
    GInetAddress *sinet;
    guint32 sbytes;
    while (t->run_flag)
    {
        g_usleep(100000); /* Sleep for 0.1 seconds */
        recv_len = 0;
        memset(recv_buffer, 0, 1 << 16);
        memset(&parser, 0, sizeof(mchat_parser));
        recv_len = g_socket_receive_from(t->sock, &saddr, recv_buffer, 1 << 16, t->cancel, NULL);

        if (recv_len > 0)
        {
            /* This Requires some explanation.
             * The Peers are searched for by a 32 bit integer that is built from the 4 bytes of
             * the source IP address.  To remove the actual bytes, we first need to exract the
             * GInetAddress from the GSocketAddress returned by g_socket_receive_from.  We then
             * get the byte array from the GInetAddress and copy the bytes into an unsigned int.
             * This is really annoying to do, but it seems to work right now.
             */
            sinet = g_inet_socket_address_get_address((GInetSocketAddress*)saddr);
            const gchar *bytes = g_inet_address_to_bytes(sinet);
            memcpy(&sbytes, bytes, 4);
            g_object_unref(saddr);
            /* sinet does not need to be unref'ed.  Its already at zero reference when returned */
            recv_time = g_get_real_time();
            if (mchatv1_parse_and_validate(&parser, recv_buffer, recv_len) == 0)
            {
                switch (parser.packet_type)
                {
                    case MCHATV1_MESSAGE_TYPE_PING:
                    {
                        peerlist_update_peer(t->mchat, parser, sbytes);
                        break;
                    }
                    case MCHATV1_MESSAGE_TYPE_CDSC:
                    {
                        mchat_channel_update(t->mchat, &parser);
                        break;
                    }
                }
            }
        }
        else
        {
            /* Check that our sibling thread is still awake.  If not, we should exit */
            if (t->mchat->comm_send_thread->run_flag == 0)
            {
                t->run_flag = 0;
                t->thread_exit = MCHATV1_THREAD_ERROR_SOCKET_ERROR;
                break;
            }
        }
        peerlist_expire(t->mchat);
        mchat_channel_expire(t->mchat);
    }
}

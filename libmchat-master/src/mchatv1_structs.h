/*!
 * \file mchatv1_structs.h
 * \author Sean Tracy
 * \date 3 March 2017
 * \version 0.0.1
 * \brief MChatv1 API structure definitions
 *
 * \details
 * This file contains the structures used by libmchat.  This file
 * is for library internal use.
 */

#ifndef MCHATV1_STRUCTS_H
#define MCHATV1_STRUCTS_H

#include <glib.h>
#include <gio/gio.h>
#include "mchatv1.h"
#include "mchatv1_proto.h"

/*!
 * \brief Visible Peers list entry
 */
typedef struct mchat_peer
{
    gchar nickname[MCHAT_LIMIT_MAX_NICKNAME_SIZE];		/*!< Nickname of the peer */
    gchar channel[MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE];	/*!< Channel peer is connected to */
    guint32 nickname_len;								/*!< Length of the nickname string */
    guint32 channel_len;								/*!< Length of the channel name string */
    guint64 last_seen;									/*!< Last time the peer was seen */
    guint32 source_address;								/*!< The source address of the peer */
} mchat_peer;


/*!
 * \brief List of peers seen by mchat for use in public API functions
 */
struct mchat_peerlist_t
{
    mchat_peer *list;		/*!< List of peers */
    guint64 length;			/*!< length of list */
};


/*!
 * \brief MChat message send and receive buffer
 * \details
 * This struct is the mchat_message_t object used to store messages for use
 * by the mchat_message functions.  It is presented as an opaque object (like mchat_t),
 * so that it can be changed without breaking UI implementations.
 *
 * \todo Add to this as needed
 */
struct mchat_message_t
{
    guint32 packet_type;		//!< Packet type of the message
    gchar *nickname;			//!< nickname associated with the message
    gchar *body;				//!< message body text
    guint64 timestamp;			//!< timestamp of when the message was received
    guint32 nickname_len;		//!< length of nickname
    guint32 body_len;			//!< length of body
    guint32 source_address;		/*!< Source address of the message */
    guint32 parser_error;		//!< error associated with message from parser (if any)
    guint32 validation_error;	//!< error associated with message from validation (if any)
};


/*!
 * \brief MChat File I/O description for MChat threads
 *
 * \details
 * Used for threads that are fileio jobs.
 * \see mchat_thread
 */
typedef struct mchat_fileio
{
    guint64 *chunk_sizes;		//!< Size of each file chunk
    guint64 *chunk_offsets;		//!< Offset of each file chunk
    guint64 chunk_count;		//!< Number of chunks the file will be sent/recv'd in
    guint64 current_chunk;		//!< Current chunk being sent/Last chunk recv'd
    guint64 current_offset;		//!< Current chunk offset
    guint32 file_fd;			//!< File Descriptor for file read/write
    gchar *filename;			//!< Name of file to be sent or written to
    void *file_buffer;			//!< MMap'ed buffer for file
    gchar *file_shasum;			//!< File-long SHA256 Sum
    gchar **chunk_shasums;		//!< Per-Chunk SHA256 Sums
} mchat_fileio;


/*!
 * \brief MChat Thread Struct
 *
 * \details
 * MChat uses threads for all operations, including sending and receiving
 * text.
 */
typedef struct mchat_thread
{
    mchat_t *mchat;							/*!< pointer to parent mchat_t struct */
    GThread *thread_id;						/*!< pthread id for thread */
    GMutex mutex;							/*!< thread mutex for buffer access */
    GCond cond;								/*!< thread condition variable for buffer access */
    GSocketAddress *addr;					/*!< socket address object */
    GSocket *sock;							/*!< socket object */
    GCancellable *cancel;					/*!< cancel signal to get out of blocked IO operations */
    guint8 buffer_flag;						/*!< used as a flag to indicate status of message buffer (use varies) */
    guint8 run_flag;						/*!< used as a flag to indicate if the thread should stay running */
    mchat_message_t *buffer;				/*!< message buffer */
    struct mchat_fileio *fiocfg;			/*!< fileio structure if this thread is for a fileio job */
    guint32 thread_exit;					/*!< Exit error of thread */
} mchat_thread;


/*!
 * \brief MChat Channel Definition
 *
 * \details
 * MChat channels are named IP multicast address/UDP Port number pairs.
 * MChat channel names start with the symbol '#' (Just like that other chat protocol),
 * and consist of some sort of alphanumeric string.  Currently, there is no way to
 * automatically share channel information over the network, but such a protocol is
 * in the works.
 */
typedef struct mchat_channel
{
    gchar channel_name[MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE];		/*!< channel name including the '#' symbol */
    GInetAddress *channel_address;								/*!< IP multicast address to use for channel */
    guint16 channel_portno;										/*!< UDP port number for channel */
    guint32 channel_id;											/*!< ID number used for indexing */
    guint64 last_seen;											/*!< if the channel was not added, this is when it was last seen */
} mchat_channel;


/*!
 * \brief List of channels from an mchat object
 */
struct mchat_chanlist_t
{
    mchat_channel *list;	/*!< list of channels */
    guint64 length;			/*!< length of the list */
};


/*!
 * \brief The primary object used to interact with the MChat API
 *
 * \details
 * This is the struct used to interact with the MChat API.  It is kept
 * as a opaque object in the API so that it can be changed without affecting
 * UI applications.
 *
 * \note I'm not sure if we really need a thread for sending or not -Sean
 */
struct mchat_t
{
    mchat_thread *text_send_thread;			//!< thread used for text send operations
    mchat_thread *text_recv_thread;			//!< thread used for text recv operations
    mchat_thread *comm_send_thread;			//!< thread used for sending message on common channel
    mchat_thread *comm_recv_thread;			//!< thread used to receive messages on common channel
    mchat_thread *fileio_thread;			//!< thread used for fileio jobs
    gchar *bind_address;					//!< Optional bind address for mchat traffic
    gchar *nickname;						//!< nickname for mchat connections
    guint8 nickname_size;					//!< nickname size (if that wasn't obvious to you)
    guint8 is_connected : 1;				/*!< boolean set if the mchat object is connected to a channel */
    guint8 stealth_mode: 1;					/*!< boolean set if stealth mode is on */
    guint8 reserved : 6;					/*!< unused boolean flags space */
    GArray *peerlist;						/*!< List of peers seen (Only used by recv threads) */
    GMutex peerlist_mutex;					/*!< Mutex for write access to peerlist by send/recv threads */
    GPtrArray *added_channels;				/*!< List of added channels */
    GPtrArray *cdsc_channels;				/*!< List of channels discovered through CDSC packets */
    GMutex channels_mutex;					/*!< Mutex for write access to channels members by send/recv threads */
    mchat_channel *current_channel;			/*!< Current connected channel (Undefined when not connected) */
};

/*!
 * \brief MChatv1 Received message parser struct
 *
 * \details This is the object the parser system uses to store message information.
 * It is designed to be converted to an mchatv1_message or a file chunk after it has
 * been parsed and validated.
 */
typedef struct mchat_parser
{
    guint16 version_major;								//!< MChat Version Major Number
    guint16 version_minor;								//!< MChat Version Minor Number
    guint32 packet_type;								//!< MChat message type
    guint32 total_size;									//!< Total packet length in bytes
    guint32 parser_error;								//!< Error number from parser if any
    guint32 validation_error;							//!< Error number for validation if any
    gchar *body;										//!< Pointer to message body, if any
    gint32 body_size;									//!< Length of the body in bytes
    gchar *header_offset[MCHATV1_HEADER_TYPES_COUNT];	//!< Pointer to header values
    guint32 header_len[MCHATV1_HEADER_TYPES_COUNT];		//!< Lengths of header values

    gchar *message_start;								//!< Pointer to start of the Mchat Message
} mchat_parser;

#endif // MCHATV1_STRUCTS_H

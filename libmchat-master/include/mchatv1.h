/*!
 * \file mchatv1.h
 * \author Sean Tracy
 * \date 2 March 2017
 * \version 0.0.1
 * \brief Public API calls and definitions for the MChat Library
 *
 * \details
 * This file contains the API for libmchat MCHATv1.  MChat is a peer-to-peer
 * chatting protocol that uses IP multicast.  It is geared for use
 * on a local area network (LAN), but can be used over the internet
 * if multicast routing is available.  MChat uses a http-like message
 * structure to send and receive simple text messages and files.
 */

#ifndef MCHATV1_H
#define MCHATV1_H

//! \name MChat Protocol Version
//! @{

//! Major Number
#define MCHAT_PROTOCOL_VERSION_MAJOR 1
//! Minor Number
#define MCHAT_PROTOCOL_VERSION_MINOR 0
//! @}

//! \name MChat Limits
//! @{

//! The maximum length for a MChat Nickname.
#define MCHAT_LIMIT_MAX_NICKNAME_SIZE 64

//! The maximum number of concurrent File I/0 Jobs. (Not used yet -Sean)
#define MCHAT_LIMIT_MAX_FILEIO_JOBS 16

//! The maximum number of defined channels allowed by the MChat Library.
#define MCHAT_LIMIT_MAX_CHANNELS 64

//! The maximum message Size (32 KB)
#define MCHAT_LIMIT_MAX_MESSAGE_SIZE 1 << 15

//! The maximum length of an MChat channel name
#define MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE 64

//! @}


/*!
 * \name MChat Common Channel Definition
 * @{
 */

/*! Common channel multicast address */
#define MCHAT_PROTOCOL_COMMON_CHANNEL_ADDRESS "230.0.0.0"

/*! Common channel udp port */
#define MCHAT_PROTOCOL_COMMON_CHANNEL_PORT 9009

/*! @} */


//! \name Default #mchat Channel Definition
//! @{

//! Channel Name
#define MCHAT_PROTOCOL_DEFAULT_CHANNEL_NAME "#mchat"
//! Channel IP Address
#define MCHAT_PROTOCOL_DEFAULT_CHANNEL_ADDRESS "230.0.0.1"
//! Channel UDP Port Number
#define MCHAT_PROTOCOL_DEFAULT_CHANNEL_PORT 9009
//! @}

//! The time interval to send a ping for keepalive (in seconds)
#define MCHAT_PROTOCOL_DEFAULT_KEEPALIVE_TIMER 3

/*! The time interval a user has not been seen before removal
 * from the peer list */
#define MCHAT_PROTOCOL_DEFAULT_EXPIRE_INTERVAL \
    MCHAT_PROTOCOL_DEFAULT_KEEPALIVE_TIMER * 5

/*! The time interval between sending cdsc messages */
#define MCHAT_PROTOCOL_DEFAULT_CDSC_TIMER 10

#define MCHAT_PROTOCOL_DEFAULT_CDSC_EXPIRE \
    MCHAT_PROTOCOL_DEFAULT_CDSC_TIMER * 5

#ifndef _MCHAT_T_TYPEDEF_DEFINED
//! \brief Set anywhere mchat_t typedef is specified
#define _MCHAT_T_TYPEDEF_DEFINED
/*!
 * \brief The primary mchat structure
 * \see mchatv1_structs.h
 */
typedef struct mchat_t mchat_t;
#endif // _MCHAT_T_TYPEDEF_DEFINED

/*!
 * \brief MChat message object
 * \see mchatv1_structs.h
 */
typedef struct mchat_message_t mchat_message_t;

/*!
 * \brief MChat peerlist object
 * \see mchatv1_structs.h
 */
typedef struct mchat_peerlist_t mchat_peerlist_t;

/*!
 * \brief MChat chanlist object
 * \see mchatv1_structs.h
 */
typedef struct mchat_chanlist_t mchat_chanlist_t;

/*!
 * \name MChat API
 * @{
 */

/*!
 * \brief Create an mchat object to interact with the rest of the API
 * \param cfg_filename Optional filename for predefined nickname and channel information
 *        (Set to NULL to disable)
 * \return An initialized mchat object or NULL on error
 *
 * \details
 * This function allocates and initializes an mchat object for use with the rest of the MChat API.
 * It allocates all of the internal structures, adds the default channel \#mchat, and generates a
 * random nickname if one was not supplied by the file \p cfg_filename.  If this function returns
 * NULL, check the value of errno, as it is most likely caused by a library call.
 */
mchat_t *mchatv1_init(char *cfg_filename);

/*!
 * \brief Cleanup an mchat object and gracefully shutdown any connections
 * \param mchat Reference to the mchat object pointer returned by ::mchatv1_init()
 * \return 0 on success or -1 on error
 */
int mchatv1_destroy(mchat_t **mchat);


/*!
 * \brief Get the value of the stealth-mode bit
 * \param mchat Pointer to an mchat object
 * \return 1 if stealth mode enabled, 0 if not
 * \see ::mchatv1_get_stealth_mode for details
 */
int mchatv1_get_stealth_mode(mchat_t *mchat);

/*!
 * \brief Set whether to use stealth mode or not
 * \param mchat Pointer to an mchat object
 * \param stealth If 0, stealth mode is disabled; if not 0 stealth mode is enabled
 * \return 1 if stealth mode has been turned on, 0 if not
 *
 * \details
 * Stealth mode makes us less noisy to other nodes on a network.  When stealth mode is enabled,
 * PING messages are no longer sent, on either the common or connected channel, and CDSCs are
 * not sent.  This does not prevent other users from seeing you on a network if you send TEXT packets,
 * but allows for an almost silent receive.  The only packets transmitted are IGMP join, leave and query
 * messages.  This does not prevent, say, a user running a packet capture/analyzer from seeing you, but
 * prevents other nodes from seeing you in a program using libmchat.
 *
 * \note This function is more to reduce network noise than to act as some sort of security or anonymity
 * feature.
 */
int mchatv1_set_stealth_mode(mchat_t *mchat, int stealth);

/*!
 * \brief Connect to an MChat Channel
 * \param mchat Pointer to an mchat object
 * \param channel Name of the channel to connect to, or NULL for default \#mchat channel
 * \return 0 on success or -1 on error
 *
 * \details
 * Make a connection to an MChat channel, to begin chatting.
 */
int mchatv1_connect(mchat_t *mchat, char *channel);

/*!
 * \brief Diconnect from the active MChat channel
 * \param mchat Pointer to an mchat object
 * \return 0 on success or -1 on error
 */
int mchatv1_disconnect(mchat_t *mchat);

/*!
 * \brief Check if an mchat object is connected to a channel
 * \param mchat Pointer to an mchat object
 * \return 1 if connected, 0 if not
 */
int mchatv1_is_connected(mchat_t *mchat);

/*!
 * \brief Send a text message to a connected mchat object
 * \param mchat Pointer to an mchat object
 * \param message Character pointer to a message buffer to send
 * \return The size of the string sent or -1 on error
 */
int mchatv1_send_message(mchat_t *mchat, char *message);

/*!
 * \brief Get a message from a connected mchat object
 * \param mchat Pointer to an mchat object
 * \param message Double point to an mchat message object
 * \return 1 on available messege, 0 on no message, and -1 on error
 *
 * \details
 * Returns a message from mchat if available.  Does not block (for very long) and returns null
 * if no message is available or the mchat is not connected.  Use the mchatv1_message functions to
 * decode the message.
 *
 */
int mchatv1_recv_message(mchat_t *mchat, mchat_message_t** message);

/*!
 * \brief Start a file send job
 * \param mchat Pointer to an mchat object
 * \param filename Filename to send
 * \return 0 on success or -1 on error
 */
int mchatv1_send_file(mchat_t *mchat, char *filename);

/*!
 * \brief Stop a file send job
 * \param mchat Pointer to an mchat object
 * \return 0 on success or -1 on error
 */
int mchatv1_send_file_stop(mchat_t *mchat);

/*!
 * \brief Start a file receive job
 * \param mchat Pointer to an mchat object
 * \param filename Filename to save file to
 * \return 0 on sucess or -1 on error
 */
int mchatv1_recv_file(mchat_t *mchat, char *filename);

/*!
 * \brief Stop a file receive job
 * \param mchat Pointer to an mchat object
 * \return 0 on success or -1 on error
 */
int mchatv1_recv_file_stop(mchat_t *mchat);

/*!
 * \brief Set the mchat nickname used in chat
 * \param mchat Pointer to an mchat object
 * \param new_nickname Character pointer to new nickname
 * \param len Length of the new nickname pointed to by \p new_nickname
 * \return 0 on success or -1 on error
 */
int mchatv1_set_nickname(mchat_t *mchat, char *new_nickname, unsigned int len);

/*!
 * \brief Get the mchat nickname from an mchat object
 * \param mchat Pointer to an mchat object
 * \param buf Buffer to place the nickname into
 * \param buf_size Size of \p buf
 * \return Length of nickname put into \p buf or -1 on error
 */
int mchatv1_get_nickname(mchat_t *mchat, char *buf, unsigned int buf_size);

/*!
 * \brief Not implemented
 * \param mchat Pointer to an mchat object
 * \param presence
 * \return 0
 */
int mchatv1_set_presence(mchat_t *mchat, unsigned int presence);

/*!
 * \brief Not implemented
 * \param mchat Pointer to an mchat object
 * \return 0
 */
int mchatv1_get_presence(mchat_t *mchat);


//! @}

/*!
 * \name MChat Channel API
 * @{
 */

/*!
 * \brief Add a new channel definition to MChat
 * \param mchat Pointer to an mchat object
 * \param channel Channel name to add
 * \param channel_address IP Address the channel will use
 * \param channel_portno UDP Port Number the channel will use
 * \return 0 on success or -1 on error
 *
 * \details
 * Add a new named channel to MChat.  This allows a new channel to be defined on the fly.
 * \note \p channel_address should be an IPv4 Multicast address (224.0.0.0/4)
 * \note \p channel_portno should be greater than 1024 (running as root is a bad idea, A REALLY BAD IDEA)
 */
int mchatv1_add_channel(mchat_t *mchat, char *channel, char *channel_address, unsigned short channel_portno);

/*!
 * \brief Delete a channel definition from MChat
 * \param mchat Pointer to an mchat object
 * \param channel Channel name to delete
 * \return 0 on success or -1 on error
 *
 * \warning You cannot delete the channel \#mchat
 */
int mchatv1_del_channel(mchat_t *mchat, char *channel);

/*!
 * \brief Get the current connected channel
 * \param mchat Pointer to an mchat object
 * \param buf Buffer to place the channel name into
 * \param buf_size The size of \p buf
 * \return The size of the string put into \p buf or -1 on error
 *
 * \details
 * This function copies the current connected channel into \p buf.  If mchat is not connected,
 * -1 is returned.
 * \note if the return value is equal to \p buf_size, there is a good chance the channel name was longer
 * than the allocated buffer.  Increase your buffer size and try again.
 */
int mchatv1_get_channel(mchat_t *mchat, char *buf, unsigned int buf_size);

/*!
 * \brief Get the count of currently defined channels in MChat
 * \param mchat Pointer to an mchat object
 * \return The number of defined channels or -1 on error
 */
int mchatv1_get_channel_count(mchat_t *mchat);

/*!
 * \brief mchatv1_get_added_channels
 * \param mchat
 * \param channel_list
 * \return
 */
int mchatv1_get_added_channels(mchat_t *mchat, mchat_chanlist_t **channel_list);

/*!
 * \brief mchatv1_get_cdsc_channels
 * \param mchat
 * \param channel_list
 * \return
 */
int mchatv1_get_cdsc_channels(mchat_t *mchat, mchat_chanlist_t **channel_list);

/*!
 * \brief mchatv1_chanlist_size
 * \param channel_list
 * \return
 */
int mchatv1_chanlist_size(mchat_chanlist_t *channel_list);

/*!
 * \brief mchatv1_chanlist_get_channel_name
 * \param channel
 * \param index
 * \param buf
 * \param buf_size
 * \return
 */
int mchatv1_chanlist_get_channel_name(mchat_chanlist_t *channel, unsigned int index, char *buf, unsigned int buf_size);

/*!
 * \brief mchatv1_chanlist_get_channel_address
 * \param channel
 * \param index
 * \param buf
 * \param buf_size
 * \return
 */
int mchatv1_chanlist_get_channel_address(mchat_chanlist_t *channel, unsigned int index, char *buf, unsigned int buf_size);

/*!
 * \brief mchatv1_chanlist_get_channel_port
 * \param channel
 * \param index
 * \param port
 * \return
 */
int mchatv1_chanlist_get_channel_port(mchat_chanlist_t *channel, unsigned int index, unsigned short *port);

/*!
 * \brief mchatv1_chanlist_get_channel_time
 * \param channel
 * \param index
 * \param time
 * \return
 */
int mchatv1_chanlist_get_channel_time(mchat_chanlist_t *channel, unsigned int index, long *time);

/*!
 * \brief mchatv1_chanlist_get_channel_info
 * \param channel
 * \param index
 * \param name_buf
 * \param name_buf_size
 * \param addr_buf
 * \param addr_buf_size
 * \param portno
 * \param time
 * \return
 */
int mchatv1_chanlist_get_channel_info(mchat_chanlist_t *channel, unsigned int index,
                                      char *name_buf, unsigned int name_buf_size,
                                      char *addr_buf, unsigned int addr_buf_size,
                                      unsigned short *portno, long *time);

/*!
 * \brief mchatv1_chanlist_destroy
 * \param channel_list
 * \return
 */
int mchatv1_chanlist_destroy(mchat_chanlist_t **channel_list);

/*! @} */


/*!
 * \name MChat Message API
 * @{
 */

/*!
 * \brief Return the message body size of the message in \p packet
 * \param packet Pointer to an mchat message object
 * \return Size of the message body in \p packet or -1 on error
 */
int mchatv1_message_get_body_size(mchat_message_t *packet);

/*!
 * \brief Copy the message body in \p packet into a local buffer \p buf
 * \param packet Pointer to an mchat message object
 * \param buf Pointer to an allocated buffer to place the message body in
 * \param len Size of the the allocated buffer \p buf
 * \return The number of bytes written to \p buf or -1 on error
 */
int mchatv1_message_get_body(mchat_message_t *packet, char *buf, unsigned int len);

/*!
 * \brief Copy the nickname associated with the message body in \p packet
 * \param packet Pointer to an mchat message object
 * \param buf Pointer to an allocated buffer to place the nickname in
 * \param len Size of the allocated buffer \p buf
 * \return The number of bytes written to \p buf or -1 on error
 *
 * \note It is recommended that buf be allocated to the size specified in
 * MCHAT_LIMIT_MAX_NICKNAME_SIZE
 */
int mchatv1_message_get_nickname(mchat_message_t *packet, char *buf, unsigned int len);

/*!
 * \brief Get the timestamp associated with message in \p packet
 * \param packet Pointer to an mchat message object
 * \param t Pointer to a time_t type to place timestamp into
 * \return 0 on success or -1 on error
 */
int mchatv1_message_get_timestamp(mchat_message_t *packet, long *t);

/*!
 * \brief mchatv1_message_has_error
 * \param packet
 * \return
 */
int mchatv1_message_has_error(mchat_message_t *packet);

/*!
 * \brief mchatv1_message_strerror
 * \param packet
 * \return
 */
const char *mchatv1_message_strerror(mchat_message_t *packet);

/*!
 * \brief mchatv1_message_source_address
 * \param packet
 * \param buf
 * \param size
 * \return
 */
int mchatv1_message_source_address(mchat_message_t *packet, char *buf, unsigned int size);

/*!
 * \brief Destroy an mchat message object when it is no longer needed
 * \param packet Double pointer to an mchat message object
 * \return 0 on success or -1 on error
 */
int mchatv1_message_destroy(mchat_message_t **packet);

//! @}


/*!
 * \name MChat Peer API
 * \details
 * This is the initial version of the peer api.  Its designed to present a static
 * view of the current peer list.  This may change in the future when we start
 * focusing on GUI oriented code, but they should work for now.
 * @{
 */

/*!
 * \brief Determine if there are peers in the peerlist
 * \param mchat Pointer to an mchat object
 * \return 1 if peers are available, 0 if otherwise
 */
int mchatv1_peers_available(mchat_t *mchat);

/*!
 * \brief Returns a list of seen peers in an array of mchat_peer_t object
 * \param mchat Pointer to an mchat object
 * \param peerlist Double pointer to a mchat peerlist object
 * \return 1 if a peerlist was returned, 0 if no peers were available, or -1 on error
 *
 * \details
 * This function returns a list of peers seen at the time of the function call.
 * This list does NOT update dynamically and does not necessarily represent the
 * the peer list right after the call completes.
 */
int mchatv1_get_peerlist(mchat_t *mchat, mchat_peerlist_t **peerlist);

/*!
 * \brief Return the total size of the peer list (The number of peers)
 * \param peerlist Pointer to an mchat peerlist object
 * \return The length of the peer list (0 or greater) or -1 on error
 */
int mchatv1_peerlist_get_size(mchat_peerlist_t *peerlist);

/*!
 * \brief Get the nickname of the peer in the peerlist at the specified index
 * \param peerlist Pointer to an mchat peerlist object
 * \param index Index of the peer to inspect
 * \param buf Pointer to a character buffer to place the nickname in
 * \param buf_size Length of the buffer pointed to by \p buf (Should be MCHAT_LIMIT_MAX_NICKNAME_SIZE)
 * \return Number of bytes copied or -1 on error
 */
int mchatv1_peer_get_name(mchat_peerlist_t *peerlist, unsigned int index, char *buf, unsigned int buf_size);

/*!
 * \brief Get the channel name associated with the peer in the peerlist at the specified index
 * \param peerlist Pointer to an mchat peerlist object
 * \param index Index of the peer to inspect
 * \param buf Pointer to a character buffer to place the channel name in
 * \param buf_size Length of the buffer pointed to by \p buf (Should be MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE)
 * \return Number of bytes copied or -1 on error
 */
int mchatv1_peer_get_channel(mchat_peerlist_t *peerlist, unsigned int index, char *buf, unsigned int buf_size);

/*!
 * \brief Get the last seen time of a peer in the peerlist at the specified index
 * \param peerlist Pointer to an mchat peerlist object
 * \param index Index of the peer to inspect
 * \param t Pointer to a long to put the last seen time (stored as UNIX epoch time in microseconds)
 * \return 0 on success or -1 on error
 */
int mchatv1_peer_get_timestamp(mchat_peerlist_t *peerlist, unsigned int index, long *t);

/*!
 * \brief Get the most relevant information about a peer in the peerlist at the specified index
 * \param peerlist Pointer to an mchat peerlist object
 * \param index Index of the peer to inspect
 * \param nick_buf Pointer to a character buffer to place the nickname into
 * \param channel_buf Pointer to a character buffer to place the channel name into
 * \param nick_buf_size Length of the buffer pointed to by \p nick_buf (Should be MCHAT_LIMIT_MAX_NICKNAME_SIZE)
 * \param channel_buf_size Lenth of the buffer pointed to by \p channel_buf (Should be MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE)
 * \param t Pointer to a long to put the last seen time (stored as UNIX epoch time as microseconds)
 * \return 0 on success, -1 on invalid index, -2 on \p nick_buf too small, or -3 on \p channel_buf too small
 */
int mchatv1_peer_get_peer(mchat_peerlist_t *peerlist, unsigned int index, char *nick_buf, char *channel_buf,
                          unsigned int nick_buf_size, unsigned int channel_buf_size, long *t);

/*!
 * \brief Get the source address of a peer (their IP address)
 * \param peerlist Pointer to an mchat peerlist object
 * \param index Index of the peer to inspect
 * \param addr_buf Character buffer to place the IP address into
 * \param size Size of \p (Note: should be 15 right now as we only are handling IPv4 at the moment)
 * \return Number of bytes copied or -1 on error
 *
 * \details
 * This function returns an IP address as a character string of the peer in \p peerlist at index \p index.
 * The IP address is returned as an ASCII (UTF-8) string in dotted quad notation.  This can be used for
 * both statistical purposes or possibly direct chat configuration sometime in the future.
 *
 * \warning A peer can change their IP address, so this should NOT be used for any sort of validation
 * or access control mechanisms.  This function should be used for informational purposes only
 */
int mchatv1_peer_get_source_address(mchat_peerlist_t *peerlist, unsigned int index, char *addr_buf, unsigned int size);

/*!
 * \brief Free an mchat peerlist object
 * \param peerlist Double pointer to an mchat peerlist object
 * \return 0 on success or -1 on error
 */
int mchatv1_peerlist_destroy(mchat_peerlist_t **peerlist);

/*! @} */

#endif // MCHATV1_H

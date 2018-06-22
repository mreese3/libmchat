/*!
 * \file mchatv1_threads.h
 * \author Sean Tracy
 * \version 0.0.1
 * \date 24 March 2017
 * \brief MChatv1 Thread related functions
 *
 * \details
 * \todo More documentation on this file
 */
#ifndef MCHATV1_THREADS_H
#define MCHATV1_THREADS_H

#include <gio/gio.h>
#include "mchatv1.h"
#include "mchatv1_structs.h"
#include "mchatv1_macro_hell.h"

/*!
 * \name MChat Thread Error Macros
 * @{
 */

/*! MAP of error names and error strings */
#define MCHATV1_THREAD_ERRORS_MAP(MAP_MACRO) \
    MAP_MACRO(NO_ERROR, "Success") /*!< No error was detected (thread shut down normally) */ \
    MAP_MACRO(SOCKET_ERROR, "Thread Socket Error") /*!< The socket for the thread reported an error condition (get socket error from glib) */ \
    /*! */ \

/*! MAP Macro to convert names in map to proper enum constants */
#define MCHATV1_THREAD_ERRORS_ENUM_NAME(name) \
    MCHATV1_THREAD_ERROR_##name

/*! MAP Macro to convert names in map to an list of enum constants */
#define MAP_MACRO_THREAD_ERRORS_ENUM(name, string) \
    MCHATV1_THREAD_ERRORS_ENUM_NAME(name),

/* @} */

enum mchatv1_thread_errors
{
    MCHATV1_THREAD_ERRORS_MAP(MAP_MACRO_THREAD_ERRORS_ENUM)
};

extern const char *mchatv1_thread_error_strings[];

/*!
 * \name MChat Thread Utility Functions
 * @{
 */


/*!
 * \brief Initialize an mchat_thread struct
 * \param mchat Pointer to mchat_t struct
 * \param tptr Double Pointer to mchat_thread struct
 * \param thread_name Optional thread name
 * \param addr Address for the socket operations
 * \param sock Socket object
 * \param thread_func Thread function to lauch
 * \param fiocfg File I/O struct or NULL
 * \return 0 on success or -1 on error
 */
int mchatv1_thread_init(mchat_t *mchat, mchat_thread **tptr, gchar *thread_name,
                      GSocketAddress *addr, GSocket *sock,
                      gpointer (*thread_func)(gpointer), mchat_fileio *fiocfg);

/*!
 * \brief Teardown an mchat_thread struct
 * \param tptr Double Pointer to an mchat_thread struct
 * \return Always 0 (for now)
 *
 * \details
 * This function tears down an mchat_thread struct.
 *
 * \warning This function assumes a number of things, including
 * that the message buffer has been initialized and that the
 * mchat_thread struct has been initialized by mchat_thread_init().
 * Use with caution if these things have not been done.
 */
int mchatv1_thread_destroy(struct mchat_thread **tptr);

/*! @} */


/*!
 * \name MChat Thread Functions
 * \brief These are the main threading functions for mchat
 * @{
 */

/*!
 * \brief Thread used for message send operations and pings
 * \param args Void pointer to mchat_thread struct
 * \returns NULL (Technically no one listens for it, so it is irrelevant)
 */
gpointer mchatv1_thread_text_send(gpointer args);

/*!
 * \brief Thread used for message receive operations
 * \param args Void pointer to mchat_thread struct
 * \returns NULL (Technically no one listens for it, so it is irrelevant)
 */
gpointer mchatv1_thread_text_recv(gpointer args);

/*!
 * \brief Thread used to send messsages on common channel
 * \param args Void pointer to mchat_thread struct
 * \returns NULL (Technically no one listens for it, so it is irrelevant)
 *
 * \details
 * This function is used to send on the common channel for mchat.  This
 * is primarily used for presence info (pings) and channel information (CDSC).
 * It can also send other types of non-chat information in the future, such as
 * non channel-bound file I/O jobs, explicit connect and disconnect messages,
 * Channel encryption information, and others.
 *
 */
gpointer mchatv1_thread_comm_send(gpointer args);

/*!
 * \brief Thread used to receive messages on common channel
 * \param args Void pointer to mchat_thread struct
 * \returns NULL (Technically no one listens for it, so it is irrelevant)
 *
 */
gpointer mchatv1_thread_comm_recv(gpointer args);
/*! @} */

#endif // MCHATV1_THREADS_H

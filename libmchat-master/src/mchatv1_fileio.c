/*!
 * \file mchatv1_fileio.c
 * \author Sean Tracy
 * \date 3 March 2017
 * \version 0.0.1
 * \brief MChatv1 File I/O functions
 *
 * \details
 * This file contains the mchat_send_file and mchat_recv_file functions.
 * These functions will need a LOT of boilerplate to deal with file I/O,
 * SHA sums, etc., so they are put into their own file.
 */

//To Do
#include "mchatv1.h"
#include "mchatv1_structs.h"

/*!
 * \brief File Send Thread
 * \param arg Argument struct (todo)
 * \return NULL
 */
void *mchat_send_file_thread(void *arg)
{
    //! \todo This Needs to be written
    return (void*)0; //NULL
}


/*!
 * \brief File Receive Thread
 * \param arg Argument struct (todo)
 * \return NULL
 */
void *mchat_recv_file_thread(void *arg)
{
    //! \todo This Needs to be written
    return (void*)0; //NULL
}


// API Functions
int mchatv1_send_file(mchat_t *mchat, char *filename)
{
    //! \todo This Needs to be written
    return 0;
}


int mchatv1_send_file_stop(mchat_t *mchat)
{
    //! \todo This Needs to be written
    return 0;
}


int mchatv1_recv_file(mchat_t *mchat, char *filename)
{
    //! \todo This Needs to be written
    return 0;
}


int mchatv1_recv_file_stop(mchat_t *mchat)
{
    //! \todo This Needs to be written
    return 0;
}

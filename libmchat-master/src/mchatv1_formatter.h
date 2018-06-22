/*!
 * \file mchatv1_formatter.h
 * \author Sean Tracy
 * \date 9 March 2017
 * \version 0.0.1
 * \brief MChatv1 Message formatting functions
 *
 * \details
 * This file defines the internal API to format MChatv1 message for sending.
 * The format function convert mchatv1 structs into a text message to send
 * out over the network.
 */
#ifndef MCHATV1_FORMATTER_H
#define MCHATV1_FORMATTER_H

#include "mchatv1_structs.h"

#define HEADER_FORMAT_START(header_type, _offset, _dest) \
    _offset = strlen(mchatv1_header_type_strings[header_type]); \
    memcpy(_dest, mchatv1_header_type_strings[header_type], _offset); \
    _dest[_offset++] = ':'; \
    _dest[_offset++] = ' '

#define FORMAT_CRLF(_offset, _dest) \
    _dest[_offset++] = '\r'; \
    _dest[_offset++] = '\n'


/*!
 * \brief Create a formatted message string for sending
 * \param thread_info Pointer to calling mchat_thread struct
 * \param dest Destination buffer to put the formatted packet
 * \param type MChat message type to send
 * \return Size of the message in \p dest or -1 on error
 */
int mchatv1_format(mchat_thread *thread_info, char *dest, enum mchatv1_type type);

#endif // MCHATV1_FORMATTER_H

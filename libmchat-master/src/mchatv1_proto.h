/*!
 * \file mchatv1_proto.h
 * \author Sean Tracy
 * \date 7 March 2017
 * \version 0.0.1
 * \brief MChatv1 Protocol Definitions
 *
 * \details
 * This file defines the MChatv1 message structure, header types,
 * message types, etc.  The MChatv1 protocol resembles http with the
 * format:
 * \verbatim
 * MESG MCHAT/1.0
 * Nickname: NoNick123456
 * Length: 5
 *
 * Hello \endverbatim
 *
 *
 * \todo finish documenting this file.
 */
#ifndef MCHATV1_PROTO_H
#define MCHATV1_PROTO_H

#include "mchatv1_macro_hell.h"

/*!
 * \name Maps
 * \brief MChatv1 Protocol Name/String maps
 *
 * \note These are called X Macros by the way
 *
 * @{
 */

/*!
 * \brief Define MChat v1 Message types
 *
 * \details
 * This defines the possible message types in MChatv1.  This map is converted
 * into \link #mchatv1_type \endlink enum by the \link #MAP_MACRO_MESSAGE_TYPE_ENUM \endlink
 * macro.
 *
 * \note the format of the map is MAP_MACRO(NAME, LIST OF REQUIRED HEADERS) / * ! < comment * /.
 * The name should be in all capitals and the comment is put into the doxygen documentation.
 * Try to keep new message types 4 characters long (for uniformity's sake)
 *
 * \warning Keep the empty comment string as the last line in the macro.  This is due to the
 * way doxygen works.
 */
#define MCHATV1_MESSAGE_TYPES_MAP(MAP_MACRO) \
    MAP_MACRO(NONE) /*!< Used for error indication; should not be sent */ \
    MAP_MACRO(TEXT, NICKNAME, LENGTH, CHANNEL) /*!< Chat message for printing */ \
    MAP_MACRO(FILE, NICKNAME, LENGTH, FILENAME, FILESUM, CHUNK, \
        CHUNKCOUNT, FILESUM, CHUNKSUM) /*!< File chuck message used in file sending */ \
    MAP_MACRO(PING, NICKNAME, CHANNEL) /*!< (Not used yet) for presence information */ \
    MAP_MACRO(CDSC, CHANNEL, ADDRESS, PORT) /*!< (Not used yet) Channel Description informantion */ \
    /*! */

/*!
 * \brief Define MChatv1 Header types
 *
 * \details
 * This map macro defines the accepted headers in MChatv1.
 *
 * \note To add new header types, the format is MAP_MACRO(NAME, name, Name) / * ! < comment * / \\
 *
 * \note Make sure to add a _parse function to mchatv1_parser.c
 *
 * \warning Keep the empty comment string as the last line in the macro.  This is due to the
 * way doxygen works.
 *
 */
#define MCHATV1_HEADER_TYPES_MAP(MAP_MACRO) \
    MAP_MACRO(NICKNAME, nickname, Nickname) /*!< Nickname of the message sender */ \
    MAP_MACRO(LENGTH, length, Length) /*!< Length of the message body */ \
    MAP_MACRO(FILENAME, filename, Filename) /*!< Filename of file being sent */ \
    MAP_MACRO(FILESUM, filesum, Filesum) /*!< File-long checksum (SHA256) */ \
    MAP_MACRO(CHUNK, chunk, Chunk) /*!< File chunk in message body */ \
    MAP_MACRO(CHUNKCOUNT, chunkcount, Chunkcount) /*!< The amount of chunks the file has been cut into */ \
    MAP_MACRO(CHUNKSUM, chunksum, Chunksum) /*!< The chunk checksum (SHA256) */ \
    MAP_MACRO(CHANNEL, channel, Channel) /*!< The current channel the message is associated with */ \
    MAP_MACRO(PRESENCE, presence, Presence) /*!< Not used yet - used for setting presence info */ \
    MAP_MACRO(ADDRESS, address, Address) /*!< Not used yet - Channel IP Address in CDSC */ \
    MAP_MACRO(PORT, port, Port) /*!< Not used yet - Channel Port Number in CDSC */ \
    /*! */

//! @}


/*!
 * \name Mapping Function Macros
 * \brief These macro convert the data in the Maps into a usable format for use with C datatypes.
 *
 * @{
 */

/*!
 * \brief MAP \link #MCHATV1_MESSAGE_TYPES_MAP \endlink to \link #mchatv1_type \endlink enum
 */
#define MAP_MACRO_MESSAGE_TYPE_ENUM_(name) MCHATV1_MESSAGE_TYPE_##name

/*!
 * \brief MAP \link #MCHATV1_MESSAGE_TYPES_MAP \endlink to \link #mchatv1_type \endlink enum
 */
#define MAP_MACRO_MESSAGE_TYPE_ENUM(name, ...) MAP_MACRO_MESSAGE_TYPE_ENUM_(name),

/*!
 * \brief MAP \link #MCHATV1_MESSAGE_TYPES_MAP \endlink to string arrays for
 * \link #mchatv1_message_type_strings \endlink
 */
#define MAP_MACRO_MESSAGE_TYPE_STRING(name, ...)  #name,

/*!
 *\brief MAP \link #MCHATV1_HEADER_TYPES_MAP \endlink to \link #mchatv1_header_type \endlink enum
 */
#define MAP_MACRO_HEADER_TYPE_ENUM_(name) MCHATV1_HEADER_TYPE_##name

/*!
 *\brief MAP \link #MCHATV1_HEADER_TYPES_MAP \endlink to \link #mchatv1_header_type \endlink enum
 */
#define MAP_MACRO_HEADER_TYPE_ENUM(uname, lname, string) MAP_MACRO_HEADER_TYPE_ENUM_(uname),

/*!
 * \brief MAP \link #MCHATV1_HEADER_TYPES_MAP \endlink to an array of parsing functions
 * \see mchatv1_parser.c
 */
#define MAP_MACRO_HEADER_TYPE_PARSING_FUNCTION(uname, lname, string) lname ## _parse,

/*!
 * \brief MAP \link #MCHATV1_HEADER_TYPES_MAP \endlink to an array of format functions
 * \see mchatv1_format.c
 */
#define MAP_MACRO_HEADER_TYPE_FORMAT_FUNCTION(uname, lname, string) lname ## _format,

/*!
 * \brief MAP \link #MCHATV1_HEADER_TYPES_MAP \endlink to an array of strings for matching
 * \see mchatv1_parser.c
 */
#define MAP_MACRO_HEADER_TYPE_STRING(uname, lname, string)  #string,


//! @}


/*!
 * \name Message Type Required Headers Map
 * \brief Map Message types to required headers
 *
 * \details
 * This uses some of the crazy things in mchatv1_macro_hell.h to generate a 2D array of integers
 * mapped to the first index being the message type enum value and the second index being the
 * header type enum value.
 *
 * \warning You are not expected to understand this
 *
 * @{
 */

/*!
 * \brief MAP \link #MCHATV1_MESSAGE_TYPES_MAP \endlink to a 2D array of
 * \link #mchatv1_header_type \endlink enum values
 */
#define MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS(name, ...) \
    IF ( IS_LIST_NOT_EMPTY( __VA_ARGS__ ) ) \
        ( MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_FOR_EACH(__VA_ARGS__), \
            (int*)(0), \
        )

/*!
 * \brief Map list to \link #MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_F \endlink
 */
#define MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_FOR_EACH(...) \
    (int[]){ FOR_EACH(MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_F, __VA_ARGS__) },

/*!
 * \brief Indirect call to \link #MAP_MACRO_HEADER_TYPE_ENUM \endlink
 */
#define MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_F(name) MAP_MACRO_HEADER_TYPE_ENUM_(name),

/*!
 * \brief MAP to calculate lengths for \link #mchatv1_message_type_required_headers \endlink
 */
#define MAP_MACRO_MESSAGE_TYPE_REQUIRED_HEADERS_LEN(name,...) \
    COUNT(__VA_ARGS__),

//! @}

/*!
 * \name Mapped Type Counts
 * @{
 */

//! The count of message types mapped
#define MCHATV1_MESSAGE_TYPES_COUNT MAPPED_COUNT(MCHATV1_MESSAGE_TYPES_MAP)

//! The count of header types mapped
#define MCHATV1_HEADER_TYPES_COUNT MAPPED_COUNT(MCHATV1_HEADER_TYPES_MAP)

//! @}

/*!
 * \brief Enum of the possible MChatv1 message types
 *
 * \details
 * This enum contains constants for the possible MChatv1 message types,
 * with MCHATV1_MESSAGE_TYPE_ prepended to the type name.  This is
 * generated by the \link #MCHATV1_MESSAGE_TYPES_MAP \endlink and
 * \link #MAP_MACRO_MESSAGE_TYPE_ENUM \endlink.
 */
enum mchatv1_type
{
    MCHATV1_MESSAGE_TYPES_MAP(MAP_MACRO_MESSAGE_TYPE_ENUM)
};

/*!
 * \brief Enum of the possible MChatv1 Header types
 *
 * \details
 * This enum contains constants for the possible MChatv1 header types,
 * with MCHATV1_HEADER_TYPE_ appended to the type name.  This is generated by
 * \link #MCHATV1_HEADER_TYPES_MAP \endlink and \link #MAP_MACRO_HEADER_TYPE_ENUM
 * \endlink.
 */
enum mchatv1_header_type
{
    MCHATV1_HEADER_TYPES_MAP(MAP_MACRO_HEADER_TYPE_ENUM)
};

/*!
 * \brief MChatv1 Message Type Strings
 */
extern const char *mchatv1_message_type_strings[];

/*!
 * \brief MChatv1 Header Type Strings
 */
extern const char *mchatv1_header_type_strings[];

/*!
 * \brief MChatv1 Message Type to Required Header Type map
 *
 * \details
 * This is a 2D array of \link #mchatv1_type \endlink to
 * \link #mchatv1_header_type \endlink values.
 *
 * \note The Initial value presented in Doxygen is wrong due to preprocessing
 * craziness in mchatv1_macro_hell.h
 *
 */
extern const int *mchatv1_message_type_required_headers[];

/*!
 * \brief MChatv1 Message Type to Required Header Type map lengths
 *
 * \details
 * This is an array of lengths of mchatv1_header_type_strings
 *
 * \note The Initial value presented in Doxygen is wrong due to preprocessing
 * craziness in mchatv1_macro_hell.h
 *
 */
extern const int mchatv1_message_type_required_headers_len[];


/*!
 * \brief MChatv1 Protocol Header (TYPE MCHAT/M.m)
 */
extern const char *mchatv1_protocol_line_string;

extern const char *mchatv1_header_line_string;

/*!
 * \brief Used by the common channel as a filler value for the
 * channel header when not connected.
 */
extern const char *mchatv1_not_connected_channel_string;

/*!
 * \brief Convert a protocol header message type to enum value
 * \param ptr Pointer to mchat protocol header
 * \param len Length of the protocol header (should be 4)
 * \return Value of message type or -1 on error
 */
int mchatv1_find_message_type(char *ptr, unsigned int len);

/*!
 * \brief Convert a header name to header type enum value
 * \param ptr Pointer to header name
 * \param len Length of the header name string
 * \return Value of header type or -1 on error
 */
int mchatv1_find_header_type(char *ptr, unsigned int len);

/*!
 * \brief Determine if packet type has a message body or not
 * \return 1 if the packet should have a body, 0 if not
 */
int mchatv1_message_type_has_body(enum mchatv1_type type);
#endif // MCHATV1_PROTO_H

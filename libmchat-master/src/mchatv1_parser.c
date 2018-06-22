/*!
 * \file mchatv1_parser.c
 * \author Sean Tracy
 * \date 18 March 2017
 * \version 0.0.1
 * \brief Received message parsing and validation functions
 *
 * \todo Document header parsers and header parser creation
 * \todo Implement File I/O related header parsers
 * \todo Document error number macros and create public api error handling functions
 * \todo Create header validation functions (or make the parser function dual-use)
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "mchatv1.h"
#include "mchatv1_proto.h"
#include "mchatv1_structs.h"
#include "mchatv1_parser.h"
#include "mchatv1_macro_hell.h"

/*!
 * \name Parser Error Numbers
 * @{
 */

#define MAP_MACRO_ERROR_STRINGS(name, string) string,

#define MCHATV1_PARSER_ERRORS(MAP_MACRO) \
    MAP_MACRO(NO_ERROR, "Success") \
    /*!< No error was encountered during parsing */ \
    MAP_MACRO(INVALID_TYPE, "The message type is invalid") \
    /*!< Invalid Message type (not in \link #MCHATV1_MESSAGE_TYPES_MAP \endlink) */ \
    MAP_MACRO(INVALID_PROTOCOL, "The protocol name was invalid") \
    /*!< The protocol name in the message is not MCHAT */ \
    MAP_MACRO(INVALID_VERSION, "The protocol version was invalid") \
    /*!< the protocol version is not one we know if */ \
    MAP_MACRO(UNKNOWN_HEADER, "A header in the message was not understood") \
    /*!< A header name we didn't kwow about was received */ \
    MAP_MACRO(INCORRECT_HEADER_VALUE, "A head had an incorrect or unexpected value") \
    /*!< A header value was not what we expected */ \
    MAP_MACRO(INVALID_BODY_SIZE, "The length of the body supplied in the message was too long") \
    /*!< The length made the body too long (someone is doing something fishy?) */ \
    /*! */\

#define MAP_MACRO_PARSER_ERROR_ENUM_NAME(name, ...) \
    MCHATV1_PARSER_ERROR_##name
#define MAP_MACRO_PARSER_ERROR_ENUM(name, index) \
    MAP_MACRO_PARSER_ERROR_ENUM_NAME(name) = (index > 0 ? 1 << (index-1) : 0),

enum mchatv1_parser_error
{
    FOR_EACH_INDEX(MAP_MACRO_PARSER_ERROR_ENUM, 0, MCHATV1_PARSER_ERRORS(MAP_MACRO_MAKE_LIST))
};

const char *mchatv1_parser_error_strings[] = {
    MCHATV1_PARSER_ERRORS(MAP_MACRO_ERROR_STRINGS)
};

//! @}

#define MCHATV1_VALIDATION_ERRORS(MAP_MACRO) \
    MAP_MACRO(NO_ERROR, "Success") \
    /*!< No error was detected during validation */ \
    MAP_MACRO(REQUIRED_HEADER_MISSING, "A required header was not present") \
    /*!< An required header for the message type was not found */ \
    MAP_MACRO(BAD_MESSAGE_TYPE, "The message type was unknown") \
    /*!< The parser could not determine the type of message; verification not preformed */ \
    /*! */ \

#define MAP_MACRO_VALIDATION_ERROR_ENUM_NAME(name, ...) MCHATV1_VALIDATION_ERROR_##name
#define MAP_MACRO_VALIDATION_ERROR_ENUM(name, index) \
    MAP_MACRO_VALIDATION_ERROR_ENUM_NAME(name) = (index > 0 ? 1 << (index) : 0),

enum mchatv1_validation_error
{
    FOR_EACH_INDEX(MAP_MACRO_VALIDATION_ERROR_ENUM, 0, MCHATV1_VALIDATION_ERRORS(MAP_MACRO_MAKE_LIST))
};

const char *mchatv1_validation_error_strings[] = {
    MCHATV1_VALIDATION_ERRORS(MAP_MACRO_ERROR_STRINGS)
};

/*****************************************************************************
 * 								Header Parsers								 *
 *****************************************************************************/

/*!
 * \name Header Parsers
 * @{
 */

/* Template for copy and paste
int _parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}
*/


int nickname_parse(struct mchat_parser *parser, char *ptr, int len)
{
    if (len > MCHAT_LIMIT_MAX_NICKNAME_SIZE)
        return -1;
    parser->header_offset[MCHATV1_HEADER_TYPE_NICKNAME] = ptr;
    parser->header_len[MCHATV1_HEADER_TYPE_NICKNAME] = len;
    return 0;
}


int length_parse(struct mchat_parser *parser, char *ptr, int len)
{
    parser->header_offset[MCHATV1_HEADER_TYPE_LENGTH] = ptr;
    parser->header_len[MCHATV1_HEADER_TYPE_LENGTH] = len;
    parser->body_size = strtol(ptr, NULL, 10);
    return 0;
}


int filename_parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}


int filesum_parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}


int chunk_parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}


int chunkcount_parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}


int chunksum_parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}


int channel_parse(struct mchat_parser *parser, char *ptr, int len)
{
    if (len > MCHAT_LIMIT_MAX_CHANNEL_NAME_SIZE)
        return -1;
    parser->header_offset[MCHATV1_HEADER_TYPE_CHANNEL] = ptr;
    parser->header_len[MCHATV1_HEADER_TYPE_CHANNEL] = len;
    return 0;
}


int presence_parse(struct mchat_parser *parser, char *ptr, int len)
{
    return 0;
}


int address_parse(struct mchat_parser *parser, char *ptr, int len)
{
    if (len > 39) /* The max length of an IPv6 address */
        return -1;
    parser->header_offset[MCHATV1_HEADER_TYPE_ADDRESS] = ptr;
    parser->header_len[MCHATV1_HEADER_TYPE_ADDRESS] = len;
    return 0;
}


int port_parse(struct mchat_parser *parser, char *ptr, int len)
{
    if (len > 5) /* The max port number is 65535, or 5 digits long */
        return -1;
    parser->header_offset[MCHATV1_HEADER_TYPE_PORT] = ptr;
    parser->header_len[MCHATV1_HEADER_TYPE_PORT] = len;
    return 0;
}

//! @}

/*****************************************************************************
 * 							Header Parsers - End							 *
 *****************************************************************************/

int (*mchatv1_header_parsers[])
    (struct mchat_parser *parser, char *ptr, int len) = {
    MCHATV1_HEADER_TYPES_MAP(MAP_MACRO_HEADER_TYPE_PARSING_FUNCTION)
};

int mchatv1_parse(struct mchat_parser *parser, char *ptr, int len)
{
    // Parser State machine
    enum
    {
        state_init,
        state_protocol,
        state_headername,
        state_headervalue,
        state_body,
        state_end,
        state_continue,
        state_error
    } state = state_init;

    memset(parser, 0, sizeof(*parser));

    parser->message_start = ptr;
    parser->total_size = len;
    char *cur, *start, *end;
    int mid = -1, hid  = -1, nlcnt = 0;

    for (cur = ptr; cur != ptr + len; cur++)
    {
        switch (state)
        {
            case state_init:
            {
                start = cur;
                end = cur;	// make sure end is a valid pointer
                state = state_protocol;
                break;
            }

            case state_continue:
            {
                if (*cur == '\n')
                    nlcnt++;
                else if (!isspace(*cur))
                {
                    nlcnt = 0;
                    state = state_headername;
                    hid = -1;
                    start = cur;
                }

                if (nlcnt >= 2)
                {
                    if (cur == ptr + len)
                        state = state_end;
                    else
                        state = state_body;
                }
                break;
            }

            case state_protocol:
            {
                if (isblank(*cur))
                {
                    mid = mchatv1_find_message_type(start, cur - start);
                    if (mid < 0)
                    {
                        parser->packet_type = MCHATV1_MESSAGE_TYPE_NONE;
                        parser->parser_error |= MCHATV1_PARSER_ERROR_INVALID_TYPE;
                    }
                    else
                        parser->packet_type = mid;

                    // Skip white space
                    while (isblank(*cur) && cur < ptr + len) cur++;
                    // This can be done in one pass as the length of the mchat protocol
                    // header is known.
                    if (g_ascii_strncasecmp(cur, "MCHAT", 4) != 0)
                    {
                        // This is not an MCHAT message
                        parser->parser_error |= MCHATV1_PARSER_ERROR_INVALID_PROTOCOL;
                        state = state_error;
                    }
                    else
                    {
                        cur += 5;
                        if (*cur == '/' && *(cur+2) == '.')
                        {
                            if (isdigit(*(cur+1)) && isdigit(*(cur+3)))
                            {
                                parser->version_major = *(cur+1) - 0x30;
                                parser->version_minor = *(cur+3) - 0x30;
                            }
                            else
                                parser->parser_error |= MCHATV1_PARSER_ERROR_INVALID_VERSION;
                        }
                        else
                        {
                            parser->parser_error |= MCHATV1_PARSER_ERROR_INVALID_PROTOCOL;
                            state = state_error;
                        }
                    }
                    while (*cur != '\n' && cur < ptr + len) cur++;
                    cur--;	// Make state_continue increment nlcnt
                    state = state_continue;
                }
                break;
            }

            case state_headername:
            {
                if (*cur == ':')
                {
                    hid = mchatv1_find_header_type(start, cur - start);

                    if (hid < 0)
                    {
                        // skip this line if we don't know the header type
                        parser->parser_error |= MCHATV1_PARSER_ERROR_UNKNOWN_HEADER;
                        while (*cur != '\n' && cur < ptr + len) cur++;
                        cur--;
                        state = state_continue;
                    }
                    else
                    {
                        cur++;
                        while(isblank(*cur) && cur < ptr + len) cur++;
                        start = cur;
                        state = state_headervalue;
                    }
                }
                break;
            }

            case state_headervalue:
            {
                if (*cur == '\r' || *cur == '\n')
                {
                    if (mchatv1_header_parsers[hid](parser, start, cur - start))
                        parser->parser_error |= MCHATV1_PARSER_ERROR_INCORRECT_HEADER_VALUE;
                    cur--; // Let state_contine see the newline
                    state = state_continue;
                }
                break;
            }

            case state_body:
            {
                while ((*cur == '\n' || *cur == '\r') && cur < ptr + len) cur++;
                if (parser->body_size == 0)
                {
                    // we didn't get a body length, so assume the rest of the
                    // packet is the body
                    parser->body_size = ptr + len - cur;
                }
                else
                {
                    parser->body_size = strtol(parser->header_offset[MCHATV1_HEADER_TYPE_LENGTH], NULL, 10);
                    if (cur + parser->body_size > ptr + len)
                    {
                        parser->parser_error |= MCHATV1_PARSER_ERROR_INVALID_BODY_SIZE;
                        parser->body_size = ptr + len - cur;
                    }
                }
                parser->body = cur;
                state = state_end;
            }

            case state_end:
            {
                // break out of the loop, we are done
                return 0;
                break;
            }

            case state_error:
            {
                // We got an invalid protocol type
                return -1;
                break;
            }

            default:
            {
                // This shouldn't happen, but consider making it
                // here an error
                return -1;
                break;
            }
        }
    }
    return 0;
}


int mchatv1_validate(struct mchat_parser *parser)
{
    //! \todo Provide more validation and more descriptive returns
    // A none-type message is an error
    if (parser->packet_type == MCHATV1_MESSAGE_TYPE_NONE)
        return -1;

    // Check for required headers
    if (mchatv1_message_type_required_headers_len[parser->packet_type] != 0)
    {
        for (int i = 0; i < mchatv1_message_type_required_headers_len[parser->packet_type]; i++)
        {
            int hid = mchatv1_message_type_required_headers[parser->packet_type][i];
            if (parser->header_offset[hid] == NULL || parser->header_len[hid] == 0)
                return -1;
        }
    }

    return 0;
}

int mchatv1_parse_and_validate(struct mchat_parser *parser, char *ptr, int len)
{
    if (mchatv1_parse(parser, ptr, len))
    {
        return 1;
    }
    if (mchatv1_validate(parser))
    {
        return 2;
    }

    return 0;
}

int mchatv1_parser_to_message(struct mchat_parser *parser, mchat_message_t *message)
{
    memset(message->body, 0, MCHAT_LIMIT_MAX_MESSAGE_SIZE);
    memset(message->nickname, 0, MCHAT_LIMIT_MAX_NICKNAME_SIZE);
    memcpy(message->body, parser->body, parser->body_size);
    message->body_len = parser->body_size;

    memcpy(message->nickname,
           parser->header_offset[MCHATV1_HEADER_TYPE_NICKNAME],
           parser->header_len[MCHATV1_HEADER_TYPE_NICKNAME]);
    message->nickname_len = parser->header_len[MCHATV1_HEADER_TYPE_NICKNAME];
    message->validation_error = parser->validation_error;
    message->parser_error = parser->parser_error;
    message->packet_type = parser->packet_type;

    return 0;
}


const char *mchatv1_parser_strerror(mchat_parser *parser)
{
    int err = parser->parser_error;
    int cnt = 0;
    if (err == 0)
        return mchatv1_parser_error_strings[0];
    while ((err & 1) == 0)
    {
        cnt++;
        err >>= 1;
    }
    parser->parser_error &= ~(1 << cnt);
    return mchatv1_parser_error_strings[cnt+1];
}

/*!
 * \file mchatv1_parser.h
 * \author Sean Tracy
 * \date 8 March 2017
 * \version 0.0.1
 * \brief MChatv1 Received message parsing functions
 *
 * \details
 * This file defines the internal API to parse MChatv1 messages.
 * The parsing functions convert a raw message in a character buffer to
 * a usable format for the rest of libmchat.
 */
#ifndef MCHATV1_PARSER_H
#define MCHATV1_PARSER_H

#include "mchatv1_structs.h"


/*!
 * \brief Parse a raw MChatv1 message
 * \param parser Pointer to an allocated mchatv1_parser struct
 * \param ptr Pointer to a character buffer which contains the raw
 * 				MChatv1 message
 * \param len Length of the message in \p ptr
 * \return 0 on success or -1 on error
 */
int mchatv1_parse(struct mchat_parser *parser, char *ptr, int len);

/*!
 * \brief Validate a parsed MChatv1 message
 * \param parser Pointer to an allocated mchatv1_parser struct
 * 					that has been through mchatv1_parse
 * \return 0 on success or -1 on error
 */
int mchatv1_validate(struct mchat_parser *parser);

/*!
 * \brief Parse and Validate a raw MChatv1 packet
 * \param parser Pointer to an allocated mchat_parser struct
 * \param ptr Pointer to a character buffer to get raw data from
 * \param len Length of the packet in \p ptr
 * \return 0 on succes or -1 on error
 *
 * \details
 * This function combines the results of mchatv1_parse and mchatv1_validate.
 *
 */
int mchatv1_parse_and_validate(struct mchat_parser *parser, char *ptr, int len);

/*!
 * \brief Convert a parsed MChatv1 message to an mchatv1_message struct
 * \param parser Pointer to an allocated mchatv1_parser struct that
 * 					has been through mchatv1_parse
 * \param message Pointer to an allocated mchatv1_message struct
 * \return 0 on success or an error number on failure
 */
int mchatv1_parser_to_message(struct mchat_parser *parser, mchat_message_t *message);

/*!
 * \brief Return a string explaining a parser error
 * \param parser Pointer to an mchat_parser struct with an error
 * \return Pointer to an error string
 *
 * \details
 * This function returns the first encountered error from message parsing.  Subsequent
 * calls return other parser errors until none are left.
 *
 * \warning
 * This function does modify the parser error number, so each call makes the error codes
 * disappear.
 */
const char *mchatv1_parser_strerror(mchat_parser *parser);
#endif // MCHATV1_PARSER_H

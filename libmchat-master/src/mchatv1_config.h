/*!
 * \file mchatv1_config.h
 * \author Sean Tracy
 * \date 3 March 2017
 * \version 0.0.1
 * \brief MChatv1 configuration file parser
 *
 * \details
 * This file declare the configuration parsing functions for libmchat.
 */
#ifndef MCHATV1_CONFIG_H
#define MCHATV1_CONFIG_H

#ifndef _MCHAT_T_TYPEDEF_DEFINED
//! \brief Set anywhere mchat_t typedef is specified
#define _MCHAT_T_TYPEDEF_DEFINED
typedef struct mchat_t mchat_t;
#endif // _MCHAT_T_TYPEDEF_DEFINED

/*!
 * \brief Parse and add configuration options to mchat object
 * \param mchat Pointer to an mchat object
 * \param filename Pointer to file name character string
 * \return 0 on success or -1 on error
 *
 * \details
 * Libmchat can be configured through an rc file (the default should be ~/.mchatrc).
 * This file should be an INI style argument = value file.
 */
int mchat_config_parse(mchat_t *mchat, char *filename);

#endif // MCHATV1_CONFIG_H

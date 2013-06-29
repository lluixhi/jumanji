/* See LICENSE file for license and copyright information */

#ifndef SESSION_H
#define SESSION_H

#include <girara/types.h>

#include "jumanji.h"

/**
 * Load a session from a file
 *
 * @param session The used girara session
 * @param argument_list The used argument
 * @return true if no error ocurred otherwise false 
 */
bool
cmd_sessionload(girara_session_t* session, girara_list_t* argument_list);

/**
 * Saves the current session to a file
 *
 * @param session The used girara session
 * @param argument_list The used argument
 * @return true if no error ocurred otherwise false 
 */
bool
cmd_sessionsave(girara_session_t* session, girara_list_t* argument_list);

#endif // SESSION_H

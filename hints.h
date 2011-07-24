/* See LICENSE file for license and copyright information */

#ifndef HINTS_H
#define HINTS_H

#include <girara.h>

#include "jumanji.h"

/**
 * Shortcut to enable hint mode
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_hints(girara_session_t* session, girara_argument_t* argument, unsigned int t);

#endif // HINTS_H

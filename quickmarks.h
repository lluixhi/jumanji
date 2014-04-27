/* See LICENSE file for license and copyright information */

#ifndef QUICKMARKS_H
#define QUICKMARKS_H

#include "jumanji.h"

/**
 * Add a new quickmark for the current URL
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quickmark_add(girara_session_t* session, girara_argument_t* argument,
    girara_event_t* event, unsigned int t);

/**
 * Jump to a quickmark (in a new tab)
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quickmark_evaluate(girara_session_t* session, girara_argument_t*
    argument, girara_event_t* event, unsigned int t);

/**
 * Add a quickmark
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_quickmarks_add(girara_session_t* session, girara_list_t* argument_list);

/**
 * Delete quickmark(s)
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_quickmarks_delete(girara_session_t* session, girara_list_t* argument_list);

#endif // QUICKMARKS_H

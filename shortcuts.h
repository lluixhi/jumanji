/* See LICENSE file for license and copyright information */

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <girara.h>

/**
 * Focus the inputbar
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int t);

/**
 * Quit jumanji
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quit(girara_session_t* session, girara_argument_t* argument, unsigned int t);

#endif // SHORTCUTS_H

/* See LICENSE file for license and copyright information */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <girara.h>

/**
 * Closes the current buffer
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_buffer_delete(girara_session_t* session, girara_list_t* argument_list);

/**
 * Opens URL in the current tab
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_open(girara_session_t* session, girara_list_t* argument_list);

/**
 * Open URL in a new tab
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_tabopen(girara_session_t* session, girara_list_t* argument_list);

#endif // COMMANDS_H

/* See LICENSE file for license and copyright information */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <girara.h>

/**
 * Close jumanji
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_close(girara_session_t* session, girara_list_t* argument_list);

#endif // COMMANDS_H

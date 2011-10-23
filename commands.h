/* See LICENSE file for license and copyright information */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <girara/girara.h>

/**
 * Add a bookmark
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_bookmark_add(girara_session_t* session, girara_list_t* argument_list);

/**
 * Delete a bookmark
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_bookmark_delete(girara_session_t* session, girara_list_t* argument_list);

/**
 * Closes the current buffer
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_buffer_delete(girara_session_t* session, girara_list_t* argument_list);

/**
 * Show the download widget
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_downloads(girara_session_t* session, girara_list_t* argument_list);

/**
 * Opens URL in the current tab
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_open(girara_session_t* session, girara_list_t* argument_list);

/**
 * Prints the main frame of the website
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_print(girara_session_t* session, girara_list_t* argument_list);

/**
 * Adds a proxy to the proxy list
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_proxy(girara_session_t* session, girara_list_t* argument_list);

/**
 * Search the website for specific content
 *
 * @param session The used girara session
 * @param input Input text
 * @param argument User data
 * @return true if no error occured
 */
bool cmd_search(girara_session_t* session, char* input, girara_argument_t* argument);

/**
 * Adds a search engine
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_search_engine(girara_session_t* session, girara_list_t* argument_list);

/**
 * Stops loading the current web page
 *
 * @param session The used girara session
 * @param input Input text
 * @param argument User data
 * @return true if no error occured
 */
bool cmd_stop(girara_session_t* session, girara_list_t* argument_list);

/**
 * Open URL in a new tab
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_tabopen(girara_session_t* session, girara_list_t* argument_list);

/**
 * Open URL in a new window
 *
 * @param session The used girara session
 * @param argument_list List of passed arguments
 * @return true if no error occured
 */
bool cmd_winopen(girara_session_t* session, girara_list_t* argument_list);

#endif // COMMANDS_H

/* See LICENSE file for license and copyright information */

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <girara/types.h>

/**
 * Open homepage (in a new tab)
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_goto_homepage(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Go to (nth) parent (directory)
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_goto_parent_directory(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Focus the inputbar
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Navigates through the tabs
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_tab_navigate(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Navigates through the history of the current tab
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_navigate_history(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Open (put) a URL based on the current clipboard content
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_put(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Quit jumanji
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quit(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Reloads the current page
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_reload(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Restore a closed tab
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_restore(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Scroll through the page
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_scroll(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Search for the last given search item on the current tab
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_search(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Saves (or removes) the current website as a bookmark
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_toggle_bookmark(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Toggle through the proxy list
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_toggle_proxy(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Toggle "enable-plugins" setting
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_toggle_plugins(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Toggle between the rendered and source code view
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_toggle_source_mode(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Yank current url
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_yank(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Zoom in/out
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_zoom(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Toggle user stylesheet
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_toggle_stylesheet(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

#endif // SHORTCUTS_H

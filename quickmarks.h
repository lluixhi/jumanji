/* See LICENSE file for license and copyright information */

#ifndef QUICKMARKS_H
#define QUICKMARKS_H

#include "jumanji.h"

/**
 * Add a new quickmark for the current URL
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quickmark_add(girara_session_t* session, girara_argument_t* argument,
    unsigned int t);

/**
 * Callback for key-press-event when adding a quickmark
 *
 * @param widget View
 * @param event Event
 * @param session Girara session
 * @return true if no error occured
 */
bool cb_quickmarks_view_key_press_event_add(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session);

/**
 * Jump to a quickmark (in a new tab)
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_quickmark_evaluate(girara_session_t* session, girara_argument_t*
    argument, unsigned int t);

/**
 * Callback for key-press-event when evaluating a quickmark
 *
 * @param widget View
 * @param event Event
 * @param session Girara session
 * @return true if no error occured
 */
bool cb_quickmarks_view_key_press_event_evaluate(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session);

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

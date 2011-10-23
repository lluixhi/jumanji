/* See LICENSE file for license and copyright information */

#ifndef MARKS_H
#define MARKS_H

#include <girara/girara.h>

#include "jumanji.h"

typedef struct jumanji_marker_s
{
  int key; /**> Marks key */
  char* uri; /**> The marks uri */
  double horizontal_adjustment; /**> Horizontal adjustment */
  double vertical_adjustment; /**> Vertical adjustment */
  float zoom_level; /**> Zoom level */
} jumanji_mark_t;

/**
 * Saves a mark
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_mark_add(girara_session_t* session, girara_argument_t* argument,
    unsigned int t);

/**
 * Evaluates a mark
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_mark_evaluate(girara_session_t* session, girara_argument_t* argument,
    unsigned int t);

/**
 * Callback for key-press-event when adding a mark
 *
 * @param widget View
 * @param event Event
 * @param session Girara session
 * @return true if no error occured
 */
bool cb_marks_view_key_press_event_add(GtkWidget* widget, GdkEventKey* event,
    girara_session_t* session);

/**
 * Callback for key-press-event when evaluating a mark
 *
 * @param widget View
 * @param event Event
 * @param session Girara session
 * @return true if no error occured
 */
bool cb_marks_view_key_press_event_evaluate(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session);

/**
 * Mark current location within the web page
 *
 * @param session The girara session
 * @param argument_list Argument list
 * @return true if no error occured otherwise false
 */
bool cmd_marks_add(girara_session_t* session, girara_list_t* argument_list);

/**
 * Delete the specified marks
 *
 * @param session The girara session
 * @param argument_list Argument list
 * @return true if no error occured otherwise false
 */
bool cmd_marks_delete(girara_session_t* session, girara_list_t* argument_list);

/**
 * Adds a mark
 *
 * @param jumanji Jumanji session
 * @param tab Current tab
 * @param key Key value
 */
void mark_add(jumanji_t* jumanji, jumanji_tab_t* tab, int key);

/**
 * Evaluates a mark
 *
 * @param jumanji Jumanji session
 * @param tab Current tab
 * @param key Key value
 */
void mark_evaluate(jumanji_t* jumanji, jumanji_tab_t* tab, int key);

/**
 * Free function for a mark
 *
 * @param data Mark
 */
void mark_free(void* data);

#endif // MARKS_H

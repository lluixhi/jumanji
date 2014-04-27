/* See LICENSE file for license and copyright information */

#ifndef HINTS_H
#define HINTS_H

#include <girara/types.h>

#include "jumanji.h"

/**
 * Shortcut to enable hint mode
 *
 * @param session The used girara session
 * @param argument The used argument
 * @param event Girara event
 * @param t Number of executions
 * @return true if no error occured otherwise false
 */
bool sc_hints(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t);

/**
 * Displays all hints
 *
 * @param jumanji
 * @param tab
 */
void hints_show(jumanji_t* jumanji, jumanji_tab_t* tab);

/**
 * Clears the hints from the web site
 *
 * @param jumanji Jumanji session
 */
void hints_clear(jumanji_t* jumanji);

/**
 * Simulates a click on the hint
 *
 * @param jumanji Jumanji session
 * @param n Index of the hint
 * @return true if the hint has been processed
 */
bool hints_process(jumanji_t* jumanji, guint n);

/**
 * Updates the current hints based on the input
 *
 * @param jumanji Jumanji session
 * @param input Input
 * @return true if a hint has been processed through the update
 */
bool hints_update(jumanji_t* jumanji, char* input);

/**
 * Resets all hint settings
 *
 * @param jumanji The jumanji session
 */
void hints_reset(jumanji_t* jumanji);

#endif // HINTS_H

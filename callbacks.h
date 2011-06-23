/* See LICENSE file for license and copyright information */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include <girara.h>

#include "jumanji.h"

/**
 * Quits the current jumanji session
 *
 * @param widget The gtk window of jumanji
 * @param data NULL
 * @return TRUE
 */
gboolean cb_destroy(GtkWidget* widget, gpointer data);

/**
 * This function gets called when the buffer of girara changes
 *
 * @param session The girara session
 */
void cb_girara_buffer_changed(girara_session_t* session);

/**
 * Called when a jumanji tab is destroyed
 *
 * @param object The tab scrolld window widget
 * @param tab The jumanji tab
 */
void cb_jumanji_tab_destroy(GObject* object, jumanji_tab_t* tab);

void cb_jumanji_tab_load_status(WebKitWebView* web_view, GParamSpec* pspec, gpointer data);

#endif // CALLBACKS_H

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

/**
 * Update the title of the tab if status changes
 *
 * @param web_view Webkit web view
 * @param pspec -
 * @param data Custom data
 */
void cb_jumanji_tab_load_status(WebKitWebView* web_view, GParamSpec* pspec, gpointer data);

/**
 * Updates the statusbar entry
 *
 * @param tabs Tab container
 * @param page Current tab
 * @param page_num Current tab id
 * @param jumanji The jumanji session
 */
void cb_jumanji_tab_changed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji);

/**
 * Tab has been removed
 *
 * @param tabs Tab container
 * @param page Current tab
 * @param page_num Current tab id
 * @param jumanji The jumanji session
 */
void cb_jumanji_tab_removed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji);

/**
 * Called when a webkit settings has been changed
 *
 * @param session The girara session
 * @param setting The girara setting
 */
void cb_settings_webkit(girara_session_t* session, girara_setting_t* setting);

#endif // CALLBACKS_H

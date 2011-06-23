/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <gtk/gtk.h>
#include <stdlib.h>

#include "callbacks.h"
#include "jumanji.h"

gboolean
cb_destroy(GtkWidget* widget, gpointer data)
{
  return TRUE;
}

void
cb_girara_buffer_changed(girara_session_t* session)
{
  g_return_if_fail(session != NULL);
  g_return_if_fail(session->global.data != NULL);

  jumanji_t* jumanji = session->global.data;

  char* buffer = girara_buffer_get(session);

  if (buffer) {
    girara_statusbar_item_set_text(session, jumanji->ui.statusbar.buffer, buffer);
    free(buffer);
  } else {
    girara_statusbar_item_set_text(session, jumanji->ui.statusbar.buffer, "");
  }
}

void
cb_jumanji_tab_destroy(GObject* object, jumanji_tab_t* tab)
{
	if (object == NULL || tab == NULL) {
		return;
	}

	jumanji_tab_free(tab);
}

void
cb_jumanji_tab_load_status(WebKitWebView* web_view, GParamSpec* pspec, gpointer data)
{
	if (web_view == NULL || data == NULL) {
		return;
	}

	jumanji_tab_t* tab = (jumanji_tab_t*) data;
  const gchar* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));

	if (title) {
		girara_tab_title_set(tab->girara_tab, title);
	} else {
		girara_tab_title_set(tab->girara_tab, "Connecting...");
	}
}

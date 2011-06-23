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
buffer_changed(girara_session_t* session)
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

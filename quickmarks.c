/* See LICENSE file for license and copyright information */

#include "quickmarks.h"

bool
sc_quickmark_add(girara_session_t* session, girara_argument_t* argument,
    unsigned int t)
{
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);
  return false;
}

bool
cb_quickmarks_view_key_press_event_add(GtkWidget* widget, GdkEventKey* event,
    girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);
  return true;
}

bool
sc_quickmark_evaluate(girara_session_t* session, girara_argument_t* argument,
    unsigned int t)
{
  return false;
}

bool cb_quickmarks_view_key_press_event_evaluate(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL, FALSE);
  return true;
}

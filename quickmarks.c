/* See LICENSE file for license and copyright information */

#include <string.h>

#include "database.h"
#include "quickmarks.h"
#include <girara/session.h>
#include <girara/datastructures.h>
#include <girara/callbacks.h>
#include <girara/settings.h>

bool
sc_quickmark_add(girara_session_t* session, girara_argument_t* argument,
    girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL,           false);
  g_return_val_if_fail(session->gtk.view != NULL, false);

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(cb_quickmarks_view_key_press_event_add), session);

  return false;
}

bool
cb_quickmarks_view_key_press_event_add(GtkWidget* widget, GdkEventKey* event,
    girara_session_t* session)
{
  g_return_val_if_fail(session != NULL,              false);
  g_return_val_if_fail(session->gtk.view != NULL,    false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* evaluate key */
  if (((event->keyval >= 0x30 && event->keyval <= 0x39) || (event->keyval >= 0x41 && event->keyval <= 0x5A) ||
      (event->keyval >= 0x61 && event->keyval <= 0x7A)) == false) {
    return false;
  }

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
  if (tab == NULL || tab->web_view == NULL || jumanji->database == NULL) {
    return false;
  }

  const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
  jumanji_db_quickmark_add(jumanji->database, event->keyval, uri);

  return true;
}

bool
sc_quickmark_evaluate(girara_session_t* session, girara_argument_t* argument,
    girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL,              false);
  g_return_val_if_fail(session->gtk.view != NULL,    false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(cb_quickmarks_view_key_press_event_evaluate), session);

  if (argument->n == NEW_TAB) {
    jumanji->global.quickmark_open_mode = NEW_TAB;
  } else {
    jumanji->global.quickmark_open_mode = DEFAULT;
  }

  return false;
}

bool
cb_quickmarks_view_key_press_event_evaluate(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL,              false);
  g_return_val_if_fail(session->gtk.view != NULL,    false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* evaluate key */
  if (((event->keyval >= 0x30 && event->keyval <= 0x39) || (event->keyval >= 0x41 && event->keyval <= 0x5A) ||
      (event->keyval >= 0x61 && event->keyval <= 0x7A)) == false) {
    return false;
  }

  if (jumanji->database == NULL) {
    return false;
  }

  char* uri = jumanji_db_quickmark_find(jumanji->database, event->keyval);

  if (uri == NULL) {
    return false;
  }

  if (jumanji->global.quickmark_open_mode == NEW_TAB) {
    bool focus_new_tabs;
    girara_setting_get(jumanji->ui.session, "focus-new-tabs", &focus_new_tabs);
    jumanji_tab_new(jumanji, uri, focus_new_tabs);
  } else {
    jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
    jumanji_tab_load_url(tab, uri);
  }

  return true;
}

bool
cmd_quickmarks_add(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  if (jumanji->database == NULL) {
    return false;
  }

  if (girara_list_size(argument_list) < 2) {
    return false;
  }

  char* identifier_string = girara_list_nth(argument_list, 0);
  char* url               = girara_list_nth(argument_list, 1);

  if (identifier_string == NULL || url == NULL) {
    return false;
  }

  if (strlen(identifier_string) < 1 || strlen(identifier_string) > 1) {
    return false;
  }

  char identifier = identifier_string[0];

  if (((identifier >= 0x30 && identifier <= 0x39) || (identifier >= 0x41 && identifier <= 0x5A) ||
      (identifier >= 0x61 && identifier <= 0x7A)) == false) {
    return false;
  }

  jumanji_db_quickmark_add(jumanji->database, identifier, url);

  return false;
}

bool
cmd_quickmarks_delete(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  if (jumanji->database == NULL) {
    return false;
  }

  if (girara_list_size(argument_list) < 1) {
    return false;
  }

  girara_list_iterator_t* iter = girara_list_iterator(argument_list);
  do {
    char* identifier_string = girara_list_iterator_data(iter);
    if (identifier_string == NULL) {
      continue;
    }

    for (unsigned int i = 0; i < strlen(identifier_string); i++) {
      char identifier = identifier_string[i];
      if (((identifier >= 0x30 && identifier <= 0x39) || (identifier >= 0x41 && identifier <= 0x5A) ||
          (identifier >= 0x61 && identifier <= 0x7A)) == false) {
        return true;
      }

      jumanji_db_quickmark_remove(jumanji->database, identifier);
    }
  } while (girara_list_iterator_next(iter) != NULL);
  girara_list_iterator_free(iter);

  return false;
}


/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>

#include "marks.h"

bool
sc_mark_add(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL,           FALSE);
  g_return_val_if_fail(session->gtk.view != NULL, FALSE);

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(cb_marks_view_key_press_event_add), session);

  return false;
}

bool
sc_mark_evaluate(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL,           FALSE);
  g_return_val_if_fail(session->gtk.view != NULL, FALSE);

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(cb_marks_view_key_press_event_evaluate), session);

  return false;
}

bool
cb_marks_view_key_press_event_add(GtkWidget* widget, GdkEventKey* event,
    girara_session_t* session)
{
  g_return_val_if_fail(session != NULL,              FALSE);
  g_return_val_if_fail(session->gtk.view != NULL,    FALSE);
  g_return_val_if_fail(session->global.data != NULL, FALSE);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* evaluate key */
  if (((event->keyval >= 0x41 && event->keyval <= 0x5A) || (event->keyval >=
          0x61 && event->keyval <= 0x7A)) == false) {
    return false;
  }

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
  if (tab == NULL) {
    return false;
  }

  mark_add(jumanji, tab, event->keyval);

  return true;
}

bool cb_marks_view_key_press_event_evaluate(GtkWidget* widget, GdkEventKey*
    event, girara_session_t* session)
{
  g_return_val_if_fail(session != NULL,              FALSE);
  g_return_val_if_fail(session->gtk.view != NULL,    FALSE);
  g_return_val_if_fail(session->global.data != NULL, FALSE);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view), session->signals.view_key_pressed);
  session->signals.view_key_pressed = g_signal_connect(G_OBJECT(session->gtk.view), "key-press-event",
      G_CALLBACK(girara_callback_view_key_press_event), session);

  /* evaluate key */
  if (((event->keyval >= 0x41 && event->keyval <= 0x5A) || (event->keyval >=
          0x61 && event->keyval <= 0x7A)) == false) {
    return false;
  }

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
  if (tab == NULL) {
    return false;
  }

  mark_evaluate(jumanji, tab, event->keyval);

  return true;
}

void
mark_add(jumanji_t* jumanji, jumanji_tab_t* tab, int key)
{
  if (jumanji == NULL || jumanji->global.marks == NULL || tab == NULL ||
      tab->web_view == NULL) {
    return;
  }

  const char* uri             = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
  GtkAdjustment* v_adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));
  GtkAdjustment* h_adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));

  if (uri == NULL || v_adjustment == NULL || h_adjustment == NULL) {
    return;
  }

  double horizontal_adjustment = gtk_adjustment_get_value(h_adjustment);
  double vertical_adjustment   = gtk_adjustment_get_value(v_adjustment);
  float zoom_level             = webkit_web_view_get_zoom_level(WEBKIT_WEB_VIEW(tab->web_view));

  /* search for existing mark */
  if (girara_list_size(jumanji->global.marks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.marks);
    do {
      jumanji_mark_t* mark = (jumanji_mark_t*) girara_list_iterator_data(iter);
      if (mark == NULL) {
        continue;
      }

      if (mark->key == key && strcmp(mark->uri, uri) == 0) {
        g_free(mark->uri);
        mark->uri                   = g_strdup(uri);
        mark->horizontal_adjustment = horizontal_adjustment;
        mark->vertical_adjustment   = vertical_adjustment;
        mark->zoom_level            = zoom_level;
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  /* add new mark */
  jumanji_mark_t* mark = malloc(sizeof(jumanji_mark_t));

  if (mark == NULL) {
    return;
  }

  mark->key                   = key;
  mark->uri                   = g_strdup(uri);
  mark->horizontal_adjustment = horizontal_adjustment;
  mark->vertical_adjustment   = vertical_adjustment;
  mark->zoom_level            = zoom_level;

  girara_list_append(jumanji->global.marks, mark);
}

void
mark_evaluate(jumanji_t* jumanji, jumanji_tab_t* tab, int key)
{
  if (jumanji == NULL || jumanji->global.marks == NULL || tab == NULL ||
      tab->web_view == NULL) {
    return;
  }

  const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));

  if (uri == NULL) {
    return;
  }

  /* search for existing mark */
  if (girara_list_size(jumanji->global.marks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.marks);
    do {
      jumanji_mark_t* mark = (jumanji_mark_t*) girara_list_iterator_data(iter);
      if (mark == NULL) {
        continue;
      }

      if (mark->key == key && strcmp(mark->uri, uri) == 0) {
        GtkAdjustment* v_adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));
        GtkAdjustment* h_adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));

        if (v_adjustment == NULL || h_adjustment == NULL) {
          return;
        }

        gtk_adjustment_set_value(h_adjustment, mark->horizontal_adjustment);
        gtk_adjustment_set_value(v_adjustment, mark->vertical_adjustment);
        webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), mark->zoom_level);
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }
}

void
mark_free(void* data)
{
  if (data == NULL) {
    return;
  }

  jumanji_mark_t* mark = (jumanji_mark_t*) data;

  g_free(mark->uri);
  free(mark);
}

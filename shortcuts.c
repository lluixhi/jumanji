/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "jumanji.h"
#include "shortcuts.h"

bool
sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  if (!(gtk_widget_get_visible(GTK_WIDGET(session->gtk.inputbar)))) {
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));
  }

  if (argument->data) {
    gtk_entry_set_text(session->gtk.inputbar, (char*) argument->data);
    gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar));
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar), -1);
  }

  return false;
}

bool
sc_follow_link(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  return false;
}

bool
sc_navigate_history(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab->web_view) {
    if (argument->n == NEXT) {
      webkit_web_view_go_forward(WEBKIT_WEB_VIEW(tab->web_view));
    } else {
      webkit_web_view_go_back(WEBKIT_WEB_VIEW(tab->web_view));
    }
  }

  return false;
}

bool
sc_quit(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, 0);

  cb_destroy(NULL, NULL);

  gtk_main_quit();

  return false;
}

bool
sc_scroll(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL) {
    return false;
  }

  GtkAdjustment* adjustment = NULL;
  if ( (argument->n == LEFT) || (argument->n == RIGHT) || (argument->n == BEGIN)
      || (argument->n == END) )
    adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));
  else
    adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));

  gdouble view_size   = gtk_adjustment_get_page_size(adjustment);
  gdouble value       = gtk_adjustment_get_value(adjustment);
  gdouble max         = gtk_adjustment_get_upper(adjustment) - view_size;

  int* tmp = (int*) girara_setting_get(jumanji->ui.session, "scroll-step");
  gdouble scroll_step = tmp ? *tmp : 40;

  gdouble new_value;
  switch(argument->n)
  {
    case FULL_UP:
      new_value = (value - view_size) < 0 ? 0 : (value - view_size);
      break;
    case FULL_DOWN:
      new_value = (value + view_size) > max ? max : (value + view_size);
      break;
    case HALF_UP:
      new_value = (value - (view_size / 2)) < 0 ? 0 : (value - (view_size / 2));
      break;
    case HALF_DOWN:
      new_value = (value + (view_size / 2)) > max ? max : (value + (view_size / 2));
      break;
    case LEFT:
    case UP:
      new_value = (value - scroll_step) < 0 ? 0 : (value - scroll_step);
      break;
    case RIGHT:
    case DOWN:
      new_value = (value + scroll_step) > max ? max : (value + scroll_step);
      break;
    case BEGIN:
    case TOP:
      new_value = 0;
      break;
    case END:
    case BOTTOM:
      new_value = max;
      break;
    default:
      new_value = 0;
  }

  gtk_adjustment_set_value(adjustment, new_value);

  if (new_value == max || new_value == 0) {
    return false;
  } else {
    return true;
  }
}

bool
sc_reload(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab->web_view) {
    if (argument->n == BYPASS_CACHE) {
      webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(tab->web_view));
    } else {
      webkit_web_view_reload(WEBKIT_WEB_VIEW(tab->web_view));
    }
  }

  return false;
}

bool
sc_toggle_source_mode(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL || tab->web_view == NULL) {
    return false;
  }

  char* url = (char*) webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));

  if(webkit_web_view_get_view_source_mode(WEBKIT_WEB_VIEW(tab->web_view)))
    webkit_web_view_set_view_source_mode(WEBKIT_WEB_VIEW(tab->web_view), FALSE);
  else
    webkit_web_view_set_view_source_mode(WEBKIT_WEB_VIEW(tab->web_view), TRUE);

  jumanji_tab_load_url(tab, url);

  return false;
}

bool
sc_zoom(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL) {
    return false;
  }

  int* tmp = (int*) girara_setting_get(jumanji->ui.session, "zoom-step");
  float zoom_step = tmp ? *tmp : 10;

  float zoom_level = webkit_web_view_get_zoom_level(WEBKIT_WEB_VIEW(tab->web_view));

  if(argument->n == ZOOM_IN) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), zoom_level + (float) (zoom_step / 100));
  } else if(argument->n == ZOOM_OUT) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), zoom_level - (float) (zoom_step / 100));
  } else if(argument->n == DEFAULT) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), 1.0f);
  } else if(argument->n == ZOOM_SPECIFIC) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), (float) (t / 100));
  }

  return false;
}

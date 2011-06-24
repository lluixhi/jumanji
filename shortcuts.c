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
  if ( (argument->n == LEFT) || (argument->n == RIGHT) )
    adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));
  else
    adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(tab->scrolled_window));

  gdouble view_size   = gtk_adjustment_get_page_size(adjustment);
  gdouble value       = gtk_adjustment_get_value(adjustment);
  gdouble max         = gtk_adjustment_get_upper(adjustment) - view_size;
  gdouble scroll_step = 40;

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
    case TOP:
      new_value = 0;
      break;
    case BOTTOM:
      new_value = max;
      break;
    default:
      new_value = 0;
  }

  gtk_adjustment_set_value(adjustment, new_value);

  return false;
}

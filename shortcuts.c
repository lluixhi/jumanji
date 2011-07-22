/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "database.h"
#include "jumanji.h"
#include "shortcuts.h"

bool
sc_goto_homepage(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  char* homepage = girara_setting_get(jumanji->ui.session, "homepage");
  char* url      = NULL;
  if (homepage != NULL) {
    url = jumanji_build_url_from_string(jumanji, homepage);
  }

  if (argument->n == NEW_TAB) {
    jumanji_tab_new(jumanji, url, false);
  } else {
    jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
    if (tab != NULL) {
      jumanji_tab_load_url(tab, url);
    }
  }

  free(url);

  return false;
}

bool
sc_goto_parent_directory(girara_session_t* session, girara_argument_t* argument, unsigned int t)
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

  if (url == NULL) {
    return false;
  }

  /* calculate root of the website */
  unsigned int offset      = strstr(url, "://") - url + 3;
  unsigned int root_length = offset + 1;
  char* root_string        = url + offset;

  while(root_string && *root_string != '/') {
    root_length++;
    root_string++;
  }

  char* root = g_strndup(url, root_length);

  if (argument->n == DEFAULT) {
    jumanji_tab_load_url(tab, root);
  } else {
    unsigned int count = (t == 0) ? 1 : t;
    char* directories = g_strndup(url + strlen(root), strlen(url) - strlen(root));

    if (directories == NULL || strlen(directories) == 0) {
      return false;
    }

    gchar **tokens = g_strsplit(directories, "/", -1);
    int     length = g_strv_length(tokens);
    GString* tmp   = g_string_new("");

    int limit = length - count;
    for(int i = 0; i < limit; i++) {
      if (i == 0) {
        g_string_append(tmp, tokens[i]);
      } else {
        g_string_append(tmp, "/");
        g_string_append(tmp, tokens[i]);
      }
    }

    char* new_url = g_strconcat(root, tmp->str, NULL);
    jumanji_tab_load_url(tab, new_url);

    g_free(new_url);
    g_string_free(tmp, TRUE);
    g_strfreev(tokens);
    g_free(directories);
  }

  g_free(root);

  return false;
}

bool
sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  if (!(gtk_widget_get_visible(GTK_WIDGET(session->gtk.inputbar)))) {
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));
  }

  if (argument->data) {
    if (argument->n == APPEND_URL) {
      jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
      if (tab != NULL && tab->web_view != NULL) {
        const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
        char* data      = g_strdup_printf("%s%s", (char*) argument->data, uri);
        gtk_entry_set_text(session->gtk.inputbar, data);
        g_free(data);
      } else {
        gtk_entry_set_text(session->gtk.inputbar, (char*) argument->data);
      }
    } else {
      gtk_entry_set_text(session->gtk.inputbar, (char*) argument->data);
    }

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

  if (tab && tab->web_view) {
    if (argument->n == NEXT) {
      webkit_web_view_go_forward(WEBKIT_WEB_VIEW(tab->web_view));
    } else {
      webkit_web_view_go_back(WEBKIT_WEB_VIEW(tab->web_view));
    }
  }

  return false;
}

bool
sc_put(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  if (clipboard == NULL) {
    return false;
  }

  char* text = gtk_clipboard_wait_for_text(clipboard);
  char* url  = jumanji_build_url_from_string(jumanji, text);

  if (url == NULL) {
    return false;
  }

  if (argument->n == NEW_TAB) {
    jumanji_tab_new(jumanji, url, false);
  } else {
    jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
    if (tab != NULL) {
      jumanji_tab_load_url(tab, url);
    }
  }

  g_free(url);
  g_free(text);

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
sc_search(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL || tab->web_view == NULL) {
    return false;
  }

  if (jumanji->search.item == NULL) {
    return false;
  }

  gboolean direction = (argument->n == BACKWARDS) ? FALSE : TRUE;
  webkit_web_view_search_text(WEBKIT_WEB_VIEW(tab->web_view),
      jumanji->search.item, FALSE, direction, TRUE);

  return true;
}

bool
sc_reload(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab != NULL && tab->web_view != NULL) {
    if (argument->n == BYPASS_CACHE) {
      webkit_web_view_reload_bypass_cache(WEBKIT_WEB_VIEW(tab->web_view));
    } else {
      webkit_web_view_reload(WEBKIT_WEB_VIEW(tab->web_view));
    }
  }

  return false;
}

bool
sc_toggle_bookmark(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL || tab->web_view == NULL || jumanji->database.session == NULL) {
    return false;
  }

  const char* url   = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
  const char* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));

  girara_list_t* results = db_bookmark_find(jumanji->database.session, url);
  if (results && girara_list_size(results) > 0) {
    db_bookmark_remove(jumanji->database.session, url);
  } else {
    db_bookmark_add(jumanji->database.session, url, title);
  }
  girara_list_free(results);

  return false;
}

bool
sc_toggle_proxy(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  return cb_statusbar_proxy(NULL, NULL, session);
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
sc_yank(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL) {
    return false;
  }

  char* url = (char*) webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));

  if (url == NULL) {
    return false;
  }

  GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  if (clipboard != NULL) {
    gtk_clipboard_set_text(clipboard, url, -1);
  }

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

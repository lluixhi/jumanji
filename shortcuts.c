/* See LICENSE file for license and copyright information */

#include <girara/session.h>
#include <girara/settings.h>
#include <girara/shortcuts.h>
#include <girara/datastructures.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "database.h"
#include "jumanji.h"
#include "shortcuts.h"

bool
sc_goto_homepage(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  char* homepage = NULL;
  girara_setting_get(jumanji->ui.session, "homepage", &homepage);
  char* url      = NULL;
  if (homepage != NULL) {
    url = jumanji_build_url_from_string(jumanji, homepage);
    g_free(homepage);
  }

  if (argument->n == NEW_TAB) {
    bool focus_new_tabs;
    girara_setting_get(jumanji->ui.session, "focus-new-tabs", &focus_new_tabs);
    jumanji_tab_new(jumanji, url, focus_new_tabs);
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
sc_goto_parent_directory(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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
    for (int i = 0; i < limit; i++) {
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
sc_focus_inputbar(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  if (gtk_widget_get_visible(GTK_WIDGET(session->gtk.inputbar)) == FALSE) {
    gtk_widget_show(GTK_WIDGET(session->gtk.inputbar));
  }

  if (gtk_widget_get_visible(GTK_WIDGET(session->gtk.notification_area)) == TRUE) {
    gtk_widget_hide(GTK_WIDGET(session->gtk.notification_area));
  }

  if (argument->data) {
    if (argument->n == APPEND_URL) {
      jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
      if (tab != NULL && tab->web_view != NULL) {
        const char* uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
        char* data      = g_strdup_printf("%s%s", (char*) argument->data, uri);
        gtk_entry_set_text(session->gtk.inputbar_entry, data);
        g_free(data);
      } else {
        gtk_entry_set_text(session->gtk.inputbar_entry, (char*) argument->data);
      }
    } else {
      gtk_entry_set_text(session->gtk.inputbar_entry, (char*) argument->data);
    }

    /* save the X clipboard that will be cleared by "grab focus" */
    gchar* x_clipboard_text = gtk_clipboard_wait_for_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

    gtk_widget_grab_focus(GTK_WIDGET(session->gtk.inputbar_entry));
    gtk_editable_set_position(GTK_EDITABLE(session->gtk.inputbar_entry), -1);

    if (x_clipboard_text != NULL) {
      /* reset x clipboard */
      gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), x_clipboard_text, -1);
      g_free(x_clipboard_text);
    }
  }

  return false;
}

bool
sc_tab_navigate(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(argument != NULL, false);

  girara_argument_t arg = { GIRARA_NEXT, argument->data };

  if (argument->n == PREVIOUS) {
    arg.n = GIRARA_PREVIOUS;
  }

  return girara_sc_tab_navigate(session, &arg, event, t);
}

bool
sc_navigate_history(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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
sc_put(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  GtkClipboard* clipboard;
  char* default_clipboard = NULL;
  girara_setting_get(session, "default-clipboard", &default_clipboard);
  if (g_strcmp0(default_clipboard, "secondary") == 0) {
    clipboard = gtk_clipboard_get(GDK_SELECTION_SECONDARY);
  } else if (g_strcmp0(default_clipboard, "clipboard") == 0) {
    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  } else {
    clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  }

  if (clipboard == NULL) {
    return false;
  }

  char* text = gtk_clipboard_wait_for_text(clipboard);
  char* url  = jumanji_build_url_from_string(jumanji, text);

  if (url == NULL) {
    return false;
  }

  if (argument->n == NEW_TAB) {
    bool focus_new_tabs;
    girara_setting_get(jumanji->ui.session, "focus-new-tabs", &focus_new_tabs);
    jumanji_tab_new(jumanji, url, focus_new_tabs);
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
sc_quit(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  girara_argument_t arg = { GIRARA_HIDE, NULL };
  girara_isc_completion(session, &arg, NULL, 0);

  cb_destroy(NULL, NULL);

  gtk_main_quit();

  return false;
}

bool
sc_scroll(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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

  int scroll_step = 40;
  girara_setting_get(jumanji->ui.session, "scroll-step", &scroll_step);

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
sc_search(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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
sc_reload(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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
sc_restore(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;

  if (jumanji->global.last_closed == NULL ||
      girara_list_size(jumanji->global.last_closed) == 0) {
    return false;
  }

  char* uri = girara_list_nth(jumanji->global.last_closed, 0);

  if (uri == NULL) {
    return false;
  }

  bool focus_new_tabs;
  girara_setting_get(jumanji->ui.session, "focus-new-tabs", &focus_new_tabs);
  jumanji_tab_new(jumanji, uri, focus_new_tabs);
  girara_list_remove(jumanji->global.last_closed, uri);

  return true;
}

bool
sc_toggle_bookmark(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL || tab->web_view == NULL || jumanji->database== NULL) {
    return false;
  }

  const char* url   = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
  const char* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));

  girara_list_t* results = jumanji_db_bookmark_find(jumanji->database, url);
  if (results && girara_list_size(results) > 0) {
    jumanji_db_bookmark_remove(jumanji->database, url);
    girara_notify(session, GIRARA_INFO, "Removed bookmark: %s", url);
  } else {
    jumanji_db_bookmark_add(jumanji->database, url, title);
    girara_notify(session, GIRARA_INFO, "Added bookmark: %s", url);
  }
  girara_list_free(results);

  return false;
}

bool
sc_toggle_proxy(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  return cb_statusbar_proxy(NULL, NULL, session);
}

bool
sc_toggle_plugins(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);

  bool value = true;
  girara_setting_get(session, "enable-plugins", &value);

  value = !value;
  girara_setting_set(session, "enable-plugins", &value);

  return false;
}

bool
sc_toggle_source_mode(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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

  if(webkit_web_view_get_view_source_mode(WEBKIT_WEB_VIEW(tab->web_view))) {
    webkit_web_view_set_view_source_mode(WEBKIT_WEB_VIEW(tab->web_view), FALSE);
  } else {
    webkit_web_view_set_view_source_mode(WEBKIT_WEB_VIEW(tab->web_view), TRUE);
  }

  jumanji_tab_load_url(tab, url);

  return false;
}

bool
sc_yank(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
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

  GtkClipboard* clipboard;
  char* default_clipboard = NULL;
  girara_setting_get(session, "default-clipboard", &default_clipboard);
  if (g_strcmp0(default_clipboard, "secondary") == 0) {
    clipboard = gtk_clipboard_get(GDK_SELECTION_SECONDARY);
  } else if (g_strcmp0(default_clipboard, "clipboard") == 0) {
    clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  } else {
    clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  }

  if (clipboard != NULL) {
    gtk_clipboard_set_text(clipboard, url, -1);
    girara_notify(session, GIRARA_INFO, "Yanked: %s", url);
  }

  return false;
}

bool
sc_zoom(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL) {
    return false;
  }

  int zoom_step = 10;
  girara_setting_get(jumanji->ui.session, "zoom-step", &zoom_step);

  const float zoom_level = webkit_web_view_get_zoom_level(WEBKIT_WEB_VIEW(tab->web_view));
  const float step = zoom_step / 100.0f;

  if(argument->n == ZOOM_IN) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), zoom_level + step);
  } else if(argument->n == ZOOM_OUT) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), zoom_level - step);
  } else if(argument->n == DEFAULT) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), 1.0f);
  } else if(argument->n == ZOOM_SPECIFIC) {
    webkit_web_view_set_zoom_level(WEBKIT_WEB_VIEW(tab->web_view), t / 100.0f);
  }

  return false;
}

bool
sc_toggle_stylesheet(girara_session_t* session, girara_argument_t* argument, girara_event_t* event, unsigned int t)
{
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(session != NULL, false);
  gchar* user_stylesheet_uri = NULL;
  girara_setting_get(session, "user-stylesheet-uri", &user_stylesheet_uri);
  gchar* empty_string = "";

  if (g_strcmp0(user_stylesheet_uri, empty_string) == 0) {
    girara_setting_set(session, "user-stylesheet-uri", jumanji->global.user_stylesheet_uri);
  }
  else if (g_strcmp0(user_stylesheet_uri, jumanji->global.user_stylesheet_uri) == 0) {
    girara_setting_set(session, "user-stylesheet-uri", empty_string);
  }
  else {
    g_free(jumanji->global.user_stylesheet_uri);
    jumanji->global.user_stylesheet_uri = g_strdup(user_stylesheet_uri);
    girara_setting_set(session, "user-stylesheet-uri", empty_string);
  }

  return false;
}

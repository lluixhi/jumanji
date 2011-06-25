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
  jumanji_tab_t* tab = (jumanji_tab_t*) data;

  if (web_view == NULL || tab == NULL || tab->jumanji == NULL || tab->jumanji->ui.session == NULL) {
    return;
  }

  unsigned int position = girara_tab_position_get(tab->jumanji->ui.session, tab->girara_tab) + 1;
  const gchar* title = webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));
  title = title ? title : "Loading...";

  char* text = g_strdup_printf("%d | %s", position, title);
  girara_tab_title_set(tab->girara_tab, text);
  g_free(text);

  if (tab == jumanji_tab_get_current(tab->jumanji)) {
    const gchar* url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    girara_statusbar_item_set_text(tab->jumanji->ui.session, tab->jumanji->ui.statusbar.url, url ? (char*) url : "Loading...");
  }
}

void
cb_jumanji_tab_changed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji)
{
  if (tabs == NULL || page == NULL || jumanji == NULL || jumanji->ui.statusbar.url == NULL) {
    return;
  }

  jumanji_tab_t* tab = jumanji_tab_get_nth(jumanji, page_num);

  if (tab != NULL) {
    const gchar* url = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.url, url ? (char*) url : "Loading...");
  }
}

void
cb_jumanji_tab_removed(GtkNotebook* tabs, GtkWidget* page, guint page_num, jumanji_t* jumanji)
{
  if (tabs == NULL || jumanji == NULL) {
    return;
  }

  if (gtk_notebook_get_n_pages(tabs) == 0) {
    char* homepage = girara_setting_get(jumanji->ui.session, "homepage");
    if (homepage != NULL) {
      char* url = jumanji_build_url_from_string(jumanji, homepage);
      jumanji_tab_new(jumanji, url, false);
      free(url);
    }
  }
}

void
cb_settings_webkit(girara_session_t* session, girara_setting_t* setting)
{
  g_return_if_fail(session != NULL);
  g_return_if_fail(setting != NULL);
  g_return_if_fail(session->global.data != NULL);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  WebKitWebSettings* browser_settings = NULL;
  jumanji_tab_t* tab                  = jumanji_tab_get_current(jumanji);

  /* get settings */
  if (girara_get_number_of_tabs(session) == 0) {
    browser_settings = jumanji->global.browser_settings;
  } else if (tab != NULL) {
    browser_settings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(tab->web_view));
  } else {
    return;
  }

  /* special case: set value in webkitview */
  if (g_strcmp0(setting->name, "full-content-zoom") == 0) {
    if (tab && tab->web_view) {
      g_object_set(G_OBJECT(tab->web_view), setting->name, setting->value.b, NULL);
    }
  } else if (browser_settings != NULL) {
    switch (setting->type) {
      case STRING:
        g_object_set(G_OBJECT(browser_settings), setting->name, setting->value.s, NULL);
        break;
      case INT:
        g_object_set(G_OBJECT(browser_settings), setting->name, setting->value.i, NULL);
        break;
      case FLOAT:
        g_object_set(G_OBJECT(browser_settings), setting->name, setting->value.f, NULL);
        break;
      case BOOLEAN:
        g_object_set(G_OBJECT(browser_settings), setting->name, setting->value.b, NULL);
        break;
      default:
        return;
    }
  }
}

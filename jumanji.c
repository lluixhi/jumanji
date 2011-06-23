/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "config.h"
#include "jumanji.h"

#define GLOBAL_RC  "/etc/jumanjirc"
#define JUMANJI_RC "jumanjirc"

jumanji_t*
jumanji_init(int argc, char* argv[])
{
  /* parse command line options */
  gchar* config_dir = NULL, *data_dir = NULL;
  GOptionEntry entries[] = {
    { "config-dir", 'c', 0, G_OPTION_ARG_FILENAME, &config_dir, "Path to the config directory", "path" },
    { "data-dir",   'd', 0, G_OPTION_ARG_FILENAME, &data_dir,   "Path to the data directory",   "path" },
    { NULL }
  };

  GOptionContext* context = g_option_context_new(" [url]");
  g_option_context_add_main_entries(context, entries, NULL);

  GError* error = NULL;
  if (g_option_context_parse(context, &argc, &argv, &error) == FALSE) {
    girara_error("Error parsing command line arguments: %s\n", error->message);
    g_option_context_free(context);
    g_error_free(error);
    goto error_out;
  }
  g_option_context_free(context);

  /* jumanji */
  jumanji_t* jumanji  = malloc(sizeof(jumanji_t));

  if (jumanji == NULL) {
    goto error_out;
  }

  if (config_dir) {
    jumanji->config.config_dir = g_strdup(config_dir);
  } else {
    gchar* path = girara_get_xdg_path(XDG_CONFIG);
    jumanji->config.config_dir = g_build_filename(path, "jumanji", NULL);
    g_free(path);
  }

  if (data_dir) {
    jumanji->config.data_dir = g_strdup(config_dir);
  } else {
    gchar* path = girara_get_xdg_path(XDG_DATA);
    jumanji->config.data_dir = g_build_filename(path, "jumanji", NULL);
    g_free(path);
  }

  /* UI */
  if ((jumanji->ui.session = girara_session_create()) == NULL) {
    goto error_free;
  }

  jumanji->ui.session->global.data  = jumanji;
  jumanji->ui.statusbar.buffer      = NULL;

  /* configuration */
  config_load_default(jumanji);

  /* load global configuration files */
  config_load_file(jumanji, GLOBAL_RC);

  /* load local configuration files */
  char* configuration_file = g_build_filename(jumanji->config.config_dir, JUMANJI_RC, NULL);
  config_load_file(jumanji, configuration_file);
  g_free(configuration_file);

  /* initialize girara */
  if (girara_session_init(jumanji->ui.session) == false) {
    goto error_free;
  }

  /* enable tabs */
  girara_tabs_enable(jumanji->ui.session);

  /* girara events */
  jumanji->ui.session->events.buffer_changed = cb_girara_buffer_changed;

  /* statusbar */
  jumanji->ui.statusbar.url = girara_statusbar_item_add(jumanji->ui.session, TRUE, TRUE, TRUE, NULL);
  if (jumanji->ui.statusbar.url == NULL) {
    goto error_free;
  }

  jumanji->ui.statusbar.buffer = girara_statusbar_item_add(jumanji->ui.session, FALSE, FALSE, FALSE, NULL);
  if (jumanji->ui.statusbar.buffer == NULL) {
    goto error_free;
  }

  /* webkit */
  jumanji->global.browser_settings = webkit_web_settings_new();
  if (jumanji->global.browser_settings == NULL) {
    goto error_free;
  }

  /* load tabs */
  if(argc < 2) {
    jumanji_tab_new(jumanji, "http://pwmt.org", false);
  } else {
    for (unsigned int i = argc - 1; i >= 1; i--) {
      jumanji_tab_new(jumanji, argv[i], false);
    }

  }

  return jumanji;

error_free:

  if (jumanji && jumanji->ui.session) {
    girara_session_destroy(jumanji->ui.session);
  }

  free(jumanji);

error_out:

  return NULL;
}

void
jumanji_free(jumanji_t* jumanji)
{
  if (jumanji == NULL) {
    return;
  }

  if (jumanji->ui.session != NULL) {
    girara_session_destroy(jumanji->ui.session);
  }

  free(jumanji);
}

jumanji_tab_t*
jumanji_tab_new(jumanji_t* jumanji, const char* url, bool background)
{
  if (jumanji == NULL || url == NULL) {
    goto error_out;
  }

  jumanji_tab_t* tab = malloc(sizeof(jumanji_tab_t));

  if (tab == NULL) {
    goto error_out;
  }

  tab->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  tab->web_view        = webkit_web_view_new();
  tab->jumanji         = jumanji;

  if (tab->scrolled_window == NULL || tab->web_view == NULL) {
    goto error_free;
  }

  g_signal_connect(G_OBJECT(tab->scrolled_window), "destroy", G_CALLBACK(cb_jumanji_tab_destroy), tab);

  gtk_container_add(GTK_CONTAINER(tab->scrolled_window), tab->web_view);
  gtk_widget_show_all(tab->scrolled_window);

  /* apply browser setting */
  webkit_web_view_set_settings(WEBKIT_WEB_VIEW(tab->web_view), webkit_web_settings_copy(jumanji->global.browser_settings));

  /* load url */
  jumanji_tab_load_url(tab, url);

  /* create new tab */
  girara_tab_new(jumanji->ui.session, NULL, tab->scrolled_window, true, jumanji);
  g_object_set_data(G_OBJECT(tab->scrolled_window), "jumanji-tab", tab);

  return tab;

error_free:

  free(tab);

error_out:

  return NULL;
}

void
jumanji_tab_free(jumanji_tab_t* tab)
{
  if (tab == NULL) {
    return;
  }

  free(tab);
}

jumanji_tab_t*
jumanji_tab_get_current(jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->ui.session == NULL) {
    return NULL;
  }

  girara_tab_t* girara_tab = girara_tab_current_get(jumanji->ui.session);

  if (girara_tab == NULL) {
    return NULL;
  }

  return g_object_get_data(G_OBJECT(girara_tab->widget), "jumanji-tab");
}

jumanji_tab_t*
jumanji_tab_get_nth(jumanji_t* jumanji, unsigned int index)
{
  if (jumanji == NULL || jumanji->ui.session == NULL) {
    return NULL;
  }

  girara_tab_t* girara_tab = girara_tab_get(jumanji->ui.session, index);

  if (girara_tab == NULL) {
    return NULL;
  }

  return g_object_get_data(G_OBJECT(girara_tab->widget), "jumanji-tab");
}

void
jumanji_tab_load_url(jumanji_tab_t* tab, const char* url)
{
  if (tab == NULL || url == NULL || tab->web_view == NULL) {
    return;
  }

  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(tab->web_view), url);
}

/* main function */
int main(int argc, char* argv[])
{
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc, &argv);

  jumanji_t* jumanji = jumanji_init(argc, argv);
  if (jumanji == NULL) {
    printf("error: coult not initialize jumanji\n");
    return -1;
  }

  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();

  jumanji_free(jumanji);

  return 0;
}

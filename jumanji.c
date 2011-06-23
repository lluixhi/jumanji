/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara.h>

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
  jumanji->ui.session->events.buffer_changed = buffer_changed;

  /* statusbar */
  jumanji->ui.statusbar.url = girara_statusbar_item_add(jumanji->ui.session, TRUE, TRUE, TRUE, NULL);
  if (jumanji->ui.statusbar.url == NULL) {
    goto error_free;
  }

  jumanji->ui.statusbar.buffer = girara_statusbar_item_add(jumanji->ui.session, FALSE, FALSE, FALSE, NULL);
  if (jumanji->ui.statusbar.buffer == NULL) {
    goto error_free;
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

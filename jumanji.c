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
  jumanji_t* jumanji = malloc(sizeof(jumanji_t));

  if (jumanji == NULL) {
    goto error_out;
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

  /* initialize girara */
  if (girara_session_init(jumanji->ui.session) == false) {
    goto error_free;
  }

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

  girara_session_destroy(jumanji->ui.session);
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

/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "userscripts.h"

girara_list_t*
user_script_load_dir(const char* path)
{
  /* create list */
  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  girara_list_set_free_function(list, user_script_free);

  /* open directory */
  GDir* dir = g_dir_open(path, 0, NULL);
  if (dir == NULL) {
    /* Return an empty list if we are not able to open/read the user scripts
     * directory */
    return list;
  }

  /* read files */
  const char* file = NULL;

  while ((file = g_dir_read_name(dir)) != NULL) {
    char* filename = g_build_filename(path, file, NULL);

    if (g_file_test(filename, G_FILE_TEST_IS_REGULAR) == TRUE) {
      user_script_t* user_script = user_script_load_file(filename);
      if (user_script != NULL) {
        girara_list_append(list, user_script);
      } else {
        fprintf(stderr, "could not parse user script: %s\n", filename);
      }
    }

    g_free(filename);
  }

  g_dir_close(dir);

  return list;
}

user_script_t*
user_script_load_file(const char* filename)
{
  return NULL;
}

void
user_script_free(void* data)
{
  if (data == NULL) {
    return;
  }

  user_script_t* user_script = (user_script_t*) data;

  free(user_script->name);
  free(user_script->description);
  free(user_script->filename);

  /* free include list */
  if (girara_list_size(user_script->include) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(user_script->include);
    do {
      free(girara_list_iterator_data(iter));
    } while (girara_list_iterator_next(iter));
    girara_list_iterator_free(iter);
  }
  girara_list_free(user_script->include);

  /* free exclude list */
  if (girara_list_size(user_script->exclude) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(user_script->exclude);
    do {
      free(girara_list_iterator_data(iter));
    } while (girara_list_iterator_next(iter));
    girara_list_iterator_free(iter);
  }
  girara_list_free(user_script->exclude);
}

void
user_script_init_tab(jumanji_tab_t* tab, girara_list_t* user_scripts)
{
  if (tab == NULL || tab->web_view == NULL || user_scripts == NULL) {
    return;
  }

  g_signal_connect(G_OBJECT(tab->web_view), "notify::load-status",
      G_CALLBACK(cb_user_script_tab_load_status), user_scripts);
}

void
cb_user_script_tab_load_status(WebKitWebView* web_view, GParamSpec* pspec,
    girara_list_t* user_scripts)
{
}

/* See LICENSE file for license and copyright information */

#include <gtk/gtk.h>

#include "utils.h"

girara_list_t*
build_girara_list(const char* string)
{
  if (string == NULL) {
    return NULL;
  }

  gchar** argv = NULL;
  gint    argc = 0;

  if (g_shell_parse_argv(string, &argc, &argv, NULL) == FALSE) {
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    g_strfreev(argv);
    return NULL;
  }

  girara_list_set_free_function(list, girara_list_free_data);

  for(int i = 0; i < argc; i++) {
    girara_list_append(list, (void*) g_strdup(argv[i]));
  }

  g_strfreev(argv);

  return list;
}

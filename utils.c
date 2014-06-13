/* See LICENSE file for license and copyright information */

#include <glib.h>

#include "utils.h"
#include <girara/datastructures.h>
#include <string.h>

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

  girara_list_set_free_function(list, g_free);

  for (int i = 0; i < argc; i++) {
    girara_list_append(list, (void*) g_strdup(argv[i]));
  }

  g_strfreev(argv);

  return list;
}

char*
url_encode(const char* string)
{
  if (string == NULL) {
    return NULL;
  }

  char* escaped = g_uri_escape_string(string, NULL, true);
  if (strchr(escaped, '+') == NULL) {
    return escaped;
  }

  char* ret = g_strjoinv("%2B", g_strsplit(escaped, "+", -1));
  g_free(escaped);
  return ret;
}

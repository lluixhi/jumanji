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

  for(int i = 0; i < argc; i++) {
    girara_list_append(list, (void*) g_strdup(argv[i]));
  }

  g_strfreev(argv);

  return list;
}

char*
url_encode(const char* string) {
  char* ret_str = g_malloc(sizeof(char));
  char* tmp = NULL;

  for (unsigned int i=0 ; i<strlen(string) ; i++) {
    tmp = g_strdup(ret_str);
    g_free(ret_str);
    if (string[i] == ' ')
      ret_str = g_strdup_printf("%s+", tmp);
    else if (!g_ascii_isalnum(string[i]))
      ret_str = g_strdup_printf("%s%%%2x", tmp, string[i]);
    else
      ret_str = g_strdup_printf("%s%c", tmp, string[i]);
    g_free(tmp);
  }

  return ret_str;
}

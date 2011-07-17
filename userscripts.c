/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include "userscripts.h"

#define USER_SCRIPT_HEADER "//.*==UserScript==.*//.*==/UserScript=="
#define USER_SCRIPT_VAR_VAL_PAIR "//\\s+@(?<name>\\S+)(\\s+(?<value>.*))?"

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
    char* filepath = g_build_filename(path, file, NULL);

    if (g_file_test(filepath, G_FILE_TEST_IS_REGULAR) == TRUE) {
      user_script_t* user_script = user_script_load_file(filepath);
      if (user_script != NULL) {
        girara_list_append(list, user_script);
      } else {
        fprintf(stderr, "could not parse user script: %s\n", filepath);
      }
    }

    g_free(filepath);
  }

  g_dir_close(dir);

  return list;
}

user_script_t*
user_script_load_file(const char* path)
{
  /* init values */
  char* name                  = NULL;
  char* description           = NULL;
  const char* filename        = path;
  girara_list_t* include      = girara_list_new();
  girara_list_t* exclude      = girara_list_new();
  bool load_on_document_start = false;

  if (include == NULL || exclude == NULL) {
    return NULL;
  }

  /* read file */
  char* content = girara_file_read(path);
  if (content == NULL) {
    return NULL;
  }

  /* parse header */
  GMatchInfo* match_info;
  GRegex* regex = g_regex_new(USER_SCRIPT_HEADER, G_REGEX_MULTILINE | G_REGEX_DOTALL, 0, NULL);

  g_regex_match(regex, content, 0, &match_info);

  /* get header */
  if (g_match_info_get_match_count(match_info) > 0) {
    gchar* header = g_match_info_fetch(match_info, 0);

    /* parse header */
    GMatchInfo* header_match_info;
    GRegex* header_regex = g_regex_new(USER_SCRIPT_VAR_VAL_PAIR, 0, 0, NULL);

    g_regex_match(header_regex, header, 0, &header_match_info);

    while (g_match_info_matches(header_match_info) == TRUE) {
      char* header_name  = g_match_info_fetch_named(header_match_info, "name");
      char* header_value = g_match_info_fetch_named(header_match_info, "value");

      if (g_strcmp0(header_name, "name") == 0) {
        name = header_value;
      } else if (g_strcmp0(header_name, "description") == 0) {
        description = header_value;
      } else if (g_strcmp0(header_name, "include") == 0) {
        girara_list_append(include, header_value);
      } else if (g_strcmp0(header_name, "exclude") == 0) {
        girara_list_append(exclude, header_value);
      } else if (g_strcmp0(header_name, "run-at") == 0) {
        if (g_strcmp0(header_value, "document-start") == 0) {
          load_on_document_start = true;
        }
      } else {
        g_free(header_value);
      }

      g_free(name);
      g_match_info_next(header_match_info, NULL);
    }

    g_match_info_free(header_match_info);
    g_regex_unref(header_regex);

    g_free(header);
  /* invalid / non-existing header */
  } else {
    g_match_info_free(match_info);
    g_regex_unref(regex);
    free(content);
    return NULL;
  }

  g_regex_unref(regex);
  g_match_info_free(match_info);

  free(content);

  /* create user script object */
  user_script_t* user_script = malloc(sizeof(user_script_t));
  if (user_script == NULL) {
    return NULL;
  }

  user_script->name                   = name;
  user_script->description            = description;
  user_script->filename               = g_strdup(filename);
  user_script->include                = include;
  user_script->exclude                = exclude;
  user_script->load_on_document_start = load_on_document_start;

  return user_script;
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

  /* free object */
  free(user_script);
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

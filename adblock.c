/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
#include <girara.h>

#include "adblock.h"

girara_list_t*
adblock_filter_load_dir(const char* path)
{
  /* create list */
  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  girara_list_set_free_function(list, adblock_filter_free);

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
      adblock_filter_t* filter = adblock_filter_load(filepath);
      if (filter != NULL) {
        girara_list_append(list, filter);
        girara_info("[adblock] loaded filter: %s", filter->name ? filter->name : filepath);
      } else {
        girara_error("[adblock] could not load filter: %s", filepath);
      }
    }

    g_free(filepath);
  }

  g_dir_close(dir);

  return list;
}

adblock_filter_t*
adblock_filter_load(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  girara_list_t* pattern = girara_list_new();
  if (pattern == NULL) {
    return NULL;
  }

  /* read file */
  FILE* file = girara_file_open(path, "r");

  if (file == NULL) {
    return NULL;
  }

  /* read lines */
  char* line = NULL;
  while ((line = girara_file_read_line(file)) != NULL) {
    if (strlen(line) == 0 || line[0] == '!' || line[0] == '[') {
      continue;
    }

    girara_list_append(pattern, line);

    free(line);
  }

  fclose(file);

  /* init filter */
  adblock_filter_t* filter = malloc(sizeof(adblock_filter_t));
  if (filter == NULL) {
    girara_list_free(pattern);
  }

  filter->name    = g_strdup(path);
  filter->pattern = pattern;

  return filter;
}

void
adblock_filter_free(void* data)
{
  if (data == NULL) {
    return;
  }

  adblock_filter_t* filter = (adblock_filter_t*) data;

  free(filter->name);

  if (girara_list_size(filter->pattern) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(filter->pattern);
    do {
      free(girara_list_iterator_data(iter));
    } while (girara_list_iterator_next(iter));
    girara_list_iterator_free(iter);
  }
  girara_list_free(filter->pattern);
}

void
adblock_filter_init_tab(jumanji_tab_t* tab, girara_list_t* adblock_filters)
{
  if (tab == NULL || tab->web_view == NULL || adblock_filters == NULL) {
    return;
  }

  g_signal_connect(G_OBJECT(tab->web_view), "resource-request-starting",
      G_CALLBACK(cb_adblock_filter_resource_request_starting), adblock_filters);
}

void
cb_adblock_filter_resource_request_starting(WebKitWebView* web_view,
    WebKitWebFrame* web_frame, WebKitWebResource* web_resource,
    WebKitNetworkRequest* request, WebKitNetworkResponse* response,
    girara_list_t* adblock_filters)
{
  if (web_view == NULL || adblock_filters == NULL || web_resource == NULL ||
      request == NULL) {
    return;
  }

  /* get resource uri */
  /*const char* uri = webkit_web_resource_get_uri(web_resource);*/

  /* check all user scripts */
  girara_list_iterator_t* iter = girara_list_iterator(adblock_filters);
  do {
    adblock_filter_t* filter = (adblock_filter_t*) girara_list_iterator_data(iter);
    if (filter == NULL) {
      continue;
    }

    girara_list_iterator_t* pattern_iter = girara_list_iterator(filter->pattern);
    do {
      char* expression = girara_list_iterator_data(pattern_iter);
      if (expression == NULL) {
        continue;
      }

      /*webkit_network_request_set_uri(request, "about:blank"); */
    } while (girara_list_iterator_next(pattern_iter));
    girara_list_iterator_free(pattern_iter);
  } while (girara_list_iterator_next(iter));
  girara_list_iterator_free(iter);
}

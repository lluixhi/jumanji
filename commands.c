/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>

#include "commands.h"
#include "database.h"
#include "jumanji.h"

bool
cmd_bookmark_add(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;

  if (jumanji->database.session == NULL) {
    return false;
  }

  int number_of_arguments = girara_list_size(argument_list);
  char* url   = NULL;
  char* title = NULL;

  if (number_of_arguments > 0) {
    url   = girara_list_nth(argument_list, 0);
    title = (number_of_arguments > 1) ? girara_list_nth(argument_list, 1) : NULL;
  } else {
    jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
    if (tab != NULL && tab->web_view != NULL) {
      url   = (char*) webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
      title = (char*) webkit_web_view_get_title(WEBKIT_WEB_VIEW(tab->web_view));
    }
  }

  db_bookmark_add(jumanji->database.session, url, title);

  return true;
}

bool
cmd_bookmark_delete(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;

  if (jumanji->database.session == NULL) {
    return false;
  }

  char* url = NULL;

  if (girara_list_size(argument_list) > 0) {
    url   = girara_list_nth(argument_list, 0);
  } else {
    jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);
    if (tab != NULL && tab->web_view != NULL) {
      url = (char*) webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    }
  }

  db_bookmark_remove(jumanji->database.session, url);

  return true;
}

bool
cmd_buffer_delete(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);

  return true;
}

bool
cmd_open(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  char* url = jumanji_build_url(jumanji, argument_list);
  jumanji_tab_load_url(jumanji_tab_get_current(jumanji), url);

  return true;
}

bool
cmd_proxy(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  if (jumanji->global.proxies == NULL) {
    return false;
  }

  unsigned int number_of_arguments = girara_list_size(argument_list);
  if (number_of_arguments < 1) {
    return false;
  }

  char* url         = (char*) girara_list_nth(argument_list, 0);
  char* description = (number_of_arguments > 1) ? (char*) girara_list_nth(argument_list, 1) : NULL;

  if (url == NULL) {
    return false;
  }

  url = (strstr(url, "://") != NULL) ? g_strdup(url) : g_strconcat("http://", url, NULL);

  /* search for existing search engine */
  if (girara_list_size(jumanji->global.proxies) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.proxies);

    do {
      jumanji_proxy_t* proxy = (jumanji_proxy_t*) girara_list_iterator_data(iter);
      if (proxy == NULL) {
        continue;
      }

      if (!g_strcmp0(proxy->url, url)) {
        g_free(proxy->url);
        g_free(proxy->description);
        proxy->url         = g_strdup(url);
        proxy->description = description ? g_strdup(description) : NULL;
        g_free(url);
        return true;
      }
    } while (girara_list_iterator_next(iter));
  }

  /* create new entry */
  jumanji_proxy_t* proxy = malloc(sizeof(jumanji_proxy_t));
  if (proxy == NULL) {
    g_free(url);
    return false;
  }

  proxy->url         = url;
  proxy->description = g_strdup(description);

  girara_list_append(jumanji->global.proxies, proxy);

  return true;
}

bool
cmd_search_engine(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  if (jumanji->global.search_engines == NULL) {
    return false;
  }

  if (girara_list_size(argument_list) < 2) {
    return false;
  }

  char* identifier = (char*) girara_list_nth(argument_list, 0);
  char* url        = (char*) girara_list_nth(argument_list, 1);

  /* search for existing search engine */
  if (girara_list_size(jumanji->global.search_engines) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.search_engines);

    do {
      jumanji_search_engine_t* search_engine = (jumanji_search_engine_t*) girara_list_iterator_data(iter);
      if (search_engine == NULL) {
        continue;
      }

      if (!g_strcmp0(search_engine->identifier, identifier)) {
        g_free(search_engine->url);
        search_engine->url = g_strdup(url);
        return true;
      }
    } while (girara_list_iterator_next(iter));
  }

  /* create new entry */
  jumanji_search_engine_t* search_engine = malloc(sizeof(jumanji_search_engine_t));
  if (search_engine == NULL) {
    return false;
  }

  search_engine->url        = g_strdup(url);
  search_engine->identifier = g_strdup(identifier);

  girara_list_append(jumanji->global.search_engines, search_engine);

  return true;
}

bool
cmd_tabopen(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = (jumanji_t*) session->global.data;

  char* url = jumanji_build_url(jumanji, argument_list);
  jumanji_tab_new(jumanji, url, false);

  return true;
}

/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>

#include "session.h"
#include "database.h"
#include <girara/session.h>
#include <girara/callbacks.h>
#include <girara/datastructures.h>
#include <girara/tabs.h>
#include <girara/settings.h>

bool
sessionload(girara_session_t* session, const char* name)
{
  jumanji_t* jumanji = (jumanji_t*) session->global.data;
  girara_list_t* url_list;
  girara_list_iterator_t* iter;
  jumanji_db_result_link_t* link;

  url_list = jumanji_db_load_session(jumanji->database, name);
  /* in case of empty session file, do nothing and return */
  if (url_list == NULL) {
    return false;
  }

  iter = girara_list_iterator(url_list);

  bool focus_new_tabs = true;
  girara_setting_get(session, "focus-new-tabs", &focus_new_tabs);

  do {
    link = girara_list_iterator_data(iter);
    if (link != NULL && link->url != NULL) {
      jumanji_tab_new(jumanji, link->url, focus_new_tabs);
    }
  } while(girara_list_iterator_next(iter) != NULL);

  girara_list_free(url_list);

  return true;
}

bool
sessionsave(girara_session_t* session, const char* name)
{
  jumanji_t* jumanji = (jumanji_t*) session->global.data;
  jumanji_tab_t* tab;
  girara_list_t* url_list = girara_list_new();
  jumanji_db_result_link_t* link;

  const int num_tabs = girara_get_number_of_tabs(jumanji->ui.session);
  for (int tab_index = 0; tab_index != num_tabs; ++tab_index) {
    tab = jumanji_tab_get_nth(jumanji, tab_index);
    if (tab == NULL) {
      continue;
    }

    link = g_malloc0(sizeof(jumanji_db_result_link_t));
    link->url = (char*) webkit_web_view_get_uri(WEBKIT_WEB_VIEW(tab->web_view));
    link->title = NULL;
    link->visited = false;
    girara_list_append(url_list, link);
  }

  jumanji_db_save_session(jumanji->database, name, url_list);

  girara_list_free(url_list);

  return true;
}

bool
cmd_sessionsave(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);

  int number_of_arguments = girara_list_size(argument_list);
  char* session_name = NULL;

  if (number_of_arguments > 0) {
    session_name = girara_list_nth(argument_list, 0);
    if (!sessionsave(session, session_name)) {
      return false;
    }
    girara_notify(session, GIRARA_INFO, "Session saved: %s", session_name);
  } else {
    girara_notify(session, GIRARA_INFO, "A session name must be specified");
  }

  return true;
}

bool
cmd_sessionload(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);

  int number_of_arguments = girara_list_size(argument_list);
  char* session_name = NULL;

  if (number_of_arguments > 0) {
    session_name = girara_list_nth(argument_list, 0);
    if (!sessionload(session, session_name)) {
      return false;
    }
    girara_notify(session, GIRARA_INFO, "Session loaded: %s", session_name);
  } else {
    girara_notify(session, GIRARA_INFO, "A session name must be specified");
  }

  return true;
}

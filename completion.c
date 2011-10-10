/* See LICENSE file for license and copyright information */

#ifndef COMPLETION_H
#define COMPLETION_H

#include <girara.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include "completion.h"
#include "database.h"
#include "utils.h"

girara_completion_t*
cc_open(girara_session_t* session, const char* input)
{
  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(session->global.data != NULL, NULL);
  jumanji_t* jumanji = session->global.data;

  girara_completion_t* completion  = girara_completion_init();

  if (completion == NULL) {
    goto error_free;
  }

  /* search history */
  girara_completion_group_t* group = NULL;
  girara_list_t* bookmark_list     = jumanji_db_bookmark_find(jumanji->database, input);

  if (bookmark_list) {
    int bookmark_length = girara_list_size(bookmark_list);

    /* add group entry */
    if (bookmark_length > 0) {
      group = girara_completion_group_create(session, "Bookmarks");
      if (group == NULL) {
        goto error_free;
      } else {
        for (int i = 0; i < bookmark_length; i++) {
          jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_nth(bookmark_list, i);
          if (link) {
            girara_completion_group_add_element(group, link->url, link->title);
          }
        }

        girara_completion_add_group(completion, group);
      }
    }

    girara_list_free(bookmark_list);
  }

  /* search bookmarks */
  group                       = NULL;
  girara_list_t* history_list = jumanji_db_history_find(jumanji->database, input);

  if (history_list) {
    int history_length = girara_list_size(history_list);

    /* add group entry */
    if (history_length > 0) {
      group = girara_completion_group_create(session, "History");
      if (group == NULL) {
        goto error_free;
      } else {
        for (int i = 0; i < history_length; i++) {
          jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_nth(history_list, i);
          if (link) {
            girara_completion_group_add_element(group, link->url, link->title);
          }
        }

        girara_completion_add_group(completion, group);
      }
    }

    girara_list_free(history_list);
  }


  return completion;

error_free:

  if (completion) {
    girara_completion_free(completion);
  }

  if (group) {
    girara_completion_group_free(group);
  }

  return NULL;
}

#endif // COMPLETION_H

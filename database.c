/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <stdlib.h>

#include "database.h"

void
jumanji_db_free_result_link(void* data)
{
  if (data == NULL) {
    return;
  }

  jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) data;
  g_free(link->url);
  g_free(link->title);
  free(link);
}

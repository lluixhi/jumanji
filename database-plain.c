/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <stdlib.h>

#include "database-plain.h"

db_session_t*
db_plain_new(jumanji_t* jumanji)
{
  return NULL;
}

bool
db_plain_init(db_session_t* session)
{
  return false;
}

void
db_plain_set_bookmark_file(db_session_t* session, const char* bookmark_file)
{
}

void
db_plain_set_history_file(db_session_t* session, const char* history_file)
{
}

void
db_plain_close(db_session_t* session)
{
}

girara_list_t*
db_plain_bookmark_find(db_session_t* session, const char* input)
{
  return NULL;
}

void
db_plain_bookmark_remove(db_session_t* session, const char* url)
{
}

void
db_plain_bookmark_add(db_session_t* session, const char* url, const char* title)
{
}

girara_list_t*
db_plain_history_find(db_session_t* session, const char* input)
{
  return NULL;
}

void
db_plain_history_add(db_session_t* session, const char* url, const char* title)
{
}

void
db_plain_history_clean(db_session_t* session, unsigned int age)
{
}

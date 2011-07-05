/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <stdlib.h>

#include "database.h"

#ifdef SQLITE
#include "database-sqlite.h"
#else
#include "database-plain.h"
#endif

typedef struct db_functions_s
{
  bool (*init)(db_session_t*);
  void (*close)(db_session_t*);
  void (*bookmark_add)(db_session_t*, const char*, const char*);
  girara_list_t* (*bookmark_find)(db_session_t*, const char*);
  void (*bookmark_remove)(db_session_t*, const char*);
  void (*history_add)(db_session_t*, const char*, const char*);
  girara_list_t* (*history_find)(db_session_t*, const char*);
  void (*history_clean)(db_session_t*, unsigned int);
} db_functions_t;

static const db_functions_t db_function = {
#ifdef SQLITE
  db_sqlite_init,
  db_sqlite_close,
  db_sqlite_bookmark_add,
  db_sqlite_bookmark_find,
  db_sqlite_bookmark_remove,
  db_sqlite_history_add,
  db_sqlite_history_find,
  db_sqlite_history_clean
#else
  db_plain_init,
  db_plain_close,
  db_plain_bookmark_add,
  db_plain_bookmark_find,
  db_plain_bookmark_remove,
  db_plain_history_add,
  db_plain_history_find,
  db_plain_history_clean
#endif
};

db_session_t*
db_new(jumanji_t* jumanji)
{
  if (jumanji == NULL) {
    return NULL;
  }

  /* initialize database object */
  db_session_t* session = malloc(sizeof(db_session_t));
  if (session == NULL) {
    return NULL;
  }

  session->bookmark_file = NULL;
  session->history_file  = NULL;
  session->jumanji       = jumanji;
  session->data          = NULL;

  return session;
}

bool
db_init(db_session_t* session)
{
  if (db_function.init) {
    return db_function.init(session);
  } else {
    return false;
  }
}

void
db_set_bookmark_file(db_session_t* session, const char* bookmark_file)
{
  if (session == NULL || bookmark_file == NULL) {
    return;
  }

  session->bookmark_file = g_strdup(bookmark_file);
}

void
db_set_history_file(db_session_t* session, const char* history_file)
{
  if (session == NULL || history_file == NULL) {
    return;
  }

  session->history_file = g_strdup(history_file);
}

void
db_close(db_session_t* session)
{
  if (db_function.close) {
    db_function.close(session);
  }
}

void
db_bookmark_add(db_session_t* session, const char* url, const char* title)
{
  if (db_function.bookmark_add) {
    db_function.bookmark_add(session, url, title);
  }
}

girara_list_t*
db_bookmark_find(db_session_t* session, const char* input)
{
  if (db_function.bookmark_find) {
    return db_function.bookmark_find(session, input);
  } else {
    return NULL;
  }
}

void
db_bookmark_remove(db_session_t* session, const char* url)
{
  if (db_function.bookmark_remove) {
    db_function.bookmark_remove(session, url);
  }
}

void
db_history_add(db_session_t* session, const char* url, const char* title)
{
  if (db_function.history_add) {
    db_function.history_add(session, url, title);
  }
}

girara_list_t*
db_history_find(db_session_t* session, const char* input)
{
  if (db_function.history_find) {
    return db_function.history_find(session, input);
  } else {
    return NULL;
  }
}

void
db_history_clean(db_session_t* session, unsigned int age)
{
  if (db_function.history_clean) {
    db_function.history_clean(session, age);
  }
}

void
db_free_result_link(void* data)
{
  if (data == NULL) {
    return;
  }

  db_result_link_t* link = (db_result_link_t*) data;
  g_free(link->url);
  g_free(link->title);
  free(link);
}

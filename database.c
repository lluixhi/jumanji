/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <sqlite3.h>

#include "database.h"

typedef struct db_sqlite_s
{
  sqlite3* session; /**> sqlite3 session */
} db_sqlite_t;

static const char SQL_INIT[] =
  /* history table */
  "CREATE TABLE IF NOT EXISTS history ("
    "id INT PRIMARY KEY,"
    "url TEXT,"
    "title TEXT,"
    "visited INT"
    ");"

  /* bookmarks table */
  "CREATE TABLE IF NOT EXISTS bookmarks ("
    "id INT PRIMARY KEY,"
    "url TEXT,"
    "title TEXT"
    ");";

static const char SQL_LOOKUP_URL[] =
  "SELECT url FROM history WHERE (url LIKE ?) AND (title LIKE ?);";

static const char SQL_LOOKUP_BOOKMARK[] =
  "SELECT url FROM bookmarks WHERE (url LIKE ?) AND (title LIKE ?);";

db_session_t*
db_open(jumanji_t* jumanji, const char* filename)
{
  if (jumanji == NULL || filename == NULL) {
    goto error_out;
  }

  /* initialize database object */
  db_session_t* session = malloc(sizeof(db_session_t));
  if (session == NULL) {
    goto error_out;
  }

  session->filename = g_strdup(filename);
  session->data     = malloc(sizeof(db_sqlite_t));

  if (session->data == NULL) {
    goto error_free;
  }

  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  sqlite_session->session     = NULL;

  /* connect to database */
  if (sqlite3_open(filename, &(sqlite_session->session)) != SQLITE_OK) {
    goto error_free;
  }

  /* initialize database scheme */
  if (sqlite3_exec(sqlite_session->session, SQL_INIT, NULL, 0, NULL) != SQLITE_OK) {
    fprintf(stderr, "%s\n", sqlite3_errmsg(sqlite_session->session));
    goto error_free;
  }

  return session;

error_free:

  if (session) {
    if (session->filename != NULL) {
      g_free(session->filename);
    }

    if (session->data != NULL) {
      db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
      sqlite3_close(sqlite_session->session);
      free(sqlite_session);
    }
  }

  free(session);

error_out:

  return NULL;
}

void
db_close(db_session_t* session)
{
  if (session == NULL) {
    return;
  }

  if (session->data != NULL) {
    db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
    sqlite3_close(sqlite_session->session);
    free(sqlite_session);
  }

  g_free(session->filename);

  free(session);
}

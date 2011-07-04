/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <stdbool.h>
#include <sqlite3.h>

#include "database.h"

/**
 * Prepares a sql statement
 *
 * @param session The database session
 * @param statement The sql statement
 * @return Statement object
 */
sqlite3_stmt* db_prepare_statement(db_session_t* session, const char* statement);

typedef struct db_sqlite_s
{
  sqlite3* session; /**> sqlite3 session */
} db_sqlite_t;

static const char SQL_INIT[] =
  /* history table */
  "CREATE TABLE IF NOT EXISTS history ("
    "url TEXT PRIMARY KEY,"
    "title TEXT,"
    "visited INT"
    ");"

  /* bookmarks table */
  "CREATE TABLE IF NOT EXISTS bookmarks ("
    "url TEXT PRIMARY KEY,"
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
      free(session->data);
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

sqlite3_stmt*
db_prepare_statement(db_session_t* session, const char* statement)
{
  if (session == NULL || session->data == NULL || statement == NULL) {
    return false;
  }

  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->session == NULL) {
    return false;
  }

  const char* pz_tail   = NULL;
  sqlite3_stmt* pp_stmt = NULL;

  if (sqlite3_prepare(sqlite_session->session, statement, -1,
        &pp_stmt, &pz_tail) != SQLITE_OK) {
    girara_error("Failed to prepare query: %s", statement);
    goto error_free;
  } else if (pz_tail && *pz_tail != '\0') {
    girara_error("Unused portion of statement: %s", pz_tail);
    goto error_free;
  }

  return pp_stmt;

error_free:

  if (pp_stmt != NULL) {
    sqlite3_finalize(pp_stmt);
  }

  return NULL;
}

void
db_bookmark_add(db_session_t* session, const char* url, const char* title)
{
  if (session == NULL || url == NULL || title == NULL || session->data == NULL) {
    return;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_BOOKMARK_ADD[] =
    "INSERT INTO bookmarks (url, title) VALUES (?, ?);";

  sqlite3_stmt* statement = db_prepare_statement(session, SQL_BOOKMARK_ADD);

  if (statement == NULL) {
    return;
  }

  if (sqlite3_bind_text(statement, 1, url,   -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(statement, 2, title, -1, NULL) != SQLITE_OK
      ) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

void
db_history_add(db_session_t* session, const char* url, const char* title)
{
  if (session == NULL || url == NULL || title == NULL || session->data == NULL) {
    return;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_HISTORY_ADD[] =
    "INSERT INTO history (url, title) VALUES (?, ?);";

  sqlite3_stmt* statement = db_prepare_statement(session, SQL_HISTORY_ADD);

  if (statement == NULL) {
    return;
  }

  if (sqlite3_bind_text(statement, 1, url,   -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(statement, 2, title, -1, NULL) != SQLITE_OK
      ) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

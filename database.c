/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <sqlite3.h>

#include "database.h"

/**
 * Prepares a sql statement
 *
 * @param session The database session
 * @param statement The sql statement
 * @return Statement object
 */
sqlite3_stmt* db_prepare_statement(sqlite3* session, const char* statement);

typedef struct db_sqlite_s
{
  sqlite3* bookmark_session; /**> sqlite3 session */
  sqlite3* history_session; /**> sqlite3 session */
} db_sqlite_t;

db_session_t*
db_new(jumanji_t* jumanji)
{
  if (jumanji == NULL) {
    goto error_out;
  }

  /* initialize database object */
  db_session_t* session = malloc(sizeof(db_session_t));
  if (session == NULL) {
    goto error_out;
  }

  session->bookmark_file = NULL;
  session->history_file  = NULL;
  session->jumanji       = jumanji;

  session->data = malloc(sizeof(db_sqlite_t));

  if (session->data == NULL) {
    goto error_free;
  }

  return session;

error_free:

  free(session->data);
  free(session);

error_out:

  return NULL;
}

bool
db_init(db_session_t* session)
{
  if (session == NULL || session->data == NULL) {
    return false;
  }

  db_sqlite_t* sqlite_session      = (db_sqlite_t*) session->data;
  sqlite_session->bookmark_session = NULL;
  sqlite_session->history_session  = NULL;

  /* connect/create to bookmark database */
  static const char SQL_BOOKMARK_INIT[] =
    /* bookmarks table */
    "CREATE TABLE IF NOT EXISTS bookmarks ("
      "url TEXT PRIMARY KEY,"
      "title TEXT"
      ");";

  if (session->bookmark_file) {
    if (sqlite3_open(session->bookmark_file, &(sqlite_session->bookmark_session)) != SQLITE_OK) {
      goto error_free;
    }

    /* initialize database scheme */
    if (sqlite3_exec(sqlite_session->bookmark_session, SQL_BOOKMARK_INIT, NULL, 0, NULL) != SQLITE_OK) {
      girara_error("Could not initialize database: %s\n", session->bookmark_file);
      goto error_free;
    }
  }

  static const char SQL_HISTORY_INIT[] =
    /* history table */
    "CREATE TABLE IF NOT EXISTS history ("
      "url TEXT PRIMARY KEY,"
      "title TEXT,"
      "visited INT"
      ");";

  if (session->history_file) {
    if (sqlite3_open(session->history_file, &(sqlite_session->history_session)) != SQLITE_OK) {
      goto error_free;
    }

    /* initialize database scheme */
    if (sqlite3_exec(sqlite_session->history_session, SQL_HISTORY_INIT, NULL, 0, NULL) != SQLITE_OK) {
      girara_error("Could not initialize database: %s\n", session->history_file);
      goto error_free;
    }
  }

  return true;

error_free:

  if (sqlite_session->bookmark_session) {
    sqlite3_close(sqlite_session->bookmark_session);
  }

  if (sqlite_session->history_session) {
    sqlite3_close(sqlite_session->history_session);
  }

  return false;
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
  if (session == NULL) {
    return;
  }

  if (session->bookmark_file) {
    g_free(session->bookmark_file);
  }

  if (session->history_file) {
    g_free(session->history_file);
  }

  free(session->data);
  free(session);
}

sqlite3_stmt*
db_prepare_statement(sqlite3* session, const char* statement)
{
  if (session == NULL || statement == NULL) {
    return false;
  }

  const char* pz_tail   = NULL;
  sqlite3_stmt* pp_stmt = NULL;

  if (sqlite3_prepare(session, statement, -1, &pp_stmt, &pz_tail) != SQLITE_OK) {
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
  if (sqlite_session->bookmark_session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_BOOKMARK_ADD[] =
    "INSERT INTO bookmarks (url, title) VALUES (?, ?);";

  sqlite3_stmt* statement =
    db_prepare_statement(sqlite_session->bookmark_session, SQL_BOOKMARK_ADD);

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
  if (sqlite_session->history_session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_HISTORY_ADD[] =
    "INSERT INTO history (url, title) VALUES (?, ?);";

  sqlite3_stmt* statement =
    db_prepare_statement(sqlite_session->history_session, SQL_HISTORY_ADD);

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

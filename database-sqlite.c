/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <stdlib.h>
#include <time.h>

#include "database-sqlite.h"

bool
db_sqlite_init(db_session_t* session)
{
  if (session == NULL) {
    return false;
  }

  db_sqlite_t* sqlite_session = malloc(sizeof(db_sqlite_t));
  if (sqlite_session == NULL) {
    return false;
  }

  sqlite_session->bookmark_session = NULL;
  sqlite_session->history_session  = NULL;

  session->data = sqlite_session;

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
db_sqlite_close(db_session_t* session)
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
db_sqlite_prepare_statement(sqlite3* session, const char* statement)
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
db_sqlite_free_result_link(void* data)
{
  if (data == NULL) {
    return;
  }

  db_result_link_t* link = (db_result_link_t*) data;
  g_free(link->url);
  g_free(link->title);
  free(link);
}

girara_list_t*
db_sqlite_bookmark_find(db_session_t* session, const char* input)
{
  if (session == NULL || input == NULL) {
    return NULL;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->history_session == NULL) {
    return NULL;
  }

  /* prepare statement */
  static const char SQL_HISTORY_FIND[] =
    "SELECT * FROM bookmarks WHERE "
    "url LIKE (SELECT '%' || ? || '%') OR "
    "title LIKE (SELECT '%' || ? || '%');";

  sqlite3_stmt* statement =
    db_sqlite_prepare_statement(sqlite_session->bookmark_session, SQL_HISTORY_FIND);

  if (statement == NULL) {
    return NULL;
  }

  /* bind values */
  if (sqlite3_bind_text(statement, 1, input, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(statement, 2, input, -1, NULL) != SQLITE_OK
      ) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return NULL;
  }

  girara_list_t* results = girara_list_new();

  if (results == NULL) {
    sqlite3_finalize(statement);
    return NULL;
  }

  girara_list_set_free_function(results, db_sqlite_free_result_link);

  while(sqlite3_step(statement) == SQLITE_ROW) {
    db_result_link_t* link = malloc(sizeof(db_result_link_t));
    if (link == NULL) {
      sqlite3_finalize(statement);
      return NULL;
    }

    char* url   = (char*) sqlite3_column_text(statement, 0);
    char* title = (char*) sqlite3_column_text(statement, 1);

    link->url   = g_strdup(url);
    link->title = g_strdup(title);

    girara_list_append(results, link);
  }

  sqlite3_finalize(statement);

  return results;
}

void
db_sqlite_bookmark_remove(db_session_t* session, const char* url)
{
  if (session == NULL || url == NULL) {
    return;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->bookmark_session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_BOOKMARK_ADD[] =
    "DELETE FROM bookmarks WHERE url = ?;";

  sqlite3_stmt* statement =
    db_sqlite_prepare_statement(sqlite_session->bookmark_session, SQL_BOOKMARK_ADD);

  if (statement == NULL) {
    return;
  }

  /* bind values */
  if (sqlite3_bind_text(statement, 1, url, -1, NULL) != SQLITE_OK) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

void
db_sqlite_bookmark_add(db_session_t* session, const char* url, const char* title)
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
    "REPLACE INTO bookmarks (url, title) VALUES (?, ?);";

  sqlite3_stmt* statement =
    db_sqlite_prepare_statement(sqlite_session->bookmark_session, SQL_BOOKMARK_ADD);

  if (statement == NULL) {
    return;
  }

  /* bind values */
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

girara_list_t*
db_sqlite_history_find(db_session_t* session, const char* input)
{
  if (session == NULL || input == NULL) {
    return NULL;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->history_session == NULL) {
    return NULL;
  }

  /* prepare statement */
  static const char SQL_HISTORY_FIND[] =
    "SELECT * FROM history WHERE "
    "url LIKE (SELECT '%' || ? || '%') OR "
    "title LIKE (SELECT '%' || ? || '%');";

  sqlite3_stmt* statement =
    db_sqlite_prepare_statement(sqlite_session->history_session, SQL_HISTORY_FIND);

  if (statement == NULL) {
    return NULL;
  }

  /* bind values */
  if (sqlite3_bind_text(statement, 1, input, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(statement, 2, input, -1, NULL) != SQLITE_OK
      ) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return NULL;
  }

  girara_list_t* results = girara_list_new();

  if (results == NULL) {
    sqlite3_finalize(statement);
    return NULL;
  }

  girara_list_set_free_function(results, db_sqlite_free_result_link);

  while(sqlite3_step(statement) == SQLITE_ROW) {
    db_result_link_t* link = malloc(sizeof(db_result_link_t));
    if (link == NULL) {
      sqlite3_finalize(statement);
      return NULL;
    }

    char* url   = (char*) sqlite3_column_text(statement, 0);
    char* title = (char*) sqlite3_column_text(statement, 0);

    link->url   = g_strdup(url);
    link->title = g_strdup(title);

    girara_list_append(results, link);
  }

  sqlite3_finalize(statement);

  return results;
}

void
db_sqlite_history_add(db_session_t* session, const char* url, const char* title)
{
  if (session == NULL || url == NULL || title == NULL || session->data == NULL) {
    return;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->history_session == NULL) {
    return;
  }

  /* add to database */
  static const char SQL_HISTORY_ADD[] =
    "REPLACE INTO history (url, title, visited) VALUES (?, ?, ?)";

  sqlite3_stmt* statement =
    db_sqlite_prepare_statement(sqlite_session->history_session, SQL_HISTORY_ADD);

  if (statement == NULL) {
    return;
  }

  if (sqlite3_bind_text(statement, 1, url,   -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(statement, 2, title, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_int( statement, 3, time(NULL))      != SQLITE_OK
      ) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

void
db_sqlite_history_clean(db_session_t* session, unsigned int age)
{
  if (session == NULL) {
    return;
  }

  /* get database connection */
  db_sqlite_t* sqlite_session = (db_sqlite_t*) session->data;
  if (sqlite_session->history_session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_HISTORY_CLEAN[] =
    "DELETE FROM history WHERE visited >= ?;";

  sqlite3_stmt* statement =
    db_sqlite_prepare_statement(sqlite_session->history_session, SQL_HISTORY_CLEAN);

  if (statement == NULL) {
    return;
  }

  /* bind values */
  int visited = time(NULL) - age;
  if (sqlite3_bind_int(statement, 1, visited) != SQLITE_OK) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

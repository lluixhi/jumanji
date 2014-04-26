/* See LICENSE file for license and copyright information */

#include <girara/datastructures.h>
#include <girara/utils.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>

#include "database.h"

#define DATABASE "jumanji.sqlite"

struct jumanji_database_s
{
  sqlite3* session;
};

jumanji_database_t*
jumanji_db_init(const char* dir)
{
  if (dir == NULL) {
    return NULL;
  }

  char* path = g_build_filename(dir, DATABASE, NULL);
  if (path == NULL) {
    goto error_ret;
  }

  jumanji_database_t* database = g_malloc0(sizeof(jumanji_database_t));
  if (database == NULL) {
    goto error_free;
  }

  /* connect/create to bookmark database */
  static const char SQL_BOOKMARK_INIT[] =
    /* bookmarks table */
    "CREATE TABLE IF NOT EXISTS bookmarks ("
      "url TEXT PRIMARY KEY,"
      "title TEXT"
      ");";

  static const char SQL_HISTORY_INIT[] =
    /* history table */
    "CREATE TABLE IF NOT EXISTS history ("
      "url TEXT PRIMARY KEY,"
      "title TEXT,"
      "visited INT"
      ");";

  static const char SQL_QUICKMARKS_INIT[] =
    /* quickmarks table */
    "CREATE TABLE IF NOT EXISTS quickmarks ("
      "identifier CHAR PRIMARY KEY,"
      "url TEXT"
      ");";

  if (sqlite3_open(path, &(database->session)) != SQLITE_OK) {
    goto error_free;
  }

  /* initialize database scheme */
  if (sqlite3_exec(database->session, SQL_BOOKMARK_INIT, NULL, 0, NULL) != SQLITE_OK) {
    girara_error("Could not initialize database: %s\n", path);
    goto error_free;
  }

  if (sqlite3_exec(database->session, SQL_HISTORY_INIT, NULL, 0, NULL) != SQLITE_OK) {
    girara_error("Could not initialize database: %s\n", path);
    goto error_free;
  }

  if (sqlite3_exec(database->session, SQL_QUICKMARKS_INIT, NULL, 0, NULL) != SQLITE_OK) {
    girara_error("Could not initialize database: %s\n", path);
    goto error_free;
  }

  return database;

error_free:

  if (database->session != NULL) {
    sqlite3_close(database->session);
  }

  g_free(database);

error_ret:

  g_free(path);

  return NULL;
}

bool
jumanji_db_check_location(const char* dir)
{
  if (dir == NULL) {
    return false;
  }

  /* check database file path */
  char* file = g_build_filename(dir, DATABASE, NULL);
  if (file == NULL) {
    return false;
  }
  if (g_file_test(file, G_FILE_TEST_EXISTS) == true) {
    return true;
  }
  free(file);

  return false;
}

void
jumanji_db_free(jumanji_database_t* database)
{
  if (database == NULL) {
    return;
  }

  if (database->session != NULL) {
    sqlite3_close(database->session);
  }

  g_free(database);
}

sqlite3_stmt*
jumanji_db_prepare_statement(sqlite3* session, const char* statement)
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

girara_list_t*
jumanji_db_bookmark_find(jumanji_database_t* database, const char* input)
{
  if (database == NULL || database->session == NULL || input == NULL) {
    return NULL;
  }

  /* prepare statement */
  static const char SQL_HISTORY_FIND[] =
    "SELECT * FROM bookmarks WHERE "
    "url LIKE (SELECT '%' || ? || '%') OR "
    "title LIKE (SELECT '%' || ? || '%');";

  sqlite3_stmt* statement =
    jumanji_db_prepare_statement(database->session, SQL_HISTORY_FIND);

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

  girara_list_set_free_function(results, jumanji_db_free_result_link);

  while(sqlite3_step(statement) == SQLITE_ROW) {
    jumanji_db_result_link_t* link = malloc(sizeof(jumanji_db_result_link_t));
    if (link == NULL) {
      sqlite3_finalize(statement);
      return NULL;
    }

    char* url   = (char*) sqlite3_column_text(statement, 0);
    char* title = (char*) sqlite3_column_text(statement, 1);

    link->url     = g_strdup(url);
    link->title   = g_strdup(title);
    link->visited = 0;

    girara_list_append(results, link);
  }

  sqlite3_finalize(statement);

  return results;
}

void
jumanji_db_bookmark_remove(jumanji_database_t* database, const char* url)
{
  if (database == NULL || database->session == NULL || url == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_BOOKMARK_ADD[] =
    "DELETE FROM bookmarks WHERE url = ?;";

  sqlite3_stmt* statement =
    jumanji_db_prepare_statement(database->session, SQL_BOOKMARK_ADD);

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
jumanji_db_bookmark_add(jumanji_database_t* database, const char* url, const char* title)
{
  if (database == NULL || database->session == NULL || url == NULL || title ==
      NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_BOOKMARK_ADD[] =
    "REPLACE INTO bookmarks (url, title) VALUES (?, ?);";

  sqlite3_stmt* statement = jumanji_db_prepare_statement(database->session,
      SQL_BOOKMARK_ADD);

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
jumanji_db_history_find(jumanji_database_t* database, const char* input)
{
  if (database == NULL || database->session == NULL || input == NULL) {
    return NULL;
  }

  /* prepare statement */
  static const char SQL_HISTORY_FIND[] =
    "SELECT * FROM history WHERE "
    "url LIKE (SELECT '%' || ? || '%') OR "
    "title LIKE (SELECT '%' || ? || '%');";

  sqlite3_stmt* statement = jumanji_db_prepare_statement(database->session,
      SQL_HISTORY_FIND);

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

  girara_list_set_free_function(results, jumanji_db_free_result_link);

  while(sqlite3_step(statement) == SQLITE_ROW) {
    jumanji_db_result_link_t* link = malloc(sizeof(jumanji_db_result_link_t));
    if (link == NULL) {
      sqlite3_finalize(statement);
      return NULL;
    }

    char* url   = (char*) sqlite3_column_text(statement, 0);
    char* title = (char*) sqlite3_column_text(statement, 1);

    link->url     = g_strdup(url);
    link->title   = g_strdup(title);
    link->visited = sqlite3_column_int(statement, 2);

    girara_list_append(results, link);
  }

  sqlite3_finalize(statement);

  return results;
}

void
jumanji_db_history_add(jumanji_database_t* database, const char* url, const char* title)
{
  if (database == NULL || database->session == NULL || url == NULL || title == NULL) {
    return;
  }

  /* add to database */
  static const char SQL_HISTORY_ADD[] =
    "REPLACE INTO history (url, title, visited) VALUES (?, ?, ?)";

  sqlite3_stmt* statement =
    jumanji_db_prepare_statement(database->session, SQL_HISTORY_ADD);

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
jumanji_db_history_clean(jumanji_database_t* database, unsigned int age)
{
  if (database == NULL || database->session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_HISTORY_CLEAN[] =
    "DELETE FROM history WHERE visited >= ?;";

  sqlite3_stmt* statement =
    jumanji_db_prepare_statement(database->session, SQL_HISTORY_CLEAN);

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

void
jumanji_db_quickmark_add(jumanji_database_t* database, const char identifier, const char* url)
{
  if (database == NULL || database->session == NULL || url == NULL) {
    return;
  }

  /* add to database */
  static const char SQL_QUICKMARK_ADD[] =
    "REPLACE INTO quickmarks (identifier, url) VALUES (?, ?)";

  sqlite3_stmt* statement = jumanji_db_prepare_statement(database->session,
      SQL_QUICKMARK_ADD);

  if (statement == NULL) {
    return;
  }

  if (sqlite3_bind_blob(statement, 1, &identifier, 1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(statement, 2, url,        -1, NULL) != SQLITE_OK
      ) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

char*
jumanji_db_quickmark_find(jumanji_database_t* database, const char identifier)
{
  if (database == NULL || database->session == NULL) {
    return NULL;
  }

  /* prepare statement */
  static const char SQL_QUICKMARKS_FIND[] =
    "SELECT url FROM quickmarks WHERE identifier = ?;";

  sqlite3_stmt* statement =
    jumanji_db_prepare_statement(database->session, SQL_QUICKMARKS_FIND);

  if (statement == NULL) {
    return NULL;
  }

  /* bind values */
  if (sqlite3_bind_blob(statement, 1, &identifier, 1, NULL) != SQLITE_OK) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return NULL;
  }

  char* url = NULL;
  while(sqlite3_step(statement) == SQLITE_ROW) {
    url = (char*) sqlite3_column_text(statement, 0);
    break;
  }

  sqlite3_finalize(statement);

  return url;
}

void
jumanji_db_quickmark_remove(jumanji_database_t* database, const char identifier)
{
  if (database == NULL || database->session == NULL) {
    return;
  }

  /* prepare statement */
  static const char SQL_QUICKMARK_ADD[] =
    "DELETE FROM quickmarks WHERE identifier = ?;";

  sqlite3_stmt* statement = jumanji_db_prepare_statement(database->session,
      SQL_QUICKMARK_ADD);

  if (statement == NULL) {
    return;
  }

  /* bind values */
  if (sqlite3_bind_blob(statement, 1, &identifier, 1, NULL) != SQLITE_OK) {
    girara_error("Could not bind query parameters");
    sqlite3_finalize(statement);
    return;
  }

  sqlite3_step(statement);
  sqlite3_finalize(statement);
}

void
jumanji_db_save_session(jumanji_database_t* database, const char* name, girara_list_t* urls)
{
  return;
}

girara_list_t*
jumanji_db_load_session(jumanji_database_t* database, const char* name)
{
  return NULL;
}

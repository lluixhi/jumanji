/* See LICENSE file for license and copyright information */

#ifndef DATABASE_SQLITE_H
#define DATABASE_SQLITE_H

#include <stdbool.h>
#include <sqlite3.h>

#include "database.h"
#include "jumanji.h"

typedef struct db_sqlite_s
{
  sqlite3* bookmark_session; /**> sqlite3 session */
  sqlite3* history_session; /**> sqlite3 session */
  sqlite3* quickmarks_session; /**> sqlite3 session */
} db_sqlite_t;

/**
 * Initializes the database
 *
 * @param session The database session
 * @return true on success otherwise false
 */
bool db_sqlite_init(db_session_t* session);

/**
 * Closes a database connection
 *
 * @param session The database session
 */
void db_sqlite_close(db_session_t* session);

/**
 * Save a new bookmark in the database
 *
 * @param session The databases session
 * @param url The url of the bookmark
 * @param title The title of the bookmark
 */
void db_sqlite_bookmark_add(db_session_t* session, const char* url, const char* title);

/**
 * Find bookmarks
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* db_sqlite_bookmark_find(db_session_t* session, const char* input);

/**
 * Removes a saved bookmark
 *
 * @param session The database session
 * @param url The url that should be removed
 */
void db_sqlite_bookmark_remove(db_session_t* session, const char* url);

/**
 * Save a new history item in the database
 *
 * @param session The database session
 * @param url The url of the history item
 * @param title The title of the history item
 */
void db_sqlite_history_add(db_session_t* session, const char* url, const char* title);

/**
 * Find history
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* db_sqlite_history_find(db_session_t* session, const char* input);

/**
 * Cleans the history
 *
 * @param session The database session
 * @param age The age of the entries in seconds
 */
void db_sqlite_history_clean(db_session_t* session, unsigned int age);

/**
 * Saves a new quickmark (or overwrites an existing one)
 *
 * @param session The database session
 * @param identifier The quickmark identifier
 * @param url Url the quickmark points to
 */
void db_sqlite_quickmark_add(db_session_t* session, const char identifier, const char* url);

/**
 * Finds a quickmark
 *
 * @param session The database session
 * @param identifier The quickmark identifier
 * @return The url of the quickmark otherwise NULL
 */
char* db_sqlite_quickmark_find(db_session_t* session, const char identifier);

/**
 * Remove a quickmark
 *
 * @param session The database session
 * @param identifier The quickmark identifier
 */
void db_sqlite_quickmark_remove(db_session_t* session, const char identifier);

/**
 * Prepares a sql statement
 *
 * @param session The database session
 * @param statement The sql statement
 * @return Statement object
 */
sqlite3_stmt* db_sqlite_prepare_statement(sqlite3* session, const char* statement);

#endif // DATABASE_SQLITE_H

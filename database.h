/* See LICENSE file for license and copyright information */

#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

#include "jumanji.h"

typedef struct db_session_s
{
  jumanji_t* jumanji; /**> Jumanji session */

  char* bookmark_file; /**> Database file of the bookmarks */
  char* history_file; /**> Database file of the history */

  void* data; /**> Implementation based data */
} db_session_t;

typedef struct db_result_link_s
{
  char* url; /**> The url of the link */
  char* title; /**> The link title */
  int visited; /**> Last time the link has been visited */
} db_result_link_t;

/**
 * Creates a new database object
 *
 * @param session The jumanji session
 * @return Session object or NULL if an error occured
 */
db_session_t* db_new(jumanji_t* session);

/**
 * Initializes the database
 *
 * @param session The database session
 * @return true on success otherwise false
 */
bool db_init(db_session_t* session);

/**
 * Sets the path to the bookmark database file
 *
 * @param session The database session
 * @param bookmark_file Path to the bookmark file
 */
void db_set_bookmark_file(db_session_t* session, const char* bookmark_file);

/**
 * Sets the path to the history database file
 *
 * @param session The database session
 * @param history_file Path to the history file
 */
void db_set_history_file(db_session_t* session, const char* history_file);

/**
 * Closes a database connection
 *
 * @param session The database session
 */
void db_close(db_session_t* session);

/**
 * Save a new bookmark in the database
 *
 * @param session The databases session
 * @param url The url of the bookmark
 * @param title The title of the bookmark
 */
void db_bookmark_add(db_session_t* session, const char* url, const char* title);

/**
 * Find bookmarks
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* db_bookmark_find(db_session_t* session, const char* input);

/**
 * Removes a saved bookmark
 *
 * @param session The database session
 * @param url The url that should be removed
 */
void db_bookmark_remove(db_session_t* session, const char* url);

/**
 * Save a new history item in the database
 *
 * @param session The database session
 * @param url The url of the history item
 * @param title The title of the history item
 */
void db_history_add(db_session_t* session, const char* url, const char* title);

/**
 * Find history
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* db_history_find(db_session_t* session, const char* input);

/**
 * Cleans the history
 *
 * @param session The database session
 * @param age The age of the entries in seconds
 */
void db_history_clean(db_session_t* session, unsigned int age);

/**
 * Frees a result link
 *
 * @param data Link data
 */
void db_free_result_link(void* data);

#endif // DATABASE_H

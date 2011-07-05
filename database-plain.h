/* See LICENSE file for license and copyright information */

#ifndef DATABASE_PLAIN_H
#define DATABASE_PLAIN_H

#include <stdbool.h>

#include "database.h"
#include "jumanji.h"

/**
 * Creates a new database object
 *
 * @param session The jumanji session
 * @return Session object or NULL if an error occured
 */
db_session_t* db_plain_new(jumanji_t* session);

/**
 * Initializes the database
 *
 * @param session The database session
 * @return true on success otherwise false
 */
bool db_plain_init(db_session_t* session);

/**
 * Sets the path to the bookmark database file
 *
 * @param session The database session
 * @param bookmark_file Path to the bookmark file
 */
void db_plain_set_bookmark_file(db_session_t* session, const char* bookmark_file);

/**
 * Sets the path to the history database file
 *
 * @param session The database session
 * @param history_file Path to the history file
 */
void db_plain_set_history_file(db_session_t* session, const char* history_file);

/**
 * Closes a database connection
 *
 * @param session The database session
 */
void db_plain_close(db_session_t* session);

/**
 * Save a new bookmark in the database
 *
 * @param session The databases session
 * @param url The url of the bookmark
 * @param title The title of the bookmark
 */
void db_plain_bookmark_add(db_session_t* session, const char* url, const char* title);

/**
 * Find bookmarks
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* db_plain_bookmark_find(db_session_t* session, const char* input);

/**
 * Removes a saved bookmark
 *
 * @param session The database session
 * @param url The url that should be removed
 */
void db_plain_bookmark_remove(db_session_t* session, const char* url);

/**
 * Save a new history item in the database
 *
 * @param session The database session
 * @param url The url of the history item
 * @param title The title of the history item
 */
void db_plain_history_add(db_session_t* session, const char* url, const char* title);

/**
 * Find history
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* db_plain_history_find(db_session_t* session, const char* input);

/**
 * Cleans the history
 *
 * @param session The database session
 * @param age The age of the entries in seconds
 */
void db_plain_history_clean(db_session_t* session, unsigned int age);

#endif // DATABASE_PLAIN_H

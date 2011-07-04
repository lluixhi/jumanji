/* See LICENSE file for license and copyright information */

#ifndef DATABASE_H
#define DATABASE_H

#include "jumanji.h"

typedef struct db_session_s
{
  jumanji_t* session; /**> Jumanji session */
  char* filename; /**> Database file */
  void* data; /**> Implementation based data */
} db_session_t;

/**
 * Initializes a database connection
 *
 * @param session The jumanji session
 * @param filename The database filetype
 * @return Session object or NULL if an error occured
 */
db_session_t* db_open(jumanji_t* session, const char* filename);

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
 * Save a new history item in the database
 *
 * @param session The database session
 * @param url The url of the history item
 * @param title The title of the history item
 */
void db_history_add(db_session_t* session, const char* url, const char* title);

#endif // DATABASE_H

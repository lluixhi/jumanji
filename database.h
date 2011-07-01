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

#endif // DATABASE_H

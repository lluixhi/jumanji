/* See LICENSE file for license and copyright information */

#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

#include "jumanji.h"

typedef struct jumanji_db_result_link_s
{
  char* url; /**> The url of the link */
  char* title; /**> The link title */
  int visited; /**> Last time the link has been visited */
} jumanji_db_result_link_t;

/**
 * Creates a new database object
 *
 * @param session The jumanji session
 * @return Session object or NULL if an error occured
 */
jumanji_database_t* jumanji_db_init(const char* dir);

/**
 * Closes a database connection
 *
 * @param session The database session
 */
void jumanji_db_free(jumanji_database_t* database);

/**
 * Save a new bookmark in the database
 *
 * @param session The databases session
 * @param url The url of the bookmark
 * @param title The title of the bookmark
 */
void jumanji_db_bookmark_add(jumanji_database_t* database, const char* url, const char* title);

/**
 * Find bookmarks
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* jumanji_db_bookmark_find(jumanji_database_t* database, const char* input);

/**
 * Removes a saved bookmark
 *
 * @param session The database session
 * @param url The url that should be removed
 */
void jumanji_db_bookmark_remove(jumanji_database_t* database, const char* url);

/**
 * Save a new history item in the database
 *
 * @param session The database session
 * @param url The url of the history item
 * @param title The title of the history item
 */
void jumanji_db_history_add(jumanji_database_t* database, const char* url, const char* title);

/**
 * Find history
 *
 * @param session The databases session
 * @param input The data that the bookmark should match
 * @return list or NULL if an error occured
 */
girara_list_t* jumanji_db_history_find(jumanji_database_t* database, const char* input);

/**
 * Cleans the history
 *
 * @param session The database session
 * @param age The age of the entries in seconds
 */
void jumanji_db_history_clean(jumanji_database_t* database, unsigned int age);

/**
 * Saves a new quickmark (or overwrites an existing one)
 *
 * @param session The database session
 * @param identifier The quickmark identifier
 * @param url Url the quickmark points to
 */
void jumanji_db_quickmark_add(jumanji_database_t* database, const char identifier, const char* url);

/**
 * Finds a quickmark
 *
 * @param session The database session
 * @param identifier The quickmark identifier
 * @return The url of the quickmark otherwise NULL
 */
char* jumanji_db_quickmark_find(jumanji_database_t* database, const char identifier);

/**
 * Remove a quickmark
 *
 * @param session The database session
 * @param identifier The quickmark identifier
 */
void jumanji_db_quickmark_remove(jumanji_database_t* database, const char identifier);

/**
 * Saves a cookie
 *
 * @param session Database session
 * @param name Name
 * @param value Value
 * @param domain Domain
 * @param path Path
 * @param expires Expire date
 * @param secure Secure
 * @param http_only Http only
 */
void jumanji_db_cookie_add(jumanji_database_t* database, const char* name, const char*
    value, const char* domain, const char* path, time_t expires, bool secure,
    bool http_only);

/**
 * Remove a cookie
 *
 * @param session Database session
 * @param domain domain
 * @param name Name
 */
void jumanji_db_cookie_remove(jumanji_database_t* database, const char* domain, const char*
    name);

/**
 * Returns a list of all SoupCookie's
 *
 * @param session The database session
 * @return List of cookies or NULL if an error occured
 */
girara_list_t* jumanji_db_cookie_list(jumanji_database_t* database);

/**
 * Frees a result link
 *
 * @param data Link data
 */
void jumanji_db_free_result_link(void* data);

#endif // DATABASE_H

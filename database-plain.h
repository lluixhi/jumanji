/* See LICENSE file for license and copyright information */

#ifndef DATABASE_PLAIN_H
#define DATABASE_PLAIN_H

#include <stdbool.h>

#include "database.h"
#include "jumanji.h"

typedef struct db_plain_s
{
  gchar* bookmark_file_path; /**> File path to the bookmark file */
  girara_list_t* bookmarks; /**> Temporary bookmarks */
  GFileMonitor* bookmark_monitor; /**> File monitor for the bookmark file */
  unsigned int bookmark_signal; /**> Signal id */

  gchar* history_file_path; /**> File path to the history file */
  girara_list_t* history; /**>  Temporary history */
  GFileMonitor* history_monitor; /**> File monitor for the history file */
  unsigned int history_signal; /**> Signal id */
} db_plain_t;

/**
 * Initializes the database
 *
 * @param session The database session
 * @return true on success otherwise false
 */
bool db_plain_init(db_session_t* session);

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

/**
 * Read all bookmarks from file
 *
 * @param filename The filename
 * @return Read bookmarks or NULL if an error occured
 */
girara_list_t* db_plain_read_urls_from_file(const char* filename);

/**
 * Write urls to file
 *
 * @param filename The filename
 * @param urls The list of urls
 * @param visited true if the last visited value should be written as well
 */
void db_plain_write_urls_to_file(const char* filename, girara_list_t* urls, bool visited);

/**
 * This function filters the given list for matching data and returns a new list
 * containing that data and clears the old list.
 *
 * @param list The list that should be filtered
 * @param input The input data
 * @return A new list or NULL if an error occured
 */
girara_list_t* db_plain_filter_url_list(girara_list_t* list, const char* input);

/**
 * Callback that gets executed when one file is changed
 *
 * @param monitor Monitor
 * @param file The watched file
 * @param other_file Other file or NULL
 * @param event The occured event
 * @param data User data
 */
void cb_db_plain_watch_file(GFileMonitor* monitor, GFile* file, GFile*
    other_file, GFileMonitorEvent event, gpointer data);

#endif // DATABASE_PLAIN_H

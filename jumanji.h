/* See LICENSE file for license and copyright information */

#ifndef JUMANJI_H
#define JUMANJI_H

#include <stdbool.h>
#include <girara/types.h>
#include <gtk/gtk.h>

#include <webkit/webkit.h>

enum { LEFT, RIGHT, UP, DOWN, FULL_UP, FULL_DOWN, HALF_UP, HALF_DOWN, TOP,
  BOTTOM, BEGIN, END, ZOOM_IN, ZOOM_OUT, DEFAULT, ZOOM_SPECIFIC, APPEND_URL,
  BYPASS_CACHE, NEW_TAB, NEXT, PREVIOUS, BACKWARDS, FORWARDS };

typedef struct jumanji_proxy_s
{
  char* url; /**> Url */
  char* description; /**> Description (optional) */
} jumanji_proxy_t;

typedef struct jumanji_database_s jumanji_database_t;

typedef struct jumanji_s
{
  struct
  {
    girara_session_t* session; /**> girara interface session */

    struct
    {
      girara_statusbar_item_t* url; /**> url statusbar entry */
      girara_statusbar_item_t* buffer; /**> buffer statusbar entry */
      girara_statusbar_item_t* tabs; /**> tabs statusbar entry */
      girara_statusbar_item_t* proxy; /**> proxy statusbar entry */
    } statusbar;
  } ui;

  struct
  {
    gchar* config_dir; /**> Path to the configuration directory */
    gchar* data_dir; /**> Path to the data directory */
    gchar* session_dir; /**> Path to the sessions directory */
  } config;

  struct
  {
    girara_mode_t normal; /**> Normal mode */
  } modes;

  struct
  {
    WebKitWebSettings* browser_settings; /*>> Browser settings */
    gchar* user_stylesheet_uri;
    girara_list_t* search_engines; /**> Search engines */
    girara_list_t* proxies; /**> Proxies */
    girara_list_t* marks; /**> Marker */
    girara_list_t* last_closed; /**> Last closed tabs */
    jumanji_proxy_t* current_proxy; /**> Current proxy */
    girara_list_t* user_scripts; /**> User scripts */
    girara_list_t* adblock_filters; /**> Adblock filters */
    girara_list_t* sessions; /**> Sessions */
    char** arguments; /**> Arguments that were passed at startup */
    int quickmark_open_mode; /**> How to open a quickmark */
    void* soup; /**> Soup session */
  } global;


  struct
  {
    char* item; /**> Search item */
  } search;

  struct
  {
    girara_list_t* list; /**> List of downloads */
    GtkWidget* widget; /**> Download widget */
  } downloads;

  struct
  {
    GPtrArray *links; /**> List of links */
    GPtrArray *hints; /**> List of hint dom elements */
    int        open_mode; /**> Open mode */
    WebKitDOMNode *hint_style; /**> Dom style node */
    WebKitDOMNode *hint_box; /**> Dom element node */
    GString* input; /**> Input buffer */
  } hints;

  jumanji_database_t* database; /**> The database */
} jumanji_t;

typedef struct jumanji_tab_s
{
  GtkWidget* scrolled_window; /**> Scrolled window */
  GtkWidget* web_view; /**> Webkit webview */
  girara_tab_t* girara_tab; /** The girara tab */
  jumanji_t* jumanji; /**> The jumanji session */
} jumanji_tab_t;

typedef struct jumanji_search_engine_s
{
  char* identifier; /**> Identifier */
  char* url; /**> Url */
} jumanji_search_engine_t;

/**
 * Initializes jumanji
 *
 * @param argc Number of arguments
 * @param argv Values of arguments
 * @return jumanji session object or NULL if jumanji could not been initialized
 */
jumanji_t* jumanji_init(int argc, char* argv[]);

/**
 * Free jumanji session
 *
 * @param jumanji The jumanji session
 */
void jumanji_free(jumanji_t* jumanji);

/**
 * Creates a new tab
 *
 * @param jumanji The jumanji session
 * @param url URL of the site that should be loaded
 * @param focus true if the tab should be focused after creation
 * @return The webkit widget or NULL if an error occured
 */
jumanji_tab_t* jumanji_tab_new(jumanji_t* jumanji, const char* url, bool focus);

/**
 * Frees and destroys a tab
 *
 * @param tab The tab
 */
void jumanji_tab_free(jumanji_tab_t* tab);

/**
 * Returns the current tab
 *
 * @param jumanji The jumanji session
 * @return The tab or NULL if an error occured
 */
jumanji_tab_t* jumanji_tab_get_current(jumanji_t* jumanji);

/**
 * Returns the tab on the given index
 *
 * @param jumanji The jumanji session
 * @param index The index
 * @return The tab or NULL if an error occured
 */
jumanji_tab_t* jumanji_tab_get_nth(jumanji_t* jumanji, unsigned int index);

/**
 * Loads a new url in the given tab
 *
 * @param tab The jumanji tab
 * @param url The url that should be loaded
 */
void jumanji_tab_load_url(jumanji_tab_t* tab, const char* url);

/**
 * Show search results based on the latest search item in the tab
 *
 * @param tab The tab
 */
void jumanji_tab_show_search_results(jumanji_tab_t* tab);

/**
 * Builds an url based upon a string
 *
 * @param jumanji The jumanji session
 * @param string The string
 * @return A url that can be passed to jumanji_tab_load_url or NULL if an error
 * occured
 */
char* jumanji_build_url_from_string(jumanji_t* jumanji, const char* string);

/**
 * Builds an url based upon the input
 *
 * @param jumanji The jumanji session
 * @param list The input list
 * @return A uri that can be passed to jumanji_tab_load_url or NULL if an error
 * occured
 */
char* jumanji_build_url(jumanji_t* jumanji, girara_list_t* list);

/**
 * Builds a search engine url based on the search url and the given arguments
 *
 * @param search_url Search url
 * @param list Search items
 * @param all_arguments true if all arguments of the list should be appended
 * otherwise the first one will be skipped
 * @return String that has to be freed or NULL if an error occured
 */
char* jumanji_build_search_engine_url(const char* search_url, girara_list_t* list, bool all_arguments);

/**
 * Creates a new jumanji instance
 *
 * @param jumanji The jumanji session
 * @param uri The uri that should be opened in the new window
 */
void jumanji_window_new(jumanji_t* jumanji, char* uri);

/**
 * Frees last closed urls
 *
 * @param data Last closed url
 */
void jumanji_last_closed_free(void* data);

/**
 * Free a search engine
 *
 * @param data Search engine
 */
void jumanji_search_engine_free(void* data);

/**
 * Free a proxy
 *
 * @param data Proxy
 */
void jumanji_proxy_free(void* data);

#endif // JUMANJI_H

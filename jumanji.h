/* See LICENSE file for license and copyright information */

#ifndef JUMANJI_H
#define JUMANJI_H

#include <stdbool.h>
#include <girara.h>
#include <gtk/gtk.h>
#include <libsoup/soup.h>

#include <webkit/webkit.h>

enum { LEFT, RIGHT, UP, DOWN, FULL_UP, FULL_DOWN, HALF_UP, HALF_DOWN, TOP,
  BOTTOM, BEGIN, END, ZOOM_IN, ZOOM_OUT, DEFAULT, ZOOM_SPECIFIC, APPEND_URL,
  BYPASS_CACHE, NEW_TAB, NEXT, PREVIOUS };

typedef struct jumanji_proxy_s
{
  char* url; /**> Url */
  char* description; /**> Description (optional) */
} jumanji_proxy_t;

typedef struct jumanji_s
{
  struct
  {
    girara_session_t* session; /**> girara interface session */

    struct
    {
      girara_statusbar_item_t* buffer; /**> buffer statusbar entry */
      girara_statusbar_item_t* url; /**> url statusbar entry */
      girara_statusbar_item_t* proxy; /**> proxy statusbar entry */
    } statusbar;
  } ui;

  struct
  {
    gchar* config_dir; /**> Path to the configuration directory */
    gchar* data_dir; /**> Path to the data directory */
  } config;

  struct
  {
    girara_mode_t normal; /**> Normal mode */
  } modes;

  struct
  {
    WebKitWebSettings* browser_settings; /*>> Browser settings */
    SoupSession* soup_session; /*>> Soup session */
    girara_list_t* search_engines; /**> Search engines */
    girara_list_t* proxies; /**> Proxies */
    jumanji_proxy_t* current_proxy; /**> Current proxy */
  } global;
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
 * @param background true if the tab should not be focused after creation
 * @return The webkit widget or NULL if an error occured
 */
jumanji_tab_t* jumanji_tab_new(jumanji_t* jumanji, const char* url, bool background);

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
 * Activates a jumanji proxy
 *
 * @param jumanji The jumanji session
 * @param proxy The jumanji proxy
 */
void jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy);

#endif // JUMANJI_H

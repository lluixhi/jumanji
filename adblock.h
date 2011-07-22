/* See LICENSE file for license and copyright information */

#ifndef ADBLOCK_H
#define ADBLOCK_H

#include <girara.h>

#include "jumanji.h"

#define ADBLOCK_FILTER_LIST_DIR "adblock"

typedef struct adblock_filter_list_s
{
  char* name; /**> Name of the filter list*/
  girara_list_t* pattern; /**> List of included url patterns */
} adblock_filter_t;

/**
 * Loads a files from a directory as filter listsand returns a list
 * of correctly parsed lists
 *
 * @param path Path to the directory
 * @return List of parsed filters or NULL if an error occured
 */
girara_list_t* adblock_filter_load_dir(const char* path);

/**
 * Loads a single file as a filter list
 *
 * @param path Path to the file
 * @return User script object or NULL if an error occured
 */
adblock_filter_t* adblock_filter_load(const char* path);

/**
 * Frees an user script entry in the user script list
 *
 * @param data User script
 */
void adblock_filter_free(void* data);

/**
 * Setup adblock filter for tab
 *
 * @param tab Jumanji tab
 * @param adblock_filters Filter list
 */
void adblock_filter_init_tab(jumanji_tab_t* tab, girara_list_t*
    adblock_filters);

/**
 * Check if external resource should be blocked by filter list
 *
 * @param web_view The web view
 * @param web_frame The frame
 * @param web_resource The resource
 * @param request Network request
 * @param response Network response
 * @param adblock_filters Filter list
 */
void
cb_adblock_filter_resource_request_starting(WebKitWebView* web_view,
    WebKitWebFrame* web_frame, WebKitWebResource* web_resource,
    WebKitNetworkRequest* request, WebKitNetworkResponse* response,
    girara_list_t* adblock_filters);

#endif // ADBLOCK_H

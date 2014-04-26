/* See LICENSE file for license and copyright information */

#ifndef USERSCRIPTS_H
#define USERSCRIPTS_H

#include <girara/types.h>

#include "jumanji.h"

#define USER_SCRIPTS_DIR "scripts"

typedef struct user_script_s
{
  char* name; /**> Name of the user script */
  char* description; /**> Description of the user script */
  char* content; /**> User script code */
  girara_list_t* include; /**> List of included url patterns */
  girara_list_t* exclude; /**> List of excluded url patterns */
  bool load_on_document_start; /**> Load on document start */
} user_script_t;

/**
 * Loads all files from a directory as user scripts and returns a list
 * of correctly parsed scripts
 *
 * @param path Path to the directory
 * @return List of parsed scripts or NULL if an error occured
 */
girara_list_t* user_script_load_dir(const char* path);

/**
 * Loads a single file as a userscript
 *
 * @param path Path to the file
 * @return User script object or NULL if an error occured
 */
user_script_t* user_script_load_file(const char* path);

/**
 * Frees an user script entry in the user script list
 *
 * @param data User script
 */
void user_script_free(void* data);

/**
 * Load user script on webkit view
 *
 * @param web_view Webkit view
 * @param user_script The user script
 */
void user_script_inject(WebKitWebView* web_view, user_script_t* user_script);

/**
 * Load user script by content
 *
 * @param web_view Webkit view
 * @param text The javascript code
 */
void user_script_inject_text(WebKitWebView* web_view, const char* text);

/**
 * Sets up a webkit tab to use the user script implementation
 *
 * @param tab The jumanji tab
 * @param user_scripts The list of user scripts
 */
void user_script_init_tab(jumanji_tab_t* tab, girara_list_t* user_scripts);

/**
 * Callback that is used to load user scripts at the correct time
 *
 * @param web_view Webkit view
 * @param pspec -
 * @param user_scripts The list of user scripts
 */
void cb_user_script_tab_load_status(WebKitWebView* web_view, GParamSpec* pspec,
    girara_list_t* user_scripts);

#endif // USERSCRIPTS_H

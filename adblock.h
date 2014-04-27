/* See LICENSE file for license and copyright information */

#ifndef ADBLOCK_H
#define ADBLOCK_H

#include <girara/types.h>

#include "jumanji.h"

#define ADBLOCK_FILTER_LIST_DIR "adblock"

enum {
  ADBLOCK_NONE      = 0,
  ADBLOCK_BEGINNING = 1 << 1,
  ADBLOCK_ENDING    = 1 << 2,
  ADBLOCK_DOMAIN    = 1 << 3,
} adblock_position_t;

typedef struct adblock_rule_s
{
  char* pattern; /**> Pattern to match */
  char* css_rule; /**> CSS rule */
  int options; /**> Filter options */
  int position; /**> Position */
} adblock_rule_t;

typedef struct adblock_filter_list_s
{
  char* name; /**> Name of the filter list*/
  girara_list_t* pattern; /**> List of included url patterns */
  girara_list_t* exceptions; /**> List of exceptions */
  girara_list_t* css_rules; /**> List of css filters */
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
 * Frees an adblock filter and its entire rule lists
 *
 * @param data Adblock filter
 */
void adblock_filter_free(void* data);

/**
 * Frees an adblock rule
 *
 * @param data adblock rule
 */
void adblock_rule_free(void* data);

/**
 * Setup adblock filter for tab
 *
 * @param tab Jumanji tab
 * @param adblock_filters Filter list
 */
void adblock_filter_init_tab(jumanji_tab_t* tab, girara_list_t*
    adblock_filters);

/**
 * Evaluate filter rule
 *
 * @param filter Adblock filter
 * @param line Rule to evaluate
 */
void adblock_rule_parse(adblock_filter_t* filter, const char* line);

/**
 * Evaluates a single rule on an uri
 *
 * @param rule The rule
 * @param uri The uri to check
 * @return true if rule matches otherwise false
 */
bool adblock_rule_evaluate(adblock_rule_t* rule, const char* uri);

#endif // ADBLOCK_H

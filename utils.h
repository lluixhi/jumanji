/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include <girara/types.h>

/**
 * This function builds a girara list out of a string
 * by splitting it at the given seperator
 *
 * @param string The input
 * @return The list or NULL if an error occured 
 */
girara_list_t* build_girara_list(const char* string);

/**
 * This function creates a new string that url encode special chars.
 * It returns a new string that is dynamically allocated and needs
 * to be manually freed.
 */
char* url_encode(const char* string);

#endif // UTILS_H

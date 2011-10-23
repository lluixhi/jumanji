/* See LICENSE file for license and copyright information */

#ifndef UTILS_H
#define UTILS_H

#include <girara/girara.h>

/**
 * This function builds a girara list out of a string
 * by splitting it at the given seperator
 *
 * @param string The input
 * @return The list or NULL if an error occured 
 */
girara_list_t* build_girara_list(const char* string);

#endif // UTILS_H

/* See LICENSE file for license and copyright information */

#ifndef CONFIG_H
#define CONFIG_H

#include "jumanji.h"

/**
 * This function loads the default values of the configuration
 *
 * @param jumanji the jumanji session
 */
void config_load_default(jumanji_t* jumanji);

/**
 * Loads and evaluates a configuration file
 *
 * @param path Path to the configuration file
 */
void config_load_file(jumanji_t* jumanji, char* path);

#endif // CONFIG_H

/* See LICENSE file for license and copyright information */

#ifndef COOKIES_H
#define COOKIES_H

#include "jumanji.h"

#define JUMANJI_COOKIE_FILE "cookies"

typedef struct jumanji_soup_s jumanji_soup_t;


/**
 * Initializes the soup session for cookie support
 *
 * @param jumanji The jumanji session
 * @return Soup session object or NULL
 */
jumanji_soup_t* jumanji_soup_init(jumanji_t* jumanji);

/**
 * Frees a soup session
 *
 * @param soup
 */
void jumanji_soup_free(jumanji_soup_t* soup);

/**
 * Activates a jumanji proxy
 *
 * @param jumanji The jumanji session
 * @param proxy The jumanji proxy
 */
void jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy);

#endif // COOKIES_H

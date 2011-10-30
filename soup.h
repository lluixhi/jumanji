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
 * Emitted when the jar changes.
 *
 * @param jar Cookie jar
 * @param old_cookie Old cookie (if cookie will be deleted)
 * @param new_cookie New cookie (should be created)
 * @param jumanji The jumanji session
 */
void cb_jumanji_soup_jar_changed(SoupCookieJar* jar, SoupCookie* old_cookie,
    SoupCookie* new_cookie, jumanji_t* jumanji);

/**
 * Emitted just before a request is sent
 *
 * @param soup_session The soup session
 * @param message The message
 * @param socket The socket the request is being sent on
 * @param jumanji The jumanji session
 */
void cb_jumanji_soup_session_request_started(SoupSession* soup_session,
    SoupMessage* message, SoupSocket* socket, jumanji_t* jumanji);

/**
 * Add cookies from the list to jumanji
 *
 * @param jumanji The jumanji session
 * @param cookies List of SoupCookie's
 */
void jumanji_soup_add_cookies(jumanji_t* jumanji, girara_list_t* cookies);

/**
 * Activates a jumanji proxy
 *
 * @param jumanji The jumanji session
 * @param proxy The jumanji proxy
 */
void jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy);

#endif // COOKIES_H

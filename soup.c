/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara/girara.h>

#include <libsoup/soup.h>

#include "soup.h"

struct jumanji_soup_s
{
  SoupSession* session; /*>> Soup session */
};

jumanji_soup_t*
jumanji_soup_init(jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->config.config_dir == NULL) {
    return NULL;
  }

  jumanji_soup_t* soup = malloc(sizeof(jumanji_soup_t));
  if (soup == NULL) {
    return NULL;
  }

  /* libsoup */
  soup->session = webkit_get_default_session();
  if (soup->session == NULL) {
    free(soup);
    return NULL;
  }

  char* cookie_file = g_build_filename(jumanji->config.config_dir,
      JUMANJI_COOKIE_FILE, NULL);
  if (cookie_file == NULL) {
    free(soup);
    return NULL;
  }

  SoupCookieJar* cookie_jar = soup_cookie_jar_text_new(cookie_file, FALSE);
  if (cookie_jar == NULL) {
    g_free(cookie_file);
    free(soup);
    return NULL;
  }
  g_free(cookie_file);

  soup_session_add_feature(soup->session, (SoupSessionFeature*) cookie_jar);

  return soup;
}

void
jumanji_soup_free(jumanji_soup_t* soup)
{
  if (soup == NULL) {
    return;
  }

  free(soup);
}

void
jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy)
{
  if (jumanji == NULL || jumanji->global.soup == NULL) {
    return;
  }

  jumanji_soup_t* soup = (jumanji_soup_t*) jumanji->global.soup;

  if (proxy != NULL && proxy->url != NULL) {
    SoupURI* soup_uri = soup_uri_new(proxy->url);
    g_object_set(soup->session, "proxy-uri", soup_uri, NULL);
    soup_uri_free(soup_uri);
    jumanji->global.current_proxy = proxy;

    char* text = (proxy->description != NULL) ? proxy->description : proxy->url;
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, text);
  } else {
    g_object_set(soup->session, "proxy-uri", NULL, NULL);
    jumanji->global.current_proxy = NULL;

    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, "Proxy disabled");
  }
}

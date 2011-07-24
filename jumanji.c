/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara.h>
#include <string.h>

#include "callbacks.h"
#include "config.h"
#include "database.h"
#include "jumanji.h"
#include "userscripts.h"
#include "marks.h"
#include "utils.h"

#define GLOBAL_RC  "/etc/jumanjirc"
#define JUMANJI_RC "jumanjirc"
#define JUMANJI_COOKIE_FILE "cookies"
#define JUMANJI_BOOKMARKS_FILE "bookmarks"
#define JUMANJI_HISTORY_FILE "history"
#define JUMANJI_QUICKMARKS_FILE "quickmarks"

jumanji_t*
jumanji_init(int argc, char* argv[])
{
  /* parse command line options */
  gchar* config_dir = NULL, *data_dir = NULL;
  GOptionEntry entries[] = {
    { "config-dir", 'c', 0, G_OPTION_ARG_FILENAME, &config_dir, "Path to the config directory", "path" },
    { "data-dir",   'd', 0, G_OPTION_ARG_FILENAME, &data_dir,   "Path to the data directory",   "path" },
    { NULL }
  };

  GOptionContext* context = g_option_context_new(" [url]");
  g_option_context_add_main_entries(context, entries, NULL);

  GError* error = NULL;
  if (g_option_context_parse(context, &argc, &argv, &error) == FALSE) {
    girara_error("Error parsing command line arguments: %s\n", error->message);
    g_option_context_free(context);
    g_error_free(error);
    goto error_out;
  }
  g_option_context_free(context);

  /* jumanji */
  jumanji_t* jumanji = malloc(sizeof(jumanji_t));

  if (jumanji == NULL) {
    goto error_out;
  }

  /* set default values */
  jumanji->ui.session          = NULL;
  jumanji->ui.statusbar.url    = NULL;
  jumanji->ui.statusbar.buffer = NULL;
  jumanji->ui.statusbar.tabs   = NULL;
  jumanji->ui.statusbar.proxy  = NULL;

  jumanji->config.config_dir = NULL;
  jumanji->config.data_dir   = NULL;

  jumanji->modes.normal = 0;

  jumanji->global.browser_settings = NULL;
  jumanji->global.soup_session     = NULL;
  jumanji->global.search_engines   = NULL;
  jumanji->global.proxies          = NULL;
  jumanji->global.marks            = NULL;
  jumanji->global.last_closed      = NULL;
  jumanji->global.current_proxy    = NULL;
  jumanji->global.user_scripts     = NULL;
  jumanji->global.arguments        = argv;

  jumanji->database.session = NULL;

  jumanji->search.item = NULL;

  jumanji->downloads.widget = NULL;

  /* begin initialization */
  if (config_dir) {
    jumanji->config.config_dir = g_strdup(config_dir);
  } else {
    gchar* path = girara_get_xdg_path(XDG_CONFIG);
    jumanji->config.config_dir = g_build_filename(path, "jumanji", NULL);
    g_free(path);
  }

  if (data_dir) {
    jumanji->config.data_dir = g_strdup(config_dir);
  } else {
    gchar* path = girara_get_xdg_path(XDG_DATA);
    jumanji->config.data_dir = g_build_filename(path, "jumanji", NULL);
    g_free(path);
  }

  /* UI */
  if ((jumanji->ui.session = girara_session_create()) == NULL) {
    goto error_free;
  }

  jumanji->ui.session->global.data = jumanji;

  jumanji->global.search_engines = girara_list_new();
  if (jumanji->global.search_engines == NULL) {
    goto error_free;
  }

  jumanji->global.proxies = girara_list_new();
  if (jumanji->global.proxies == NULL) {
    goto error_free;
  }

  jumanji->global.marks = girara_list_new();
  if (jumanji->global.marks == NULL) {
    goto error_free;
  }

  girara_list_set_free_function(jumanji->global.marks, mark_free);

  jumanji->global.last_closed = girara_list_new();
  if (jumanji->global.last_closed == NULL) {
    goto error_free;
  }

  girara_list_set_free_function(jumanji->global.last_closed, jumanji_last_closed_free);

  /* user scripts */
  char* user_script_dir = g_build_filename(jumanji->config.config_dir, USER_SCRIPTS_DIR, NULL);
  jumanji->global.user_scripts = user_script_load_dir(user_script_dir);
  if (jumanji->global.user_scripts == NULL) {
    goto error_free;
  }
  g_free(user_script_dir);

  /* webkit */
  jumanji->global.browser_settings = webkit_web_settings_new();
  if (jumanji->global.browser_settings == NULL) {
    goto error_free;
  }

  /* libsoup */
  jumanji->global.soup_session = webkit_get_default_session();
  if (jumanji->global.soup_session == NULL) {
    goto error_free;
  }

  char* cookie_file = g_build_filename(jumanji->config.config_dir, JUMANJI_COOKIE_FILE, NULL);
  if (cookie_file == NULL) {
    goto error_free;
  }

  SoupCookieJar* cookie_jar = soup_cookie_jar_text_new(cookie_file, FALSE);
  if (cookie_jar != NULL) {
    soup_session_add_feature(jumanji->global.soup_session, (SoupSessionFeature*) cookie_jar);
  } else {
    girara_error("Could not initialize cookie jar");
  }
  g_free(cookie_file);

  /* configuration */
  config_load_default(jumanji);

  /* load global configuration files */
  config_load_file(jumanji, GLOBAL_RC);

  /* load local configuration files */
  char* configuration_file = g_build_filename(jumanji->config.config_dir, JUMANJI_RC, NULL);
  config_load_file(jumanji, configuration_file);
  g_free(configuration_file);

  /* initialize girara */
  if (girara_session_init(jumanji->ui.session) == false) {
    goto error_free;
  }

  /* initialize download widget */
  jumanji->downloads.widget = gtk_vbox_new(FALSE, 0);
  if (jumanji->downloads.widget == NULL) {
    goto error_free;
  }

  /* enable tabs */
  girara_tabs_enable(jumanji->ui.session);

  /* girara events */
  jumanji->ui.session->events.buffer_changed = cb_girara_buffer_changed;

  /* connect additional signals */
  g_signal_connect(G_OBJECT(jumanji->ui.session->gtk.tabs), "switch-page",  G_CALLBACK(cb_jumanji_tab_changed), jumanji);
  g_signal_connect(G_OBJECT(jumanji->ui.session->gtk.tabs), "page-removed", G_CALLBACK(cb_jumanji_tab_removed), jumanji);

  /* statusbar */
  jumanji->ui.statusbar.url = girara_statusbar_item_add(jumanji->ui.session, TRUE, TRUE, TRUE, NULL);
  if (jumanji->ui.statusbar.url == NULL) {
    goto error_free;
  }

  girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.url, "[No name]");

  jumanji->ui.statusbar.buffer = girara_statusbar_item_add(jumanji->ui.session, FALSE, FALSE, FALSE, NULL);
  if (jumanji->ui.statusbar.buffer == NULL) {
    goto error_free;
  }

  jumanji->ui.statusbar.tabs = girara_statusbar_item_add(jumanji->ui.session, FALSE, FALSE, FALSE, NULL);
  if (jumanji->ui.statusbar.tabs == NULL) {
    goto error_free;
  }

  girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.tabs, "[1/1]");

  jumanji->ui.statusbar.proxy = girara_statusbar_item_add(jumanji->ui.session, FALSE, FALSE, FALSE, cb_statusbar_proxy);
  if (jumanji->ui.statusbar.proxy == NULL) {
    goto error_free;
  }

  if (jumanji->global.proxies && girara_list_size(jumanji->global.proxies) > 0 && jumanji->global.current_proxy == NULL) {
    bool* auto_set_proxy = (bool*) girara_setting_get(jumanji->ui.session, "auto-set-proxy");
    if (auto_set_proxy && *auto_set_proxy == true) {
      jumanji_proxy_t* proxy = (jumanji_proxy_t*) girara_list_nth(jumanji->global.proxies, 0);
      jumanji_proxy_set(jumanji, proxy);
    } else {
      girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, "Proxy disabled");
    }
  }

  /* database */
  jumanji->database.session = db_new(jumanji);
  if (jumanji->database.session == NULL) {
    girara_error("Could not create database object");
    goto error_free;
  }

  char* bookmark_file = g_build_filename(jumanji->config.config_dir, JUMANJI_BOOKMARKS_FILE, NULL);
  if (bookmark_file != NULL) {
    db_set_bookmark_file(jumanji->database.session, bookmark_file);
    g_free(bookmark_file);
  } else {
    goto error_free;
  }

  char* history_file = g_build_filename(jumanji->config.config_dir, JUMANJI_HISTORY_FILE, NULL);
  if (history_file != NULL) {
    db_set_history_file(jumanji->database.session, history_file);
    g_free(history_file);
  } else {
    goto error_free;
  }

  char* quickmarks_file = g_build_filename(jumanji->config.config_dir, JUMANJI_QUICKMARKS_FILE, NULL);
  if (quickmarks_file != NULL) {
    db_set_quickmarks_file(jumanji->database.session, quickmarks_file);
    g_free(quickmarks_file);
  } else {
    goto error_free;
  }

  if (db_init(jumanji->database.session) == false) {
    girara_error("Could not initialize database");
    goto error_free;
  }

  /* load tabs */
  if(argc < 2) {
    char* homepage = girara_setting_get(jumanji->ui.session, "homepage");
    if (homepage != NULL) {
      char* url = jumanji_build_url_from_string(jumanji, homepage);
      jumanji_tab_new(jumanji, url, false);
      free(url);
    }
    free(homepage);
  } else {
    for (unsigned int i = argc - 1; i >= 1; i--) {
      char* url = jumanji_build_url_from_string(jumanji, argv[i]);
      jumanji_tab_new(jumanji, url, false);
      free(url);
    }
  }

  return jumanji;

error_free:

  if (jumanji) {
    if (jumanji->ui.session) {
      girara_session_destroy(jumanji->ui.session);
    }

    if (jumanji->database.session) {
      db_close(jumanji->database.session);
    }
  }

  free(jumanji);

error_out:

  return NULL;
}

void
jumanji_free(jumanji_t* jumanji)
{
  if (jumanji == NULL) {
    return;
  }

  /* destroy girara session */
  if (jumanji->ui.session != NULL) {
    girara_session_destroy(jumanji->ui.session);
  }

  g_free(jumanji->config.config_dir);
  g_free(jumanji->config.data_dir);

  if (jumanji->global.browser_settings == NULL) {
    g_object_unref(jumanji->global.browser_settings);
  }

  /* free search engines */
  if (jumanji->global.search_engines != NULL) {
    if (girara_list_size(jumanji->global.search_engines) > 0) {
      girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.search_engines);
      do {
        jumanji_search_engine_t* search_engine = (jumanji_search_engine_t*) girara_list_iterator_data(iter);
        if (search_engine != NULL) {
          g_free(search_engine->identifier);
          g_free(search_engine->url);
          g_free(search_engine);
        }
      } while (girara_list_iterator_next(iter));
      girara_list_iterator_free(iter);
    }
    girara_list_free(jumanji->global.search_engines);
  }

  /* free proxies */
  if (jumanji->global.proxies != NULL) {
    if (girara_list_size(jumanji->global.proxies) > 0) {
      girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.proxies);
      do {
        jumanji_proxy_t* proxy = (jumanji_proxy_t*) girara_list_iterator_data(iter);
        if (proxy != NULL) {
          g_free(proxy->description);
          g_free(proxy->url);
          g_free(proxy);
        }
      } while (girara_list_iterator_next(iter));
      girara_list_iterator_free(iter);
    }
    girara_list_free(jumanji->global.proxies);
  }

  /* free marks */
  girara_list_free(jumanji->global.marks);

  /* free user scipts */
  girara_list_free(jumanji->global.user_scripts);

  /* free last closed */
  girara_list_free(jumanji->global.last_closed);

  free(jumanji);
}

jumanji_tab_t*
jumanji_tab_new(jumanji_t* jumanji, const char* url, bool background)
{
  if (jumanji == NULL || url == NULL) {
    goto error_out;
  }

  jumanji_tab_t* tab = malloc(sizeof(jumanji_tab_t));

  if (tab == NULL) {
    goto error_out;
  }

  tab->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  tab->web_view        = webkit_web_view_new();
  tab->jumanji         = jumanji;

  if (tab->scrolled_window == NULL || tab->web_view == NULL) {
    goto error_free;
  }

  g_object_ref_sink(tab->web_view);

  /* save reference to tab */
  g_object_set_data(G_OBJECT(tab->scrolled_window), "jumanji-tab", tab);

  /* ui */
  gtk_container_add(GTK_CONTAINER(tab->scrolled_window), tab->web_view);
  gtk_widget_show_all(tab->scrolled_window);

  /* apply browser setting */
  webkit_web_view_set_settings(WEBKIT_WEB_VIEW(tab->web_view), webkit_web_settings_copy(jumanji->global.browser_settings));

  /* set web inspector */
  WebKitWebInspector* web_inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(tab->web_view));
  if (web_inspector != NULL) {
    g_signal_connect(G_OBJECT(web_inspector), "inspect-web-view", G_CALLBACK(cb_jumanji_tab_web_inspector), tab);
  }

  /* load url */
  jumanji_tab_load_url(tab, url);

  /* create new tab */
  tab->girara_tab = girara_tab_new(jumanji->ui.session, NULL, tab->scrolled_window, true, jumanji);

  /* connect signals */
  g_signal_connect(G_OBJECT(tab->scrolled_window), "destroy",             G_CALLBACK(cb_jumanji_tab_destroy),            tab);
  g_signal_connect(G_OBJECT(tab->web_view),        "hovering-over-link",  G_CALLBACK(cb_jumanji_tab_hovering_over_link), tab);
  g_signal_connect(G_OBJECT(tab->web_view),        "notify::load-status", G_CALLBACK(cb_jumanji_tab_load_status),        tab);
  g_signal_connect(G_OBJECT(tab->web_view),        "load-finished",       G_CALLBACK(cb_jumanji_tab_load_finished),      tab);
  g_signal_connect(G_OBJECT(tab->web_view),        "download-requested",  G_CALLBACK(cb_jumanji_tab_download_requested), tab);

  /* setup userscripts */
  user_script_init_tab(tab, jumanji->global.user_scripts);

  return tab;

error_free:

  free(tab);

error_out:

  return NULL;
}

void
jumanji_tab_free(jumanji_tab_t* tab)
{
  if (tab == NULL) {
    return;
  }

  g_object_unref(tab->web_view);
  free(tab);
}

jumanji_tab_t*
jumanji_tab_get_current(jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->ui.session == NULL) {
    return NULL;
  }

  girara_tab_t* girara_tab = girara_tab_current_get(jumanji->ui.session);

  if (girara_tab == NULL) {
    return NULL;
  }

  return g_object_get_data(G_OBJECT(girara_tab->widget), "jumanji-tab");
}

jumanji_tab_t*
jumanji_tab_get_nth(jumanji_t* jumanji, unsigned int index)
{
  if (jumanji == NULL || jumanji->ui.session == NULL) {
    return NULL;
  }

  girara_tab_t* girara_tab = girara_tab_get(jumanji->ui.session, index);

  if (girara_tab == NULL) {
    return NULL;
  }

  return g_object_get_data(G_OBJECT(girara_tab->widget), "jumanji-tab");
}

void
jumanji_tab_load_url(jumanji_tab_t* tab, const char* url)
{
  if (tab == NULL || url == NULL || tab->web_view == NULL) {
    return;
  }

  webkit_web_view_load_uri(WEBKIT_WEB_VIEW(tab->web_view), url);
}

void
jumanji_tab_show_search_results(jumanji_tab_t* tab)
{
  if (tab == NULL || tab->web_view == NULL) {
    return;
  }

  webkit_web_view_unmark_text_matches(WEBKIT_WEB_VIEW(tab->web_view));

  if (tab->jumanji != NULL && tab->jumanji->search.item != NULL) {
    webkit_web_view_mark_text_matches(WEBKIT_WEB_VIEW(tab->web_view),
        tab->jumanji->search.item, FALSE, 0);
    webkit_web_view_set_highlight_text_matches(WEBKIT_WEB_VIEW(tab->web_view),
        TRUE);
  }
}

char*
jumanji_build_url_from_string(jumanji_t* jumanji, const char* string)
{
  if (jumanji == NULL || string == NULL || jumanji->ui.session == NULL) {
    return NULL;
  }

  girara_list_t* list = build_girara_list(string);
  if (list == NULL) {
    return NULL;
  }

  char* url = jumanji_build_url(jumanji, list);
  girara_list_free(list);

  return url;
}

char*
jumanji_build_url(jumanji_t* jumanji, girara_list_t* list)
{
  if (jumanji == NULL || list == NULL || jumanji->ui.session == NULL) {
    return NULL;
  }

  unsigned int list_length = girara_list_size(list);
  char* url                = NULL;

  if (list_length == 0) {
    /* open homepage */
    char* homepage = girara_setting_get(jumanji->ui.session, "homepage");
    if (homepage != NULL) {
      if (list != NULL) {
        url = jumanji_build_url_from_string(jumanji, homepage);
      }
    }
  } else if (list_length > 1) {
    char* identifier = (char*) girara_list_nth(list, 0);
    char* search_url = NULL;

    /* search matching search engine */
    if (girara_list_size(jumanji->global.search_engines) > 0) {
      girara_list_iterator_t* iter = girara_list_iterator(jumanji->global.search_engines);
      do {
        jumanji_search_engine_t* search_engine = (jumanji_search_engine_t*) girara_list_iterator_data(iter);

        if (search_engine == NULL) {
          continue;
        }

        if (!g_strcmp0(search_engine->identifier, identifier)) {
          search_url = search_engine->url;
          break;
        }
      } while (girara_list_iterator_next(iter));
      girara_list_iterator_free(iter);

      /* if no search engine matches, we use the default one (first one) */
      if (search_url == NULL) {
        jumanji_search_engine_t* search_engine = (jumanji_search_engine_t*) girara_list_nth(jumanji->global.search_engines, 0);
        search_url = search_engine ? g_strdup(search_engine->url) : NULL;
        if (search_url == NULL) {
          return NULL;
        }
      }
    /* there is no search engine available */
    } else {
      return NULL;
    }

    /* if the search url does not contain any %s we abort */
    if (strstr(search_url, "%s") == NULL) {
      girara_error("Search engine (%s) url is invalid", identifier);
      g_free(search_url);
      return NULL;
    }

    /* build search item */
    char* search_item = g_strdup((char*) girara_list_nth(list, 1));
    for (unsigned int i = 2; i < girara_list_size(list); i++) {
      char* tmp = g_strjoin("+", search_item, (char*) girara_list_nth(list, i), NULL);
      g_free(search_item);
      search_item = tmp;
    }

    /* replace all spaces in the search item with '+' */
    for (unsigned int i = 0; i < strlen(search_item); i++ ) {
      if (search_item[i] == ' ') {
        search_item[i] = '+';
      }
    }

    url = g_strdup_printf(search_url, search_item);

    g_free(search_url);
    g_free(search_item);
  } else {
    char* input = (char*) girara_list_nth(list, 0);

    /* file path */
    if (input[0] == '/' || strncmp(input, "./", 2) == 0) {
      url = g_strconcat("file://", input, NULL);
    /* uri does not contain any '.', ':', '/' nor starts with localhost so the default
     * search engine will be used */
    } else if (strpbrk(input, ".:/") == NULL
        && strncmp(input, "localhost", 9) != 0 ) {
    /* just use the url as it is */
    } else {
      url = strstr(input, "://") ? g_strdup(input) : g_strconcat("http://", input, NULL);
    }
  }

  return url;
}

void
jumanji_proxy_set(jumanji_t* jumanji, jumanji_proxy_t* proxy)
{
  if (jumanji == NULL || jumanji->global.soup_session == NULL) {
    return;
  }

  if (proxy != NULL && proxy->url != NULL) {
    SoupURI* soup_uri = soup_uri_new(proxy->url);
    g_object_set(jumanji->global.soup_session, "proxy-uri", soup_uri, NULL);
    soup_uri_free(soup_uri);
    jumanji->global.current_proxy = proxy;

    char* text = (proxy->description != NULL) ? proxy->description : proxy->url;
    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, text);
  } else {
    g_object_set(jumanji->global.soup_session, "proxy-uri", NULL, NULL);
    jumanji->global.current_proxy = NULL;

    girara_statusbar_item_set_text(jumanji->ui.session, jumanji->ui.statusbar.proxy, "Proxy disabled");
  }
}

void
jumanji_window_new(jumanji_t* jumanji, char* uri)
{
  if (jumanji == NULL) {
    return;
  }

  char* argv[] = {
    *(jumanji->global.arguments),
    uri,
    NULL
  };

  g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
}

void
jumanji_last_closed_free(void* data)
{
  free(data);
}

/* main function */
int main(int argc, char* argv[])
{
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc, &argv);

  jumanji_t* jumanji = jumanji_init(argc, argv);
  if (jumanji == NULL) {
    printf("error: coult not initialize jumanji\n");
    return -1;
  }

  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();

  jumanji_free(jumanji);

  return 0;
}

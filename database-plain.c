/* See LICENSE file for license and copyright information */

#include <girara/datastructures.h>
#include <girara/utils.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "database.h"

#define BOOKMARKS "bookmarks"
#define HISTORY "history"
#define QUICKMARKS "quickmarks"
#define COOKIES "cookies"

#define file_lock_set(fd, cmd) \
  { \
  struct flock lock = { .l_type = cmd, .l_start = 0, .l_whence = SEEK_SET, .l_len = 0}; \
  fcntl(fd, F_SETLK, lock); \
  }

/* forward declaration */
static void jumanji_db_write_quickmarks_to_file(const char* filename,
    girara_list_t* quickmarks);
static void cb_jumanji_db_watch_file(GFileMonitor* monitor, GFile* file, GFile*
    other_file, GFileMonitorEvent event, jumanji_database_t* database);
static bool jumanji_db_check_file(const char* path);
static girara_list_t* jumanji_db_read_urls_from_file(const char* filename);
static girara_list_t* jumanji_db_read_quickmarks_from_file(const char*
    filename);
static void jumanji_db_free_quickmark(void* data);
static girara_list_t* jumanji_db_filter_url_list(girara_list_t* list, const
    char* input);
static void jumanji_db_write_urls_to_file(const char* filename, girara_list_t*
    urls, bool visited);
static girara_list_t* jumanji_db_read_cookies_from_file(const char* filename);
static void jumanji_db_write_cookies_to_file(const char* filename,
    girara_list_t* cookies);
static void jumanji_db_free_cookie(void* data);

struct jumanji_database_s
{
  gchar* bookmark_file; /**> File path to the bookmark file */
  girara_list_t* bookmarks; /**> Temporary bookmarks */
  GFileMonitor* bookmark_monitor; /**> File monitor for the bookmark file */

  gchar* history_file; /**> File path to the history file */
  girara_list_t* history; /**>  Temporary history */
  GFileMonitor* history_monitor; /**> File monitor for the history file */

  gchar* quickmarks_file; /**> File path to the quickmarks file */
  girara_list_t* quickmarks; /**>  Temporary quickmarks */
  GFileMonitor* quickmarks_monitor; /**> File monitor for the quickmarks file */

  gchar* cookie_file; /**> File path to the cookie file */
  girara_list_t* cookies; /**>  Temporary cookies */
  GFileMonitor* cookie_monitor; /**> File monitor for the cookie file */
};

typedef struct jumanji_db_quickmark_s
{
  char identifier; /**> Quickmark identifier */
  char* url; /**> Url */
} jumanji_db_quickmark_t;

typedef struct jumanji_database_cookie_s
{
  char* name; /**> Name of the cookie */
  char* value; /**> Value */
  char* domain; /**> Domain */
  char* path; /**> Path */
  time_t expires; /**> Expire date */
  bool secure; /**> Secure */
  bool http_only; /**> Http only */
} jumanji_database_cookie_t;

static bool
jumanji_db_check_file(const char* path)
{
  if (path == NULL) {
    return false;
  }

  if (g_file_test(path, G_FILE_TEST_EXISTS) == false) {
    FILE* file = fopen(path, "w");
    if (file != NULL) {
      fclose(file);
    } else {
      return false;
    }
  } else if (g_file_test(path, G_FILE_TEST_IS_DIR) == true) {
    return false;
  }

  return true;
}

jumanji_database_t*
jumanji_db_init(const char* dir)
{
  if (dir == NULL) {
    goto error_ret;
  }

  jumanji_database_t* database = g_malloc0(sizeof(jumanji_database_t));
  if (database == NULL) {
    goto error_ret;
  }

  /* get bookmark file path */
  database->bookmark_file = g_build_filename(dir, BOOKMARKS, NULL);
  if (database->bookmark_file == NULL ||
      jumanji_db_check_file(database->bookmark_file) == false) {
    goto error_free;
  }

  /* get history file path */
  database->history_file = g_build_filename(dir, HISTORY, NULL);
  if (database->history_file == NULL ||
      jumanji_db_check_file(database->history_file) == false) {
    goto error_free;
  }

  /* get quickmarks file path */
  database->quickmarks_file = g_build_filename(dir, QUICKMARKS, NULL);
  if (database->quickmarks_file == NULL ||
      jumanji_db_check_file(database->quickmarks_file) == false) {
    goto error_free;
  }

  /* get cookie file path */
  database->cookie_file = g_build_filename(dir, COOKIES, NULL);
  if (database->cookie_file == NULL ||
      jumanji_db_check_file(database->cookie_file) == false) {
    goto error_free;
  }

  /* read files */
  database->bookmarks  = jumanji_db_read_urls_from_file(database->bookmark_file);
  database->history    = jumanji_db_read_urls_from_file(database->history_file);
  database->quickmarks = jumanji_db_read_quickmarks_from_file(database->quickmarks_file);
  database->cookies    = jumanji_db_read_cookies_from_file(database->cookie_file);

  girara_list_set_free_function(database->bookmarks,  jumanji_db_free_result_link);
  girara_list_set_free_function(database->history,    jumanji_db_free_result_link);
  girara_list_set_free_function(database->quickmarks, jumanji_db_free_quickmark);
  girara_list_set_free_function(database->cookies,    jumanji_db_free_cookie);

  /* setup file monitors */
  GFile* bookmark_file = g_file_new_for_path(database->bookmark_file);
  if (bookmark_file != NULL) {
    database->bookmark_monitor = g_file_monitor(bookmark_file,
        G_FILE_MONITOR_NONE, NULL, NULL);
  } else {
    goto error_free;
  }
  g_object_unref(bookmark_file);

  GFile* history_file = g_file_new_for_path(database->history_file);
  if (history_file != NULL) {
    database->history_monitor = g_file_monitor(history_file,
        G_FILE_MONITOR_NONE, NULL, NULL);
  } else {
    goto error_free;
  }
  g_object_unref(history_file);

  GFile* quickmarks_file = g_file_new_for_path(database->quickmarks_file);
  if (quickmarks_file != NULL) {
    database->quickmarks_monitor = g_file_monitor(quickmarks_file,
        G_FILE_MONITOR_NONE, NULL, NULL);
  } else {
    goto error_free;
  }
  g_object_unref(quickmarks_file);

  GFile* cookie_file = g_file_new_for_path(database->cookie_file);
  if (cookie_file != NULL) {
    database->cookie_monitor = g_file_monitor(cookie_file,
        G_FILE_MONITOR_NONE, NULL, NULL);
  } else {
    goto error_free;
  }
  g_object_unref(cookie_file);

  if (database->bookmark_monitor == NULL || database->history_monitor == NULL ||
      database->quickmarks_monitor == NULL || database->cookie_monitor == NULL) {
    goto error_free;
  }

  g_signal_connect(G_OBJECT(database->bookmark_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
  g_signal_connect(G_OBJECT(database->history_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
  g_signal_connect(G_OBJECT(database->quickmarks_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
  g_signal_connect(G_OBJECT(database->cookie_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);

  return database;

error_free:

  jumanji_db_free(database);

error_ret:

  return NULL;
}

void
jumanji_db_free(jumanji_database_t* database)
{
  if (database == NULL) {
    return;
  }

  g_free(database->bookmark_file);
  g_free(database->history_file);
  g_free(database->quickmarks_file);
  g_free(database->cookie_file);

  girara_list_free(database->bookmarks);
  girara_list_free(database->history);
  girara_list_free(database->quickmarks);
  girara_list_free(database->cookies);

  if (database->bookmark_monitor != NULL) {
    g_object_unref(database->bookmark_monitor);
  }

  if (database->history_monitor != NULL) {
    g_object_unref(database->history_monitor);
  }

  if (database->quickmarks_monitor != NULL) {
    g_object_unref(database->quickmarks_monitor);
  }

  if (database->cookie_monitor != NULL) {
    g_object_unref(database->cookie_monitor);
  }

  g_free(database);
}

girara_list_t*
jumanji_db_bookmark_find(jumanji_database_t* database, const char* input)
{
  if (database == NULL || database->bookmarks == NULL || input == NULL) {
    return NULL;
  }

  return jumanji_db_filter_url_list(database->bookmarks, input);
}

void
jumanji_db_bookmark_remove(jumanji_database_t* database, const char* url)
{
  if (database == NULL || database->bookmarks == NULL || url == NULL) {
    return;
  }

  /* remove url from list */
  if (girara_list_size(database->bookmarks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->bookmarks);

    do {
      jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_iterator_data(iter);

      if (strcmp(link->url, url) == 0) {
        girara_list_remove(database->bookmarks, link);
      }
    } while (girara_list_iterator_next(iter) != NULL);

    girara_list_iterator_free(iter);

    jumanji_db_write_urls_to_file(database->bookmark_file, database->bookmarks, false);
    g_signal_connect(G_OBJECT(database->bookmark_monitor), "changed",
        G_CALLBACK(cb_jumanji_db_watch_file), database);
  }
}

void
jumanji_db_bookmark_add(jumanji_database_t* database, const char* url, const char* title)
{
  if (database == NULL || database->bookmarks == NULL || url == NULL) {
    return;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->bookmarks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->bookmarks);
    do {
      jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_iterator_data(iter);
      if (link == NULL) {
        continue;
      }

      if (strstr(link->url, url) != NULL) {
        g_free(link->title);
        link->title = title ? g_strdup(title) : NULL;
        jumanji_db_write_urls_to_file(database->bookmark_file, database->bookmarks, false);
        g_signal_connect(G_OBJECT(database->bookmark_monitor), "changed",
            G_CALLBACK(cb_jumanji_db_watch_file), database);
        girara_list_iterator_free(iter);
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  /* add url to list */
  jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) malloc(sizeof(jumanji_db_result_link_t));
  if (link == NULL) {
    return;
  }

  link->url     = g_strdup(url);
  link->title   = g_strdup(title);
  link->visited = 0;

  girara_list_append(database->bookmarks, link);

  /* write to file */
  jumanji_db_write_urls_to_file(database->bookmark_file, database->bookmarks, false);
  g_signal_connect(G_OBJECT(database->bookmark_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
}

girara_list_t*
jumanji_db_history_find(jumanji_database_t* database, const char* input)
{
  if (database == NULL || database->history == NULL || input == NULL) {
    return NULL;
  }

  return jumanji_db_filter_url_list(database->history, input);
}

void
jumanji_db_history_add(jumanji_database_t* database, const char* url, const char* title)
{
  if (database == NULL || database->history == NULL || url == NULL) {
    return;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->history) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->history);
    do {
      jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_iterator_data(iter);
      if (link == NULL) {
        continue;
      }

      if (strstr(link->url, url) != NULL) {
        g_free(link->title);
        link->title   = title ? g_strdup(title) : NULL;
        link->visited = time(NULL);
        jumanji_db_write_urls_to_file(database->history_file, database->history, false);
        g_signal_connect(G_OBJECT(database->history_monitor), "changed",
            G_CALLBACK(cb_jumanji_db_watch_file), database);
        girara_list_iterator_free(iter);
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  /* add url to list */
  jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) malloc(sizeof(jumanji_db_result_link_t));
  if (link == NULL) {
    return;
  }

  link->url     = g_strdup(url);
  link->title   = g_strdup(title);
  link->visited = time(NULL);

  girara_list_append(database->history, link);

  /* write to file */
  jumanji_db_write_urls_to_file(database->history_file, database->history, false);
  g_signal_connect(G_OBJECT(database->history_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
}

void
jumanji_db_history_clean(jumanji_database_t* database, unsigned int age)
{
  if (database == NULL || database->history == NULL) {
    return;
  }

  /* remove url from list */
  if (girara_list_size(database->history) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->history);

    int visited = time(NULL) - age;
    do {
      jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_iterator_data(iter);

      if (link->visited >= visited) {
        girara_list_remove(database->history, link);
      }
    } while (girara_list_iterator_next(iter) != NULL);

    girara_list_iterator_free(iter);

    jumanji_db_write_urls_to_file(database->history_file, database->history, false);
    g_signal_connect(G_OBJECT(database->history_monitor), "changed",
        G_CALLBACK(cb_jumanji_db_watch_file), database);
  }
}

void
jumanji_db_quickmark_add(jumanji_database_t* database, const char identifier, const char* url)
{
  if (database == NULL || database->quickmarks == NULL || url == NULL) {
    return;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->quickmarks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->quickmarks);
    do {
      jumanji_db_quickmark_t* quickmark = (jumanji_db_quickmark_t*) girara_list_iterator_data(iter);
      if (quickmark == NULL) {
        continue;
      }

      if (quickmark->identifier == identifier) {
        g_free(quickmark->url);
        quickmark->url = g_strdup(url);

        jumanji_db_write_quickmarks_to_file(database->quickmarks_file, database->quickmarks);
        g_signal_connect(G_OBJECT(database->quickmarks_monitor), "changed",
            G_CALLBACK(cb_jumanji_db_watch_file), database);
        girara_list_iterator_free(iter);
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  /* add url to list */
  jumanji_db_quickmark_t* quickmark = (jumanji_db_quickmark_t*) malloc(sizeof(jumanji_db_quickmark_t));
  if (quickmark == NULL) {
    return;
  }

  quickmark->url        = g_strdup(url);
  quickmark->identifier = identifier;

  girara_list_append(database->quickmarks, quickmark);

  /* write to file */
  jumanji_db_write_quickmarks_to_file(database->quickmarks_file, database->quickmarks);
  g_signal_connect(G_OBJECT(database->quickmarks_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
}

char*
jumanji_db_quickmark_find(jumanji_database_t* database, const char identifier)
{
  if (database == NULL || database->quickmarks == NULL) {
    return NULL;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->quickmarks) > 0) {
    char* url = NULL;
    girara_list_iterator_t* iter = girara_list_iterator(database->quickmarks);
    do {
      jumanji_db_quickmark_t* quickmark = (jumanji_db_quickmark_t*) girara_list_iterator_data(iter);
      if (quickmark == NULL) {
        continue;
      }

      if (quickmark->identifier == identifier) {
        url = g_strdup(quickmark->url);
        break;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);

    return url;
  } else {
    return NULL;
  }
}

void
jumanji_db_quickmark_remove(jumanji_database_t* database, const char identifier)
{
  if (database == NULL || database->quickmarks == NULL) {
    return;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->quickmarks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->quickmarks);
    do {
      jumanji_db_quickmark_t* quickmark = (jumanji_db_quickmark_t*) girara_list_iterator_data(iter);
      if (quickmark == NULL) {
        continue;
      }

      if (quickmark->identifier == identifier) {
        girara_list_remove(database->quickmarks, quickmark);
        break;
      }
    } while (girara_list_iterator_next(iter) != NULL);

    jumanji_db_write_quickmarks_to_file(database->quickmarks_file, database->quickmarks);
    g_signal_connect(G_OBJECT(database->quickmarks_monitor), "changed",
        G_CALLBACK(cb_jumanji_db_watch_file), database);
    girara_list_iterator_free(iter);
  }
}

static girara_list_t*
jumanji_db_read_urls_from_file(const char* filename)
{
  if (filename == NULL) {
    return NULL;
  }

  /* open file */
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    close(fd);
    return NULL;
  }

  girara_list_set_free_function(list, jumanji_db_free_result_link);

  file_lock_set(fd, F_WRLCK);

  /* read lines */
  char* line = NULL;
  while ((line = girara_file_read_line_from_fd(fd)) != NULL) {
    /* skip empty lines */
    if (strlen(line) == 0) {
      free(line);
      continue;
    }

    /* parse line */
    gchar** argv = NULL;
    gint    argc = 0;

    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      jumanji_db_result_link_t* link = malloc(sizeof(link));
      if (link == NULL) {
        g_strfreev(argv);
        free(line);
        continue;
      }

      link->url     = g_strdup(argv[0]);
      link->title   = (argc > 1) ? g_strdup(argv[1]) : NULL;
      link->visited = (argc > 2) ? atoi(argv[2])     : 0;

      girara_list_append(list, link);
    }

    g_strfreev(argv);
    free(line);
  }

  file_lock_set(fd, F_UNLCK);
  close(fd);

  return list;
}

static girara_list_t*
jumanji_db_read_quickmarks_from_file(const char* filename)
{
  if (filename == NULL) {
    return NULL;
  }

  /* open file */
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    close(fd);
    return NULL;
  }

  file_lock_set(fd, F_WRLCK);

  /* read lines */
  char* line = NULL;
  while ((line = girara_file_read_line_from_fd(fd)) != NULL) {
    /* skip empty lines */
    if (strlen(line) == 0) {
      free(line);
      continue;
    }

    /* parse line */
    gchar** argv = NULL;
    gint    argc = 0;

    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      if (argc < 2) {
        free(line);
        continue;
      }

      jumanji_db_quickmark_t* quickmark = malloc(sizeof(jumanji_db_quickmark_t));
      if (quickmark == NULL) {
        free(line);
        continue;
      }

      quickmark->identifier = argv[0][0];
      quickmark->url        = g_strdup(argv[1]);

      girara_list_append(list, quickmark);
    }

    g_strfreev(argv);
    free(line);
  }

  file_lock_set(fd, F_UNLCK);
  close(fd);

  return list;
}

static void
jumanji_db_write_urls_to_file(const char* filename, girara_list_t* urls, bool visited)
{
  if (filename == NULL || urls == NULL) {
    return;
  }

  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    return;
  }

  file_lock_set(fd, F_WRLCK);

  if (girara_list_size(urls) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(urls);
    do {
      jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_iterator_data(iter);
      if (link == NULL) {
        continue;
      }

      /* write url */
      write(fd, link->url, strlen(link->url));

      /* write title */
      char* title_quoted = g_shell_quote(link->title ? link->title : "");
      char* text = g_strdup_printf(" %s", title_quoted);
      write(fd, text, strlen(text));
      g_free(title_quoted);
      g_free(text);

      /* write last visit */
      if (visited == true) {
        char* text = g_strdup_printf(" %d", link->visited);
        write(fd, text, strlen(text));
        g_free(text);
      }

      write(fd, "\n", 1);
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  file_lock_set(fd, F_UNLCK);

  close(fd);
}

static void
jumanji_db_write_quickmarks_to_file(const char* filename, girara_list_t* quickmarks)
{
  if (filename == NULL || quickmarks == NULL) {
    return;
  }

  /* open file */
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    return;
  }

  file_lock_set(fd, F_WRLCK);

  if (girara_list_size(quickmarks) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(quickmarks);
    do {
      jumanji_db_quickmark_t* quickmark = (jumanji_db_quickmark_t*) girara_list_iterator_data(iter);
      if (quickmark == NULL) {
        continue;
      }

      char* text = g_strdup_printf("%c %s", quickmark->identifier, quickmark->url);
      write(fd, text, strlen(text));
      g_free(text);

      write(fd, "\n", 1);
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  file_lock_set(fd, F_UNLCK);
  close(fd);
}

static girara_list_t*
jumanji_db_filter_url_list(girara_list_t* list, const char* input)
{
  if (list == NULL || girara_list_size(list) == 0) {
    return NULL;
  }

  girara_list_t* new_list = girara_list_new();
  if (new_list == NULL) {
    girara_list_free(list);
    return NULL;
  }

  girara_list_set_free_function(new_list, jumanji_db_free_result_link);
  girara_list_iterator_t* iter = girara_list_iterator(list);

  do {
    jumanji_db_result_link_t* link = (jumanji_db_result_link_t*) girara_list_iterator_data(iter);

    if (strstr(link->url, input) != NULL || strstr(link->title, input)) {
      /* duplicate entry */
      jumanji_db_result_link_t* link_dup = malloc(sizeof(jumanji_db_result_link_t));
      if (link_dup != NULL) {
        link_dup->url     = g_strdup(link->url);
        link_dup->title   = g_strdup(link->title);
        link_dup->visited = link->visited;
        girara_list_append(new_list, link_dup);
      }
    }
  } while (girara_list_iterator_next(iter) != NULL);

  girara_list_iterator_free(iter);

  return new_list;
}

static void
cb_jumanji_db_watch_file(GFileMonitor* monitor, GFile* file, GFile* other_file,
    GFileMonitorEvent event, jumanji_database_t* database)
{
  if (event != G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT || database == NULL) {
    return;
  }

  char* path = g_file_get_path(file);

  if (path == NULL) {
    return;
  }

  if (database->bookmark_file && strcmp(database->bookmark_file, path) == 0) {
    girara_list_free(database->bookmarks);
    database->bookmarks = jumanji_db_read_urls_from_file(database->bookmark_file);
    girara_list_set_free_function(database->bookmarks,  jumanji_db_free_result_link);
  } else if (database->history_file && strcmp(database->history_file, path) == 0) {
    girara_list_free(database->history);
    database->history = jumanji_db_read_urls_from_file(database->history_file);
    girara_list_set_free_function(database->history,    jumanji_db_free_result_link);
  } else if (database->quickmarks_file && strcmp(database->quickmarks_file, path) == 0) {
    girara_list_free(database->quickmarks);
    database->quickmarks = jumanji_db_read_quickmarks_from_file(database->quickmarks_file);
    girara_list_set_free_function(database->quickmarks, jumanji_db_free_quickmark);
  } else if (database->cookie_file && strcmp(database->cookie_file, path) == 0) {
    girara_list_free(database->cookies);
    database->cookies = jumanji_db_read_cookies_from_file(database->cookie_file);
    girara_list_set_free_function(database->cookies, jumanji_db_free_cookie);
  }

  g_free(path);
}

static void
jumanji_db_free_quickmark(void* data)
{
  if (data == NULL) {
    return;
  }

  jumanji_db_quickmark_t* quickmark = (jumanji_db_quickmark_t*) data;
  g_free(quickmark->url);
  free(quickmark);
}

static girara_list_t*
jumanji_db_read_cookies_from_file(const char* filename)
{
  if (filename == NULL) {
    return NULL;
  }

  /* open file */
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    close(fd);
    return NULL;
  }

  girara_list_set_free_function(list, jumanji_db_free_cookie);

  file_lock_set(fd, F_WRLCK);

  /* read lines */
  char* line = NULL;
  while ((line = girara_file_read_line_from_fd(fd)) != NULL) {
    /* skip empty lines */
    if (strlen(line) == 0) {
      free(line);
      continue;
    }

    /* parse line */
    gchar** argv = NULL;
    gint    argc = 0;

    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      jumanji_database_cookie_t* cookie = malloc(sizeof(jumanji_database_cookie_t));
      if (cookie == NULL || argc != 7) {
        g_strfreev(argv);
        free(line);
        continue;
      }

      cookie->domain    = g_strdup(argv[0]);
      cookie->secure    = (strcmp(argv[1], "TRUE") == 0) ? true : false;
      cookie->path      = g_strdup(argv[2]);
      cookie->http_only = (strcmp(argv[3], "TRUE") == 0) ? true : false;
      cookie->name      = g_strdup(argv[5]);
      cookie->value     = g_strdup(argv[6]);

      /* parse expire value */
      long long unsigned int tmp = 0;
      char* p = argv[4];
      for (; *p != '\0'; p++) {
        tmp = 10 * tmp + (*p - '0');
      }

      cookie->expires = (time_t) tmp;

      /* add parsed cookie to list */
      girara_list_append(list, cookie);

      /* add cookie to cookie jar */
    }

    g_strfreev(argv);
    free(line);
  }

  file_lock_set(fd, F_UNLCK);

  close(fd);

  return list;
}

static void
jumanji_db_write_cookies_to_file(const char* filename, girara_list_t* cookies)
{
  if (filename == NULL || cookies == NULL) {
    return;
  }

  /* open file */
  int fd = open(filename, O_RDWR);
  if (fd == -1) {
    return;
  }

  file_lock_set(fd, F_WRLCK);

  if (girara_list_size(cookies) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(cookies);
    do {
      jumanji_database_cookie_t* cookie = (jumanji_database_cookie_t*) girara_list_iterator_data(iter);
      if (cookie == NULL || cookie->domain == NULL || cookie->name == NULL ||
          cookie->value == NULL || cookie->path == NULL) {
        continue;
      }

      char* tmp = g_strdup_printf("%llu", (long long unsigned int) cookie->expires);
      if (tmp == NULL) {
        continue;
      }

      /* write domain */
      write(fd, cookie->domain, strlen(cookie->domain));
      write(fd, "\t", 1);

      /* write secure */
      char* value = (cookie->secure == true) ? "TRUE" : "FALSE";
      write(fd, value, strlen(value));
      write(fd, "\t", 1);

      /* write path */
      write(fd, cookie->path, strlen(cookie->path));
      write(fd, "\t", 1);

      /* write https */
      value = (cookie->http_only == true) ? "TRUE" : "FALSE";
      write(fd, value, strlen(value));
      write(fd, "\t", 1);

      /* write expire date */
      write(fd, tmp, strlen(tmp));
      write(fd, "\t", 1);

      /* write name */
      write(fd, cookie->name, strlen(cookie->name));
      write(fd, "\t", 1);

      /* write value */
      write(fd, cookie->value, strlen(cookie->value));
      write(fd, "\t", 1);

      write(fd, "\n", 1);

      g_free(tmp);
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  file_lock_set(fd, F_UNLCK);
  close(fd);
}

void
jumanji_db_cookie_add(jumanji_database_t* database, const char* name, const char* value,
    const char* domain, const char* path, time_t expires, bool secure, bool
    http_only)
{
  if (database == NULL || name == NULL || value == NULL || domain == NULL || path
      == NULL || database->cookies == NULL) {
    return;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->cookies) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->cookies);
    do {
      jumanji_database_cookie_t* cookie = (jumanji_database_cookie_t*) girara_list_iterator_data(iter);
      if (cookie == NULL || cookie->name == NULL || cookie->path == NULL ||
          cookie->value == NULL) {
        continue;
      }

      if (strcmp(cookie->name, name) == 0 && strcmp(cookie->path, path) == 0 &&
          strcmp(cookie->value, value) == 0)  {
        g_free(cookie->domain);
        cookie->domain    = g_strdup(domain);
        cookie->expires   = expires;
        cookie->secure    = secure;
        cookie->http_only = http_only;

        jumanji_db_write_cookies_to_file(database->cookie_file,
            database->cookies);

        girara_list_iterator_free(iter);
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  /* add url to list */
  jumanji_database_cookie_t* cookie = (jumanji_database_cookie_t*) malloc(sizeof(jumanji_database_cookie_t));
  if (cookie == NULL) {
    return;
  }

  cookie->name      = g_strdup(name);
  cookie->value     = g_strdup(value);
  cookie->domain    = g_strdup(domain);
  cookie->path      = (path != NULL) ? g_strdup(path) : g_strdup("/");
  cookie->expires   = expires;
  cookie->secure    = secure;
  cookie->http_only = http_only;

  girara_list_append(database->cookies, cookie);

  /* write to file */
  jumanji_db_write_cookies_to_file(database->cookie_file, database->cookies);
}

void
jumanji_db_cookie_remove(jumanji_database_t* database, const char* domain, const char*
    name)
{
  if (database == NULL || name == NULL || domain == NULL || database->cookies ==
      NULL) {
    return;
  }

  /* search for existing entry and update it */
  if (girara_list_size(database->cookies) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(database->cookies);
    do {
      jumanji_database_cookie_t* cookie = (jumanji_database_cookie_t*) girara_list_iterator_data(iter);
      if (cookie == NULL) {
        continue;
      }

      if (strcmp(cookie->name, name) == 0 && strcmp(cookie->domain, domain) == 0) {
        girara_list_remove(database->cookies, cookie);

        jumanji_db_write_cookies_to_file(database->cookie_file, database->cookies);
        girara_list_iterator_free(iter);
        return;
      }
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }
}

girara_list_t*
jumanji_db_cookie_list(jumanji_database_t* database)
{
  if (database == NULL || database->cookies == NULL || girara_list_size(database->cookies) == 0) {
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  girara_list_iterator_t* iter = girara_list_iterator(database->cookies);
  do {
    jumanji_database_cookie_t* plain_cookie = (jumanji_database_cookie_t*) girara_list_iterator_data(iter);

    if (plain_cookie == NULL) {
      continue;
    }

    time_t now = time(NULL);
    if (now >= plain_cookie->expires) {
      continue;
    }
    int max_age = (plain_cookie->expires - now <= G_MAXINT ? plain_cookie->expires - now : G_MAXINT);

    SoupCookie* cookie = soup_cookie_new(
        plain_cookie->name,
        plain_cookie->value,
        plain_cookie->domain,
        plain_cookie->path,
        max_age
        );

    if (cookie == NULL) {
      continue;
    }

    soup_cookie_set_secure(cookie, plain_cookie->secure);
    soup_cookie_set_http_only(cookie, plain_cookie->http_only);

    if (plain_cookie->expires > 0) {
      SoupDate* date = soup_date_new_from_time_t(plain_cookie->expires);
      soup_cookie_set_expires(cookie, date);
      soup_date_free(date);
    } else if (plain_cookie->expires == -1) {
      soup_cookie_set_max_age(cookie, plain_cookie->expires);
    }

    girara_list_append(list, cookie);
  } while (girara_list_iterator_next(iter));
  girara_list_iterator_free(iter);

  return list;
}

static void
jumanji_db_free_cookie(void* data)
{
  if (data == NULL) {
    return;
  }

  jumanji_database_cookie_t* cookie = (jumanji_database_cookie_t*) data;

  g_free(cookie->name);
  g_free(cookie->value);
  g_free(cookie->domain);
  g_free(cookie->path);
  free(data);
}

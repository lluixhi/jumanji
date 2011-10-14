/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "database.h"

#define BOOKMARKS "bookmarks"
#define HISTORY "history"
#define QUICKMARKS "quickmarks"

#define file_lock_set(fd, cmd) \
  { \
  struct flock lock = { .l_type = cmd, .l_start = 0, .l_whence = SEEK_SET, .l_len = 0}; \
  fcntl(fd, F_SETLK, lock); \
  }

/* forward declaration */
static void jumanji_db_write_quickmarks_to_file(const char* filename, girara_list_t* quickmarks);
static void cb_jumanji_db_watch_file(GFileMonitor* monitor, GFile* file, GFile* other_file, GFileMonitorEvent event, jumanji_database_t* database);
static bool jumanji_db_check_file(const char* path);
static girara_list_t* jumanji_db_read_urls_from_file(const char* filename);
static girara_list_t* jumanji_db_read_quickmarks_from_file(const char* filename);
static void jumanji_db_free_quickmark(void* data);
static girara_list_t* jumanji_db_filter_url_list(girara_list_t* list, const char* input);
static void jumanji_db_write_urls_to_file(const char* filename, girara_list_t* urls, bool visited);

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
};

typedef struct jumanji_db_quickmark_s
{
  char identifier; /**> Quickmark identifier */
  char* url; /**> Url */
} jumanji_db_quickmark_t;

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
  } else if (g_file_test(path, G_FILE_TEST_IS_REGULAR) == false) {
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

  /* read files */
  database->bookmarks  = jumanji_db_read_urls_from_file(database->bookmark_file);
  database->history    = jumanji_db_read_urls_from_file(database->history_file);
  database->quickmarks = jumanji_db_read_quickmarks_from_file(database->quickmarks_file);

  girara_list_set_free_function(database->bookmarks,  jumanji_db_free_result_link);
  girara_list_set_free_function(database->history,    jumanji_db_free_result_link);
  girara_list_set_free_function(database->quickmarks, jumanji_db_free_quickmark);

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

  if (database->bookmark_monitor == NULL || database->history_monitor == NULL ||
      database->quickmarks_monitor == NULL) {
    goto error_free;
  }

  g_signal_connect(G_OBJECT(database->bookmark_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
  g_signal_connect(G_OBJECT(database->history_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);
  g_signal_connect(G_OBJECT(database->quickmarks_monitor), "changed",
      G_CALLBACK(cb_jumanji_db_watch_file), database);

  return database;

error_free:

  if (database != NULL) {
    g_free(database->bookmark_file);
    g_free(database->history_file);
    g_free(database->quickmarks_file);

    if (database->bookmark_monitor != NULL) {
      g_object_unref(database->bookmark_monitor);
    }

    if (database->history_monitor != NULL) {
      g_object_unref(database->history_monitor);
    }

    if (database->quickmarks_monitor != NULL) {
      g_object_unref(database->quickmarks_monitor);
    }

    girara_list_free(database->bookmarks);
    girara_list_free(database->history);
    girara_list_free(database->quickmarks);

  }

  g_free(database);

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

  girara_list_free(database->bookmarks);
  girara_list_free(database->history);
  girara_list_free(database->quickmarks);

  if (database->bookmark_monitor != NULL) {
    g_object_unref(database->bookmark_monitor);
  }

  if (database->history_monitor != NULL) {
    g_object_unref(database->history_monitor);
  }

  if (database->quickmarks_monitor != NULL) {
    g_object_unref(database->quickmarks_monitor);
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

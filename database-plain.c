/* See LICENSE file for license and copyright information */

#include <girara.h>
#include <stdlib.h>
#include <string.h>

#include "database-plain.h"

bool
db_plain_init(db_session_t* session)
{
  if (session == NULL) {
    return false;
  }

  db_plain_t* plain_session = malloc(sizeof(db_plain_t));
  if (plain_session == NULL) {
    return false;
  }

  plain_session->bookmark_file_path = NULL;
  plain_session->history_file_path  = NULL;
  plain_session->history            = NULL;
  plain_session->bookmarks          = NULL;

  session->data = plain_session;

  /* get bookmark file path */
  if (session->bookmark_file) {
    plain_session->bookmark_file_path = g_build_filename(session->bookmark_file, NULL);

    if (plain_session->bookmark_file_path == NULL) {
      goto error_free;
    }

    if (g_file_test(plain_session->bookmark_file_path, G_FILE_TEST_EXISTS) == false) {
      FILE* file = fopen(plain_session->bookmark_file_path, "w");
      if (file != NULL) {
        fclose(file);
      } else {
        goto error_free;
      }
    } else if (g_file_test(plain_session->bookmark_file_path, G_FILE_TEST_IS_REGULAR) == false) {
      goto error_free;
    }
  }

  /* get history file path */
  if (session->history_file) {
    plain_session->history_file_path = g_build_filename(session->history_file, NULL);

    if (plain_session->history_file_path == NULL) {
      goto error_free;
    }

    if (g_file_test(plain_session->history_file_path, G_FILE_TEST_EXISTS) == false) {
      FILE* file = fopen(plain_session->history_file_path, "w");
      if (file != NULL) {
        fclose(file);
      } else {
        goto error_free;
      }
    } else if (g_file_test(plain_session->history_file_path, G_FILE_TEST_IS_REGULAR) == false) {
      goto error_free;
    }
  }

  /* read files */
  plain_session->bookmarks = db_plain_read_urls_from_file(plain_session->bookmark_file_path);
  plain_session->history   = db_plain_read_urls_from_file(plain_session->history_file_path);

  return true;

error_free:

  if (plain_session != NULL) {
    g_free(plain_session->bookmark_file_path);
    g_free(plain_session->history_file_path);
    free(plain_session);
  }

  session->data = NULL;

  return false;
}

void
db_plain_close(db_session_t* session)
{
  if (session == NULL) {
    return;
  }

  if (session->bookmark_file) {
    g_free(session->bookmark_file);
  }

  if (session->history_file) {
    g_free(session->history_file);
  }

  if (session->data) {
    db_plain_t* plain_session = (db_plain_t*) session->data;
    g_free(plain_session->bookmark_file_path);
    g_free(plain_session->history_file_path);
    girara_list_free(plain_session->bookmarks);
    girara_list_free(plain_session->history);
    free(session->data);
  }

  free(session);
}

girara_list_t*
db_plain_bookmark_find(db_session_t* session, const char* input)
{
  if (session == NULL || session->data == NULL || input == NULL) {
    return NULL;
  }

  db_plain_t* plain_session = (db_plain_t*) session->data;

  if (plain_session->bookmarks == NULL) {
    return NULL;
  }

  return db_plain_filter_url_list(plain_session->bookmarks, input);
}

void
db_plain_bookmark_remove(db_session_t* session, const char* url)
{
  if (session == NULL || session->data == NULL || url == NULL) {
    return;
  }

  db_plain_t* plain_session = (db_plain_t*) session->data;

  if (plain_session->bookmarks == NULL) {
    return;
  }

  /* remove url from list */
  girara_list_iterator_t* iter = girara_list_iterator(plain_session->bookmarks);

  do {
    db_result_link_t* link = (db_result_link_t*) girara_list_iterator_data(iter);

    if (strcmp(link->url, url) == 0) {
      girara_list_remove(plain_session->bookmarks, link);
    }
  } while (girara_list_iterator_next(iter) != NULL);

  girara_list_iterator_free(iter);

  db_plain_write_urls_to_file(plain_session->bookmark_file_path, plain_session->bookmarks, false);
}

void
db_plain_bookmark_add(db_session_t* session, const char* url, const char* title)
{
  if (session == NULL || session->data == NULL || url == NULL) {
    return;
  }

  db_plain_t* plain_session = (db_plain_t*) session->data;

  if (plain_session->bookmarks == NULL) {
    return;
  }

  /* search for existing entry and update it */
  girara_list_iterator_t* iter = girara_list_iterator(plain_session->bookmarks);
  do {
    db_result_link_t* link = (db_result_link_t*) girara_list_iterator_data(iter);
    if (link == NULL) {
      continue;
    }

    if (strstr(link->url, url) != NULL) {
      g_free(link->title);
      link->title = title ? g_strdup(title) : NULL;
      db_plain_write_urls_to_file(plain_session->bookmark_file_path, plain_session->bookmarks, false);
      girara_list_iterator_free(iter);
      return;
    }
  } while (girara_list_iterator_next(iter) != NULL);
  girara_list_iterator_free(iter);

  /* add url to list */
  db_result_link_t* link = (db_result_link_t*) malloc(sizeof(db_result_link_t));
  if (link == NULL) {
    return;
  }

  link->url     = g_strdup(url);
  link->title   = g_strdup(title);
  link->visited = 0;

  girara_list_append(plain_session->bookmarks, link);

  /* write to file */
  db_plain_write_urls_to_file(plain_session->bookmark_file_path, plain_session->bookmarks, false);
}

girara_list_t*
db_plain_history_find(db_session_t* session, const char* input)
{
  if (session == NULL || session->data == NULL || input == NULL) {
    return NULL;
  }

  db_plain_t* plain_session = (db_plain_t*) session->data;

  if (plain_session->history == NULL) {
    return NULL;
  }

  return db_plain_filter_url_list(plain_session->history, input);
}

void
db_plain_history_add(db_session_t* session, const char* url, const char* title)
{
  if (session == NULL || session->data == NULL || url == NULL) {
    return;
  }

  db_plain_t* plain_session = (db_plain_t*) session->data;

  if (plain_session->history == NULL) {
    return;
  }

  /* search for existing entry and update it */
  girara_list_iterator_t* iter = girara_list_iterator(plain_session->history);
  do {
    db_result_link_t* link = (db_result_link_t*) girara_list_iterator_data(iter);
    if (link == NULL) {
      continue;
    }

    if (strstr(link->url, url) != NULL) {
      g_free(link->title);
      link->title   = title ? g_strdup(title) : NULL;
      link->visited = time(NULL);
      db_plain_write_urls_to_file(plain_session->history_file_path, plain_session->history, true);
      girara_list_iterator_free(iter);
      return;
    }
  } while (girara_list_iterator_next(iter) != NULL);
  girara_list_iterator_free(iter);

  /* add url to list */
  db_result_link_t* link = (db_result_link_t*) malloc(sizeof(db_result_link_t));
  if (link == NULL) {
    return;
  }

  link->url     = g_strdup(url);
  link->title   = g_strdup(title);
  link->visited = time(NULL);

  girara_list_append(plain_session->history, link);

  /* write to file */
  db_plain_write_urls_to_file(plain_session->history_file_path, plain_session->history, true);
}

void
db_plain_history_clean(db_session_t* session, unsigned int age)
{
  if (session == NULL || session->data == NULL) {
    return;
  }

  db_plain_t* plain_session = (db_plain_t*) session->data;

  if (plain_session->history == NULL) {
    return;
  }

  /* remove url from list */
  girara_list_iterator_t* iter = girara_list_iterator(plain_session->history);

  int visited = time(NULL) - age;
  do {
    db_result_link_t* link = (db_result_link_t*) girara_list_iterator_data(iter);

    if (link->visited >= visited) {
      girara_list_remove(plain_session->history, link);
    }
  } while (girara_list_iterator_next(iter) != NULL);

  girara_list_iterator_free(iter);

  db_plain_write_urls_to_file(plain_session->bookmark_file_path, plain_session->history, false);
}

girara_list_t*
db_plain_read_urls_from_file(const char* filename)
{
  if (filename == NULL) {
    return NULL;
  }

  /* open file */
  FILE* file = girara_file_open(filename, "r");
  if (file == NULL) {
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    fclose(file);
    return NULL;
  }

  /* read lines */
  char* line = NULL;
  while ((line = girara_file_read_line(file)) != NULL) {
    /* skip empty lines */
    if (strlen(line) == 0) {
      continue;
    }

    /* parse line */
    gchar** argv = NULL;
    gint    argc = 0;

    if (g_shell_parse_argv(line, &argc, &argv, NULL) != FALSE) {
      db_result_link_t* link = malloc(sizeof(link));
      if (link == NULL) {
        continue;
      }

      link->url     = g_strdup(argv[0]);
      link->title   = (argc > 1) ? g_strdup(argv[1]) : NULL;
      link->visited = (argc > 2) ? atoi(argv[2])     : 0;

      girara_list_append(list, link);
    }

    g_strfreev(argv);
  }

  fclose(file);

  return list;
}

void
db_plain_write_urls_to_file(const char* filename, girara_list_t* urls, bool visited)
{
  if (filename == NULL || urls == NULL) {
    return;
  }

  /* open file */
  FILE* file = girara_file_open(filename, "w");
  if (file == NULL) {
    return;
  }

  if (girara_list_size(urls) > 0) {
    girara_list_iterator_t* iter = girara_list_iterator(urls);
    do {
      db_result_link_t* link = (db_result_link_t*) girara_list_iterator_data(iter);
      if (link == NULL) {
        continue;
      }

      /* write url */
      fwrite(link->url, sizeof(char), strlen(link->url), file);

      /* write title */
      char* title_quoted = g_shell_quote(link->title);
      char* text = g_strdup_printf(" %s", title_quoted);
      fwrite(text, sizeof(char), strlen(text), file);
      g_free(title_quoted);
      g_free(text);

      /* write last visit */
      if (visited == true) {
        char* text = g_strdup_printf(" %d", link->visited);
        fwrite(text, sizeof(char), strlen(text), file);
        g_free(text);
      }

      fwrite("\n", sizeof(char), 1, file);
    } while (girara_list_iterator_next(iter) != NULL);
    girara_list_iterator_free(iter);
  }

  fclose(file);
}

girara_list_t*
db_plain_filter_url_list(girara_list_t* list, const char* input)
{
  if (list == NULL || girara_list_size(list) == 0) {
    return NULL;
  }

  girara_list_t* new_list = girara_list_new();
  if (new_list == NULL) {
    girara_list_free(list);
    return NULL;
  }

  girara_list_set_free_function(new_list, db_free_result_link);
  girara_list_iterator_t* iter = girara_list_iterator(list);

  do {
    db_result_link_t* link = (db_result_link_t*) girara_list_iterator_data(iter);

    if (strstr(link->url, input) != NULL) {
      /* duplicate entry */
      db_result_link_t* link_dup = malloc(sizeof(db_result_link_t));
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

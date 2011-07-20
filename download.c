/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdlib.h>

#include "download.h"

bool
jumanji_download_file(jumanji_t* jumanji, WebKitDownload* download)
{
  if (jumanji == NULL || jumanji->ui.session == NULL || download == NULL) {
    return false;
  }

  const char* uri = webkit_download_get_uri(download);
  char* file      = (char*) webkit_download_get_suggested_filename(download);

  if (uri == NULL) {
    return false;
  }

  /* get download dir */
  char* download_dir_tmp = girara_setting_get(jumanji->ui.session, "download-dir");
  if (download_dir_tmp == NULL) {
    return false;
  }

  char* download_dir = girara_fix_path(download_dir_tmp);
  g_free(download_dir_tmp);

  if (download_dir == NULL) {
    return false;
  }

  /* build filename */
  if (file == NULL) {
    file = g_strdup(uri);
    for (unsigned int i = 0; i < strlen(file); i++) {
      if (file[i] == '/') {
        file[i] = '-';
      }
    }
  } else {
    file = g_strdup(file);
  }

  char* filename = g_build_filename(download_dir, file, NULL);

  if (filename == NULL) {
    g_free(download_dir);
    return false;
  }

  /* check for custom download command */
  char* download_command = girara_setting_get(jumanji->ui.session, "download-command");
  if (download_command != NULL) {
    char* command = strstr(download_command, "%s");
    if (command == NULL) {
      girara_error("Invalid download command: %s", download_command);
      g_free(download_command);
      return false;
    }

    /* one argument (uri) */
    if ((command = strstr(command, "%s")) == NULL) {
      command = g_strdup_printf(download_command, uri);
    /* two arguments (uri, filename) */
    } else {
      command = g_strdup_printf(download_command, uri, filename);
    }

    g_spawn_command_line_async(command, NULL);

    free(download_command);
    g_free(command);
  /* internal download handler */
  } else {
    jumanji_download_t* jumanji_download = malloc(sizeof(jumanji_download_t));
    if (jumanji_download == NULL) {
      return false;
    }

    /* set up download */
    char* file_uri = g_filename_to_uri(filename, NULL, NULL);
    webkit_download_set_destination_uri(download, file_uri);
    g_free(file_uri);

    g_signal_connect(G_OBJECT(download), "notify::status",   G_CALLBACK(cb_jumanji_download_status),   jumanji_download);
    g_signal_connect(G_OBJECT(download), "notify::progress", G_CALLBACK(cb_jumanji_download_progress), jumanji_download);

    jumanji_download->size     = -1;
    jumanji_download->uri      = uri;
    jumanji_download->download = download;
    jumanji_download->file     = file;
    jumanji_download->jumanji  = jumanji;
    jumanji_download->widget   = gtk_label_new(NULL);

    if (jumanji_download->widget == NULL) {
      g_free(filename);
      g_free(download_dir);
      return false;
    }

    gtk_box_pack_end(GTK_BOX(jumanji->downloads.widget), jumanji_download->widget, FALSE, FALSE, 0);
    gtk_widget_show(jumanji_download->widget);

    /* start download */
    webkit_download_start(download);
  }

  g_free(filename);
  g_free(download_dir);

  return true;
}

void
jumanji_download_free(jumanji_download_t* download)
{
  if (download == NULL) {
    return;
  }

  if (download->jumanji != NULL && download->widget != NULL) {
    gtk_container_remove(GTK_CONTAINER(download->jumanji->downloads.widget), download->widget);
  }

  free(download->file);
  free(download);
}

void
cb_jumanji_download_status(WebKitDownload* download, GParamSpec* pspec, jumanji_download_t* jumanji_download)
{
  if (download == NULL || jumanji_download == NULL) {
    return;
  }

  switch (webkit_download_get_status(download)) {
    case WEBKIT_DOWNLOAD_STATUS_STARTED:
      gtk_label_set_markup(GTK_LABEL(jumanji_download->widget), "hello");
      jumanji_download->size = webkit_download_get_total_size(download);
      break;
    case WEBKIT_DOWNLOAD_STATUS_CANCELLED:
      girara_error("Download cancelled: %s", jumanji_download->uri);
      jumanji_download_free(jumanji_download);
      break;
    case WEBKIT_DOWNLOAD_STATUS_FINISHED:
      girara_info("Download finished: %s", jumanji_download->uri);
      jumanji_download_free(jumanji_download);
      break;
    case WEBKIT_DOWNLOAD_STATUS_ERROR:
      girara_error("Download error: %s", jumanji_download->uri);
      jumanji_download_free(jumanji_download);
      break;
    default:
      break;
  }
}

void
cb_jumanji_download_progress(WebKitDownload* download, GParamSpec* pspec, jumanji_download_t* jumanji_download)
{
  if (download == NULL || jumanji_download == NULL) {
    return;
  }

  double progress = webkit_download_get_progress(download);
  fprintf(stderr, "download status (%s) %d%%\n", jumanji_download->uri, (int) (progress * 100.0));
}

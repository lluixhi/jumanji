/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdlib.h>

#include "download.h"
#include <girara/settings.h>
#include <girara/utils.h>
#include <girara/session.h>
#include <girara/datastructures.h>

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
  char* download_dir_tmp = NULL;
  girara_setting_get(jumanji->ui.session, "download-dir", &download_dir_tmp);
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
  char* download_command = NULL;
  girara_setting_get(jumanji->ui.session, "download-command", &download_command);
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

    girara_notify(jumanji->ui.session, GIRARA_INFO, "Started download: %s", filename);

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

    jumanji_download->size        = -1;
    jumanji_download->uri         = uri;
    jumanji_download->download    = download;
    jumanji_download->file        = file;
    jumanji_download->jumanji     = jumanji;

    if (jumanji_download_create_widget(jumanji, jumanji_download) == false) {
      free(jumanji_download);
      g_free(filename);
      g_free(download_dir);
      return false;
    }

    jumanji_download_set_status(jumanji_download);

    /* add to list */
    girara_list_append(jumanji->downloads.list, jumanji_download);

    /* start download */
    webkit_download_start(download);

    girara_notify(jumanji->ui.session, GIRARA_INFO, "Started download: %s",
        file);
  }

  g_free(filename);
  g_free(download_dir);

  return true;
}

void
jumanji_download_free(void* data)
{
  if (data == NULL) {
    return;
  }

  jumanji_download_t* download = (jumanji_download_t*) data;

  if (download->jumanji != NULL && download->widget.main != NULL) {
    gtk_container_remove(GTK_CONTAINER(download->jumanji->downloads.widget), download->widget.main);
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

  jumanji_download_set_status(jumanji_download);
}

void
cb_jumanji_download_progress(WebKitDownload* download, GParamSpec* pspec, jumanji_download_t* jumanji_download)
{
  if (download == NULL || jumanji_download == NULL) {
    return;
  }

  jumanji_download_set_status(jumanji_download);
}

bool
jumanji_download_create_widget(jumanji_t* jumanji, jumanji_download_t* download)
{
  if (jumanji == NULL || download == NULL || jumanji->downloads.widget == NULL) {
    return false;
  }

#if (GTK_MAJOR_VERSION == 3)
  download->widget.main     = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_set_homogeneous(GTK_BOX(download->widget.main), TRUE);
#else
  download->widget.main     = gtk_vbox_new(TRUE, 0);
#endif
  download->widget.filename = gtk_label_new(NULL);
  download->widget.status   = gtk_label_new(NULL);

  if (download->widget.main     == NULL ||
      download->widget.filename == NULL ||
      download->widget.status   == NULL) {
    return false;
  }

  /* set style */
#if (GTK_MAJOR_VERSION == 3)
  gtk_widget_override_color(GTK_WIDGET(download->widget.filename), GTK_STATE_NORMAL, &(jumanji->ui.session->style.inputbar_foreground));
  gtk_widget_override_font(GTK_WIDGET(download->widget.filename),  jumanji->ui.session->style.font);
  gtk_widget_override_color(GTK_WIDGET(download->widget.status), GTK_STATE_NORMAL, &(jumanji->ui.session->style.statusbar_foreground));
  gtk_widget_override_font(GTK_WIDGET(download->widget.status),  jumanji->ui.session->style.font);
#else
  gtk_widget_modify_fg(GTK_WIDGET(download->widget.filename), GTK_STATE_NORMAL, &(jumanji->ui.session->style.inputbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(download->widget.filename),  jumanji->ui.session->style.font);
  gtk_widget_modify_fg(GTK_WIDGET(download->widget.status), GTK_STATE_NORMAL, &(jumanji->ui.session->style.statusbar_foreground));
  gtk_widget_modify_font(GTK_WIDGET(download->widget.status),  jumanji->ui.session->style.font);
#endif

  /* set properties */
  gtk_misc_set_alignment(GTK_MISC(download->widget.filename), 0.0, 0.0);
  gtk_misc_set_alignment(GTK_MISC(download->widget.status),   0.0, 0.0);
  gtk_misc_set_padding(GTK_MISC(download->widget.filename),       4, 0);
  gtk_misc_set_padding(GTK_MISC(download->widget.status),         4, 0);

  gtk_box_pack_start(GTK_BOX(download->widget.main), download->widget.filename, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(download->widget.main), download->widget.status,   FALSE, FALSE, 0);
  gtk_widget_show_all(download->widget.main);

  gtk_box_pack_start(GTK_BOX(jumanji->downloads.widget), download->widget.main, FALSE, FALSE, 0);

  return true;
}

void
jumanji_download_set_status(jumanji_download_t* download)
{
  if (download == NULL || download->download == NULL) {
    return;
  }

  /* download progress */
  double progress    = webkit_download_get_progress(download->download);
  int percent        = (int) (progress * 100.0);
  char* percent_text = g_strdup_printf("%d%%", percent);

  /* download status */
  char* status = NULL;
  switch (webkit_download_get_status(download->download)) {
    case WEBKIT_DOWNLOAD_STATUS_STARTED:
      download->size = webkit_download_get_total_size(download->download);
      break;
    case WEBKIT_DOWNLOAD_STATUS_CANCELLED:
      status = "Cancelled";
      break;
    case WEBKIT_DOWNLOAD_STATUS_FINISHED:
      status = "Finished";
      break;
    case WEBKIT_DOWNLOAD_STATUS_ERROR:
      status = "Error";
      break;
    default:
      break;
  }

  if (status != NULL && download->jumanji != NULL && download->jumanji->ui.session != NULL) {
    girara_notify(download->jumanji->ui.session, GIRARA_INFO, "%s download: %s",
        status, download->file);
  }

  /* update entry */
  char* text = g_strdup_printf("%s - %s", (status != NULL) ? status : percent_text, download->uri);

  gtk_label_set_text(GTK_LABEL(download->widget.filename), download->file);
  gtk_label_set_text(GTK_LABEL(download->widget.status),   text);

  g_free(text);
  g_free(percent_text);
}

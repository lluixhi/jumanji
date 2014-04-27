/* See LICENSE file for license and copyright information */

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <webkit/webkit.h>

#include "jumanji.h"

typedef struct jumanji_download_s
{
  char* file; /**> File name */
  const char* uri; /**> Download uri */
  int64_t size; /**> Size of the downloaded file */
  WebKitDownload* download; /**> Webkit download object */
  jumanji_t* jumanji; /**> Jumanji session */

  struct
  {
    GtkWidget* main; /**> Webkit widget */
    GtkWidget* filename; /**> Filename */
    GtkWidget* status; /**> Status of the download */
  } widget;
} jumanji_download_t;

/**
 * Download a file
 *
 * @param jumanji Jumanji session
 * @param download Webkit download object
 * @return true if no error occured processing
 */
bool jumanji_download_file(jumanji_t* jumanji, WebKitDownload* download);

/**
 * Frees a jumanji download object
 *
 * @param data Jumani download object
 */
void jumanji_download_free(void* data);

/**
 * Creates the widget that will be displayed for the jumanji download
 *
 * @param jumanji The jumanji session
 * @param download The jumanji download
 * @return false if an error occured
 */
bool jumanji_download_create_widget(jumanji_t* jumanji, jumanji_download_t* download);

/**
 * Updates the status of a download
 *
 * @param download The jumanji download
 */
void jumanji_download_set_status(jumanji_download_t* download);

#endif // DOWNLOAD_H

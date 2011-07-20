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
  GtkWidget* widget; /**> Webkit widget */
  jumanji_t* jumanji; /**> Jumanji session */
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
 * @param download Jumani download object
 */
void jumanji_download_free(jumanji_download_t* download);

/**
 * Callback for the status of the download
 *
 * @param download Webkit download object
 * @param pspec -
 * @param jumanji_download Jumanji download object
 */
void cb_jumanji_download_status(WebKitDownload* download, GParamSpec* pspec, jumanji_download_t* jumanji_download);

/**
 * Callback for the progress of the download
 *
 * @param download Webkit download object
 * @param pspec -
 * @param jumanji_download Jumanji download object
 */
void cb_jumanji_download_progress(WebKitDownload* download, GParamSpec* pspec, jumanji_download_t* jumanji_download);

#endif // DOWNLOAD_H

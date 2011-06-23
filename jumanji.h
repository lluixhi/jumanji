/* See LICENSE file for license and copyright information */

#ifndef JUMANJI_H
#define JUMANJI_H

#include <stdbool.h>
#include <girara.h>

typedef struct jumanji_s
{
  struct
  {
    girara_session_t* session; /**> girara interface session */

    struct
    {
      girara_statusbar_item_t* buffer; /**> buffer statusbar entry */
      girara_statusbar_item_t* url; /**> url statusbar entry */
    } statusbar;
  } ui;

  struct
  {
    gchar* config_dir; /**> Path to the configuration directory */
    gchar* data_dir; /**> Path to the data directory */
  } config;

  struct
  {
    girara_mode_t normal; /**> Normal mode */
  } modes;
} jumanji_t;

/**
 * Initializes jumanji
 *
 * @param argc Number of arguments
 * @param argv Values of arguments
 * @return jumanji session object or NULL if jumanji could not been initialized
 */
jumanji_t* jumanji_init(int argc, char* argv[]);

/**
 * Free jumanji session
 *
 * @param jumanji The jumanji session
 */
void jumanji_free(jumanji_t* jumanji);

#endif // JUMANJI_H

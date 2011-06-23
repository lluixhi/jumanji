/* See LICENSE file for license and copyright information */

#include "config.h"
#include "jumanji.h"

void
config_load_default(jumanji_t* jumanji)
{
  if (jumanji == NULL) {
    return;
  }
}

void
config_load_file(jumanji_t* jumanji, char* path)
{
  if (jumanji == NULL) {
    return;
  }

  girara_config_parse(jumanji->ui.session, path);
}

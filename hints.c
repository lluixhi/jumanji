/* See LICENSE file for license and copyright information */

#include "hints.h"

bool
sc_hints(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  /*jumanji_t* jumanji = session->global.data;*/
  g_return_val_if_fail(argument != NULL, false);

  return false;
}

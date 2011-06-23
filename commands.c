/* See LICENSE file for license and copyright information */

#include "commands.h"
#include "jumanji.h"

bool
cmd_close(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);

  return true;
}

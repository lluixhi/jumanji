/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include "completion.h"
#include "utils.h"

girara_completion_t*
cc_open(girara_session_t* session, char* input)
{
  g_return_val_if_fail(session != NULL, NULL);
  g_return_val_if_fail(session->global.data != NULL, NULL);

  return NULL;
}

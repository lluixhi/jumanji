/* See LICENSE file for license and copyright information */

#include "config.h"
#include "commands.h"
#include "completion.h"
#include "jumanji.h"
#include "shortcuts.h"

void
config_load_default(jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->ui.session == NULL) {
    return;
  }

  girara_session_t* gsession = jumanji->ui.session;

  /* mode settings */
  jumanji->modes.normal = gsession->modes.normal;

#define NORMAL jumanji->modes.normal

  /* define default shortcuts */
  girara_shortcut_add(gsession, 0, GDK_KEY_o, NULL, sc_focus_inputbar, NORMAL, 0, &(":open "));
  girara_shortcut_add(gsession, 0, GDK_KEY_t, NULL, sc_focus_inputbar, NORMAL, 0, &(":tabopen "));

  /* define default inputbar commands */
  girara_inputbar_command_add(gsession, "open",    "o",  cmd_open,    cc_open, "Open URL in the current tab");
  girara_inputbar_command_add(gsession, "tabopen", "t",  cmd_tabopen, cc_open, "Open URL in a new tab");
}

void
config_load_file(jumanji_t* jumanji, char* path)
{
  if (jumanji == NULL) {
    return;
  }

  girara_config_parse(jumanji->ui.session, path);
}

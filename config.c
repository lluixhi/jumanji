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

  int int_value              = 0;
  char* string_value         = NULL;
  girara_session_t* gsession = jumanji->ui.session;

  /* mode settings */
  jumanji->modes.normal = gsession->modes.normal;

#define NORMAL jumanji->modes.normal

	girara_mode_set(gsession, NORMAL);

  /* zathura settings */
  string_value = "http://pwmt.org";
  girara_setting_add(gsession, "homepage",    string_value, STRING, true, "Home page",   NULL);
  int_value = 40;
  girara_setting_add(gsession, "scroll-step", &int_value,   INT,    true, "Scroll step", NULL);

  /* define default shortcuts */
  girara_shortcut_add(gsession, 0,                GDK_KEY_o,      NULL, sc_focus_inputbar, NORMAL, 0,          &(":open "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_t,      NULL, sc_focus_inputbar, NORMAL, 0,          &(":tabopen "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_h,      NULL, sc_scroll,         NORMAL, LEFT,       NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_j,      NULL, sc_scroll,         NORMAL, DOWN,       NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_k,      NULL, sc_scroll,         NORMAL, UP,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_l,      NULL, sc_scroll,         NORMAL, RIGHT,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Left,   NULL, sc_scroll,         NORMAL, LEFT,       NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Up,     NULL, sc_scroll,         NORMAL, UP,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Down,   NULL, sc_scroll,         NORMAL, DOWN,       NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Right,  NULL, sc_scroll,         NORMAL, RIGHT,      NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_d,      NULL, sc_scroll,         NORMAL, HALF_DOWN,  NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_u,      NULL, sc_scroll,         NORMAL, HALF_UP,    NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_f,      NULL, sc_scroll,         NORMAL, FULL_DOWN,  NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_b,      NULL, sc_scroll,         NORMAL, FULL_UP,    NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_space,  NULL, sc_scroll,         NORMAL, FULL_DOWN,  NULL);
  girara_shortcut_add(gsession, GDK_SHIFT_MASK,   GDK_KEY_space,  NULL, sc_scroll,         NORMAL, FULL_UP,    NULL);
  girara_shortcut_add(gsession, 0,                0,              "gg", sc_scroll,         NORMAL, TOP,        NULL);
  girara_shortcut_add(gsession, 0,                0,              "G",  sc_scroll,         NORMAL, BOTTOM,     NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_0,      NULL, sc_scroll,         NORMAL, BEGIN,      NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_dollar, NULL, sc_scroll,         NORMAL, END,        NULL);

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

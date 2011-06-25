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
  int_value = 10;
  girara_setting_add(gsession, "zoom-step",   &int_value,   INT,    true, "Zoom step", NULL);

  /* define default shortcuts */
  girara_shortcut_add(gsession, 0,                GDK_KEY_d,        NULL, girara_sc_tab_close,    NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_K,        NULL, girara_sc_tab_navigate, NORMAL, GIRARA_NEXT,     NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_J,        NULL, girara_sc_tab_navigate, NORMAL, GIRARA_PREVIOUS, NULL);
  girara_shortcut_add(gsession, 0,                0,                "gh", sc_goto_homepage,       NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                0,                "gH", sc_goto_homepage,       NORMAL, NEW_TAB,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_slash,    NULL, sc_focus_inputbar,      NORMAL, 0,               &("/"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_question, NULL, sc_focus_inputbar,      NORMAL, APPEND_URL,      &("?"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_colon,    NULL, sc_focus_inputbar,      NORMAL, 0,               &(":"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_o,        NULL, sc_focus_inputbar,      NORMAL, 0,               &(":open "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_O,        NULL, sc_focus_inputbar,      NORMAL, APPEND_URL,      &(":open "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_t,        NULL, sc_focus_inputbar,      NORMAL, 0,               &(":tabopen "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_f,        NULL, sc_follow_link,         NORMAL, DEFAULT,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_F,        NULL, sc_follow_link,         NORMAL, NEW_TAB,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_T,        NULL, sc_focus_inputbar,      NORMAL, APPEND_URL,      &(":tabopen "));
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_i,        NULL, sc_navigate_history,    NORMAL, NEXT,            NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_o,        NULL, sc_navigate_history,    NORMAL, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_L,        NULL, sc_navigate_history,    NORMAL, NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_H,        NULL, sc_navigate_history,    NORMAL, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_r,        NULL, sc_reload,              NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_R,        NULL, sc_reload,              NORMAL, BYPASS_CACHE,    NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_h,        NULL, sc_scroll,              NORMAL, LEFT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_j,        NULL, sc_scroll,              NORMAL, DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_k,        NULL, sc_scroll,              NORMAL, UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_l,        NULL, sc_scroll,              NORMAL, RIGHT,           NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Left,     NULL, sc_scroll,              NORMAL, LEFT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Up,       NULL, sc_scroll,              NORMAL, UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Down,     NULL, sc_scroll,              NORMAL, DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Right,    NULL, sc_scroll,              NORMAL, RIGHT,           NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_d,        NULL, sc_scroll,              NORMAL, HALF_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_u,        NULL, sc_scroll,              NORMAL, HALF_UP,         NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_f,        NULL, sc_scroll,              NORMAL, FULL_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_b,        NULL, sc_scroll,              NORMAL, FULL_UP,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_space,    NULL, sc_scroll,              NORMAL, FULL_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_SHIFT_MASK,   GDK_KEY_space,    NULL, sc_scroll,              NORMAL, FULL_UP,         NULL);
  girara_shortcut_add(gsession, 0,                0,                "gg", sc_scroll,              NORMAL, TOP,             NULL);
  girara_shortcut_add(gsession, 0,                0,                "G",  sc_scroll,              NORMAL, BOTTOM,          NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_0,        NULL, sc_scroll,              NORMAL, BEGIN,           NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_dollar,   NULL, sc_scroll,              NORMAL, END,             NULL);
  girara_shortcut_add(gsession, 0,                0,                "gf", sc_toggle_source_mode,  NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_plus,     "zI", sc_zoom,                NORMAL, ZOOM_IN,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_minus,    "zO", sc_zoom,                NORMAL, ZOOM_OUT,        NULL);
  girara_shortcut_add(gsession, 0,                0,                "z0", sc_zoom,                NORMAL, DEFAULT,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Z,        NULL, sc_zoom,                NORMAL, ZOOM_SPECIFIC,   NULL);

  /* define default inputbar commands */
  girara_inputbar_command_add(gsession, "open",    "o",  cmd_open,    cc_open, "Open URL in the current tab");
  girara_inputbar_command_add(gsession, "tabopen", "t",  cmd_tabopen, cc_open, "Open URL in a new tab");

  /* add shortcut mappings */
  girara_shortcut_mapping_add(gsession, "focus_inputbar", sc_focus_inputbar);
  girara_shortcut_mapping_add(gsession, "quit",           sc_quit);
  girara_shortcut_mapping_add(gsession, "scroll",         sc_scroll);
  girara_shortcut_mapping_add(gsession, "zoom",           sc_zoom);

  /* add argument mappings */
  girara_argument_mapping_add(gsession, "left",       LEFT);
  girara_argument_mapping_add(gsession, "right",      RIGHT);
  girara_argument_mapping_add(gsession, "up",         UP);
  girara_argument_mapping_add(gsession, "down",       DOWN);
  girara_argument_mapping_add(gsession, "full_up",    FULL_UP);
  girara_argument_mapping_add(gsession, "full_down",  FULL_DOWN);
  girara_argument_mapping_add(gsession, "half_up",    HALF_UP);
  girara_argument_mapping_add(gsession, "half_down",  HALF_DOWN);
  girara_argument_mapping_add(gsession, "top",        TOP);
  girara_argument_mapping_add(gsession, "bottom",     BOTTOM);
  girara_argument_mapping_add(gsession, "begin",      BEGIN);
  girara_argument_mapping_add(gsession, "end",        END);
  girara_argument_mapping_add(gsession, "in",         ZOOM_IN);
  girara_argument_mapping_add(gsession, "out",        ZOOM_OUT);
  girara_argument_mapping_add(gsession, "default",    DEFAULT);
  girara_argument_mapping_add(gsession, "specific",   ZOOM_SPECIFIC);

  /* add config handles */
  girara_config_handle_add(gsession, "searchengine", cmd_search_engine);
}

void
config_load_file(jumanji_t* jumanji, char* path)
{
  if (jumanji == NULL) {
    return;
  }

  girara_config_parse(jumanji->ui.session, path);
}

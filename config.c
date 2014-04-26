/* See LICENSE file for license and copyright information */

#include "callbacks.h"
#include "config.h"
#include "commands.h"
#include "completion.h"
#include "hints.h"
#include "jumanji.h"
#include "marks.h"
#include "quickmarks.h"
#include "shortcuts.h"
#include "session.h"

#include <girara/settings.h>
#include <girara/session.h>
#include <girara/shortcuts.h>
#include <girara/commands.h>
#include <girara/config.h>

void
config_load_default(jumanji_t* jumanji)
{
  if (jumanji == NULL || jumanji->ui.session == NULL) {
    return;
  }

  int int_value              = 0;
  char* string_value         = NULL;
  bool bool_value            = true;
  girara_session_t* gsession = jumanji->ui.session;

  /* mode settings */
  jumanji->modes.normal = gsession->modes.normal;

#define NORMAL jumanji->modes.normal

  girara_mode_set(gsession, NORMAL);

  /* jumanji settings */
  bool_value = true;
  girara_setting_add(gsession, "adblock",                     &bool_value,  BOOLEAN, true,  "Block ads",                   NULL, NULL);
  bool_value = true;
  girara_setting_add(gsession, "auto-set-proxy",              &bool_value,  BOOLEAN, true,  "Set proxy on initialization", NULL, NULL);
  bool_value = true;
  girara_setting_add(gsession, "close-window-with-last-tab",  &bool_value,  BOOLEAN, false, "Close window with last tab", NULL, NULL);
  string_value = "primary";
  girara_setting_add(gsession, "default-clipboard",           string_value, STRING,  false, "Default clipboard",           NULL, NULL);
  string_value = "~/dl";
  girara_setting_add(gsession, "download-dir",                string_value, STRING,  false, "Download directory",          NULL, NULL);
  string_value = NULL;
  girara_setting_add(gsession, "download-command",            string_value, STRING,  false, "Download command",            NULL, NULL);
  string_value = "http://pwmt.org";
  girara_setting_add(gsession, "homepage",                    string_value, STRING,  false, "Home page",                   NULL, NULL);
  int_value = 40;
  girara_setting_add(gsession, "scroll-step",                 &int_value,   INT,     true,  "Scroll step",                 NULL, NULL);
  int_value = 10;
  girara_setting_add(gsession, "zoom-step",                   &int_value,   INT,     true,  "Zoom step",                   NULL, NULL);
  bool_value = true;
  girara_setting_add(gsession, "save-session-at-exit",        &bool_value,  BOOLEAN, true,  "Save open tabs at exit",              NULL, NULL);
  bool_value = true;
  girara_setting_add(gsession, "load-session-at-startup",     &bool_value,  BOOLEAN, true,  "Load the default session at startup", NULL, NULL);
  bool_value = true;
  girara_setting_add(gsession, "focus-new-tabs",              &bool_value,  BOOLEAN, true,  "Focus newly opened tabs",     NULL, NULL);

  /* hint settings */
  string_value =
    "padding: 0px 2px;"
    "-webkit-border-radius: 4px;"
    "font-family: monospace;"
    "font-size: 12px;"
    "font-weight: normal;"
    "color: #ffffff;"
    "border: 1px solid #3D3D3D;"
    "opacity: 0.85;"
    "background-color: #1F7DA0;";

  girara_setting_add(gsession, "hint-css", string_value, STRING,  false, "CSS of one hint node",          NULL, NULL);

  /* webkit settings */
  bool_value = true;
  girara_setting_add(gsession, "auto-load-images",            &bool_value,   BOOLEAN, false, "Load images automatically",             cb_settings_webkit, NULL);
  bool_value = true;
  girara_setting_add(gsession, "auto-shrink-images",          &bool_value,   BOOLEAN, false, "Shrink standalone images to fit",       cb_settings_webkit, NULL);
  string_value = "serif";
  girara_setting_add(gsession, "cursive-font-family",         &string_value, STRING,  false, "Default cursive font family",           cb_settings_webkit, NULL);
  string_value = "iso-8859-1";
  girara_setting_add(gsession, "default-encoding",            &string_value, STRING , false, "Default encoding",                      cb_settings_webkit, NULL);
  string_value = "sans-serif";
  girara_setting_add(gsession, "default-font-family",         &string_value, STRING , false, "Default font family",                   cb_settings_webkit, NULL);
  int_value = 12;
  girara_setting_add(gsession, "default-font-size",           &int_value,    INT,     false, "Default font size",                     cb_settings_webkit, NULL);
  int_value = 10;
  girara_setting_add(gsession, "default-monospace-font-size", &int_value,    INT,     false, "Default monospace font size",           cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "enable-caret-browsing",       &bool_value,   BOOLEAN, false, "Whether to enable caret browsing mode", cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "enable-developer-extras",     &bool_value,   BOOLEAN, false, "Enable webkit developer extensions",    cb_settings_webkit, NULL);
  bool_value = true;
  girara_setting_add(gsession, "enable-java-applet",          &bool_value,   BOOLEAN, false, "Enable java applets",                   cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "enable-page-cache",           &bool_value,   BOOLEAN, false, "Enable page caching",                   cb_settings_webkit, NULL);
  bool_value = true;
  girara_setting_add(gsession, "enable-plugins",              &bool_value,   BOOLEAN, false, "Enable plugins",                        cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "enable-private-browsing",     &bool_value,   BOOLEAN, false, "Enable private browsing",               cb_settings_webkit, NULL);
  bool_value = true;
  girara_setting_add(gsession, "enable-scripts",              &bool_value,   BOOLEAN, false, "Enable scripts",                        cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "enable-spell-checking",       &bool_value,   BOOLEAN, false, "Enable spell checking",                 cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "enforce-96-dpi",              &bool_value,   BOOLEAN, false, "Enforce a resolution of 96 DPI",        cb_settings_webkit, NULL);
  string_value = "serif";
  girara_setting_add(gsession, "fantasy-font-family",         &string_value, STRING , false, "Fantasy font family",                   cb_settings_webkit, NULL);
  bool_value = false;
  girara_setting_add(gsession, "full-content-zoom",           &bool_value,   BOOLEAN, false, "Full-content zoom",                     cb_settings_webkit, NULL);
  int_value = 5;
  girara_setting_add(gsession, "minimum-font-size",           &int_value,    INT,     false, "Minimum font size",                     cb_settings_webkit, NULL);
  string_value = "monospace";
  girara_setting_add(gsession, "monospace-font-family",       &string_value, STRING,  false, "Monospace font family",                 cb_settings_webkit, NULL);
  bool_value = true;
  girara_setting_add(gsession, "print-backgrounds",           &bool_value,   BOOLEAN, false, "Print background images",               cb_settings_webkit, NULL);
  bool_value = true;
  girara_setting_add(gsession, "resizable-text-areas",        &bool_value,   BOOLEAN, false, "Allow resizable text areas",            cb_settings_webkit, NULL);
  string_value = "sans-serif";
  girara_setting_add(gsession, "sans-serif-font-family",      &string_value, STRING,  false, "Sans-serif font family",                cb_settings_webkit, NULL);
  string_value = "serif";
  girara_setting_add(gsession, "serif-font-family",           &string_value, STRING,  false, "Serif font family",                     cb_settings_webkit, NULL);
  string_value = NULL;
  girara_setting_add(gsession, "spell-checking-languages",    &string_value, STRING,  false, "Spell checking languages",              cb_settings_webkit, NULL);
  string_value = NULL;
  girara_setting_add(gsession, "user-agent",                  &string_value, STRING,  false, "User agent",                            cb_settings_webkit, NULL);
  string_value = NULL;
  girara_setting_add(gsession, "user-stylesheet-uri",         &string_value, STRING,  false, "Custom stylesheet",                     cb_settings_webkit, NULL);

  /* define default shortcuts */
  girara_shortcut_add(gsession, 0,                GDK_KEY_apostrophe, NULL, sc_mark_evaluate,         NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_d,          NULL, girara_sc_tab_close,      NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_K,          NULL, girara_sc_tab_navigate,   NORMAL, GIRARA_NEXT,     NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_J,          NULL, girara_sc_tab_navigate,   NORMAL, GIRARA_PREVIOUS, NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gh", sc_goto_homepage,         NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gH", sc_goto_homepage,         NORMAL, NEW_TAB,         NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gu", sc_goto_parent_directory, NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gU", sc_goto_parent_directory, NORMAL, DEFAULT,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_f,          NULL, sc_hints,                 NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_F,          NULL, sc_hints,                 NORMAL, NEW_TAB,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_slash,      NULL, sc_focus_inputbar,        NORMAL, 0,               &("/"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_question,   NULL, sc_focus_inputbar,        NORMAL, APPEND_URL,      &("?"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_colon,      NULL, sc_focus_inputbar,        NORMAL, 0,               &(":"));
  girara_shortcut_add(gsession, 0,                GDK_KEY_o,          NULL, sc_focus_inputbar,        NORMAL, 0,               &(":open "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_O,          NULL, sc_focus_inputbar,        NORMAL, APPEND_URL,      &(":open "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_t,          NULL, sc_focus_inputbar,        NORMAL, 0,               &(":tabopen "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_T,          NULL, sc_focus_inputbar,        NORMAL, APPEND_URL,      &(":tabopen "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_w,          NULL, sc_focus_inputbar,        NORMAL, 0,               &(":winopen "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_W,          NULL, sc_focus_inputbar,        NORMAL, APPEND_URL,      &(":winopen "));
  girara_shortcut_add(gsession, 0,                GDK_KEY_m,          NULL, sc_mark_add,              NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_i,          NULL, sc_navigate_history,      NORMAL, NEXT,            NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_o,          NULL, sc_navigate_history,      NORMAL, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_L,          NULL, sc_navigate_history,      NORMAL, NEXT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_H,          NULL, sc_navigate_history,      NORMAL, PREVIOUS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_p,          NULL, sc_put,                   NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_P,          "gP", sc_put,                   NORMAL, NEW_TAB,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_M,          NULL, sc_quickmark_add,         NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                0,                  "go", sc_quickmark_evaluate,    NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gn", sc_quickmark_evaluate,    NORMAL, NEW_TAB,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_r,          NULL, sc_reload,                NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_R,          NULL, sc_reload,                NORMAL, BYPASS_CACHE,    NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_u,          NULL, sc_restore,               NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_h,          NULL, sc_scroll,                NORMAL, LEFT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_j,          NULL, sc_scroll,                NORMAL, DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_k,          NULL, sc_scroll,                NORMAL, UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_l,          NULL, sc_scroll,                NORMAL, RIGHT,           NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Left,       NULL, sc_scroll,                NORMAL, LEFT,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Up,         NULL, sc_scroll,                NORMAL, UP,              NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Down,       NULL, sc_scroll,                NORMAL, DOWN,            NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Right,      NULL, sc_scroll,                NORMAL, RIGHT,           NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_d,          NULL, sc_scroll,                NORMAL, HALF_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_u,          NULL, sc_scroll,                NORMAL, HALF_UP,         NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_f,          NULL, sc_scroll,                NORMAL, FULL_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_b,          NULL, sc_scroll,                NORMAL, FULL_UP,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_space,      NULL, sc_scroll,                NORMAL, FULL_DOWN,       NULL);
  girara_shortcut_add(gsession, GDK_SHIFT_MASK,   GDK_KEY_space,      NULL, sc_scroll,                NORMAL, FULL_UP,         NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gg", sc_scroll,                NORMAL, TOP,             NULL);
  girara_shortcut_add(gsession, 0,                0,                  "G",  sc_scroll,                NORMAL, BOTTOM,          NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_0,          NULL, sc_scroll,                NORMAL, BEGIN,           NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_dollar,     NULL, sc_scroll,                NORMAL, END,             NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_n,          NULL, sc_search,                NORMAL, FORWARDS,        NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_N,          NULL, sc_search,                NORMAL, BACKWARDS,       NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_A,          NULL, sc_toggle_bookmark,       NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, GDK_CONTROL_MASK, GDK_KEY_p,          NULL, sc_toggle_proxy,          NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                0,                  "gf", sc_toggle_source_mode,    NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_y,          NULL, sc_yank,                  NORMAL, 0,               NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_plus,       "zI", sc_zoom,                  NORMAL, ZOOM_IN,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_minus,      "zO", sc_zoom,                  NORMAL, ZOOM_OUT,        NULL);
  girara_shortcut_add(gsession, 0,                0,                  "z0", sc_zoom,                  NORMAL, DEFAULT,         NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_Z,          NULL, sc_zoom,                  NORMAL, ZOOM_SPECIFIC,   NULL);
  girara_shortcut_add(gsession, 0,                GDK_KEY_e,          NULL, sc_toggle_stylesheet,     NORMAL, 0,               NULL);

  /* define default inputbar commands */
  girara_inputbar_command_add(gsession, "bmark",         NULL,    cmd_bookmark_add,      NULL,    "Add a bookmark");
  girara_inputbar_command_add(gsession, "delbmarks",     NULL,    cmd_bookmark_delete,   NULL,    "Delete a bookmark");
  girara_inputbar_command_add(gsession, "delmarks",      "delm",  cmd_marks_delete,      NULL,    "Delete the specified marks");
  girara_inputbar_command_add(gsession, "delqmarks",     "delqm", cmd_quickmarks_delete, NULL,    "Add quickmark");
  girara_inputbar_command_add(gsession, "downloads",     NULL,    cmd_downloads,         NULL,    "Show downloads");
  girara_inputbar_command_add(gsession, "mark",          NULL,    cmd_marks_add,         NULL,    "Mark current location within the web page");
  girara_inputbar_command_add(gsession, "open",          "o",     cmd_open,              cc_open, "Open URL in the current tab");
  girara_inputbar_command_add(gsession, "print",         NULL,    cmd_print,             NULL,    "Show print dialog");
  girara_inputbar_command_add(gsession, "qmark",         NULL,    cmd_quickmarks_add,    NULL,    "Add quickmark");
  girara_inputbar_command_add(gsession, "stop",          NULL,    cmd_stop,              NULL,    "Stop loading the current page");
  girara_inputbar_command_add(gsession, "tabopen",       "t",     cmd_tabopen,           cc_open, "Open URL in a new tab");
  girara_inputbar_command_add(gsession, "winopen",       "w",     cmd_winopen,           cc_open, "Open URL in a new window");
  girara_inputbar_command_add(gsession, "sessionsave",   "save",  cmd_sessionsave,       NULL,    "Save the current session");
  girara_inputbar_command_add(gsession, "sessionload",   "load",  cmd_sessionload,       NULL,    "Load a specific session");

  /* special commands */
  girara_special_command_add(gsession, '/', cmd_search, true, 0, NULL),

  /* add shortcut mappings */
  girara_shortcut_mapping_add(gsession, "focus_inputbar",   sc_focus_inputbar);
  girara_shortcut_mapping_add(gsession, "goto_homepage",    sc_goto_homepage);
  girara_shortcut_mapping_add(gsession, "goto_parent_dir",  sc_goto_parent_directory);
  girara_shortcut_mapping_add(gsession, "navigate_history", sc_navigate_history);
  girara_shortcut_mapping_add(gsession, "put",              sc_put);
  girara_shortcut_mapping_add(gsession, "quit",             sc_quit);
  girara_shortcut_mapping_add(gsession, "reload",           sc_reload);
  girara_shortcut_mapping_add(gsession, "restore",          sc_restore);
  girara_shortcut_mapping_add(gsession, "scroll",           sc_scroll);
  girara_shortcut_mapping_add(gsession, "show_source",      sc_toggle_source_mode);
  girara_shortcut_mapping_add(gsession, "proxy",            sc_toggle_proxy);
  girara_shortcut_mapping_add(gsession, "plugins",          sc_toggle_plugins);
  girara_shortcut_mapping_add(gsession, "user_stylesheet",  sc_toggle_stylesheet);
  girara_shortcut_mapping_add(gsession, "close",            girara_sc_tab_close);
  girara_shortcut_mapping_add(gsession, "search",           sc_search);
  girara_shortcut_mapping_add(gsession, "bookmark",         sc_toggle_bookmark);
  girara_shortcut_mapping_add(gsession, "hints",            sc_hints);
  girara_shortcut_mapping_add(gsession, "qmark_add",        sc_quickmark_add);
  girara_shortcut_mapping_add(gsession, "qmark_eval",       sc_quickmark_evaluate);
  girara_shortcut_mapping_add(gsession, "mark_add",         sc_mark_add);
  girara_shortcut_mapping_add(gsession, "mark_eval",        sc_mark_evaluate);
  girara_shortcut_mapping_add(gsession, "tab_navigate",     sc_tab_navigate);

  girara_shortcut_mapping_add(gsession, "yank",             sc_yank);
  girara_shortcut_mapping_add(gsession, "zoom",             sc_zoom);

  /* add argument mappings */
  girara_argument_mapping_add(gsession, "append_url", APPEND_URL);
  girara_argument_mapping_add(gsession, "begin",      BEGIN);
  girara_argument_mapping_add(gsession, "bottom",     BOTTOM);
  girara_argument_mapping_add(gsession, "default",    DEFAULT);
  girara_argument_mapping_add(gsession, "down",       DOWN);
  girara_argument_mapping_add(gsession, "end",        END);
  girara_argument_mapping_add(gsession, "full_down",  FULL_DOWN);
  girara_argument_mapping_add(gsession, "full_up",    FULL_UP);
  girara_argument_mapping_add(gsession, "half_down",  HALF_DOWN);
  girara_argument_mapping_add(gsession, "half_up",    HALF_UP);
  girara_argument_mapping_add(gsession, "in",         ZOOM_IN);
  girara_argument_mapping_add(gsession, "left",       LEFT);
  girara_argument_mapping_add(gsession, "next",       NEXT);
  girara_argument_mapping_add(gsession, "no_cache",   BYPASS_CACHE);
  girara_argument_mapping_add(gsession, "out",        ZOOM_OUT);
  girara_argument_mapping_add(gsession, "previous",   PREVIOUS);
  girara_argument_mapping_add(gsession, "right",      RIGHT);
  girara_argument_mapping_add(gsession, "specific",   ZOOM_SPECIFIC);
  girara_argument_mapping_add(gsession, "tab",        NEW_TAB);
  girara_argument_mapping_add(gsession, "top",        TOP);
  girara_argument_mapping_add(gsession, "up",         UP);
  girara_argument_mapping_add(gsession, "forward",    FORWARDS);
  girara_argument_mapping_add(gsession, "backward",   BACKWARDS);

  /* add config handles */
  girara_config_handle_add(gsession, "searchengine", cmd_search_engine);
  girara_config_handle_add(gsession, "proxy",        cmd_proxy);
}

void
config_load_file(jumanji_t* jumanji, char* path)
{
  if (jumanji == NULL) {
    return;
  }

  girara_config_parse(jumanji->ui.session, path);
}

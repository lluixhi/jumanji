/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <JavaScriptCore/JavaScript.h>

#include "userscripts.h"
#include <girara/datastructures.h>
#include <girara/utils.h>

#define USER_SCRIPT_HEADER ".*//.*(==UserScript==.*//.*==/UserScript==).*"
#define USER_SCRIPT_VAR_VAL_PAIR "//\\s+@(?<name>\\S+)(\\s+(?<value>.*))?"

/* GreaseMonkey's GM_functions by Jim Tuttle (C) 2009
 * http://userscripts.org/scripts/show/41441
 */
static const char* gm_functions =
"// ==UserScript=="
"// @name           gm_functions"
"// @namespace      *"
"// @description    Replicated GreaseMonkey's GM_ functions for GreaseKit."
"// @include        *"
"// @exclude       "
"// ==/UserScript=="
""
"/*"
"  History:"
"  1.0   2009/01/30  Initial release, based on other people's posts at http://groups.google.com/group/greasekit-users/,"
"                    so they deserve the credit"
"  1.0.1 2009/03/08  Changed the way GM_addStyle and GM_log are defined, as it they simply didn't work."
"                    Thanks to \"samuel365\" for pointing this out."
"*/"
""
"if(typeof GM_getValue === \"undefined\") {"
"  GM_getValue = function(name){"
"    var nameEQ = escape(\"_greasekit\" + name) + \"=\", ca = document.cookie.split(';');"
"    for (var i = 0, c; i < ca.length; i++) {"
"      var c = ca[i];"
"      while (c.charAt(0) == ' ') c = c.substring(1, c.length);"
"      if (c.indexOf(nameEQ) == 0) {"
"        var value = unescape(c.substring(nameEQ.length, c.length));"
"        //alert(name + \": \" + value);"
"        return value;"
"      }"
"    }"
"    return null;"
"  }"
"}"
""
"if(typeof GM_setValue === \"undefined\") {"
"  GM_setValue = function( name, value, options ){"
"    options = (options || {});"
"    if ( options.expiresInOneYear ){"
"      var today = new Date();"
"      today.setFullYear(today.getFullYear()+1, today.getMonth, today.getDay());"
"      options.expires = today;"
"    }"
"    var curCookie = escape(\"_greasekit\" + name) + \"=\" + escape(value) +"
"    ((options.expires) ? \"; expires=\" + options.expires.toGMTString() : \"\") +"
"    ((options.path)    ? \"; path=\"    + options.path : \"\") +"
"    ((options.domain)  ? \"; domain=\"  + options.domain : \"\") +"
"    ((options.secure)  ? \"; secure\" : \"\");"
"    document.cookie = curCookie;"
"  }"
"}"
""
"if(typeof GM_xmlhttpRequest === \"undefined\") {"
"  GM_xmlhttpRequest = function(/* object */ details) {"
"    details.method = details.method.toUpperCase() || \"GET\";"
"    if(!details.url) {"
"      throw(\"GM_xmlhttpRequest requires an URL.\");"
"      return;"
"    }"
"    // build XMLHttpRequest object"
"    var oXhr, aAjaxes = [];"
"    if(typeof ActiveXObject !== \"undefined\") {"
"      var oCls = ActiveXObject;"
"      aAjaxes[aAjaxes.length] = {cls:oCls, arg:\"Microsoft.XMLHTTP\"};"
"      aAjaxes[aAjaxes.length] = {cls:oCls, arg:\"Msxml2.XMLHTTP\"};"
"      aAjaxes[aAjaxes.length] = {cls:oCls, arg:\"Msxml2.XMLHTTP.3.0\"};"
"    }"
"    if(typeof XMLHttpRequest !== \"undefined\")"
"      aAjaxes[aAjaxes.length] = {cls:XMLHttpRequest, arg:undefined};"
"    for (var i=aAjaxes.length; i--; )"
"      try{"
"  oXhr = new aAjaxes[i].cls(aAjaxes[i].arg);"
"  if(oXhr) break;"
"      } catch(e) {}"
"    // run it"
"    if(oXhr) {"
"      if(\"onreadystatechange\" in details)"
"  oXhr.onreadystatechange = function()"
"    { details.onreadystatechange(oXhr) };"
"      if(\"onload\" in details)"
"  oXhr.onload = function() { details.onload(oXhr) };"
"      if(\"onerror\" in details)"
"  oXhr.onerror = function() { details.onerror(oXhr) };"
"      oXhr.open(details.method, details.url, true);"
"      if(\"headers\" in details)"
"  for (var header in details.headers)"
"    oXhr.setRequestHeader(header, details.headers[header]);"
"      if(\"data\" in details)"
"  oXhr.send(details.data);"
"      else"
"  oXhr.send();"
"    }"
"    else {"
"      throw (\"This Browser is not supported, please upgrade.\");"
"    }"
"  }"
"}"
""
"if(typeof GM_addStyle === \"undefined\") {"
"  GM_addStyle = function(/* String */ styles) {"
"    var oStyle = document.createElement(\"style\");"
"    oStyle.setAttribute(\"type\", \"text/css\");"
"    oStyle.appendChild(document.createTextNode(styles));"
"    document.getElementsByTagName(\"head\")[0].appendChild(oStyle);"
"  }"
"}"
""
"if(typeof GM_log === \"undefined\") {"
"  GM_log = function(log) {"
"    if(console)"
"      console.log(log);"
"    else"
"      alert(log);"
"  }"
"}";

girara_list_t*
user_script_load_dir(const char* path)
{
  /* create list */
  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  girara_list_set_free_function(list, user_script_free);

  /* open directory */
  GDir* dir = g_dir_open(path, 0, NULL);
  if (dir == NULL) {
    /* Return an empty list if we are not able to open/read the user scripts
     * directory */
    return list;
  }

  /* read files */
  const char* file = NULL;

  while ((file = g_dir_read_name(dir)) != NULL) {
    char* filepath = g_build_filename(path, file, NULL);

    if (g_file_test(filepath, G_FILE_TEST_IS_REGULAR) == TRUE) {
      user_script_t* user_script = user_script_load_file(filepath);
      if (user_script != NULL) {
        girara_list_append(list, user_script);
        girara_list_set_free_function(list, user_script_free);
        girara_info("loaded user script: %s", user_script->name ? user_script->name : filepath);
      } else {
        girara_error("could not parse user script: %s", filepath);
      }
    }

    g_free(filepath);
  }

  g_dir_close(dir);

  return list;
}

user_script_t*
user_script_load_file(const char* path)
{
  /* init values */
  char* name                  = NULL;
  char* description           = NULL;
  girara_list_t* include      = girara_list_new2(free);
  girara_list_t* exclude      = girara_list_new2(free);
  bool load_on_document_start = false;

  if (include == NULL || exclude == NULL) {
    girara_list_free(include);
    girara_list_free(exclude);

    return NULL;
  }

  /* read file */
  char* content = girara_file_read(path);
  if (content == NULL) {
    girara_list_free(include);
    girara_list_free(exclude);
    return NULL;
  }

  /* parse header */
  GMatchInfo* match_info;
  GRegex* regex = g_regex_new(USER_SCRIPT_HEADER, G_REGEX_MULTILINE | G_REGEX_DOTALL, 0, NULL);

  g_regex_match(regex, content, 0, &match_info);

  /* get header */
  if (g_match_info_get_match_count(match_info) > 0) {
    gchar* header = g_match_info_fetch(match_info, 0);

    /* parse header */
    GMatchInfo* header_match_info;
    GRegex* header_regex = g_regex_new(USER_SCRIPT_VAR_VAL_PAIR, 0, 0, NULL);

    g_regex_match(header_regex, header, 0, &header_match_info);

    while (g_match_info_matches(header_match_info) == TRUE) {
      char* header_name  = g_match_info_fetch_named(header_match_info, "name");
      char* header_value = g_match_info_fetch_named(header_match_info, "value");

      if (g_strcmp0(header_name, "name") == 0) {
        name = g_strdup(header_value);
      } else if (g_strcmp0(header_name, "description") == 0) {
        description = g_strdup(header_value);
      } else if (g_strcmp0(header_name, "include") == 0) {
        /* update url for further processing */
        GRegex* regex = g_regex_new("\\*", 0, 0, NULL);
        char* tmp = g_regex_replace(regex, header_value, -1, 0, ".*", 0, NULL);
        girara_list_append(include, tmp);
        g_regex_unref(regex);
      } else if (g_strcmp0(header_name, "exclude") == 0) {
        /* update url for further processing */
        GRegex* regex = g_regex_new("\\*", 0, 0, NULL);
        char* tmp = g_regex_replace(regex, header_value, -1, 0, ".*", 0, NULL);
        girara_list_append(exclude, tmp);
        g_regex_unref(regex);
      } else if (g_strcmp0(header_name, "run-at") == 0) {
        if (g_strcmp0(header_value, "document-start") == 0) {
          load_on_document_start = true;
        }
      }

      g_free(header_value);
      g_free(header_name);
      g_match_info_next(header_match_info, NULL);
    }

    g_match_info_free(header_match_info);
    g_regex_unref(header_regex);

    g_free(header);
  /* invalid / non-existing header */
  } else {
    g_match_info_free(match_info);
    g_regex_unref(regex);
    free(content);
    girara_list_free(include);
    girara_list_free(exclude);
    return NULL;
  }

  g_regex_unref(regex);
  g_match_info_free(match_info);

  /* create user script object */
  user_script_t* user_script = malloc(sizeof(user_script_t));
  if (user_script == NULL) {
    girara_list_free(include);
    girara_list_free(exclude);
    free(content);
    return NULL;
  }

  user_script->name                   = name;
  user_script->description            = description;
  user_script->content                = content;
  user_script->include                = include;
  user_script->exclude                = exclude;
  user_script->load_on_document_start = load_on_document_start;

  return user_script;
}

void
user_script_free(void* data)
{
  if (data == NULL) {
    return;
  }

  user_script_t* user_script = (user_script_t*) data;

  free(user_script->name);
  free(user_script->description);
  free(user_script->content);

  /* free object */
  free(user_script);
}

void
user_script_inject(WebKitWebView* web_view, user_script_t* user_script)
{
  if (user_script == NULL) {
    return;
  }

  user_script_inject_text(web_view, user_script->content);
}

void
user_script_inject_text(WebKitWebView* web_view, const char* text)
{
  if (web_view == NULL || text == NULL) {
    return;
  }

  WebKitWebFrame* frame = webkit_web_view_get_main_frame(web_view);

  if (frame == NULL) {
    return;
  }

  JSContextRef context = webkit_web_frame_get_global_context(frame);

  if (context == NULL) {
    return;
  }

  JSObjectRef object = JSContextGetGlobalObject(context);

  if (object == NULL) {
    return;
  }

  JSStringRef script = JSStringCreateWithUTF8CString(text);

  if (script == NULL) {
    return;
  }

  JSEvaluateScript(context, script, object, NULL, 0, NULL);
  JSStringRelease(script);
}

void
user_script_init_tab(jumanji_tab_t* tab, girara_list_t* user_scripts)
{
  if (tab == NULL || tab->web_view == NULL || user_scripts == NULL) {
    return;
  }

  g_signal_connect(G_OBJECT(tab->web_view), "notify::load-status",
      G_CALLBACK(cb_user_script_tab_load_status), user_scripts);
}

void
cb_user_script_tab_load_status(WebKitWebView* web_view, GParamSpec* pspec,
    girara_list_t* user_scripts)
{
  if (web_view == NULL || user_scripts == NULL ||
      girara_list_size(user_scripts) == 0) {
    return;
  }

  /* check status */
  WebKitLoadStatus status = webkit_web_view_get_load_status(web_view);

  static bool loaded_gm_functions = false;
  if (status == WEBKIT_LOAD_PROVISIONAL) {
    loaded_gm_functions = false;
  } else if (status != WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT &&
      status != WEBKIT_LOAD_FINISHED) {
    return;
  }

  /* get website uri */
  const char* uri = webkit_web_view_get_uri(web_view);

  /* check all user scripts */
  girara_list_iterator_t* iter = girara_list_iterator(user_scripts);
  do {
    user_script_t* user_script = (user_script_t*) girara_list_iterator_data(iter);
    if (user_script == NULL) {
      continue;
    }

    /* do not load user script by default */
    bool load_user_script = false;

    /* do not accidentally load a script multiple times */
    if ((status == WEBKIT_LOAD_FIRST_VISUALLY_NON_EMPTY_LAYOUT &&
        user_script->load_on_document_start == false) ||
        (status == WEBKIT_LOAD_FINISHED &&
         user_script->load_on_document_start == true)) {
      continue;
    }

    int n_included = girara_list_size(user_script->include);
    int n_excluded = girara_list_size(user_script->exclude);

    if (n_included == 0) {
      load_user_script = true;
    }

    /* check if script is excluded on web site */
    for (unsigned int i = 0; i < n_excluded; i++) {
      char* exclude = girara_list_nth(user_script->exclude, i);

      GRegex* regex = g_regex_new(exclude, 0, 0, NULL);
      if (g_regex_match(regex, uri, 0, NULL) == TRUE) {
        load_user_script = false;
      }
      g_regex_unref(regex);
    }

    /* check if script is included on web site */
    if (load_user_script == false) {
      for (unsigned int i = 0; i < n_included; i++) {
        char* include = girara_list_nth(user_script->include, i);

        GRegex* regex = g_regex_new(include, 0, 0, NULL);
        if (g_regex_match(regex, uri, 0, NULL) == TRUE) {
          load_user_script = true;
        }
        g_regex_unref(regex);
      }
    }

    /* load user script */
    if (load_user_script == true) {
      if (loaded_gm_functions == false) {
        /* load GreaseMonkey's GM_ functions */
        user_script_inject_text(web_view, gm_functions);
        loaded_gm_functions = true;
      }

      user_script_inject(web_view, user_script);
    }
  } while (girara_list_iterator_next(iter));
  girara_list_iterator_free(iter);
}

/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
#include <girara/datastructures.h>
#include <girara/utils.h>

#include "adblock.h"

girara_list_t*
adblock_filter_load_dir(const char* path)
{
  /* create list */
  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  girara_list_set_free_function(list, adblock_filter_free);

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
      adblock_filter_t* filter = adblock_filter_load(filepath);
      if (filter != NULL) {
        girara_list_append(list, filter);
        girara_info("[adblock] loaded filter: %s", filter->name ? filter->name : filepath);
      } else {
        girara_error("[adblock] could not load filter: %s", filepath);
      }
    }

    g_free(filepath);
  }

  g_dir_close(dir);

  return list;
}

adblock_filter_t*
adblock_filter_load(const char* path)
{
  if (path == NULL) {
    return NULL;
  }

  girara_list_t* pattern = girara_list_new();

  if (pattern == NULL) {
    return NULL;
  }

  girara_list_t* exceptions = girara_list_new();

  if (exceptions == NULL) {
    girara_list_free(pattern);
    return NULL;
  }

  girara_list_t* css_rules = girara_list_new();

  if (css_rules == NULL) {
    girara_list_free(pattern);
    girara_list_free(exceptions);
    return NULL;
  }

  girara_list_set_free_function(pattern,    adblock_rule_free);
  girara_list_set_free_function(exceptions, adblock_rule_free);
  girara_list_set_free_function(css_rules,  adblock_rule_free);

  /* read file */
  FILE* file = girara_file_open(path, "r");

  if (file == NULL) {
    return NULL;
  }

  /* init filter */
  adblock_filter_t* filter = malloc(sizeof(adblock_filter_t));
  if (filter == NULL) {
    girara_list_free(pattern);
    girara_list_free(exceptions);
    fclose(file);
    return NULL;
  }

  filter->name       = g_strdup(path);
  filter->pattern    = pattern;
  filter->exceptions = exceptions;
  filter->css_rules  = css_rules;

  /* read lines */
  char* line = NULL;
  while ((line = girara_file_read_line(file)) != NULL) {
    adblock_rule_parse(filter, line);
    free(line);
  }

  fclose(file);

  return filter;
}

void
adblock_filter_free(void* data)
{
  if (data == NULL) {
    return;
  }

  adblock_filter_t* filter = (adblock_filter_t*) data;

  free(filter->name);

  /* free pattern */
  girara_list_free(filter->pattern);

  /* free exception rules */
  girara_list_free(filter->exceptions);
}

void
adblock_rule_free(void* data)
{
  if (data == NULL) {
    return;
  }

  adblock_rule_t* rule = (adblock_rule_t*) data;
  free(rule->pattern);
  free(rule->css_rule);
}

void
adblock_filter_init_tab(jumanji_tab_t* tab, girara_list_t* adblock_filters)
{
  if (tab == NULL || tab->web_view == NULL || adblock_filters == NULL) {
    return;
  }

  g_signal_connect(G_OBJECT(tab->web_view), "resource-request-starting",
      G_CALLBACK(cb_adblock_filter_resource_request_starting), adblock_filters);
  g_signal_connect(G_OBJECT(tab->web_view), "window-object-cleared",
      G_CALLBACK(cb_adblock_tab_window_object_cleared), adblock_filters);
}

void
cb_adblock_tab_window_object_cleared(WebKitWebView* web_view, WebKitWebFrame* frame,
    gpointer context, gpointer window_object, girara_list_t* adblock_filters)
{
  if (web_view == NULL || adblock_filters == NULL ||
      girara_list_size(adblock_filters) == 0) {
    return;
  }

  /* get web view uri */
  const char* uri = webkit_web_view_get_uri(web_view);

  GString* css = g_string_new(NULL);
  if (css == NULL) {
    return;
  }

  /* check all filter lists */
  girara_list_iterator_t* iter = girara_list_iterator(adblock_filters);
  do {
    adblock_filter_t* filter = (adblock_filter_t*) girara_list_iterator_data(iter);
    if (filter == NULL) {
      continue;
    }

    /* check css list */
    if (girara_list_size(filter->css_rules) > 0) {
      girara_list_iterator_t* css_iter = girara_list_iterator(filter->css_rules);
      do {
        adblock_rule_t* rule = girara_list_iterator_data(css_iter);
        if (rule == NULL || rule->css_rule == NULL) {
          continue;
        }

        /* if url pattern is given and does not match, continue */
        if (rule->pattern && (adblock_rule_evaluate(rule, uri) == false)) {
          continue;
        }

        g_string_append(css, rule->css_rule);
      } while (girara_list_iterator_next(css_iter));
      girara_list_iterator_free(css_iter);
    }
  } while (girara_list_iterator_next(iter));
  girara_list_iterator_free(iter);

  /* modify dom */
  WebKitDOMDocument *dom_document = webkit_web_view_get_dom_document(web_view);

  if (dom_document == NULL) {
    goto error_free;
  }

  WebKitDOMElement *style = webkit_dom_document_create_element(dom_document, "style", NULL);

  if (style == NULL) {
    goto error_free;
  }

  webkit_dom_element_set_attribute(style, "type", "text/css", NULL);
  webkit_dom_html_element_set_inner_html(WEBKIT_DOM_HTML_ELEMENT(style), css->str, NULL);

  WebKitDOMNodeList *list = webkit_dom_document_get_elements_by_tag_name(dom_document, "head");

  if (list == NULL) {
    goto error_free;
  }

  WebKitDOMNode *head = webkit_dom_node_list_item(list, 0);

  if (head == NULL) {
    goto error_free;
  }

  webkit_dom_node_append_child(head, WEBKIT_DOM_NODE(style), NULL);

error_free:

  g_string_free(css, TRUE);
}

void
cb_adblock_filter_resource_request_starting(WebKitWebView* web_view,
    WebKitWebFrame* web_frame, WebKitWebResource* web_resource,
    WebKitNetworkRequest* request, WebKitNetworkResponse* response,
    girara_list_t* adblock_filters)
{
  if (web_view == NULL || adblock_filters == NULL || web_resource == NULL ||
      request == NULL || girara_list_size(adblock_filters) == 0) {
    return;
  }

  /* get resource uri */
  const char* uri = webkit_web_resource_get_uri(web_resource);

  /* check all filter lists */
  girara_list_iterator_t* iter = girara_list_iterator(adblock_filters);
  do {
    adblock_filter_t* filter = (adblock_filter_t*) girara_list_iterator_data(iter);
    if (filter == NULL) {
      continue;
    }

    /* check exceptions */
    if (girara_list_size(filter->exceptions) > 0) {
      girara_list_iterator_t* exceptions_iter = girara_list_iterator(filter->exceptions);
      do {
        adblock_rule_t* rule = girara_list_iterator_data(exceptions_iter);
        if (rule == NULL) {
          continue;
        }

        if (adblock_rule_evaluate(rule, uri) == true) {
          girara_list_iterator_free(exceptions_iter);
          girara_list_iterator_free(iter);
          return;
        }
      } while (girara_list_iterator_next(exceptions_iter));
      girara_list_iterator_free(exceptions_iter);
    }

    /* check rules */
    if (girara_list_size(filter->pattern) > 0) {
      girara_list_iterator_t* rule_iter = girara_list_iterator(filter->pattern);
      do {
        adblock_rule_t* rule = girara_list_iterator_data(rule_iter);
        if (rule == NULL) {
          continue;
        }

        if (adblock_rule_evaluate(rule, uri) == true) {
          webkit_network_request_set_uri(request, "about:blank");
          girara_list_iterator_free(rule_iter);
          girara_list_iterator_free(iter);
          return;
        }
      } while (girara_list_iterator_next(rule_iter));
      girara_list_iterator_free(rule_iter);
    }
  } while (girara_list_iterator_next(iter));
  girara_list_iterator_free(iter);
}

void
adblock_rule_parse(adblock_filter_t* filter, const char* line)
{
  /* skip comments */
  if (filter == NULL || line == NULL || strlen(line) == 0 || line[0] == '!' ||
      line[0] == '[') {
    return;
  }

  /* create rule object */
  adblock_rule_t* rule = malloc(sizeof(adblock_rule_t));
  if (rule == NULL) {
    return;
  }

  rule->pattern  = NULL;
  rule->options  = ADBLOCK_NONE;
  rule->position = ADBLOCK_NONE;

  bool exception = false;

  /* check for exception rules */
  if (strncmp(line, "@@", 2) == 0) {
    line = line + 2;
    exception = true;
  }

  char* tmp = NULL;

  /* check for element hiding */
  bool css       = false;
  char* css_rule = NULL;
  if ((tmp = strstr(line, "##")) != NULL) {
    css_rule = g_strdup_printf("%s { display: none; }\n", tmp + 2);
    tmp      = g_strndup(line, tmp - line);
    css      = true;
  } else {
    /* check for filter options */
    char* options = strstr(line, "$");
    if (options != NULL) {
      tmp = g_strndup(line, options - line);
      /* TODO: parse options */
    } else {
      tmp = g_strdup(line);
    }
  }

  /* check for position markers */
  if (strncmp(tmp, "||", 2) == 0) {
    rule->position |= ADBLOCK_DOMAIN;

    char* t = g_strdup(tmp + 2);
    g_free(tmp);
    tmp = t;
  } else  if (strncmp(tmp, "|", 1) == 0) {
    rule->position |= ADBLOCK_BEGINNING;

    char* t = g_strdup(tmp + 1);
    g_free(tmp);
    tmp = t;
  }

  if (tmp[strlen(tmp) - 1] == '|') {
    rule->position |= ADBLOCK_ENDING;

    char* t = g_strndup(tmp, strlen(tmp) - 2);
    g_free(tmp);
    tmp = t;
  }

  /* prepare pattern */
  GString* pattern = g_string_new(NULL);

  if (pattern == NULL) {
    g_free(css_rule);
    free(rule);
    return;
  }

  /* replace seperators ^ with correspondending regex expression */
  for (unsigned int i = 0; i < strlen(tmp); i++) {
    if (tmp[i] == '^') {
      g_string_append(pattern, "[-,.,%,\\d,\\w]");
    } else if (tmp[i] == '*') {
      g_string_append(pattern, ".*");
    } else if (tmp[i] == '.') {
      g_string_append(pattern, "\\.");
    } else if (tmp[i] == '|') {
      g_string_append(pattern, "\\|");
    } else {
      g_string_append_c(pattern, tmp[i]);
    }
  }

  if (rule->position & ADBLOCK_BEGINNING) {
    g_string_append(pattern,  ".*");
    g_string_prepend(pattern, "^");
  } else if (rule->position & ADBLOCK_ENDING) {
    g_string_append(pattern, "$");
    g_string_prepend(pattern, ".*");
  } else {
    g_string_append(pattern,  ".*");
    g_string_prepend(pattern, ".*");
  }

  rule->pattern  = g_strdup(pattern->str);
  rule->css_rule = css_rule;

  g_string_free(pattern, TRUE);
  g_free(tmp);

  if (css == true) {
    girara_list_append(filter->css_rules, rule);
  } else if (exception == true) {
    girara_list_append(filter->exceptions, rule);
  } else {
    girara_list_append(filter->pattern, rule);
  }
}

bool
adblock_rule_evaluate(adblock_rule_t* rule, const char* uri)
{
  if (rule == NULL || rule->pattern == NULL || uri == NULL) {
    return false;
  }

  GRegex* regex = g_regex_new(rule->pattern, 0, 0, NULL);
  if (regex == NULL) {
    return false;
  }

  bool match = false;
  GMatchInfo* match_info;
  g_regex_match(regex, uri, 0, &match_info);

  if (g_match_info_matches(match_info) == TRUE) {
    match = true;
  }

  /*g_match_info_free(match_info);*/
  g_regex_unref(regex);

  return match;
}

/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <math.h>
#include <webkit/webkit.h>

#include "hints.h"

bool
sc_hints(girara_session_t* session, girara_argument_t* argument, unsigned int t)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
  jumanji_t* jumanji = session->global.data;
  g_return_val_if_fail(argument != NULL, false);

  if (argument->n == NEW_TAB) {
    jumanji->hints.open_mode = NEW_TAB;
  } else {
    jumanji->hints.open_mode = DEFAULT;
  }

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL) {
    return false;
  }

  hints_show(jumanji, tab);

  return false;
}

void
hints_show(jumanji_t* jumanji, jumanji_tab_t* tab)
{
  if (tab == NULL ||
      tab->web_view == NULL ||
      jumanji == NULL ||
      jumanji->ui.session == NULL) {
    return;
  }

  WebKitWebView *web_view         = WEBKIT_WEB_VIEW(tab->web_view);
  WebKitDOMDocument *dom_document = webkit_web_view_get_dom_document(web_view);

  if (dom_document == NULL) {
    return;
  }

  WebKitDOMElement *style = webkit_dom_document_create_element(dom_document, "style", NULL);

  if (style == NULL) {
    return;
  }

  webkit_dom_element_set_attribute(style, "type", "text/css", NULL);

  char* string_value = girara_setting_get(jumanji->ui.session, "hint-css");
  if (string_value == NULL) {
    return;
  }

  char* hint_div_style = g_strdup_printf(".__jumanji_hint { %s }\n", string_value);
  if (hint_div_style == NULL) {
    return;
  }

  webkit_dom_html_element_set_inner_html(WEBKIT_DOM_HTML_ELEMENT(style), hint_div_style, NULL);

  free(string_value);
  g_free(hint_div_style);

  WebKitDOMNodeList *list = webkit_dom_document_get_elements_by_tag_name(dom_document, "head");

  if (list == NULL) {
    return;
  }

  WebKitDOMNode *head = webkit_dom_node_list_item(list, 0);

  if (head == NULL) {
    return;
  }

  webkit_dom_node_append_child(head, WEBKIT_DOM_NODE(style), NULL);

  list = webkit_dom_document_get_elements_by_tag_name(dom_document, "body");

  if (list == NULL) {
    return;
  }

  WebKitDOMNode *body = webkit_dom_node_list_item(list, 0);

  /* get fields */
  WebKitDOMXPathNSResolver *ns_resolver = webkit_dom_document_create_ns_resolver(dom_document, body);

  if (ns_resolver == NULL) {
    return;
  }

  WebKitDOMXPathResult *result = webkit_dom_document_evaluate(dom_document,
      "//a | //button | //textarea | //select | //input[not(@type='hidden')]",
      WEBKIT_DOM_NODE(dom_document), ns_resolver, 7, NULL, NULL);

  if (result == NULL) {
    return;
  }

  WebKitDOMElement *hints = webkit_dom_document_create_element(dom_document, "div", NULL);

  if (hints == NULL) {
    return;
  }

  gulong snapshot_length  = webkit_dom_xpath_result_get_snapshot_length(result, NULL);
  jumanji->hints.links = g_ptr_array_sized_new(snapshot_length);

  if (jumanji->hints.links == NULL) {
    return;
  }

  /* retreive visible nodes */
  for(guint i = 0; i < snapshot_length; i++) {
    WebKitDOMNode *node = webkit_dom_xpath_result_snapshot_item(result, i, NULL);

    if (node == NULL) {
      continue;
    }

    WebKitDOMCSSStyleDeclaration* css_style = webkit_dom_element_get_style(WEBKIT_DOM_ELEMENT(node));

    if (css_style == NULL) {
      continue;
    }

    gchar *visibility = webkit_dom_css_style_declaration_get_property_value(css_style, "visibility");

    if (visibility != NULL && g_strcmp0(visibility, "hidden") == 0) {
      continue;
    }

    gchar *display = webkit_dom_css_style_declaration_get_property_value(css_style, "display");

    if (display != NULL && g_strcmp0(display, "none") == 0) {
      continue;
    }

    g_ptr_array_add(jumanji->hints.links,  node);
  }

  guint number_of_hints   = jumanji->hints.links->len;
  guint number_of_letters = log(number_of_hints)/log(26) + 1;
  gchar *id               = g_strnfill(number_of_letters, 'a');

  jumanji->hints.hints = g_ptr_array_sized_new(number_of_hints);

  if (jumanji->hints.hints == NULL) {
    return;
  }

  for(guint i = 0; i < number_of_hints; i++) {
    WebKitDOMElement *hint = webkit_dom_document_create_element(dom_document, "div", NULL);

    if (hint == NULL) {
      continue;
    }

    webkit_dom_html_element_set_inner_text(WEBKIT_DOM_HTML_ELEMENT(hint), id, NULL);
    webkit_dom_html_element_set_class_name(WEBKIT_DOM_HTML_ELEMENT(hint), "__jumanji_hint");

    guint decimal = number_of_letters - 1;
    while (id[decimal] == 'z') {
      --decimal;
    }

    id[decimal] += 1;
    for (guint j = decimal + 1; j < number_of_letters; j++) {
      id[j] = 'a';
    }

    WebKitDOMNode *node = g_ptr_array_index(jumanji->hints.links, i);

    if (node == NULL) {
      continue;
    }

    glong left = webkit_dom_element_get_offset_left(WEBKIT_DOM_ELEMENT(node));
    glong top  = webkit_dom_element_get_offset_top(WEBKIT_DOM_ELEMENT(node));

    WebKitDOMElement* parent_node = webkit_dom_element_get_offset_parent(WEBKIT_DOM_ELEMENT(node));
    while (parent_node != NULL) {
      left += webkit_dom_element_get_offset_left(parent_node);
      top  += webkit_dom_element_get_offset_top(parent_node);

      parent_node = webkit_dom_element_get_offset_parent(parent_node);
    }

    left += webkit_dom_element_get_offset_width(WEBKIT_DOM_ELEMENT(node));
    top  += webkit_dom_element_get_offset_height(WEBKIT_DOM_ELEMENT(node));

    static const gchar *hint_div_position =
      "position: absolute !important;"
      "left: %lipx !important;"
      "top:%lipx !important;"
      "width:auto !important;"
      "height:auto !important;";

    gchar *style = g_strdup_printf(hint_div_position, left, top);

    if (style == NULL) {
      return;
    }

    WebKitDOMCSSStyleDeclaration *css = webkit_dom_element_get_style(hint);

    if (css == NULL) {
      g_free(style);
      return;
    }

    webkit_dom_css_style_declaration_set_css_text(css, style, NULL);
    webkit_dom_node_append_child(WEBKIT_DOM_NODE(hints), WEBKIT_DOM_NODE(hint), NULL);

    g_ptr_array_add(jumanji->hints.hints, hint);

    g_free(style);
  }

  g_free(id);

  webkit_dom_node_append_child(head, WEBKIT_DOM_NODE(style), NULL);
  webkit_dom_node_append_child(body, WEBKIT_DOM_NODE(hints), NULL);
}

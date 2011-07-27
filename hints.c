/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
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

  /* set hints open mode */
  if (argument->n == NEW_TAB) {
    jumanji->hints.open_mode = NEW_TAB;
  } else {
    jumanji->hints.open_mode = DEFAULT;
  }

  /* show hints */
  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL) {
    return false;
  }

  hints_show(jumanji, tab);

  /* redirect signal handler */
  g_signal_handler_disconnect(G_OBJECT(session->gtk.view),
      session->signals.view_key_pressed);

  session->signals.view_key_pressed =
    g_signal_connect(
        G_OBJECT(session->gtk.view),
        "key-press-event",
        G_CALLBACK(cb_hints_key_press_event_add),
        jumanji
        );

  g_signal_handler_disconnect(G_OBJECT(session->gtk.inputbar),
      session->signals.inputbar_key_pressed);

  session->signals.inputbar_key_pressed =
    g_signal_connect(
        G_OBJECT(session->gtk.inputbar),
        "key-press-event",
        G_CALLBACK(cb_hints_key_press_event_add),
        jumanji
        );

  return false;
}

bool
cb_hints_key_press_event_add(GtkWidget* widget, GdkEventKey* event,
    jumanji_t* jumanji)
{
  g_return_val_if_fail(jumanji != NULL, FALSE);
  g_return_val_if_fail(jumanji->ui.session != NULL, FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  /* evaluate event */
  if (jumanji->hints.input == NULL) {
    jumanji->hints.input = g_string_new("");
    if (jumanji->hints.input == NULL) {
      return false;
    }
  }

  /* only allow numbers and characters */
  if (((event->keyval >= 0x30 && event->keyval <= 0x39) || (event->keyval >= 0x41 && event->keyval <= 0x5A) ||
      (event->keyval >= 0x61 && event->keyval <= 0x7A)) == true) {
    g_string_append_c(jumanji->hints.input, (char) event->keyval);
  } else if (event->keyval == GDK_KEY_Escape || event->keyval == GDK_KEY_Return) {
    hints_reset(jumanji);
    return true;
  }

  hints_update(jumanji, jumanji->hints.input->str);

  return true;
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

  hints_clear(jumanji);

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
      continue;
    }

    WebKitDOMCSSStyleDeclaration *css = webkit_dom_element_get_style(hint);

    if (css == NULL) {
      g_free(style);
      continue;
    }

    webkit_dom_css_style_declaration_set_css_text(css, style, NULL);
    webkit_dom_node_append_child(WEBKIT_DOM_NODE(hints), WEBKIT_DOM_NODE(hint), NULL);

    g_ptr_array_add(jumanji->hints.hints, hint);

    g_free(style);
  }

  g_free(id);

  webkit_dom_node_append_child(head, WEBKIT_DOM_NODE(style), NULL);
  webkit_dom_node_append_child(body, WEBKIT_DOM_NODE(hints), NULL);

  jumanji->hints.hint_style = WEBKIT_DOM_NODE(style);
  jumanji->hints.hint_box   = WEBKIT_DOM_NODE(hints);
}

void
hints_clear(jumanji_t* jumanji)
{
  if(jumanji->hints.hint_box != NULL) {
    WebKitDOMNode* parent = webkit_dom_node_get_parent_node(jumanji->hints.hint_box);
    webkit_dom_node_remove_child(parent, jumanji->hints.hint_box, NULL);
    jumanji->hints.hint_box = NULL;
  }

  if(jumanji->hints.hint_style != NULL) {
    WebKitDOMNode* parent = webkit_dom_node_get_parent_node(jumanji->hints.hint_style);
    webkit_dom_node_remove_child(parent, jumanji->hints.hint_style, NULL);
    jumanji->hints.hint_style = NULL;
  }

  if(jumanji->hints.links != NULL) {
    g_ptr_array_free(jumanji->hints.links, TRUE);
    jumanji->hints.links = NULL;
  }

  if(jumanji->hints.hints != NULL) {
    g_ptr_array_free(jumanji->hints.hints, TRUE);
    jumanji->hints.hints = NULL;
  }
}

void
hints_process(jumanji_t* jumanji, guint n)
{
  if (jumanji == NULL) {
    return;
  }

  jumanji_tab_t* tab = jumanji_tab_get_current(jumanji);

  if (tab == NULL || tab->web_view == NULL) {
    return;
  }

  WebKitDOMDocument *dom_document = webkit_web_view_get_dom_document(WEBKIT_WEB_VIEW(tab->web_view));

  if (dom_document == NULL) {
    return;
  }

  if (jumanji->hints.links == NULL || jumanji->hints.links->len < n) {
    return;
  }

  WebKitDOMNode *item = g_ptr_array_index(jumanji->hints.links, n);

  if (item == NULL) {
    return;
  }

  gchar *tag_ = webkit_dom_element_get_tag_name(WEBKIT_DOM_ELEMENT(item));
  gchar *type = webkit_dom_element_get_attribute(WEBKIT_DOM_ELEMENT(item), "type");

  GString* tag = g_string_new(tag_);
  g_string_ascii_down(tag);

  /* input field */
  if ((g_ascii_strcasecmp(tag->str, "input") == 0 &&
        (type == NULL || type[0] == 0 || g_ascii_strcasecmp(type, "search") == 0 ||
        g_ascii_strcasecmp(type, "text") == 0 || g_ascii_strcasecmp(type, "password") == 0))
      || g_ascii_strcasecmp(tag->str, "textarea") == 0) {
    webkit_dom_element_focus(WEBKIT_DOM_ELEMENT(item));
    hints_reset(jumanji);
  /* open link */
  } else {
    WebKitDOMEvent *event = webkit_dom_document_create_event(dom_document, "MouseEvents", NULL);

    gushort button = 0; /*left click*/
    if (jumanji->hints.open_mode == NEW_TAB) {
      button = 1; /*middle click*/
    }

    webkit_dom_mouse_event_init_mouse_event(
        WEBKIT_DOM_MOUSE_EVENT(event),
        "click",
        TRUE,
        TRUE,
        webkit_dom_document_get_default_view(dom_document),
        1, 0, 0, 0, 0,
        FALSE,FALSE, FALSE, FALSE,
        button,
        WEBKIT_DOM_EVENT_TARGET(item));

    webkit_dom_node_dispatch_event(item, event, NULL);

    hints_reset(jumanji);
  }

  g_string_free(tag, TRUE);
}

void
hints_update(jumanji_t* jumanji, char* input)
{
  if (jumanji == NULL || input == NULL || jumanji->hints.links == NULL) {
    return;
  }

  int number_of_hints       = jumanji->hints.links->len;
  int number_of_letters_max = log(number_of_hints) / log(26) + 1;

  if (strlen(input) == number_of_letters_max) {
    int id = 0;
    for (int i = 0; i < number_of_letters_max; i++) {
      id += (input[i] - 'a') * pow(26, number_of_letters_max - i - 1);
    }

    hints_process(jumanji, id);
  } else {
    for (int i = 0; i < number_of_hints; i++)
    {
      WebKitDOMNode *hint               = g_ptr_array_index(jumanji->hints.hints, i);
      WebKitDOMCSSStyleDeclaration *css = webkit_dom_element_get_style(WEBKIT_DOM_ELEMENT(hint));

      gchar* current_text = webkit_dom_html_element_get_inner_text(WEBKIT_DOM_HTML_ELEMENT(hint));
      if(strncmp(current_text, input, strlen(input)) != 0) {
        gchar* position = g_strdup("display: none;");
        webkit_dom_css_style_declaration_set_css_text(css, position, NULL);
        g_free(position);
      } else {
        webkit_dom_html_element_set_inner_text(WEBKIT_DOM_HTML_ELEMENT(hint), current_text + 1, NULL);
      }

      g_free(current_text);
    }
  }
}

void
hints_reset(jumanji_t* jumanji)
{
  if (jumanji == NULL) {
    return;
  }

  /* clear hints */
  hints_clear(jumanji);

  /* clear buffer */
  if (jumanji->hints.input != NULL) {
    g_string_free(jumanji->hints.input, TRUE);
    jumanji->hints.input = NULL;
  }

  /* reset signal handler */
  g_signal_handler_disconnect(G_OBJECT(jumanji->ui.session->gtk.view),
      jumanji->ui.session->signals.view_key_pressed);

  jumanji->ui.session->signals.view_key_pressed =
    g_signal_connect(
        G_OBJECT(jumanji->ui.session->gtk.view),
        "key-press-event",
        G_CALLBACK(girara_callback_view_key_press_event),
        jumanji->ui.session
        );

  g_signal_handler_disconnect(G_OBJECT(jumanji->ui.session->gtk.inputbar),
      jumanji->ui.session->signals.inputbar_key_pressed);

  jumanji->ui.session->signals.inputbar_key_pressed =
    g_signal_connect(
        G_OBJECT(jumanji->ui.session->gtk.inputbar),
        "key-press-event",
        G_CALLBACK(girara_callback_inputbar_key_press_event),
        jumanji->ui.session
        );
}

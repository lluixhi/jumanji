/* See LICENSE file for license and copyright information */

#include "commands.h"
#include "jumanji.h"

bool
cmd_buffer_delete(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);

  return true;
}

bool
cmd_open(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
	jumanji_t* jumanji = (jumanji_t*) session->global.data;

	if (girara_list_size(argument_list) == 0) {
		return false;
	}

	const char* url = (const char*) girara_list_nth(argument_list, 0);
	jumanji_tab_load_url(jumanji_tab_get_current(jumanji), url);

  return true;
}

bool
cmd_tabopen(girara_session_t* session, girara_list_t* argument_list)
{
  g_return_val_if_fail(session != NULL, false);
  g_return_val_if_fail(session->global.data != NULL, false);
	jumanji_t* jumanji = (jumanji_t*) session->global.data;

	girara_list_iterator_t* iter = girara_list_iterator(argument_list);
	do {
		const char* url = (const char*) girara_list_iterator_data(iter);
		jumanji_tab_new(jumanji, url, false);
	} while (girara_list_iterator_next(iter));
	girara_list_iterator_free(iter);

  return true;
}

/*
Copyright (©) 2003-2026 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifdef __cplusplus
extern "C" {
#endif

  const char * bibledit_get_version_number ();
  const char * bibledit_get_network_port ();
  void bibledit_initialize_library (const char * package, const char * webroot);
  void bibledit_set_touch_enabled (bool enabled);
  void bibledit_start_library ();
  const char * bibledit_get_last_page ();
  bool bibledit_is_running ();
  const char * bibledit_is_synchronizing ();
  const char * bibledit_get_external_url ();
  const char * bibledit_get_pages_to_open ();
  void bibledit_stop_library ();
  void bibledit_shutdown_library ();
  void bibledit_log (const char * message);
  void bibledit_run_on_chrome_os ();
  const char * bibledit_disable_selection_popup_chrome_os ();
  const char * bibledit_get_reference_for_accordance ();
  void bibledit_put_reference_from_accordance (const char * reference);

#ifdef __cplusplus
}
#endif

/*
Copyright (©) 2003-2023 Teus Benschop.

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


#pragma once

#include <config/libraries.h>
#include <assets/view.h>

class Assets_Header
{
public:
  Assets_Header (std::string title, void * webserver_request_in);
  ~Assets_Header ();
  Assets_Header(const Assets_Header&) = delete;
  Assets_Header operator=(const Assets_Header&) = delete;
  void jquery_touch_on ();
  void touch_css_on ();
  void notify_it_on ();
  void set_navigator ();
  void set_stylesheet ();
  void set_editor_stylesheet ();
  bool display_topbar ();
  void refresh (int seconds, std::string url = "");
  void set_fading_menu (std::string html);
  void add_bread_crumb (std::string item, std::string text);
  std::string run ();
private:
  Assets_View * m_view {nullptr};
  bool m_jquery_touch_on {false};
  bool m_touch_css_on {false};
  bool m_notify_it_on {false};
  std::vector <std::string> m_head_lines {};
  bool m_display_navigator {false};
  std::string m_included_stylesheet {};
  std::string m_included_editor_stylesheet {};
  void * m_webserver_request {nullptr};
  std::string m_fading_menu {};
  std::vector <std::pair <std::string, std::string> > m_bread_crumbs {};
};


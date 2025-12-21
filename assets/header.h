/*
Copyright (©) 2003-2025 Teus Benschop.

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

class Webserver_Request;

class Assets_Header
{
public:
  Assets_Header (const std::string& title, Webserver_Request& webserver_request_in);
  ~Assets_Header ();
  Assets_Header(const Assets_Header&) = delete;
  Assets_Header operator=(const Assets_Header&) = delete;
  void notify_on ();
  void set_navigator ();
  void set_stylesheet ();
  void set_editor_stylesheet ();
  bool display_topbar ();
  void refresh (int seconds, const std::string& url = "");
  void set_fading_menu (const std::string& html);
  void add_bread_crumb (const std::string& item, const std::string& text);
  std::string run ();
private:
  Assets_View * m_view {nullptr};
  bool m_notify_on {false};
  std::vector <std::string> m_head_lines {};
  bool m_display_navigator {false};
  std::string m_included_stylesheet {};
  std::string m_included_editor_stylesheet {};
  Webserver_Request& m_webserver_request;
  std::string m_fading_menu {};
  std::vector <std::pair <std::string, std::string> > m_bread_crumbs {};
};


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
#include <database/styles.h>

class Html_Header
{
public:
  Html_Header (void * html_text);
  Html_Header(const Html_Header&) = delete;
  Html_Header operator=(const Html_Header&) = delete;
  void search_back_link (std::string url, std::string text);
  void create (const std::vector <std::pair <std::string, std::string> > & breadcrumbs);
private:
  void * m_html_text {nullptr};
  std::string m_search_back_link_url {};
  std::string m_search_back_link_text {};
};

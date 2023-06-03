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


#include <pugixml/utils.h>
#include <filter/string.h>
#include <database/logs.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
using namespace std;
using namespace pugi;


void pugixml_utils_error_logger (void * pugi_xml_parse_result, const string & xml)
{
  xml_parse_result * result = static_cast<xml_parse_result *>(pugi_xml_parse_result);
  if (result->status == status_ok) return;
  int start = static_cast<int>(result->offset - 10);
  if (start < 0) start = 0;
  string fragment = xml.substr (static_cast<size_t>(start), 20);
  fragment = filter::strings::replace ("\n", "", fragment);
  string msg;
  msg.append (result->description());
  msg.append (" at offset ");
  msg.append (filter::strings::convert_to_string (static_cast<size_t>(result->offset)));
  msg.append (": ");
  msg.append (fragment);
  Database_Logs::log (msg);
}

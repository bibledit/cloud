/*
Copyright (©) 2003-2024 Teus Benschop.

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


#include <html/header.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/url.h>
#include <database/books.h>
#include <html/text.h>
#include <locale/translate.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop


// Class for creating a html Bible header with breadcrumbs and search box.


Html_Header::Html_Header (HtmlText& html_text):
m_html_text (html_text)
{ }


void Html_Header::search_back_link (const std::string& url, const std::string& text)
{
  m_search_back_link_url = url;
  m_search_back_link_text = text;
}


void Html_Header::create (const std::vector <std::pair <std::string, std::string> > & breadcrumbs)
{
  pugi::xml_node table_element = m_html_text.new_table ();
  pugi::xml_node table_row_element = m_html_text.new_table_row (table_element);
  pugi::xml_node table_data_element = m_html_text.new_table_data (table_row_element);
  for (const auto breadcrumb : breadcrumbs) {
    m_html_text.add_link (table_data_element, breadcrumb.second, "", breadcrumb.first, "", ' ' + breadcrumb.first + ' ');
  }
  table_data_element = m_html_text.new_table_data (table_row_element, true);
  pugi::xml_node form_element = table_data_element.append_child ("form");
  form_element.append_attribute ("action") = "/webbb/search";
  form_element.append_attribute ("method") = "GET";
  form_element.append_attribute ("name") = "search";
  form_element.append_attribute ("id") = "search";
  pugi::xml_node input_element = form_element.append_child ("input");
  input_element.append_attribute ("name") = "q";
  input_element.append_attribute ("type") = "text";
  input_element.append_attribute ("placeholder") = translate ("Search the Bible").c_str();
  input_element = form_element.append_child ("input");
  input_element.append_attribute ("type") = "image";
  input_element.append_attribute ("name") = "search";
  input_element.append_attribute ("src") = "lens.png";
  input_element = form_element.append_child ("input");
  input_element.append_attribute ("type") = "hidden";
  input_element.append_attribute ("name") = "url";
  input_element.append_attribute ("value") = m_search_back_link_url.c_str ();
  input_element = form_element.append_child ("input");
  input_element.append_attribute ("type") = "hidden";
  input_element.append_attribute ("name") = "text";
  input_element.append_attribute ("value") = m_search_back_link_text.c_str ();
}

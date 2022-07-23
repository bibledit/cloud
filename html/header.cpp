/*
Copyright (©) 2003-2022 Teus Benschop.

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
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop


using namespace pugi;


// Class for creating a html Bible header with breadcrumbs and search box.


Html_Header::Html_Header (void * html_text)
{
  m_html_text = html_text;
}


void Html_Header::search_back_link (string url, string text)
{
  m_search_back_link_url = url;
  m_search_back_link_text = text;
}


void Html_Header::create (const vector <pair <string, string> > & breadcrumbs)
{
  Html_Text * html_text = static_cast<Html_Text *>(m_html_text);
  xml_node tableElement = html_text->new_table ();
  xml_node tableRowElement = html_text->new_table_row (tableElement);
  xml_node tableDataElement = html_text->new_table_data (tableRowElement);
  for (auto breadcrumb : breadcrumbs) {
    html_text->add_link (tableDataElement, breadcrumb.second, "", breadcrumb.first, "", ' ' + breadcrumb.first + ' ');
  }
  tableDataElement = html_text->new_table_data (tableRowElement, true);
  xml_node formElement = tableDataElement.append_child ("form");
  formElement.append_attribute ("action") = "/webbb/search";
  formElement.append_attribute ("method") = "GET";
  formElement.append_attribute ("name") = "search";
  formElement.append_attribute ("id") = "search";
  xml_node inputElement = formElement.append_child ("input");
  inputElement.append_attribute ("name") = "q";
  inputElement.append_attribute ("type") = "text";
  inputElement.append_attribute ("placeholder") = translate ("Search the Bible").c_str();
  inputElement = formElement.append_child ("input");
  inputElement.append_attribute ("type") = "image";
  inputElement.append_attribute ("name") = "search";
  inputElement.append_attribute ("src") = "lens.png";
  inputElement = formElement.append_child ("input");
  inputElement.append_attribute ("type") = "hidden";
  inputElement.append_attribute ("name") = "url";
  inputElement.append_attribute ("value") = m_search_back_link_url.c_str ();
  inputElement = formElement.append_child ("input");
  inputElement.append_attribute ("type") = "hidden";
  inputElement.append_attribute ("name") = "text";
  inputElement.append_attribute ("value") = m_search_back_link_text.c_str ();
}


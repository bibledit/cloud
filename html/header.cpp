/*
Copyright (©) 2003-2021 Teus Benschop.

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
#include <pugixml/pugixml.hpp>


using namespace pugi;


// Class for creating a html Bible header with breadcrumbs and search box.


Html_Header::Html_Header (void * html_text)
{
  htmlText = html_text;
}


void Html_Header::searchBackLink (string url, string text)
{
  searchBackLinkUrl = url;
  searchBackLinkText = text;
}


void Html_Header::create (const vector <pair <string, string> > & breadcrumbs)
{
  Html_Text * html_text = (Html_Text *) htmlText;
  xml_node tableElement = html_text->newTable ();
  xml_node tableRowElement = html_text->newTableRow (tableElement);
  xml_node tableDataElement = html_text->newTableData (tableRowElement);
  for (auto breadcrumb : breadcrumbs) {
    html_text->addLink (tableDataElement, breadcrumb.second, "", breadcrumb.first, "", ' ' + breadcrumb.first + ' ');
  }
  tableDataElement = html_text->newTableData (tableRowElement, true);
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
  inputElement.append_attribute ("value") = searchBackLinkUrl.c_str ();
  inputElement = formElement.append_child ("input");
  inputElement.append_attribute ("type") = "hidden";
  inputElement.append_attribute ("name") = "text";
  inputElement.append_attribute ("value") = searchBackLinkText.c_str ();
}


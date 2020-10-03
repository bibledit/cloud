/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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


#ifndef INCLUDED_EDITOR_HTML2FORMAT_H
#define INCLUDED_EDITOR_HTML2FORMAT_H


#include <config/libraries.h>
#include <database/styles.h>
#include <pugixml/pugixml.hpp>


using namespace pugi;


class Editor_Html2Format
{
public:
  void load (string html);
  void run ();
  vector <string> texts;
  vector <string> formats;
private:
  xml_document document; // DOMDocument holding the html.
  void preprocess ();
  void postprocess ();
  void process ();
  void processNode (xml_node node);
  void openElementNode (xml_node node);
  void closeElementNode (xml_node node);
  void openInline (string className);
  string update_quill_class (string classname);
  string current_character_format;
};


#endif

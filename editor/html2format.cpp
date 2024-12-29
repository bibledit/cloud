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


#include <editor/html2format.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logs.h>
#include <pugixml/utils.h>
#include <quill/logic.h>


void Editor_Html2Format::load (std::string html)
{
  // The web editor may insert non-breaking spaces. Convert them to normal spaces.
  html = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), " ", html);
  
  // The web editor produces <hr> and other elements following the HTML specs,
  // but the pugixml XML parser needs <hr/> and similar elements.
  html = filter::strings::html2xml (html);
  
  std::string xml = "<body>" + html + "</body>";
  // Parse document such that all whitespace is put in the DOM tree.
  // See http://pugixml.org/docs/manual.html for more information.
  // It is not enough to only parse with parse_ws_pcdata_single, it really needs parse_ws_pcdata.
  // This is significant for, for example, the space after verse numbers, among other cases.
  pugi::xml_parse_result result = document.load_string (xml.c_str(), pugi::parse_ws_pcdata);
  // Log parsing errors.
  pugixml_utils_error_logger (&result, xml);
}


void Editor_Html2Format::run ()
{
  preprocess ();
  process ();
  postprocess ();
}


void Editor_Html2Format::process ()
{
  // Iterate over the children to retrieve the "p" elements, then process them.
  pugi::xml_node body = document.first_child ();
  for (pugi::xml_node node : body.children()) {
    // Process the node.
    processNode (node);
  }
}


void Editor_Html2Format::processNode (pugi::xml_node node)
{
  switch (node.type ()) {
    case pugi::node_element:
    {
      // Skip a note with class "ql-cursor" because that is an internal Quill node.
      // The user didn't insert it.
      std::string classs = node.attribute("class").value();
      if (classs == quill_caret_class) break;
      // Process node normally.
      openElementNode (node);
      for (pugi::xml_node child : node.children()) {
        processNode (child);
      }
      closeElementNode (node);
      break;
    }
    case pugi::node_pcdata:
    {
      // Add the text with the current character format to the containers.
      std::string text = node.text ().get ();
      texts.push_back(text);
      formats.push_back(current_character_format);
      break;
    }
    case pugi::node_null:
    case pugi::node_document:
    case pugi::node_cdata:
    case pugi::node_comment:
    case pugi::node_pi:
    case pugi::node_declaration:
    case pugi::node_doctype:
    default:
    {
      std::string nodename = node.name ();
      Database_Logs::log ("XML node " + nodename + " not handled while saving editor text");
      break;
    }
  }
}


void Editor_Html2Format::openElementNode (pugi::xml_node node)
{
  // The tag and class names of this element node.
  std::string tagName = node.name ();
  std::string className = update_quill_class (node.attribute ("class").value ());
  
  if (tagName == "p")
  {
    // In the editor, it may occur that the p element does not have a class.
    // Use the 'p' class in such a case.
    if (className.empty ()) className = "p";
    texts.push_back("\n");
    formats.push_back(className);
    // A new line starts: Clear the character formatting.
    current_character_format.clear();
  }
  
  if (tagName == "span")
  {
    openInline (className);
  }
}


void Editor_Html2Format::closeElementNode (pugi::xml_node node)
{
  // The tag and class names of this element node.
  std::string tagName = node.name ();
  std::string className = update_quill_class (node.attribute ("class").value ());

  if (tagName == "p")
  {
    // While editing it happens that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (className.empty()) className = "p";
    // Clear active character styles.
    current_character_format.clear();
  }
  
  if (tagName == "span")
  {
    // End of span: Clear character formatting.
    current_character_format.clear();
  }
}


void Editor_Html2Format::openInline (std::string className)
{
  current_character_format = className;
}


void Editor_Html2Format::preprocess ()
{
  texts.clear();
  formats.clear();
}


void Editor_Html2Format::postprocess ()
{
}


std::string Editor_Html2Format::update_quill_class (std::string classname)
{
  classname = filter::strings::replace (quill_class_prefix_block, std::string(), classname);
  classname = filter::strings::replace (quill_class_prefix_inline, std::string(), classname);
  return classname;
}

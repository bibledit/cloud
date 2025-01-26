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


#include <editor/styles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/config/bible.h>
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


std::string Editor_Styles::getRecentlyUsed (Webserver_Request& webserver_request)
{
  const std::string bible = webserver_request.database_config_user()->getBible ();
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  
  // The recent styles.
  const std::string s_styles = webserver_request.database_config_user()->getRecentlyAppliedStyles ();
  const std::vector <std::string> styles = filter::strings::explode (s_styles, ' ');
  std::string fragment = translate("Select style") + ": ";
  for (const auto& marker : styles) {
    if (!fragment.empty()) fragment.append (" | ");
    database::styles1::Item data = database::styles1::get_marker_data (stylesheet, marker);
    if (data.marker.empty ()) continue;
    const std::string name = translate(data.name) + " (" + marker + ")";
    const std::string info = translate(data.info);
    pugi::xml_document document;
    pugi::xml_node a_node = document.append_child("a");
    a_node.append_attribute("href") = marker.c_str();
    a_node.append_attribute("title") = info.c_str();
    a_node.append_attribute("unselectable") = "on";
    a_node.append_attribute("class") = "unselectable";
    a_node.append_attribute("tabindex") = "-1";
    a_node.text().set(name.c_str());
    std::stringstream ss {};
    document.print(ss, "", pugi::format_raw);
    fragment.append (ss.str());
  }
  
  // Links for cancelling and for all styles.
  fragment.append(" ");
  fragment.append(R"(<a href="cancel">[)" + translate("cancel") + "]</a>");
  fragment.append(" ");
  fragment.append(R"(<a href="all">[)" + translate("all") + "]</a>");
  
  return fragment;
}


std::string Editor_Styles::getAll (Webserver_Request& webserver_request)
{
  const std::string bible = webserver_request.database_config_user()->getBible ();
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  
  // The styles.
  const std::map <std::string, std::string> data = database::styles1::get_markers_and_names (stylesheet);
  
  std::vector <std::string> lines{};
  
  lines.push_back (R"(<select id="styleslist">)");
  
  const std::string line = translate("Select style");
  lines.push_back ("<option>" + line + "</option>");
  
  for (const auto& item : data) {
    const std::string& marker = item.first;
    std::string name = item.second;
    name = translate (name);
    database::styles1::Item marker_data = database::styles1::get_marker_data (stylesheet, marker);
    std::string category = marker_data.category;
    category = styles_logic_category_text (category);
    const std::string line2 = marker + " " + name + " (" + category + ")";
    lines.push_back ("<option>" + line2 + "</option>");
  }
  
  lines.push_back ("</select>");
  
  // Link for cancelling.
  lines.push_back (" ");
  lines.push_back (R"(<a href="cancel">[)" + translate("cancel") + "]</a>");
  
  const std::string html = filter::strings::implode (lines, "\n");
  
  return html;
}


void Editor_Styles::recordUsage (Webserver_Request& webserver_request, const std::string& style)
{
  if (style.empty()) return;
  std::string s_styles = webserver_request.database_config_user()->getRecentlyAppliedStyles ();
  std::vector <std::string> styles = filter::strings::explode (s_styles, ' ');
  // Erase the style.
  styles.erase (remove (styles.begin(), styles.end(), style), styles.end());
  // Add the style to he front of the vector.
  styles.insert (styles.begin(), style);
  // Clip the amount of styles to remember.
  if (styles.size () > 8) {
    styles.pop_back ();
  }
  s_styles = filter::strings::implode (styles, " ");
  webserver_request.database_config_user()->setRecentlyAppliedStyles (s_styles);
}


std::string Editor_Styles::getAction (Webserver_Request& webserver_request, const std::string& style)
{
  const std::string bible = webserver_request.database_config_user()->getBible ();
  const std::string stylesheet = database::config::bible::get_editor_stylesheet (bible);
  database::styles1::Item data = database::styles1::get_marker_data (stylesheet, style);
  int type = data.type;
  int subtype = data.subtype;
  
  switch (type)
  {
    case StyleTypeIdentifier:
      switch (subtype)
      {
        case IdentifierSubtypePublishedVerseMarker:
          return character ();
        default:
          return mono ();
      }
      break;
    case StyleTypeNotUsedComment:
      return mono ();
    case StyleTypeNotUsedRunningHeader:
      return mono ();
    case StyleTypeStartsParagraph:
      return paragraph ();
    case StyleTypeInlineText:
      return character ();
    case StyleTypeChapterNumber:
      return paragraph ();
    case StyleTypeVerseNumber:
      return character ();
    case StyleTypeFootEndNote:
    {
      switch (subtype)
      {
        case FootEndNoteSubtypeFootnote:
        case FootEndNoteSubtypeEndnote:
          return note ();
        case FootEndNoteSubtypeContent:
        case FootEndNoteSubtypeContentWithEndmarker:
          return character ();
        case FootEndNoteSubtypeStandardContent:
        case FootEndNoteSubtypeParagraph:
          return character ();
        default:
          return unknown ();
      }
      break;
    }
    case StyleTypeCrossreference:
    {
      switch (subtype)
      {
        case CrossreferenceSubtypeCrossreference:
          return note ();
        case CrossreferenceSubtypeContent:
        case CrossreferenceSubtypeContentWithEndmarker:
          return character ();
        case CrossreferenceSubtypeStandardContent:
          return character ();
        default:
          return unknown ();
      }
      break;
    }
    case StyleTypePeripheral:
      return mono ();
    case StyleTypePicture:
      return mono ();
    case StyleTypePageBreak:
      return unknown ();
    case StyleTypeTableElement:
    {
      switch (subtype)
      {
        case TableElementSubtypeRow:
        case TableElementSubtypeHeading:
        case TableElementSubtypeCell:
        default:
          return mono ();
      }
      break;
    }
    case StyleTypeWordlistElement:
    {
      return character ();
    }
    default:
      return unknown ();
  }
}


std::string Editor_Styles::unknown ()
{
  return "u";
}


std::string Editor_Styles::paragraph ()
{
  return "p";
}


std::string Editor_Styles::character ()
{
  return "c";
}


std::string Editor_Styles::mono ()
{
  return "m";
}


std::string Editor_Styles::note ()
{
  return "n";
}

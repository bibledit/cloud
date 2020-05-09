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


#include <editor/styles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/config/bible.h>


string Editor_Styles::getRecentlyUsed (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
 
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  
  // The recent styles.
  string s_styles = request->database_config_user()->getRecentlyAppliedStyles ();
  vector <string> styles = filter_string_explode (s_styles, ' ');
  string fragment = translate("Select style") + ": ";
  for (unsigned int i = 0; i < styles.size(); i++) {
    if (i) fragment += " | ";
    string marker = styles [i];
    Database_Styles_Item data = request->database_styles()->getMarkerData (stylesheet, marker);
    if (data.marker.empty ()) continue;
    string name = data.name + " (" + marker + ")";
    string info = data.info;
    fragment += "<a href=\"" + marker + "\" title=\"" + info + "\" unselectable=\"on\" class=\"unselectable\" tabindex=\"-1\">" + name + "</a>";
  }
  
  // Links for cancelling and for all styles.
  fragment += " ";
  fragment += "<a href=\"cancel\">[" + translate("cancel") + "]</a>";
  fragment += " ";
  fragment += "<a href=\"all\" >[" + translate("all") + "]</a>";
  
  return fragment;
}


string Editor_Styles::getAll (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  
  // The styles.
  map <string, string> data = request->database_styles()->getMarkersAndNames (stylesheet);
  
  vector <string> lines;
  
  lines.push_back ("<select id=\"styleslist\">");
  
  string line = translate("Select style");
  lines.push_back ("<option>" + line + "</option>");
  
  for (auto & item : data) {
    string marker = item.first;
    string name = item.second;
    name = translate (name);
    Database_Styles_Item data = request->database_styles()->getMarkerData (stylesheet, marker);
    string category = data.category;
    category = styles_logic_category_text (category);
    string line = marker + " " + name + " (" + category + ")";
    lines.push_back ("<option>" + line + "</option>");
  }
  
  lines.push_back ("</select>");
  
  // Link for cancelling.
  lines.push_back (" ");
  lines.push_back ("<a href=\"cancel\">[" + translate("cancel") + "]</a>");
  
  string html = filter_string_implode (lines, "\n");
  
  return html;
}


void Editor_Styles::recordUsage (void * webserver_request, string style)
{
  if (style == "") return;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string s_styles = request->database_config_user()->getRecentlyAppliedStyles ();
  vector <string> styles = filter_string_explode (s_styles, ' ');
  // Erase the style.
  styles.erase (remove (styles.begin(), styles.end(), style), styles.end());
  // Add the style to he front of the vector.
  styles.insert (styles.begin(), style);
  // Clip the amount of styles to remember.
  if (styles.size () > 8) {
    styles.pop_back ();
  }
  s_styles = filter_string_implode (styles, " ");
  request->database_config_user()->setRecentlyAppliedStyles (s_styles);
}


string Editor_Styles::getAction (void * webserver_request, string style)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string bible = request->database_config_user()->getBible ();
  string stylesheet = Database_Config_Bible::getEditorStylesheet (bible);
  Database_Styles_Item data = request->database_styles()->getMarkerData (stylesheet, style);
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


string Editor_Styles::unknown ()
{
  return "u";
}


string Editor_Styles::paragraph ()
{
  return "p";
}


string Editor_Styles::character ()
{
  return "c";
}


string Editor_Styles::mono ()
{
  return "m";
}


string Editor_Styles::note ()
{
  return "n";
}


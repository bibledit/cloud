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


#include <checks/settingssentences.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <access/bible.h>
#include <menu/logic.h>
#include <checks/settings.h>
using namespace std;


string checks_settingssentences_url ()
{
  return "checks/settingssentences";
}


bool checks_settingssentences_acl ([[maybe_unused]] void * webserver_request)
{
#ifdef HAVE_CLIENT
  return true;
#else
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


string checks_settingssentences (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  string page {};
  Assets_Header header = Assets_Header (translate ("Sentence Structure"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (checks_settings_url (), menu_logic_checks_settings_text ());
  page = header.run ();
  Assets_View view {};
  
  
  string bible = access_bible::clamp (webserver_request, request->database_config_user()->getBible ());
  
  
  if (request->post.count ("capitals")) {
    Database_Config_Bible::setSentenceStructureCapitals (bible, request->post["capitals"]);
    view.set_variable ("success", translate("The capitals were stored"));
  }
  
  
  if (request->post.count ("smallletters")) {
    Database_Config_Bible::setSentenceStructureSmallLetters (bible, request->post["smallletters"]);
    view.set_variable ("success", translate("The small letters were stored"));
  }
  
  
  if (request->post.count ("endpunctuationmarks")) {
    Database_Config_Bible::setSentenceStructureEndPunctuation (bible, request->post["endpunctuationmarks"]);
    view.set_variable ("success", translate("The punctuation marks at the ends of sentences were stored"));
  }
  
  
  if (request->post.count ("middlepunctuationmarks")) {
    Database_Config_Bible::setSentenceStructureMiddlePunctuation (bible, request->post["middlepunctuationmarks"]);
    view.set_variable ("success", translate("The punctuation marks within the sentences were stored"));
  }
  
  
  if (request->post.count ("disregards")) {
    Database_Config_Bible::setSentenceStructureDisregards (bible, request->post["disregards"]);
    view.set_variable ("success", translate("The characters that should be disregarded within the sentences were stored"));
  }
  
  
  if (request->post.count ("names")) {
    Database_Config_Bible::setSentenceStructureNames (bible, request->post["names"]);
    view.set_variable ("success", translate("The names that may occur after mid-sentence punctuation were stored"));
  }

  
  if (request->post.count ("within_sentence_paragraph_markers")) {
    Database_Config_Bible::setSentenceStructureWithinSentenceMarkers (bible, request->post["within_sentence_paragraph_markers"]);
    view.set_variable ("success", translate("The markers that start a new line but not necessarily a new sentence were saved"));
  }

  
  view.set_variable ("bible", bible);
  view.set_variable ("capitals", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureCapitals (bible)));
  view.set_variable ("smallletters", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureSmallLetters (bible)));
  view.set_variable ("endpunctuationmarks", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureEndPunctuation (bible)));
  view.set_variable ("middlepunctuationmarks", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureMiddlePunctuation (bible)));
  view.set_variable ("disregards", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureDisregards (bible)));
  view.set_variable ("names", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureNames (bible)));
  view.set_variable ("within_sentence_paragraph_markers", filter::strings::escape_special_xml_characters (Database_Config_Bible::getSentenceStructureWithinSentenceMarkers (bible)));
  
  
  page += view.render ("checks", "settingssentences");
  page += assets_page::footer ();
  return page;
}

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


std::string checks_settingssentences_url ()
{
  return "checks/settingssentences";
}


bool checks_settingssentences_acl ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  return true;
#else
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


std::string checks_settingssentences (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate ("Sentence Structure"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (checks_settings_url (), menu_logic_checks_settings_text ());
  page = header.run ();
  Assets_View view {};
  
  
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  
  
  if (webserver_request.post.count ("capitals")) {
    database::config::bible::set_sentence_structure_capitals (bible, webserver_request.post["capitals"]);
    view.set_variable ("success", translate("The capitals were stored"));
  }
  
  
  if (webserver_request.post.count ("smallletters")) {
    database::config::bible::set_sentence_structure_small_letters (bible, webserver_request.post["smallletters"]);
    view.set_variable ("success", translate("The small letters were stored"));
  }
  
  
  if (webserver_request.post.count ("endpunctuationmarks")) {
    database::config::bible::set_sentence_structure_end_punctuation (bible, webserver_request.post["endpunctuationmarks"]);
    view.set_variable ("success", translate("The punctuation marks at the ends of sentences were stored"));
  }
  
  
  if (webserver_request.post.count ("middlepunctuationmarks")) {
    database::config::bible::set_sentence_structure_middle_punctuation (bible, webserver_request.post["middlepunctuationmarks"]);
    view.set_variable ("success", translate("The punctuation marks within the sentences were stored"));
  }
  
  
  if (webserver_request.post.count ("disregards")) {
    database::config::bible::set_sentence_structure_disregards (bible, webserver_request.post["disregards"]);
    view.set_variable ("success", translate("The characters that should be disregarded within the sentences were stored"));
  }
  
  
  if (webserver_request.post.count ("names")) {
    database::config::bible::set_sentence_structure_names (bible, webserver_request.post["names"]);
    view.set_variable ("success", translate("The names that may occur after mid-sentence punctuation were stored"));
  }

  
  if (webserver_request.post.count ("within_sentence_paragraph_markers")) {
    database::config::bible::set_sentence_structure_within_sentence_markers (bible, webserver_request.post["within_sentence_paragraph_markers"]);
    view.set_variable ("success", translate("The markers that start a new line but not necessarily a new sentence were saved"));
  }

  
  view.set_variable ("bible", bible);
  view.set_variable ("capitals", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_capitals (bible)));
  view.set_variable ("smallletters", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_small_letters (bible)));
  view.set_variable ("endpunctuationmarks", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_end_punctuation (bible)));
  view.set_variable ("middlepunctuationmarks", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_middle_punctuation (bible)));
  view.set_variable ("disregards", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_disregards (bible)));
  view.set_variable ("names", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_names (bible)));
  view.set_variable ("within_sentence_paragraph_markers", filter::strings::escape_special_xml_characters (database::config::bible::get_sentence_structure_within_sentence_markers (bible)));
  
  
  page += view.render ("checks", "settingssentences");
  page += assets_page::footer ();
  return page;
}

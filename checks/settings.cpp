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


#include <checks/settings.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/check.h>
#include <database/config/bible.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <access/bible.h>
#include <dialog/list.h>
#include <checks/logic.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <journal/logic.h>


std::string checks_settings_url ()
{
  return "checks/settings";
}


bool checks_settings_acl ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  return true;
#else
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


std::string checks_settings (Webserver_Request& webserver_request)
{
  std::string page {};
  Assets_Header header = Assets_Header (translate("Manage Checks"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view {};
  
  
  if (webserver_request.query.count ("bible")) {
    const std::string bible = webserver_request.query["bible"];
    if (bible.empty()) {
      Dialog_List dialog_list = Dialog_List ("settings", translate("Select which Bible to manage"), std::string(), std::string());
      std::vector <std::string> bibles = access_bible::bibles (webserver_request);
      for (const auto & selectable_bible : bibles) {
        dialog_list.add_row (selectable_bible, "bible", selectable_bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      webserver_request.database_config_user()->setBible (bible);
    }
  }
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());

  
  if (webserver_request.query.count ("run")) {
    checks_logic_start (bible);
    view.set_variable ("success", translate("Will run the checks."));
    view.set_variable ("journal", journal_logic_see_journal_for_progress ());
  }
  
  
  const std::string checkbox = webserver_request.post ["checkbox"];
  const bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);
  
                        
  if (checkbox == "doublespacesusfm") {
    Database_Config_Bible::setCheckDoubleSpacesUsfm (bible, checked);
  }
  view.set_variable ("doublespacesusfm", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckDoubleSpacesUsfm (bible)));
   
  
  if (checkbox == "fullstopheadings") {
    Database_Config_Bible::setCheckFullStopInHeadings (bible, checked);
  }
  view.set_variable ("fullstopheadings", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckFullStopInHeadings (bible)));

  
  if (checkbox == "spacebeforepunctuation") {
    Database_Config_Bible::setCheckSpaceBeforePunctuation (bible, checked);
  }
  view.set_variable ("spacebeforepunctuation", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckSpaceBeforePunctuation (bible)));

  
  if (checkbox == "spacebeforefinalnotemarkup") {
    Database_Config_Bible::setCheckSpaceBeforeFinalNoteMarker (bible, checked);
  }
  view.set_variable ("spacebeforefinalnotemarkup", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckSpaceBeforeFinalNoteMarker (bible)));

                      
  if (checkbox == "sentencestructure") {
    Database_Config_Bible::setCheckSentenceStructure (bible, checked);
  }
  view.set_variable ("sentencestructure", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckSentenceStructure (bible)));
                      
                      
  if (checkbox == "paragraphstructure") {
    Database_Config_Bible::setCheckParagraphStructure (bible, checked);
  }
  view.set_variable ("paragraphstructure", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckParagraphStructure (bible)));
                      
                      
  if (checkbox == "booksversification") {
    Database_Config_Bible::setCheckBooksVersification (bible, checked);
  }
  view.set_variable ("booksversification", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckBooksVersification (bible)));
  
  
  if (checkbox == "chaptersversesversification") {
    Database_Config_Bible::setCheckChaptesVersesVersification (bible, checked);
  }
  view.set_variable ("chaptersversesversification", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckChaptesVersesVersification (bible)));
                      
                      
  if (checkbox == "wellformedusfm") {
    Database_Config_Bible::setCheckWellFormedUsfm (bible, checked);
  }
  view.set_variable ("wellformedusfm", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckWellFormedUsfm (bible)));
                      
                      
  if (checkbox == "punctuationatendverse") {
    Database_Config_Bible::setCheckMissingPunctuationEndVerse (bible, checked);
  }
  view.set_variable ("punctuationatendverse", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckMissingPunctuationEndVerse (bible)));
                      
                      
  if (checkbox == "patterns") {
    Database_Config_Bible::setCheckPatterns (bible, checked);
  }
  view.set_variable ("patterns", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckPatterns (bible)));

  
  if (checkbox == "pairs") {
    Database_Config_Bible::setCheckMatchingPairs (bible, checked);
  }
  view.set_variable ("pairs", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckMatchingPairs (bible)));

  
  if (checkbox == "spaceendverse") {
    Database_Config_Bible::setCheckSpaceEndVerse (bible, checked);
  }
  view.set_variable ("spaceendverse", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckSpaceEndVerse (bible)));

  
  if (checkbox == "frenchpunctuation") {
    Database_Config_Bible::setCheckFrenchPunctuation (bible, checked);
  }
  view.set_variable ("frenchpunctuation", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckFrenchPunctuation (bible)));

  
  if (checkbox == "frenchcitation") {
    Database_Config_Bible::setCheckFrenchCitationStyle (bible, checked);
  }
  view.set_variable ("frenchcitation", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckFrenchCitationStyle (bible)));

  
  if (checkbox == "transposenotes") {
    Database_Config_Bible::setTransposeFixSpacesNotes (bible, checked);
  }
  view.set_variable ("transposenotes", filter::strings::get_checkbox_status (Database_Config_Bible::getTransposeFixSpacesNotes (bible)));

  
  if (checkbox == "validutf8") {
    Database_Config_Bible::setCheckValidUTF8Text (bible, checked);
  }
  view.set_variable ("validutf8", filter::strings::get_checkbox_status (Database_Config_Bible::getCheckValidUTF8Text (bible)));

  
  view.set_variable ("bible", bible);

  
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
#else
  view.enable_zone ("cloud");
#endif

  
  page += view.render ("checks", "settings");
  page += assets_page::footer ();
  return page;
}

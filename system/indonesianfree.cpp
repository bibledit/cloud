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


#include <system/indonesianfree.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <dialog/list.h>
#include <dialog/entry.h>
#include <database/config/general.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <assets/external.h>
#include <access/bible.h>
#include <search/logic.h>
#include <styles/sheets.h>


string system_indonesianfree_url ()
{
  return "system/indonesianfree";
}


bool system_indonesianfree_acl (void * webserver_request)
{
  (void) webserver_request;
#ifdef HAVE_INDONESIANCLOUDFREE
  return true;
#endif
  return false;
}


string system_indonesianfree (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string page;
  string success;
  string error;

  
  // The available localizations.
  map <string, string> localizations = locale_logic_localizations ();

  
  // User can set the system language.
  // This is to be done before displaying the header.
  if (request->query.count ("language")) {
    string language = request->query ["language"];
    if (language == "select") {
      Dialog_List dialog_list = Dialog_List ("indonesianfree", translate("Set the language for Bibledit"), "", "");
      for (auto element : localizations) {
        dialog_list.add_row (element.second, "language", element.first);
      }
      page = Assets_Page::header ("", webserver_request);
      page += dialog_list.run ();
      return page;
    } else {
      Database_Config_General::setSiteLanguage (locale_logic_filter_default_language (language));
    }
  }

  
  // Store new font sizes before displaying the header,
  // so that the page displays the new font sizes immediately.
  if (request->post.count ("fontsizegeneral")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 50) && (value <= 300)) {
      request->database_config_user ()->setGeneralFontSize (value);
    } else {
      error = translate ("Incorrect font size in percents");
    }
  }
  if (request->post.count ("fontsizemenu")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 50) && (value <= 300)) {
      request->database_config_user ()->setMenuFontSize (value);
    } else {
      error = translate ("Incorrect font size in percents");
    }
  }

  
  // The header: The language has been set already.
  Assets_Header header = Assets_Header (translate("System"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;


  // Get values for setting checkboxes.
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);
  (void) checked;

  
  // Font size for everything.
  if (request->query.count ("fontsizegeneral")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getGeneralFontSize ()), "fontsizegeneral", "");
    page += dialog_entry.run ();
    return page;
  }
  view.set_variable ("fontsizegeneral", convert_to_string (request->database_config_user ()->getGeneralFontSize ()));

  
  // Font size for the menu.
  if (request->query.count ("fontsizemenu")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getMenuFontSize ()), "fontsizemenu", "");
    page += dialog_entry.run ();
    return page;
  }
  view.set_variable ("fontsizemenu", convert_to_string (request->database_config_user ()->getMenuFontSize ()));
  
  
  // Font size for the Bible editors.
  if (request->query.count ("fontsizeeditors")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getBibleEditorsFontSize ()), "fontsizeeditors", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("fontsizeeditors")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 50) && (value <= 300)) {
      request->database_config_user ()->setBibleEditorsFontSize (value);
      styles_sheets_create_all ();
    } else {
      error = translate ("Incorrect font size in percents");
    }
  }
  view.set_variable ("fontsizeeditors", convert_to_string (request->database_config_user ()->getBibleEditorsFontSize ()));
  
  
  // Font size for the resources.
  if (request->query.count ("fontsizeresources")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getResourcesFontSize ()), "fontsizeresources", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("fontsizeresources")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 50) && (value <= 300)) {
      request->database_config_user ()->setResourcesFontSize (value);
    } else {
      error = translate ("Incorrect font size in percents");
    }
  }
  view.set_variable ("fontsizeresources", convert_to_string (request->database_config_user ()->getResourcesFontSize ()));
  
  
  // Font size for Hebrew resources.
  if (request->query.count ("fontsizehebrew")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getHebrewFontSize ()), "fontsizehebrew", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("fontsizehebrew")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 50) && (value <= 300)) {
      request->database_config_user ()->setHebrewFontSize (value);
    } else {
      error = translate ("Incorrect font size in percents");
    }
  }
  view.set_variable ("fontsizehebrew", convert_to_string (request->database_config_user ()->getHebrewFontSize ()));
  
  
  // Font size for Greek resources.
  if (request->query.count ("fontsizegreek")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getGreekFontSize ()), "fontsizegreek", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("fontsizegreek")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 50) && (value <= 300)) {
      request->database_config_user ()->setGreekFontSize (value);
    } else {
      error = translate ("Incorrect font size in percents");
    }
  }
  view.set_variable ("fontsizegreek", convert_to_string (request->database_config_user ()->getGreekFontSize ()));

  
  // Set the language on the page.
  string language = locale_logic_filter_default_language (Database_Config_General::getSiteLanguage ());
  language = localizations [language];
  view.set_variable ("language", language);
  
  
  // Since the Bible can be set, first ensure there's one available.
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  if (request->query.count ("bible")) bible = access_bible_clamp (request, request->query ["bible"]);

 
  // Change the name of the Bible.
  if (request->query.count ("bible")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indonesianfree", translate ("Please enter a name for the Bible"), bible, "bible", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("bible")) {
    string bible2 = request->post ["entry"];
    // Copy the Bible data.
    string origin_folder = request->database_bibles ()->bibleFolder (bible);
    string destination_folder = request->database_bibles ()->bibleFolder (bible2);
    filter_url_dir_cp (origin_folder, destination_folder);
    // Copy the Bible search index.
    search_logic_copy_bible (bible, bible2);
    // Remove the old Bible.
    request->database_bibles ()->deleteBible (bible);
    search_logic_delete_bible (bible);
    // Update logic.
    bible = bible2;
    // Feedback.
    success = translate ("The Bible was renamed");
  }
  view.set_variable ("bible", bible);
  
  
  view.set_variable ("external", assets_external_logic_link_addon ());

  
  int level = request->session_logic ()->currentLevel ();
  if (level >= Filter_Roles::manager ()) {
    view.enable_zone ("can_use_advanced_mode");
  }


  // Set some feedback, if any.
  view.set_variable ("success", success);
  view.set_variable ("error", error);

  
  page += view.render ("system", "indonesianfree");
  page += Assets_Page::footer ();
  return page;
}

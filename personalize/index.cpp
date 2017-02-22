/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <personalize/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <database/config/general.h>
#include <database/config/user.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <styles/sheets.h>
#include <styles/logic.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <access/bible.h>
#include <dialog/list.h>
#include <bible/logic.h>


string personalize_index_url ()
{
  return "personalize/index";
}


bool personalize_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


string personalize_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;


  // Accept values for allowed relative changes for the four Bible text editors.
  if (request->post.count ("chapterpercentage")) {
    int chapterpercentage = convert_to_int (request->post ["chapterpercentage"]);
    chapterpercentage = clip (chapterpercentage, 10, 100);
    request->database_config_user ()->setEditingAllowedDifferenceChapter (chapterpercentage);
    return "";
  }
  if (request->post.count ("versepercentage")) {
    int versepercentage = convert_to_int (request->post ["versepercentage"]);
    versepercentage = clip (versepercentage, 10, 100);
    request->database_config_user ()->setEditingAllowedDifferenceVerse (versepercentage);
    return "";
  }
  

  // Breadcrumbs: Before displaying the page, so the page does the correct thing with the bread crumbs.
  if (request->query.count ("breadcrumbs")) {
    bool state = request->database_config_user ()->getDisplayBreadcrumbs ();
    request->database_config_user ()->setDisplayBreadcrumbs (!state);
  }
  
  
  // Main menu always visible: Before displaying page, so the page does the correct thing with the menu.
  if (request->query.count ("menuvisible")) {
    bool state = request->database_config_user ()->getMainMenuAlwaysVisible ();
    request->database_config_user ()->setMainMenuAlwaysVisible (!state);
  }
  
  
  // Swipe actions.
  if (request->query.count ("swipeactions")) {
    bool state = request->database_config_user ()->getSwipeActionsAvailable ();
    request->database_config_user ()->setSwipeActionsAvailable (!state);
  }
  
  
  // Fast Bible editor switching.
  if (request->query.count ("fasteditorswitch")) {
    bool state = request->database_config_user ()->getFastEditorSwitchingAvailable ();
    request->database_config_user ()->setFastEditorSwitchingAvailable (!state);
  }
  
  
  // Styles editing in visual editors.
  if (request->query.count ("enablestylesbutton")) {
    bool state = request->database_config_user ()->getEnableStylesButtonVisualEditors ();
    request->database_config_user ()->setEnableStylesButtonVisualEditors (!state);
  }
  
  
  string page;
  string success;
  string error;


  // Store new fon sizes before displaying the header,
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
  
  
  Assets_Header header = Assets_Header (translate("Preferences"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;
  
  
  // Font size for everything.
  if (request->query.count ("fontsizegeneral")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getGeneralFontSize ()), "fontsizegeneral", "");
    page += dialog_entry.run ();
    return page;
  }
  view.set_variable ("fontsizegeneral", convert_to_string (request->database_config_user ()->getGeneralFontSize ()));

  
  // Font size for the menu.
  if (request->query.count ("fontsizemenu")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getMenuFontSize ()), "fontsizemenu", "");
    page += dialog_entry.run ();
    return page;
  }
  view.set_variable ("fontsizemenu", convert_to_string (request->database_config_user ()->getMenuFontSize ()));
  
  
  // Font size for the Bible editors.
  if (request->query.count ("fontsizeeditors")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getBibleEditorsFontSize ()), "fontsizeeditors", "");
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
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getResourcesFontSize ()), "fontsizeresources", "");
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
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getHebrewFontSize ()), "fontsizehebrew", "");
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
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a font size between 50 and 300 percent"), convert_to_string (request->database_config_user ()->getGreekFontSize ()), "fontsizegreek", "");
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
  
  
  // Vertical caret position in chapter editors.
  if (request->query.count ("caretposition")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a caret position between 20 and 80 percent"), convert_to_string (request->database_config_user ()->getVerticalCaretPosition ()), "caretposition", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("caretposition")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 20) && (value <= 80)) {
      request->database_config_user ()->setVerticalCaretPosition (value);
    } else {
      error = translate ("Incorrect caret position in percents");
    }
  }
  view.set_variable ("caretposition", convert_to_string (request->database_config_user ()->getVerticalCaretPosition ()));
  

  // Whether to display bread crumbs.
  string on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getDisplayBreadcrumbs ());
  view.set_variable ("breadcrumbs", on_off);

  
  // Workspace menu fade-out delay.
  if (request->query.count ("desktopfadeoutdelay")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a value between 1 and 100 seconds, or 0 to disable"), convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()), "desktopfadeoutdelay", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("desktopfadeoutdelay")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      request->database_config_user ()->setWorkspaceMenuFadeoutDelay (value);
    } else {
      error = translate ("Incorrect fade-out delay in seconds");
    }
  }
  view.set_variable ("desktopfadeoutdelay", convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()));

  
  // Permissable relative changes in the two to four Bible editors.
  view.set_variable ("chapterpercentage", convert_to_string (request->database_config_user ()->getEditingAllowedDifferenceChapter ()));
  view.set_variable ("versepercentage", convert_to_string (request->database_config_user ()->getEditingAllowedDifferenceVerse ()));
  
  
  // Whether to keep the main menu always visible.
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getMainMenuAlwaysVisible ());
  view.set_variable ("menuvisible", on_off);
  
  
  // Whether to enable swipe actions.
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getSwipeActionsAvailable ());
  view.set_variable ("swipeactions", on_off);
  
  
  // Whether to enable fast Bible editor switching.
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getFastEditorSwitchingAvailable ());
  view.set_variable ("fasteditorswitch", on_off);

  
  // Active visual editors.
  string editors;
  const char * activevisualeditors = "activevisualeditors";
  if (request->query.count (activevisualeditors)) {
    editors = request->query[activevisualeditors];
    if (editors.empty ()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which visual Bible editors to enable?"), "", "");
      for (int i = 0; i < 3; i++) {
        dialog_list.add_row (menu_logic_editor_settings_text (true, i), activevisualeditors, convert_to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user ()->setEnabledVisualEditors (convert_to_int (editors));
    }
  }
  editors = menu_logic_editor_settings_text (true, request->database_config_user ()->getEnabledVisualEditors ());
  view.set_variable (activevisualeditors, editors);

  
  // Active USFM editors.
  const char * activeusfmeditors = "activeusfmeditors";
  if (request->query.count (activeusfmeditors)) {
    editors = request->query[activeusfmeditors];
    if (editors.empty ()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which visual Bible editors to enable?"), "", "");
      for (int i = 0; i < 3; i++) {
        dialog_list.add_row (menu_logic_editor_settings_text (false, i), activeusfmeditors, convert_to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user ()->setEnabledUsfmEditors (convert_to_int (editors));
    }
  }
  editors = menu_logic_editor_settings_text (false, request->database_config_user ()->getEnabledUsfmEditors ());
  view.set_variable (activeusfmeditors, editors);

  
  // The names of the two, three, or four Bible editors.
  // The location of this code is important that it is after the editors have been enabled or disabled.
  string chapter_editors, verse_editors;
  for (int i = 0; i < 4; i++) {
    bool visual = (i % 2);
    bool chapter = (i <2);
    if (menu_logic_editor_enabled (webserver_request, visual, chapter)) {
      string label = menu_logic_editor_menu_text (webserver_request, visual, chapter);
      if (chapter) {
        if (!chapter_editors.empty ()) chapter_editors.append (", ");
        chapter_editors.append (label);
      } else {
        if (!verse_editors.empty ()) verse_editors.append (", ");
        verse_editors.append (label);
      }
    }
  }
  if (!chapter_editors.empty ()) view.enable_zone ("chaptereditors");
  view.set_variable ("chaptereditors", chapter_editors);
  if (!verse_editors.empty ()) view.enable_zone ("verseeditors");
  view.set_variable ("verseeditors", verse_editors);
  
  
  // Whether to enable editing styles in the visual editors.
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getEnableStylesButtonVisualEditors ());
  view.set_variable ("enablestylesbutton", on_off);
  
  
  // Enable the sections with settings relevant to the user and device.
  bool resources = access_logic_privilege_view_resources (webserver_request);
  if (resources) view.enable_zone ("resources");
  bool bibles = Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
  bool read, write;
  access_a_bible (webserver_request, read, write);
  if (read || write) bibles = true;
  if (bibles) view.enable_zone ("bibles");
  if (request->session_logic ()->touchEnabled ()) {
    view.enable_zone ("touch");
  }
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("personalize", "index");

  page += Assets_Page::footer ();
  
  return page;
}

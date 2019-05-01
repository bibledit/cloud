/*
Copyright (©) 2003-2019 Teus Benschop.

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
#include <bb/logic.h>
#include <ipc/focus.h>
#include <client/logic.h>


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

  
  // Whether to have a menu entry for the Changes in basic mode.
  if (request->query.count ("showchanges")) {
    bool state = request->database_config_user ()->getMenuChangesInBasicMode ();
    state = !state;
    request->database_config_user ()->setMenuChangesInBasicMode (state);
    menu_logic_tabbed_mode_save_json (webserver_request);
  }

  
  // Setting for whether to show the main menu in tabbed view in basic mode on phones and tablets.
  if (request->query.count ("mainmenutabs")) {
    bool state = Database_Config_General::getMenuInTabbedViewOn ();
    state = !state;
    Database_Config_General::setMenuInTabbedViewOn (state);
    menu_logic_tabbed_mode_save_json (webserver_request);
  }

  
  string page;
  string success;
  string error;


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
  if (request->query.count ("workspacefadeoutdelay")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("index", translate("Please enter a value between 1 and 100 seconds, or 0 to disable"), convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()), "workspacefadeoutdelay", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("workspacefadeoutdelay")) {
    int value = convert_to_int (request->post["entry"]);
    if ((value >= 0) && (value <= 100)) {
      request->database_config_user ()->setWorkspaceMenuFadeoutDelay (value);
    } else {
      error = translate ("Incorrect fade-out delay in seconds");
    }
  }
  view.set_variable ("workspacefadeoutdelay", convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()));

  
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

  
  // Visual editors in the fast Bible editor switcher.
  string editors;
  const char * fastswitchvisualeditors = "fastswitchvisualeditors";
  if (request->query.count (fastswitchvisualeditors)) {
    editors = request->query[fastswitchvisualeditors];
    if (editors.empty ()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which visual Bible editors to enable?"), "", "");
      for (int i = 0; i < 3; i++) {
        dialog_list.add_row (menu_logic_editor_settings_text (true, i), fastswitchvisualeditors, convert_to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user ()->setFastSwitchVisualEditors (convert_to_int (editors));
    }
  }
  editors = menu_logic_editor_settings_text (true, request->database_config_user ()->getFastSwitchVisualEditors ());
  view.set_variable (fastswitchvisualeditors, editors);

  
  // USFM editors fast Bible editor switcher.
  const char * fastswitchusfmeditors = "fastswitchusfmeditors";
  if (request->query.count (fastswitchusfmeditors)) {
    editors = request->query[fastswitchusfmeditors];
    if (editors.empty ()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Enable the USFM Bible editor?"), "", "");
      for (int i = 0; i < 2; i++) {
        dialog_list.add_row (menu_logic_editor_settings_text (false, i), fastswitchusfmeditors, convert_to_string (i));
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user ()->setFastSwitchUsfmEditors (convert_to_int (editors));
    }
  }
  editors = menu_logic_editor_settings_text (false, request->database_config_user ()->getFastSwitchUsfmEditors ());
  view.set_variable (fastswitchusfmeditors, editors);

  
  // Whether to enable editing styles in the visual editors.
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getEnableStylesButtonVisualEditors ());
  view.set_variable ("enablestylesbutton", on_off);
  

  // Change the active Bible.
  if (request->query.count ("changebible")) {
    string changebible = request->query ["changebible"];
    if (changebible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to make the active one for editing"), "", "");
      vector <string> bibles = access_bible_bibles (request);
      for (auto & bible : bibles) {
        dialog_list.add_row (bible, "changebible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setBible (changebible);
      // Going to another Bible, ensure that the focused book exists there.
      int book = Ipc_Focus::getBook (request);
      vector <int> books = request->database_bibles()->getBooks (changebible);
      if (find (books.begin(), books.end(), book) == books.end()) {
        if (!books.empty ()) book = books [0];
        else book = 0;
        Ipc_Focus::set (request, book, 1, 1);
      }
    }
  }
  string bible = access_bible_clamp (request, request->database_config_user()->getBible ());
  view.set_variable ("bible", bible);

  
  // Whether to have a menu entry for the Changes in basic mode.
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getMenuChangesInBasicMode ());
  view.set_variable ("showchanges", on_off);

  
  // Whether to put the controls for dismissing the change notificatios at the top of the page.
  if (request->query.count ("dismisschangesattop")) {
    bool state = request->database_config_user ()->getDismissChangesAtTop ();
    request->database_config_user ()->setDismissChangesAtTop (!state);
  }
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getDismissChangesAtTop ());
  view.set_variable ("dismisschangesattop", on_off);
  
  
  // Setting for whether to show the main menu in tabbed view in basic mode on phones and tablets.
  if (menu_logic_can_do_tabbed_mode ()) {
    view.enable_zone ("tabs_possible");
    on_off = styles_logic_off_on_inherit_toggle_text (Database_Config_General::getMenuInTabbedViewOn ());
    view.set_variable ("mainmenutabs", on_off);
  }

  
  // Whether to enable a quick link to edit the content of a consultation note.
  if (request->query.count ("quickeditnotecontents")) {
    bool state = request->database_config_user ()->getQuickNoteEditLink ();
    request->database_config_user ()->setQuickNoteEditLink (!state);
  }
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getQuickNoteEditLink ());
  view.set_variable ("quickeditnotecontents", on_off);

  
  // Whether the list of consultation notes shows the Bible the note refers to.
  if (request->query.count ("showbibleinnoteslist")) {
    bool state = request->database_config_user ()->getShowBibleInNotesList ();
    request->database_config_user ()->setShowBibleInNotesList (!state);
  }
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getShowBibleInNotesList ());
  view.set_variable ("showbibleinnoteslist", on_off);
  
  
  // Whether to display the note status in the notes list and the note display.
  if (request->query.count ("shownotestatus")) {
    bool state = request->database_config_user ()->getShowNoteStatus ();
    request->database_config_user ()->setShowNoteStatus (!state);
  }
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getShowNoteStatus ());
  view.set_variable ("shownotestatus", on_off);

  
  // Whether to show the text of the focuses Bible passage, while creating a new Consultation Note.
  // This helps the translators and consultants if they are using notes in full screen mode,
  // rather than from a workspace that may already show the focused Bible verse text.
  // It shows the users if they have the focus on the verse they want to comment on.
  // It makes it easy for them to grab a few words of the text to place within the note being created.
  if (request->query.count ("showversetextcreatenote")) {
    bool state = request->database_config_user ()->getShowVerseTextAtCreateNote ();
    request->database_config_user ()->setShowVerseTextAtCreateNote (!state);
  }
  on_off = styles_logic_off_on_inherit_toggle_text (request->database_config_user ()->getShowVerseTextAtCreateNote ());
  view.set_variable ("showversetextcreatenote", on_off);
  
  
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
  

  // Enable the sections for either basic or advanced mode.
  if (request->database_config_user ()->getBasicInterfaceMode ()) {
    view.enable_zone ("basicmode");
    if (request->database_config_user ()->getPrivilegeUseAdvancedMode ()) {
      view.enable_zone ("can_use_advanced_mode");
    }
  } else {
    view.enable_zone ("advancedmode");
  }
  
  
#ifdef HAVE_CLIENT
  view.enable_zone ("client_mode");
  if (client_logic_client_enabled ()) {
    view.enable_zone ("client_connected");
  }
#endif
#ifdef HAVE_CLOUD
  view.enable_zone ("cloud_mode");
#endif

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("personalize", "index");

  page += Assets_Page::footer ();
  
  return page;
}

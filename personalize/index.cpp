/*
Copyright (©) 2003-2022 Teus Benschop.

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
#include <database/logs.h>
#include <locale/logic.h>
#include <locale/translate.h>
#include <dialog/entry.h>
#include <styles/sheets.h>
#include <styles/logic.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <access/bible.h>
#include <dialog/list.h>
#include <dialog/list2.h>
#include <bb/logic.h>
#include <ipc/focus.h>
#include <client/logic.h>
#include <config/globals.h>


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
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);


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
  

  // Set the user chosen theme as the current theme.
  if (request->post.count ("themepicker")) {
    int themepicker = convert_to_int (request->post ["themepicker"]);
    request->database_config_user ()->setCurrentTheme (themepicker);
  }

  
  string page;
  string success;
  string error;


  // Store new font sizes before displaying the header,
  // so that the page displays the new font sizes immediately.
  if (request->post.count ("fontsizegeneral")) {
   int fontsizegeneral = convert_to_int (request->post["fontsizegeneral"]);
    fontsizegeneral = clip (fontsizegeneral, 50, 300);
    request->database_config_user ()->setGeneralFontSize (fontsizegeneral);
    return "";
  }
  if (request->post.count ("fontsizemenu")) {
    int fontsizemenu = convert_to_int (request->post["fontsizemenu"]);
    fontsizemenu = clip (fontsizemenu, 50, 300);
    request->database_config_user ()->setMenuFontSize (fontsizemenu);
    return "";
  }
  
  
  Assets_Header header = Assets_Header (translate("Preferences"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;

  
  // Font size for everything.
  view.set_variable ("fontsizegeneral", convert_to_string (request->database_config_user ()->getGeneralFontSize ()));

  
  // Font size for the menu.
  view.set_variable ("fontsizemenu", convert_to_string (request->database_config_user ()->getMenuFontSize ()));
  
  
  // Font size for the Bible editors.
  if (request->post.count ("fontsizeeditors")) {
    int fontsizeeditors = convert_to_int (request->post["fontsizeeditors"]);
    fontsizeeditors = clip (fontsizeeditors, 50, 300);
    request->database_config_user ()->setBibleEditorsFontSize (fontsizeeditors);
    styles_sheets_create_all ();
    return "";
  }
  view.set_variable ("fontsizeeditors", convert_to_string (request->database_config_user ()->getBibleEditorsFontSize ()));
  
  
  // Font size for the resources.
  if (request->post.count ("fontsizeresources")) {
    int fontsizeresources = convert_to_int (request->post["fontsizeresources"]);
    fontsizeresources = clip (fontsizeresources, 50, 300);
    request->database_config_user ()->setResourcesFontSize (fontsizeresources);
    return "";
  }
  view.set_variable ("fontsizeresources", convert_to_string (request->database_config_user ()->getResourcesFontSize ()));
  
  
  // Font size for Hebrew resources.
  if (request->post.count ("fontsizehebrew")) {
    int fontsizehebrew = convert_to_int (request->post["fontsizehebrew"]);
    fontsizehebrew = clip (fontsizehebrew, 50, 300);
    request->database_config_user ()->setHebrewFontSize (fontsizehebrew);
    return "";
  }
  view.set_variable ("fontsizehebrew", convert_to_string (request->database_config_user ()->getHebrewFontSize ()));
  
  
  // Font size for Greek resources.
  if (request->post.count ("fontsizegreek")) {
    int fontsizegreek = convert_to_int (request->post["fontsizegreek"]);
    fontsizegreek = clip (fontsizegreek, 50, 300);
    request->database_config_user ()->setGreekFontSize (fontsizegreek);
    return "";
  }
  view.set_variable ("fontsizegreek", convert_to_string (request->database_config_user ()->getGreekFontSize ()));
  
  
  // Vertical caret position in chapter editors.
  if (request->post.count ("caretposition")) {
    int caretposition = convert_to_int (request->post["caretposition"]);
    caretposition = clip (caretposition, 20, 80);
    request->database_config_user ()->setVerticalCaretPosition (caretposition);
    return "";
  }
  view.set_variable ("caretposition", convert_to_string (request->database_config_user ()->getVerticalCaretPosition ()));
  

  // Whether to display bread crumbs.
  if (checkbox == "breadcrumbs") {
    request->database_config_user ()->setDisplayBreadcrumbs (checked);
    return get_reload ();
  }
  view.set_variable ("breadcrumbs", get_checkbox_status (request->database_config_user ()->getDisplayBreadcrumbs ()));

  
  // Set the chosen theme on the option HTML tag.
  string theme_key = convert_to_string (request->database_config_user ()->getCurrentTheme ());
  string theme_html;
  theme_html = Options_To_Select::add_selection ("Basic", "0", theme_html);
  theme_html = Options_To_Select::add_selection ("Light", "1", theme_html);
  theme_html = Options_To_Select::add_selection ("Dark", "2", theme_html);
  theme_html = Options_To_Select::add_selection ("Red Blue Light", "3", theme_html);
  theme_html = Options_To_Select::add_selection ("Red Blue Dark", "4", theme_html);
  view.set_variable ("themepickeroptags", Options_To_Select::mark_selected (theme_key, theme_html));
  view.set_variable ("themepicker", theme_key);

  
  // Workspace menu fade-out delay.
  if (request->post.count ("workspacefadeoutdelay")) {
    int workspacefadeoutdelay = convert_to_int (request->post["workspacefadeoutdelay"]);
    workspacefadeoutdelay = clip (workspacefadeoutdelay, 0, 100);
    request->database_config_user ()->setWorkspaceMenuFadeoutDelay (workspacefadeoutdelay);
    return "";
  }
  view.set_variable ("workspacefadeoutdelay", convert_to_string (request->database_config_user ()->getWorkspaceMenuFadeoutDelay ()));

  
  // Permissable relative changes in the two to four Bible editors.
  view.set_variable ("chapterpercentage", convert_to_string (request->database_config_user ()->getEditingAllowedDifferenceChapter ()));
  view.set_variable ("versepercentage", convert_to_string (request->database_config_user ()->getEditingAllowedDifferenceVerse ()));
  
  
  // Whether to keep the main menu always visible.
  if (checkbox == "menuvisible") {
    request->database_config_user ()->setMainMenuAlwaysVisible (checked);
    return get_reload ();
  }
  view.set_variable ("menuvisible", get_checkbox_status (request->database_config_user ()->getMainMenuAlwaysVisible ()));
  
  
  // Whether to enable swipe actions.
  if (checkbox == "swipeactions") {
    request->database_config_user ()->setSwipeActionsAvailable (checked);
    return "";
  }
  view.set_variable ("swipeactions", get_checkbox_status(request->database_config_user ()->getSwipeActionsAvailable ()));
  
  
  // Whether to enable fast Bible editor switching.
  if (checkbox == "fasteditorswitch") {
    request->database_config_user ()->setFastEditorSwitchingAvailable (checked);
    return string();
  }
  view.set_variable ("fasteditorswitch", get_checkbox_status(request->database_config_user ()->getFastEditorSwitchingAvailable ()));

  
  // Visual editors in the fast Bible editor switcher.
  const char * fastswitchvisualeditors = "fastswitchvisualeditors";
  string visual_editors_html;
  for (int i = 0; i < 3; i++) {
    visual_editors_html = Options_To_Select::add_selection (menu_logic_editor_settings_text (true, i), convert_to_string (i), visual_editors_html);
  }
  if (request->post.count (fastswitchvisualeditors)) {
    int visual_editor_key = convert_to_int (request->post [fastswitchvisualeditors]);
    request->database_config_user ()->setFastSwitchVisualEditors (visual_editor_key);
    return "";
  }
  string editor_key = convert_to_string (request->database_config_user ()->getFastSwitchVisualEditors ());
  view.set_variable ("fastswitchvisualeditorsoptags", Options_To_Select::mark_selected (editor_key, visual_editors_html));
  view.set_variable (fastswitchvisualeditors, editor_key);

  
  // USFM editors fast Bible editor switcher.
  const char * fastswitchusfmeditors = "fastswitchusfmeditors";
  string usfm_editors_html;
  for (int i = 0; i < 2; i++) {
    usfm_editors_html = Options_To_Select::add_selection (menu_logic_editor_settings_text (false, i), convert_to_string (i), usfm_editors_html);
  }
  if (request->post.count (fastswitchusfmeditors)) {
    int usfm_editor_key = convert_to_int (request->post [fastswitchusfmeditors]);
    request->database_config_user ()->setFastSwitchUsfmEditors (usfm_editor_key);
    return "";
  }
  editor_key = convert_to_string(request->database_config_user ()->getFastSwitchUsfmEditors ());
  view.set_variable ("fastswitchusfmeditorsoptags", Options_To_Select::mark_selected (editor_key, usfm_editors_html));
  view.set_variable (fastswitchusfmeditors, editor_key);

  
  // Whether to enable editing styles in the visual editors.
  if (checkbox == "enablestylesbutton") {
    request->database_config_user ()->setEnableStylesButtonVisualEditors (checked);
    return string();
  }
  view.set_variable ("enablestylesbutton", get_checkbox_status(request->database_config_user ()->getEnableStylesButtonVisualEditors ()));
  

  // Change the active Bible.
  if (request->query.count ("changebible")) {
    string changebible = request->query ["changebible"];
    if (changebible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to make the active one for editing"), "", "");
      vector <string> bibles = AccessBible::Bibles (request);
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
  string bible = AccessBible::Clamp (request, request->database_config_user()->getBible ());
  view.set_variable ("bible", bible);

  
  // Whether to have a menu entry for the Changes in basic mode.
  if (checkbox == "showchanges") {
    request->database_config_user ()->setMenuChangesInBasicMode (checked);
    menu_logic_tabbed_mode_save_json (webserver_request);
    return get_reload ();
  }
  view.set_variable ("showchanges", get_checkbox_status(request->database_config_user ()->getMenuChangesInBasicMode ()));

  
  // Whether to put the controls for dismissing the change notifications at the top of the page.
  if (checkbox == "dismisschangesattop") {
    request->database_config_user ()->setDismissChangesAtTop (checked);
  }
  view.set_variable ("dismisschangesattop", get_checkbox_status(request->database_config_user ()->getDismissChangesAtTop ()));
  
  
  // Setting for whether to show the main menu in tabbed view in basic mode on phones and tablets.
  if (checkbox == "mainmenutabs") {
    Database_Config_General::setMenuInTabbedViewOn (checked);
    menu_logic_tabbed_mode_save_json (webserver_request);
  }
  if (menu_logic_can_do_tabbed_mode ()) {
    view.enable_zone ("tabs_possible");
    view.set_variable ("mainmenutabs", get_checkbox_status(Database_Config_General::getMenuInTabbedViewOn ()));
  }

  
  // Whether to enable a quick link to edit the content of a consultation note.
  if (checkbox == "quickeditnotecontents") {
    request->database_config_user ()->setQuickNoteEditLink (checked);
  }
  view.set_variable ("quickeditnotecontents", get_checkbox_status(request->database_config_user ()->getQuickNoteEditLink ()));

  
  // Whether the list of consultation notes shows the Bible the note refers to.
  if (checkbox == "showbibleinnoteslist") {
    request->database_config_user ()->setShowBibleInNotesList (checked);
  }
  view.set_variable ("showbibleinnoteslist", get_checkbox_status(request->database_config_user ()->getShowBibleInNotesList ()));
  
  
  // Whether to display the note status in the notes list and the note display.
  // Setting for whether to colour the labels of the status of the consultation notes.
  // These two settings work together.
  if (checkbox == "shownotestatus") {
    request->database_config_user ()->setShowNoteStatus (checked);
    return get_reload ();
  }
  if (checkbox == "colorednotetatus") {
    request->database_config_user ()->setUseColoredNoteStatusLabels (checked);
  }
  {
    bool state = request->database_config_user ()->getShowNoteStatus ();
    if (state) view.enable_zone ("notestatuson");
    view.set_variable ("shownotestatus", get_checkbox_status(state));
  }
  view.set_variable ("colorednotetatus", get_checkbox_status(request->database_config_user ()->getUseColoredNoteStatusLabels ()));

  
  // Whether to show the text of the focused Bible passage, while creating a new Consultation Note.
  // This helps the translators and consultants if they are using notes in full screen mode,
  // rather than from a workspace that may already show the focused Bible verse text.
  // It shows the users if they have the focus on the verse they want to comment on.
  // It makes it easy for them to grab a few words of the text to place within the note being created.
  if (checkbox == "showversetextcreatenote") {
    request->database_config_user ()->setShowVerseTextAtCreateNote (checked);
  }
  view.set_variable ("showversetextcreatenote", get_checkbox_status(request->database_config_user ()->getShowVerseTextAtCreateNote ()));
  
  
  // Whether to disable the "Copy / Paste / SelectAll / ..." popup on Chrome OS.
  // This pop-up appears on some Chrome OS devices when selecting text in an editable area.
  // See https://github.com/bibledit/cloud/issues/282 for more information.
  if (checkbox == "disableselectionpopupchromeos") {
    Database_Config_General::setDisableSelectionPopupChromeOS (checked);
  }
  view.set_variable ("disableselectionpopupchromeos", get_checkbox_status(Database_Config_General::getDisableSelectionPopupChromeOS ()));
  if (config_globals_running_on_chrome_os) {
    view.enable_zone ("chromeos");
  }

  
  // Setting for the verse separator during notes entry.
  if (request->post.count ("verseseparator")) {
    Database_Config_General::setNotesVerseSeparator (request->post["verseseparator"]);
    return "";
  }
  string separator_key = Database_Config_General::getNotesVerseSeparator ();
  string separator_html;
  separator_html = Options_To_Select::add_selection (menu_logic_verse_separator ("."), ".", separator_html);
  separator_html = Options_To_Select::add_selection (menu_logic_verse_separator (":"), ":", separator_html);
  view.set_variable ("verseseparatoroptags", Options_To_Select::mark_selected (separator_key, separator_html));
  view.set_variable ("verseseparator", menu_logic_verse_separator (separator_key));

  
  // Setting for whether to receive the focused reference from Paratext on Windows.
  if (checkbox == "referencefromparatext") {
    request->database_config_user ()->setReceiveFocusedReferenceFromParatext (checked);
  }
  view.set_variable ("referencefromparatext", get_checkbox_status(request->database_config_user ()->getReceiveFocusedReferenceFromParatext ()));

  
  // Setting for whether to receive the focused reference from Accordance on macOS.
  if (checkbox == "referencefromaccordance") {
    request->database_config_user ()->setReceiveFocusedReferenceFromAccordance (checked);
  }
  view.set_variable ("referencefromaccordance", get_checkbox_status(request->database_config_user ()->getReceiveFocusedReferenceFromAccordance ()));

  
  // Enable the sections with settings relevant to the user and device.
  bool resources = access_logic_privilege_view_resources (webserver_request);
  if (resources) view.enable_zone ("resources");
  bool bibles = Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
  auto [ read, write ] = AccessBible::Any (webserver_request);
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

  
#ifdef HAVE_WINDOWS
  view.enable_zone ("windows");
#endif

  
  bool enable_accordance_settings = false;
#ifdef HAVE_MAC
  enable_accordance_settings = true;
#endif
#ifdef HAVE_WINDOWS
  enable_accordance_settings = true;
#endif
  if (enable_accordance_settings) view.enable_zone ("macos");

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("personalize", "index");

  page += Assets_Page::footer ();
  
  return page;
}

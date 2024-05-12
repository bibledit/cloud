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


#include <personalize/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <database/config/general.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <database/cache.h>
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
#include <config/logic.h>


std::string personalize_index_url ()
{
  return "personalize/index";
}


bool personalize_index_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


std::string personalize_index (Webserver_Request& webserver_request)
{
  std::string checkbox = webserver_request.post ["checkbox"];
  bool checked = filter::strings::convert_to_bool (webserver_request.post ["checked"]);


  // Accept values for allowed relative changes for the four Bible text editors.
  if (webserver_request.post.count ("chapterpercentage")) {
    int chapterpercentage = filter::strings::convert_to_int (webserver_request.post ["chapterpercentage"]);
    chapterpercentage = clip (chapterpercentage, 10, 100);
    webserver_request.database_config_user ()->setEditingAllowedDifferenceChapter (chapterpercentage);
    return std::string();
  }
  if (webserver_request.post.count ("versepercentage")) {
    int versepercentage = filter::strings::convert_to_int (webserver_request.post ["versepercentage"]);
    versepercentage = clip (versepercentage, 10, 100);
    webserver_request.database_config_user ()->setEditingAllowedDifferenceVerse (versepercentage);
    return std::string();
  }
  

  // Set the user chosen theme as the current theme.
  if (webserver_request.post.count ("themepicker")) {
    int themepicker = filter::strings::convert_to_int (webserver_request.post ["themepicker"]);
    if (config::logic::default_bibledit_configuration ()) {
      webserver_request.database_config_user ()->setCurrentTheme (themepicker);
    }
  }

  
  std::string page;
  std::string success;
  std::string error;


  // Store new font sizes before displaying the header,
  // so that the page displays the new font sizes immediately.
  if (webserver_request.post.count ("fontsizegeneral")) {
    int fontsizegeneral = filter::strings::convert_to_int (webserver_request.post["fontsizegeneral"]);
    fontsizegeneral = clip (fontsizegeneral, 50, 300);
    if (config::logic::default_bibledit_configuration ()) {
      webserver_request.database_config_user ()->setGeneralFontSize (fontsizegeneral);
    }
    return std::string();
  }
  if (webserver_request.post.count ("fontsizemenu")) {
    int fontsizemenu = filter::strings::convert_to_int (webserver_request.post["fontsizemenu"]);
    fontsizemenu = clip (fontsizemenu, 50, 300);
    if (config::logic::default_bibledit_configuration ()) {
      webserver_request.database_config_user ()->setMenuFontSize (fontsizemenu);
    }
    return std::string();
  }
  
  
  Assets_Header header = Assets_Header (translate("Preferences"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view{};

  
  // Font size for everything.
  if (config::logic::default_bibledit_configuration ()) {
    view.set_variable ("fontsizegeneral", filter::strings::convert_to_string (webserver_request.database_config_user ()->getGeneralFontSize ()));
  }

  
  // Font size for the menu.
  if (config::logic::default_bibledit_configuration ()) {
    view.set_variable ("fontsizemenu", filter::strings::convert_to_string (webserver_request.database_config_user ()->getMenuFontSize ()));
  }
  
  
  // Font size for the Bible editors.
  if (webserver_request.post.count ("fontsizeeditors")) {
    int fontsizeeditors = filter::strings::convert_to_int (webserver_request.post["fontsizeeditors"]);
    fontsizeeditors = clip (fontsizeeditors, 50, 300);
    webserver_request.database_config_user ()->setBibleEditorsFontSize (fontsizeeditors);
    styles_sheets_create_all ();
    return std::string();
  }
  view.set_variable ("fontsizeeditors", filter::strings::convert_to_string (webserver_request.database_config_user ()->getBibleEditorsFontSize ()));
  
  
  // Font size for the resources.
  if (webserver_request.post.count ("fontsizeresources")) {
    int fontsizeresources = filter::strings::convert_to_int (webserver_request.post["fontsizeresources"]);
    fontsizeresources = clip (fontsizeresources, 50, 300);
    if (config::logic::default_bibledit_configuration ()) {
      webserver_request.database_config_user ()->setResourcesFontSize (fontsizeresources);
    }
    return std::string();
  }
  view.set_variable ("fontsizeresources", filter::strings::convert_to_string (webserver_request.database_config_user ()->getResourcesFontSize ()));
  
  
  // Font size for Hebrew resources.
  if (webserver_request.post.count ("fontsizehebrew")) {
    int fontsizehebrew = filter::strings::convert_to_int (webserver_request.post["fontsizehebrew"]);
    fontsizehebrew = clip (fontsizehebrew, 50, 300);
    if (config::logic::default_bibledit_configuration ()) {
      webserver_request.database_config_user ()->setHebrewFontSize (fontsizehebrew);
    }
    return std::string();
  }
  view.set_variable ("fontsizehebrew", filter::strings::convert_to_string (webserver_request.database_config_user ()->getHebrewFontSize ()));
  
  
  // Font size for Greek resources.
  if (webserver_request.post.count ("fontsizegreek")) {
    int fontsizegreek = filter::strings::convert_to_int (webserver_request.post["fontsizegreek"]);
    fontsizegreek = clip (fontsizegreek, 50, 300);
    if (config::logic::default_bibledit_configuration ()) {
      webserver_request.database_config_user ()->setGreekFontSize (fontsizegreek);
    }
    return std::string();
  }
  view.set_variable ("fontsizegreek", filter::strings::convert_to_string (webserver_request.database_config_user ()->getGreekFontSize ()));
  
  
  // Vertical caret position in chapter editors.
  if (webserver_request.post.count ("caretposition")) {
    int caretposition = filter::strings::convert_to_int (webserver_request.post["caretposition"]);
    caretposition = clip (caretposition, 20, 80);
    webserver_request.database_config_user ()->setVerticalCaretPosition (caretposition);
    return std::string();
  }
  view.set_variable ("caretposition", filter::strings::convert_to_string (webserver_request.database_config_user ()->getVerticalCaretPosition ()));
  

  // Whether to display bread crumbs.
  if (checkbox == "breadcrumbs") {
    webserver_request.database_config_user ()->setDisplayBreadcrumbs (checked);
    return filter::strings::get_reload ();
  }
  view.set_variable ("breadcrumbs", filter::strings::get_checkbox_status (webserver_request.database_config_user ()->getDisplayBreadcrumbs ()));

  
  // Set the chosen theme on the option HTML tag.
  std::string theme_key = filter::strings::convert_to_string (webserver_request.database_config_user ()->getCurrentTheme ());
  std::string filename = current_theme_filebased_cache_filename (webserver_request.session_identifier);
  std::string theme_html;
  theme_html = Options_To_Select::add_selection ("Basic", "0", theme_html);
  theme_html = Options_To_Select::add_selection ("Light", "1", theme_html);
  theme_html = Options_To_Select::add_selection ("Dark", "2", theme_html);
  theme_html = Options_To_Select::add_selection ("Red Blue Light", "3", theme_html);
  theme_html = Options_To_Select::add_selection ("Red Blue Dark", "4", theme_html);
  view.set_variable ("themepickeroptags", Options_To_Select::mark_selected (theme_key, theme_html));
  view.set_variable ("themepicker", theme_key);

  
  // Workspace menu fade-out delay.
  if (webserver_request.post.count ("workspacefadeoutdelay")) {
    int workspacefadeoutdelay = filter::strings::convert_to_int (webserver_request.post["workspacefadeoutdelay"]);
    workspacefadeoutdelay = clip (workspacefadeoutdelay, 0, 100);
    webserver_request.database_config_user ()->setWorkspaceMenuFadeoutDelay (workspacefadeoutdelay);
    return std::string();
  }
  view.set_variable ("workspacefadeoutdelay", filter::strings::convert_to_string (webserver_request.database_config_user ()->getWorkspaceMenuFadeoutDelay ()));

  
  // Permissable relative changes in the two to four Bible editors.
  view.set_variable ("chapterpercentage", filter::strings::convert_to_string (webserver_request.database_config_user ()->getEditingAllowedDifferenceChapter ()));
  view.set_variable ("versepercentage", filter::strings::convert_to_string (webserver_request.database_config_user ()->getEditingAllowedDifferenceVerse ()));
  
  
  // Whether to keep the main menu always visible.
  if (checkbox == "menuvisible") {
    webserver_request.database_config_user ()->setMainMenuAlwaysVisible (checked);
    return filter::strings::get_reload ();
  }
  view.set_variable ("menuvisible", filter::strings::get_checkbox_status (webserver_request.database_config_user ()->getMainMenuAlwaysVisible ()));
  
  
  // Whether to enable swipe actions.
  if (checkbox == "swipeactions") {
    webserver_request.database_config_user ()->setSwipeActionsAvailable (checked);
    return std::string();
  }
  view.set_variable ("swipeactions", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getSwipeActionsAvailable ()));
  
  
  // Whether to enable fast Bible editor switching.
  if (checkbox == "fasteditorswitch") {
    webserver_request.database_config_user ()->setFastEditorSwitchingAvailable (checked);
    return std::string();
  }
  view.set_variable ("fasteditorswitch", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getFastEditorSwitchingAvailable ()));

  
  // Visual editors in the fast Bible editor switcher.
  const char * fastswitchvisualeditors = "fastswitchvisualeditors";
  std::string visual_editors_html;
  for (int i = 0; i < 3; i++) {
    visual_editors_html = Options_To_Select::add_selection (menu_logic_editor_settings_text (true, i), filter::strings::convert_to_string (i), visual_editors_html);
  }
  if (webserver_request.post.count (fastswitchvisualeditors)) {
    int visual_editor_key = filter::strings::convert_to_int (webserver_request.post [fastswitchvisualeditors]);
    webserver_request.database_config_user ()->setFastSwitchVisualEditors (visual_editor_key);
    return std::string();
  }
  std::string editor_key = filter::strings::convert_to_string (webserver_request.database_config_user ()->getFastSwitchVisualEditors ());
  view.set_variable ("fastswitchvisualeditorsoptags", Options_To_Select::mark_selected (editor_key, visual_editors_html));
  view.set_variable (fastswitchvisualeditors, editor_key);

  
  // USFM editors fast Bible editor switcher.
  const char * fastswitchusfmeditors = "fastswitchusfmeditors";
  std::string usfm_editors_html;
  for (int i = 0; i < 2; i++) {
    usfm_editors_html = Options_To_Select::add_selection (menu_logic_editor_settings_text (false, i), filter::strings::convert_to_string (i), usfm_editors_html);
  }
  if (webserver_request.post.count (fastswitchusfmeditors)) {
    int usfm_editor_key = filter::strings::convert_to_int (webserver_request.post [fastswitchusfmeditors]);
    webserver_request.database_config_user ()->setFastSwitchUsfmEditors (usfm_editor_key);
    return std::string();
  }
  editor_key = filter::strings::convert_to_string(webserver_request.database_config_user ()->getFastSwitchUsfmEditors ());
  view.set_variable ("fastswitchusfmeditorsoptags", Options_To_Select::mark_selected (editor_key, usfm_editors_html));
  view.set_variable (fastswitchusfmeditors, editor_key);

  
  // Whether to enable editing styles in the visual editors.
  if (checkbox == "enablestylesbutton") {
    webserver_request.database_config_user ()->setEnableStylesButtonVisualEditors (checked);
    return std::string();
  }
  view.set_variable ("enablestylesbutton", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getEnableStylesButtonVisualEditors ()));
  

  // Change the active Bible.
  if (webserver_request.query.count ("changebible")) {
    std::string changebible = webserver_request.query ["changebible"];
    if (changebible == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to make the active one for editing"), "", "");
      std::vector <std::string> bibles = access_bible::bibles (webserver_request);
      for (auto & bible : bibles) {
        dialog_list.add_row (bible, "changebible", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      webserver_request.database_config_user()->setBible (changebible);
      // Going to another Bible, ensure that the focused book exists there.
      int book = Ipc_Focus::getBook (webserver_request);
      std::vector <int> books = webserver_request.database_bibles()->get_books (changebible);
      if (find (books.begin(), books.end(), book) == books.end()) {
        if (!books.empty ()) book = books [0];
        else book = 0;
        Ipc_Focus::set (webserver_request, book, 1, 1);
      }
    }
  }
  std::string bible = access_bible::clamp (webserver_request, webserver_request.database_config_user()->getBible ());
  view.set_variable ("bible", bible);

  
  // Whether to have a menu entry for the Changes in basic mode.
  if (checkbox == "showchanges") {
    webserver_request.database_config_user ()->setMenuChangesInBasicMode (checked);
    menu_logic_tabbed_mode_save_json (webserver_request);
    return filter::strings::get_reload ();
  }
  view.set_variable ("showchanges", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getMenuChangesInBasicMode ()));

  
  // Whether to put the controls for dismissing the change notifications at the top of the page.
  if (checkbox == "dismisschangesattop") {
    webserver_request.database_config_user ()->setDismissChangesAtTop (checked);
  }
  view.set_variable ("dismisschangesattop", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getDismissChangesAtTop ()));
  
  
  // Setting for whether to show the main menu in tabbed view in basic mode on phones and tablets.
  if (checkbox == "mainmenutabs") {
    Database_Config_General::setMenuInTabbedViewOn (checked);
    menu_logic_tabbed_mode_save_json (webserver_request);
  }
  if (menu_logic_can_do_tabbed_mode ()) {
    view.enable_zone ("tabs_possible");
    view.set_variable ("mainmenutabs", filter::strings::get_checkbox_status(Database_Config_General::getMenuInTabbedViewOn ()));
  }

  
  // Whether to enable a quick link to edit the content of a consultation note.
  if (checkbox == "quickeditnotecontents") {
    webserver_request.database_config_user ()->setQuickNoteEditLink (checked);
  }
  view.set_variable ("quickeditnotecontents", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getQuickNoteEditLink ()));

  
  // Whether the list of consultation notes shows the Bible the note refers to.
  if (checkbox == "showbibleinnoteslist") {
    webserver_request.database_config_user ()->setShowBibleInNotesList (checked);
  }
  view.set_variable ("showbibleinnoteslist", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getShowBibleInNotesList ()));
  
  
  // Whether to display the note status in the notes list and the note display.
  // Setting for whether to colour the labels of the status of the consultation notes.
  // These two settings work together.
  if (checkbox == "shownotestatus") {
    webserver_request.database_config_user ()->setShowNoteStatus (checked);
    return filter::strings::get_reload ();
  }
  if (checkbox == "colorednotetatus") {
    webserver_request.database_config_user ()->setUseColoredNoteStatusLabels (checked);
  }
  {
    bool state = webserver_request.database_config_user ()->getShowNoteStatus ();
    if (state) view.enable_zone ("notestatuson");
    view.set_variable ("shownotestatus", filter::strings::get_checkbox_status(state));
  }
  view.set_variable ("colorednotetatus", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getUseColoredNoteStatusLabels ()));

  
  // Whether to show the text of the focused Bible passage, while creating a new Consultation Note.
  // This helps the translators and consultants if they are using notes in full screen mode,
  // rather than from a workspace that may already show the focused Bible verse text.
  // It shows the users if they have the focus on the verse they want to comment on.
  // It makes it easy for them to grab a few words of the text to place within the note being created.
  if (checkbox == "showversetextcreatenote") {
    webserver_request.database_config_user ()->setShowVerseTextAtCreateNote (checked);
  }
  view.set_variable ("showversetextcreatenote", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getShowVerseTextAtCreateNote ()));
  
  
  // Whether to disable the "Copy / Paste / SelectAll / ..." popup on Chrome OS.
  // This pop-up appears on some Chrome OS devices when selecting text in an editable area.
  // See https://github.com/bibledit/cloud/issues/282 for more information.
  if (checkbox == "disableselectionpopupchromeos") {
    Database_Config_General::setDisableSelectionPopupChromeOS (checked);
  }
  view.set_variable ("disableselectionpopupchromeos", filter::strings::get_checkbox_status(Database_Config_General::getDisableSelectionPopupChromeOS ()));
  if (config_globals_running_on_chrome_os) {
    view.enable_zone ("chromeos");
  }

  
  // Setting for the verse separator during notes entry.
  if (webserver_request.post.count ("verseseparator")) {
    Database_Config_General::setNotesVerseSeparator (webserver_request.post["verseseparator"]);
    return std::string();
  }
  std::string separator_key = Database_Config_General::getNotesVerseSeparator ();
  std::string separator_html;
  separator_html = Options_To_Select::add_selection (menu_logic_verse_separator ("."), ".", separator_html);
  separator_html = Options_To_Select::add_selection (menu_logic_verse_separator (":"), ":", separator_html);
  view.set_variable ("verseseparatoroptags", Options_To_Select::mark_selected (separator_key, separator_html));
  view.set_variable ("verseseparator", menu_logic_verse_separator (separator_key));

  
  // Setting for whether to receive the focused reference from Paratext on Windows.
  if (checkbox == "referencefromparatext") {
    webserver_request.database_config_user ()->setReceiveFocusedReferenceFromParatext (checked);
  }
  view.set_variable ("referencefromparatext", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getReceiveFocusedReferenceFromParatext ()));

  
  // Setting for whether to receive the focused reference from Accordance on macOS.
  if (checkbox == "referencefromaccordance") {
    webserver_request.database_config_user ()->setReceiveFocusedReferenceFromAccordance (checked);
  }
  view.set_variable ("referencefromaccordance", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->getReceiveFocusedReferenceFromAccordance ()));

  
  // The date format to be used in the Consultation Notes.
  const char * dateformat = "dateformat";
  if (webserver_request.post.count (dateformat)) {
    int date_format_key = filter::strings::convert_to_int (webserver_request.post [dateformat]);
    webserver_request.database_config_user ()->setNotesDateFormat(date_format_key);
    return std::string();
  }
  std::string date_format_key = filter::strings::convert_to_string (webserver_request.database_config_user ()->getNotesDateFormat());
  std::string date_format_html;
  for (filter::date::date_format df = filter::date::dd_mm_yyyy;
       df <= filter::date::yyyy_mn_dd;
       df = static_cast<filter::date::date_format>(df + 1)) {
    date_format_html = Options_To_Select::add_selection (filter::date::date_format_to_text (df), filter::strings::convert_to_string(df), date_format_html);
  }
  view.set_variable ("dateformatoptags", Options_To_Select::mark_selected (date_format_key, date_format_html));
  view.set_variable (dateformat, date_format_key);

  
  // Spell check in the Bible editors.
  if (checkbox == "spellcheck") {
    webserver_request.database_config_user ()->set_enable_spell_check (checked);
  }
  view.set_variable ("spellcheck", filter::strings::get_checkbox_status(webserver_request.database_config_user ()->get_enable_spell_check ()));
  
  // Enable the sections with settings relevant to the user and device.
  bool resources = access_logic::privilege_view_resources (webserver_request);
  if (resources) view.enable_zone ("resources");
  bool bibles = Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
  auto [ read, write ] = access_bible::any (webserver_request);
  if (read || write) bibles = true;
  if (bibles) view.enable_zone ("bibles");
  if (webserver_request.session_logic ()->touchEnabled ()) {
    view.enable_zone ("touch");
  }


  // Enable the sections for either basic or advanced mode.
  if (webserver_request.database_config_user ()->getBasicInterfaceMode ()) {
    view.enable_zone ("basicmode");
    if (webserver_request.database_config_user ()->getPrivilegeUseAdvancedMode ()) {
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

  page += assets_page::footer ();
  
  return page;
}

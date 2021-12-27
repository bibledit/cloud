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


#include <system/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <dialog/list.h>
#include <dialog/list2.h>
#include <dialog/entry.h>
#include <dialog/upload.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/jobs.h>
#include <database/mail.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <config/globals.h>
#include <rss/feed.h>
#include <rss/logic.h>
#include <assets/external.h>
#include <jobs/index.h>
#include <tasks/logic.h>
#include <journal/logic.h>
#include <journal/index.h>
#include <fonts/logic.h>
#include <manage/index.h>
#include <client/logic.h>


string system_index_url ()
{
  return "system/index";
}


bool system_index_acl (void * webserver_request)
{
#ifdef HAVE_CLIENT
  (void) webserver_request;
  // Client: Anyone can make system settings.
  return true;
#else
  // Cloud: Manager can make system settings.
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
#endif
}


string system_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  string page;
  string success;
  string error;

  
  // User can set the system language.
  // This is to be done before displaying the header.
  if (request->post.count ("languageselection")) {
    string languageselection = request->post ["languageselection"];
    Database_Config_General::setSiteLanguage (languageselection);
  }

  
  // The header: The language has been set already.
  Assets_Header header = Assets_Header (translate("System"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();

  
  Assets_View view;


  // Get values for setting checkboxes.
  string checkbox = request->post ["checkbox"];
  bool checked = convert_to_bool (request->post ["checked"]);
#ifdef HAVE_CLIENT
  (void) checked;
#endif


  // The available localizations.
  map <string, string> localizations = locale_logic_localizations ();


  // Set the language on the page.
    // Create the option tags for interface language selection.
  // Also the current selected option.
  string language_html;
  for (auto element : localizations) {
    language_html = Options_To_Select::add_selection (element.second, element.first, language_html);
  }
  string current_user_preference = Database_Config_General::getSiteLanguage ();
  string language = current_user_preference;
  view.set_variable ("languageselectionoptags", Options_To_Select::mark_selected (language, language_html));
  view.set_variable ("languageselection", language);

  
  // Entry of time zone offset in hours.
  if (request->post.count ("timezone")) {
    string input = request->post ["timezone"];
    input = filter_string_str_replace ("UTC", "", input);
    int timezone = convert_to_int (input);
    timezone = clip (timezone, MINIMUM_TIMEZONE, MAXIMUM_TIMEZONE);
    Database_Config_General::setTimezone (timezone);
  }
  // Set the time zone offset in the GUI.
  int timezone = Database_Config_General::getTimezone();
  view.set_variable ("timezone", convert_to_string (timezone));
  // Display the section to set the site's timezone only
  // in case the calling program has not yet set this zone in the library.
  // So for example the app for iOS can set the timezone from the device,
  // and in case this has been done, the user no longer can set it through Bibledit.
  if ((config_globals_timezone_offset_utc < MINIMUM_TIMEZONE)
      || (config_globals_timezone_offset_utc > MAXIMUM_TIMEZONE)) {
    view.enable_zone ("timezone");
  }

  
#ifdef HAVE_CLOUD
  // Whether to include the author with every change in the RSS feed.
  if (checkbox == "rssauthor") {
    Database_Config_General::setAuthorInRssFeed (checked);
    return "";
  }
  view.set_variable ("rssauthor", get_checkbox_status (Database_Config_General::getAuthorInRssFeed ()));
  // The location of the RSS feed.
  view.set_variable ("rssfeed", rss_feed_url ());
  // The Bibles that send their changes to the RSS feed.
  Database_Bibles database_bibles;
  vector <string> bibles = database_bibles.getBibles ();
  string rssbibles;
  for (auto bible : bibles) {
    if (Database_Config_Bible::getSendChangesToRSS (bible)) {
      if (!rssbibles.empty ()) rssbibles.append (" ");
      rssbibles.append (bible);
    }
  }
  if (rssbibles.empty ()) {
    rssbibles.append (translate ("none"));
  }
  view.set_variable ("rssbibles", rssbibles);
#endif

  
#ifdef HAVE_CLIENT
  bool producebibles = request->query.count ("producebibles");
  bool producenotes = request->query.count ("producenotes");
  bool produceresources = request->query.count ("produceresources");
  if (producebibles || producenotes || produceresources) {
    Database_Jobs database_jobs;
    int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, Filter_Roles::member ());
    string task;
    if (producebibles) task = PRODUCEBIBLESTRANSFERFILE;
    if (producenotes) task = PRODUCERENOTESTRANSFERFILE;
    if (produceresources) task = PRODUCERESOURCESTRANSFERFILE;
    tasks_logic_queue (task, { convert_to_string (jobId) });
    redirect_browser (request, jobs_index_url () + "?id=" + convert_to_string (jobId));
    return "";
  }
#endif

  
#ifdef HAVE_CLIENT
  string importbibles = "importbibles";
  if (request->query.count (importbibles)) {
    if (request->post.count ("upload")) {
      string datafile = filter_url_tempfile () + request->post ["filename"];
      string data = request->post ["data"];
      if (!data.empty ()) {
        filter_url_file_put_contents (datafile, data);
        success = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (IMPORTBIBLESTRANSFERFILE, { datafile });
      } else {
        error = translate ("Nothing was imported");
      }
    } else {
      Dialog_Upload dialog = Dialog_Upload ("index", translate("Import a file with local Bibles"));
      dialog.add_upload_query (importbibles, "");
      page.append (dialog.run ());
      return page;
    }
  }
#endif

  
#ifdef HAVE_CLIENT
  string importnotes = "importnotes";
  if (request->query.count (importnotes)) {
    if (request->post.count ("upload")) {
      string datafile = filter_url_tempfile () + request->post ["filename"];
      string data = request->post ["data"];
      if (!data.empty ()) {
        filter_url_file_put_contents (datafile, data);
        success = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (IMPORTNOTESTRANSFERFILE, { datafile });
      } else {
        error = translate ("Nothing was imported");
      }
    } else {
      Dialog_Upload dialog = Dialog_Upload ("index", translate("Import a file with local Consultation Notes"));
      dialog.add_upload_query (importnotes, "");
      page.append (dialog.run ());
      return page;
    }
  }
#endif
  
  
#ifdef HAVE_CLIENT
  string importresources = "importresources";
  if (request->query.count (importresources)) {
    if (request->post.count ("upload")) {
      string datafile = filter_url_tempfile () + request->post ["filename"];
      string data = request->post ["data"];
      if (!data.empty ()) {
        filter_url_file_put_contents (datafile, data);
        success = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (IMPORTRESOURCESTRANSFERFILE, { datafile });
      } else {
        error = translate ("Nothing was imported");
      }
    } else {
      Dialog_Upload dialog = Dialog_Upload ("index", translate("Import a file with local Resources"));
      dialog.add_upload_query (importresources, "");
      page.append (dialog.run ());
      return page;
    }
  }
#endif

  
  // Force re-index Bibles.
  if (request->query ["reindex"] == "bibles") {
    Database_Config_General::setIndexBibles (true);
    tasks_logic_queue (REINDEXBIBLES, {"1"});
    redirect_browser (request, journal_index_url ());
    return "";
  }
  
  
  // Re-index consultation notes.
  if (request->query ["reindex"] == "notes") {
    Database_Config_General::setIndexNotes (true);
    tasks_logic_queue (REINDEXNOTES);
    redirect_browser (request, journal_index_url ());
    return "";
  }

  
  // Delete a font.
  string deletefont = request->query ["deletefont"];
  if (!deletefont.empty ()) {
    string font = filter_url_basename_web (deletefont);
    bool font_in_use = false;
    vector <string> bibles = request->database_bibles ()->getBibles ();
    for (auto & bible : bibles) {
      if (font == Fonts_Logic::getTextFont (bible)) font_in_use = true;
    }
    if (!font_in_use) {
      // Only delete a font when it is not in use.
      Fonts_Logic::erase (font);
    } else {
      error = translate("The font could not be deleted because it is in use");
    }
  }
  
  
  // Upload a font.
  if (request->post.count ("uploadfont")) {
    string filename = request->post ["filename"];
    string path = filter_url_create_root_path ("fonts", filename);
    string fontdata = request->post ["fontdata"];
    filter_url_file_put_contents (path, fontdata);
    success = translate("The font has been uploaded.");
  }
  
  
  // Assemble the font block html.
  vector <string> fonts = Fonts_Logic::getFonts ();
  stringstream fontsblock;
  for (auto & font : fonts) {
    fontsblock << "<p>";
#ifndef HAVE_CLIENT
    fontsblock << "<a href=" << quoted ("?deletefont=" + font) << " title=" << quoted(translate("Delete font")) << ">" << emoji_wastebasket () << "</a>";
#endif
    fontsblock << font;
    fontsblock << "</p>";
  }
  view.set_variable ("fontsblock", fontsblock.str());

  
  // Handle the command to clear the web and resources caches.
  if (request->query.count ("clearcache")) {
    tasks_logic_queue (CLEARCACHES);
    redirect_browser (request, journal_index_url ());
    return "";
  }
  
  
  // Handle the setting whether to keep the resource caches for an extended period of time.
#ifdef HAVE_CLOUD
  if (checkbox == "keepcache") {
    Database_Config_General::setKeepResourcesCacheForLong (checked);
    return "";
  }
  view.set_variable ("keepcache", get_checkbox_status (Database_Config_General::getKeepResourcesCacheForLong ()));
#endif


  // Handle display the number of unsent emails and clearing them.
#ifdef HAVE_CLOUD
  Database_Mail database_mail (webserver_request);
  if (request->query.count ("clearemails")) {
    vector <int> mails = database_mail.getAllMails ();
    for (auto rowid : mails) {
      database_mail.erase (rowid);
    }
  }
  vector <int> mails = database_mail.getAllMails ();
  string mailcount = convert_to_string (mails.size());
  view.set_variable ("emailscount", mailcount);
#endif

  
#ifdef HAVE_CLOUD
  view.enable_zone ("cloud");
#endif
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
  view.set_variable ("cloudlink", client_logic_link_to_cloud (manage_index_url (), ""));
#endif
  
  
  view.set_variable ("external", assets_external_logic_link_addon ());


  // Set some feedback, if any.
  view.set_variable ("success", success);
  view.set_variable ("error", error);

  
  page += view.render ("system", "index");
  page += Assets_Page::footer ();
  return page;
}

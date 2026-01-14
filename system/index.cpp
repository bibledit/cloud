/*
Copyright (©) 2003-2026 Teus Benschop.

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
#include <dialog/select.h>
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


std::string system_index_url ()
{
  return "system/index";
}


bool system_index_acl ([[maybe_unused]] Webserver_Request& webserver_request)
{
#ifdef HAVE_CLIENT
  // Client: Anyone can make system settings.
  return true;
#else
  // Cloud: Manager can make system settings.
  return roles::access_control (webserver_request, roles::manager);
#endif
}


std::string system_index (Webserver_Request& webserver_request)
{
  std::string page {};
  std::string success {};
  std::string error {};

  
  Assets_View view;

  
  // User can set the system language.
  // This is to be done before displaying the header.
  std::string language = database::config::general::get_site_language ();
  {
    constexpr const char* identification {"languageselection"};
    if (webserver_request.post_count(identification)) {
      language = webserver_request.post_get(identification);
      database::config::general::set_site_language (language);
    }
    const std::map <std::string, std::string> localizations = locale_logic_localizations ();
    std::vector<std::string> values, texts;
    for (const auto& element : localizations) {
      values.push_back(element.first);
      texts.push_back(element.second);
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = values,
      .displayed = texts,
      .selected = language,
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

  
  // The header: The language has been set already.
  Assets_Header header = Assets_Header (translate("System"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();


  // Get values for setting checkboxes.
  const std::string checkbox = webserver_request.post_get("checkbox");
  [[maybe_unused]] const bool checked = filter::string::convert_to_bool (webserver_request.post_get("checked"));


  // Entry of time zone offset in hours.
  if (webserver_request.post_count("timezone")) {
    std::string input = webserver_request.post_get("timezone");
    input = filter::string::replace ("UTC", std::string(), input);
    int input_timezone = filter::string::convert_to_int (input);
    input_timezone = std::clamp (input_timezone, MINIMUM_TIMEZONE, MAXIMUM_TIMEZONE);
    database::config::general::set_timezone (input_timezone);
  }
  // Set the time zone offset in the GUI.
  const int timezone_setting = database::config::general::get_timezone();
  view.set_variable ("timezone", std::to_string (timezone_setting));
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
    database::config::general::set_uuthor_in_rss_feed (checked);
    return std::string();
  }
  view.set_variable ("rssauthor", filter::string::get_checkbox_status (database::config::general::get_author_in_rss_feed ()));
  // The location of the RSS feed.
  view.set_variable ("rssfeed", rss_feed_url ());
  // The Bibles that send their changes to the RSS feed.
  std::string rssbibles {};
  {
    std::vector <std::string> bibles = database::bibles::get_bibles ();
    for (const auto& bible : bibles) {
      if (database::config::bible::get_send_changes_to_rss (bible)) {
        if (!rssbibles.empty ()) rssbibles.append (" ");
        rssbibles.append (bible);
      }
    }
  }
  if (rssbibles.empty ()) {
    rssbibles.append (translate ("none"));
  }
  view.set_variable ("rssbibles", rssbibles);
#endif

  
#ifdef HAVE_CLIENT
  const bool producebibles = webserver_request.query.count ("producebibles");
  const bool producenotes = webserver_request.query.count ("producenotes");
  const bool produceresources = webserver_request.query.count ("produceresources");
  if (producebibles || producenotes || produceresources) {
    Database_Jobs database_jobs;
    const int jobId = database_jobs.get_new_id ();
    database_jobs.set_level (jobId, roles::member);
    std::string task {};
    if (producebibles) task = task::produce_bibles_transferfile;
    if (producenotes) task = task::produce_notes_transferfile;
    if (produceresources) task = task::produce_resources_transferfile;
    tasks_logic_queue (task, { std::to_string(jobId) });
    redirect_browser (webserver_request, jobs_index_url () + "?id=" + std::to_string(jobId));
    return std::string();
  }
#endif

  
#ifdef HAVE_CLIENT
  const std::string importbibles = "importbibles";
  if (webserver_request.query.count (importbibles)) {
    if (webserver_request.post_count("upload")) {
      const std::string datafile = filter_url_tempfile () + webserver_request.post_get("filename");
      const std::string data = webserver_request.post_get("data");
      if (!data.empty ()) {
        filter_url_file_put_contents (datafile, data);
        success = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (task::import_bibles_transferfile, { datafile });
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
  const std::string importnotes = "importnotes";
  if (webserver_request.query.count (importnotes)) {
    if (webserver_request.post_count("upload")) {
      const std::string datafile = filter_url_tempfile () + webserver_request.post_get("filename");
      const std::string data = webserver_request.post_get("data");
      if (!data.empty ()) {
        filter_url_file_put_contents (datafile, data);
        success = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (task::import_notes_transferfile, { datafile });
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
  const std::string importresources = "importresources";
  if (webserver_request.query.count (importresources)) {
    if (webserver_request.post_count("upload")) {
      const std::string datafile = filter_url_tempfile () + webserver_request.post_get("filename");
      const std::string data = webserver_request.post_get("data");
      if (!data.empty ()) {
        filter_url_file_put_contents (datafile, data);
        success = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (task::import_resources_transferfile, { datafile });
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
  if (webserver_request.query ["reindex"] == "bibles") {
    database::config::general::set_index_bibles (true);
    tasks_logic_queue (task::reindex_bibles, {"1"});
    redirect_browser (webserver_request, journal_index_url ());
    return std::string();
  }
  
  
  // Re-index consultation notes.
  if (webserver_request.query ["reindex"] == "notes") {
    database::config::general::setIndexNotes (true);
    tasks_logic_queue (task::reindex_notes);
    redirect_browser (webserver_request, journal_index_url ());
    return std::string();
  }

  
  // Delete a font.
  const std::string deletefont = webserver_request.query ["deletefont"];
  if (!deletefont.empty ()) {
    const std::string font = filter_url_basename_web (deletefont);
    bool font_in_use = false;
    const std::vector <std::string> bibles = database::bibles::get_bibles ();
    for (const auto& bible : bibles) {
      if (font == fonts::logic::get_text_font (bible)) font_in_use = true;
    }
    if (!font_in_use) {
      // Only delete a font when it is not in use.
      fonts::logic::erase (font);
    } else {
      error = translate("The font could not be deleted because it is in use");
    }
  }
  
  
  // Upload a font.
  if (webserver_request.post_count("uploadfont")) {
    const std::string filename = webserver_request.post_get("filename");
    const std::string path = filter_url_create_root_path ({"fonts", filename});
    const std::string fontdata = webserver_request.post_get("fontdata");
    filter_url_file_put_contents (path, fontdata);
    success = translate("The font has been uploaded.");
  }
  
  
  // Assemble the font block html.
  const std::vector <std::string> fonts = fonts::logic::get_fonts ();
  std::stringstream fontsblock;
  for (const auto& font : fonts) {
    fontsblock << "<p>";
#ifndef HAVE_CLIENT
    fontsblock << "<a href=" << std::quoted ("?deletefont=" + font) << " title=" << std::quoted(translate("Delete font")) << ">" << filter::string::emoji_wastebasket () << "</a>";
#endif
    fontsblock << font;
    fontsblock << "</p>";
  }
  view.set_variable ("fontsblock", fontsblock.str());

  
  // Handle the command to clear the web and resources caches.
  if (webserver_request.query.count ("clearcache")) {
    tasks_logic_queue (task::clear_caches);
    redirect_browser (webserver_request, journal_index_url ());
    return std::string();
  }
  
  
  // Handle the setting whether to keep the resource caches for an extended period of time.
#ifdef HAVE_CLOUD
  if (checkbox == "keepcache") {
    database::config::general::set_keep_resources_cache_for_long (checked);
    return std::string();
  }
  view.set_variable ("keepcache", filter::string::get_checkbox_status (database::config::general::get_keep_resources_cache_for_long ()));
#endif


  // Handle display the number of unsent emails and clearing them.
#ifdef HAVE_CLOUD
  Database_Mail database_mail (webserver_request);
  if (webserver_request.query.count ("clearemails")) {
    const std::vector <int> mails = database_mail.getAllMails ();
    for (auto rowid : mails) {
      database_mail.erase (rowid);
    }
  }
  const std::vector <int> mails = database_mail.getAllMails ();
  const std::string mailcount = std::to_string (mails.size());
  view.set_variable ("emailscount", mailcount);
#endif

  
  // Handle the setting whether to keep available OSIS content in the SWORD resources.
#ifdef HAVE_CLOUD
  if (checkbox == "keeposis") {
    database::config::general::set_keep_osis_content_in_sword_resources (checked);
    return std::string();
  }
  view.set_variable ("keeposis", filter::string::get_checkbox_status (database::config::general::get_keep_osis_content_in_sword_resources ()));
#endif

  
#ifdef HAVE_CLOUD
  view.enable_zone ("cloud");
#endif
#ifdef HAVE_CLIENT
  view.enable_zone ("client");
  view.set_variable ("cloudlink", client_logic_link_to_cloud (manage_index_url (), std::string()));
#endif
  
  
  view.set_variable ("external", assets_external_logic_link_addon ());


  // Set some feedback, if any.
  view.set_variable ("success", success);
  view.set_variable ("error", error);

  
  page += view.render ("system", "index");
  page += assets_page::footer ();
  return page;
}

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


#include <bb/import.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <access/bible.h>
#include <tasks/logic.h>
#include <ipc/focus.h>
#include <config/config.h>
#include <menu/logic.h>
#include <bb/manage.h>
#include <assets/external.h>
#include <journal/logic.h>


std::string bible_import_url ()
{
  return "bible/import";
}


bool bible_import_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string bible_import (Webserver_Request& webserver_request)
{
  std::string page {};
  
  Assets_Header header = Assets_Header (translate("Import"), webserver_request);
  header.set_navigator ();
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (bible_manage_url (), menu_logic_bible_manage_text ());
  page = header.run ();
  
  Assets_View view {};
  
  std::string success_message {};
  std::string error_message {};
  
  // The name of the Bible.
  const std::string bible = access_bible::clamp (webserver_request, webserver_request.query["bible"]);
  view.set_variable ("bible", filter::strings::escape_special_xml_characters (bible));
  
  const int book = Ipc_Focus::getBook (webserver_request);
  const int chapter = Ipc_Focus::getChapter (webserver_request);

  // Whether the user has write access to this Bible.
  if (bool write_access = access_bible::write (webserver_request, bible); write_access) {
    view.enable_zone ("write_access");
  }

  // USFM data submission.
  if (webserver_request.post.count ("submit")) {
    // Submission may take long if there's a lot of data or the network is slow.
    std::string data = webserver_request.post ["data"];
    data = filter_url_tag_to_plus (data);
    data = filter::strings::trim (data);
    if (!data.empty()) {
      if (filter::strings::unicode_string_is_valid (data)) {
        const std::string datafile = filter_url_tempfile ();
        filter_url_file_put_contents (datafile, data);
        success_message = translate("Import has started.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
        tasks_logic_queue (IMPORTBIBLE, { datafile, bible, filter::strings::convert_to_string (book), filter::strings::convert_to_string (chapter) });
      } else {
        error_message = translate("Please supply valid Unicode UTF-8 text.");
      }
    } else {
      success_message = translate("Nothing was imported.");
    }
    // User imported something into this Bible: Set it as the default Bible.
    webserver_request.database_config_user()->setBible (bible);
  }

  // File upload.
  if (webserver_request.post.count ("upload")) {
    const std::string datafile = filter_url_tempfile () + webserver_request.post ["filename"];
    const std::string data = webserver_request.post ["data"];
    if (!data.empty ()) {
      filter_url_file_put_contents (datafile, data);
      success_message = translate("Import has started.");
      view.set_variable ("journal", journal_logic_see_journal_for_progress ());
      tasks_logic_queue (IMPORTBIBLE, { datafile, bible, filter::strings::convert_to_string (book), filter::strings::convert_to_string (chapter) });
    } else {
      error_message = translate ("Nothing was uploaded");
    }
    // User imported something into this Bible: Set it as the default Bible.
    webserver_request.database_config_user()->setBible (bible);
  }
  
#ifdef HAVE_UPLOAD
  view.enable_zone ("enable_upload");
#else
  view.enable_zone ("disable_upload");
#endif
  
  view.set_variable ("success_message", success_message);
  view.set_variable ("error_message", error_message);
  
  view.set_variable ("external", assets_external_logic_link_addon ());

  page += view.render ("bb", "import");
  
  page += assets_page::footer ();
  
  return page;
}

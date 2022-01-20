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


#include <images/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <dialog/yes.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <journal/index.h>
#include <tasks/logic.h>
#include <journal/logic.h>
#include <menu/logic.h>
#include <database/bibleimages.h>


string images_index_url ()
{
  return "images/index";
}


bool images_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string images_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_BibleImages database_bibleimages;

  
  string page;
  Assets_Header header = Assets_Header (translate("Bible images"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (images_index_url (), menu_logic_images_index_text ());
  page = header.run ();
  Assets_View view;
  string error, success;

  
  // File upload.
  if (request->post.count ("upload")) {
    string folder = filter_url_tempfile ();
    filter_url_mkdir (folder);
    string file = filter_url_create_path_cpp17 ({folder, request->post ["filename"]});
    string data = request->post ["data"];
    if (!data.empty ()) {
      filter_url_file_put_contents (file, data);
      bool background_import = filter_archive_is_archive (file);
      string extension = filter_url_get_extension_cpp17 (file);
      extension = unicode_string_casefold (extension);
      if (background_import) {
        tasks_logic_queue (IMPORTBIBLEIMAGES, { file });
        success = translate("The file was uploaded and is being processed.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
      } else {
        // Store image.
        database_bibleimages.store (file);
      }
    } else {
      error = translate ("Nothing was uploaded");
    }
  }


  // Delete image.
  string remove = request->query ["delete"];
  if (!remove.empty()) {
    string confirm = request->query ["confirm"];
    if (confirm.empty()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("index", translate("Would you like to delete this image?"));
      dialog_yes.add_query ("delete", remove);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      database_bibleimages.erase (remove);
      success = translate("The image was deleted.");
    }
  }

  
  vector <string> images = database_bibleimages.get();
  for (auto image : images) {
    view.add_iteration ("images", {
      pair ("image", image),
    } );
  }
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("images", "index");
  page += Assets_Page::footer ();
  return page;
}

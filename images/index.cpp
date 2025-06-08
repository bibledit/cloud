/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


std::string images_index_url ()
{
  return "images/index";
}


bool images_index_acl (Webserver_Request& webserver_request)
{
  return roles::access_control (webserver_request, roles::translator);
}


std::string images_index (Webserver_Request& webserver_request)
{
  Assets_Header header = Assets_Header (translate("Bible images"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (images_index_url (), menu_logic_images_index_text ());
  std::string page = header.run ();
  Assets_View view;
  std::string error, success;

  
  // File upload.
  if (webserver_request.post.count ("upload")) {
    const std::string& folder = filter_url_tempfile ();
    filter_url_mkdir (folder);
    const std::string& file = filter_url_create_path ({folder, webserver_request.post ["filename"]});
    const std::string& data = webserver_request.post ["data"];
    if (!data.empty ()) {
      filter_url_file_put_contents (file, data);
      const bool background_import = filter_archive_is_archive (file);
      std::string extension = filter_url_get_extension (file);
      extension = filter::strings::unicode_string_casefold (extension);
      if (background_import) {
        tasks_logic_queue (task::import_bible_images, { file });
        success = translate("The file was uploaded and is being processed.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
      } else {
        // Store image.
        database::bible_images::store (file);
      }
    } else {
      error = translate ("Nothing was uploaded");
    }
  }


  // Delete image.
  const std::string& remove = webserver_request.query ["delete"];
  if (!remove.empty()) {
    const std::string& confirm = webserver_request.query ["confirm"];
    if (confirm.empty()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("index", translate("Would you like to delete this image?"));
      dialog_yes.add_query ("delete", remove);
      page.append(dialog_yes.run ());
      return page;
    } if (confirm == "yes") {
      database::bible_images::erase (remove);
      success = translate("The image was deleted.");
    }
  }

  
  const std::vector <std::string> images = database::bible_images::get();
  for (const auto& image : images) {
    view.add_iteration ("images", {
      std::pair ("image", image),
    } );
  }
  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page.append (view.render ("images", "index"));
  page.append (assets_page::footer ());
  return page;
}

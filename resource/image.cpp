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


#include <resource/image.h>
#include <resource/img.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/imageresources.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <tasks/logic.h>
#include <menu/logic.h>
#include <resource/images.h>
#include <journal/logic.h>


std::string resource_image_url ()
{
  return "resource/image";
}


bool resource_image_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string resource_image (Webserver_Request& webserver_request)
{
  Database_ImageResources database_imageresources;

  
  std::string page;
  Assets_Header header = Assets_Header (translate("Image resources"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.add_bread_crumb (resource_images_url (), menu_logic_resource_images_text ());
  page = header.run ();
  Assets_View view;
  std::string error, success;
  
  
  std::string name = webserver_request.query ["name"];
  view.set_variable ("name", name);
  

  // File upload.
  if (webserver_request.post.count ("upload")) {
    std::string folder = filter_url_tempfile ();
    filter_url_mkdir (folder);
    std::string file =  filter_url_create_path ({folder, webserver_request.post ["filename"]});
    std::string data = webserver_request.post ["data"];
    if (!data.empty ()) {
      filter_url_file_put_contents (file, data);
      bool background_import = false;
      if (filter_archive_is_archive (file)) background_import = true;
      std::string extension = filter_url_get_extension (file);
      extension = filter::strings::unicode_string_casefold (extension);
      if (extension == "pdf") background_import = true;
      if (background_import) {
        tasks_logic_queue (IMPORTIMAGES, { name, file });
        success = translate("The file was uploaded and is being processed.");
        view.set_variable ("journal", journal_logic_see_journal_for_progress ());
      } else {
        // Store image.
        std::string image = database_imageresources.store (name, file);
        // Immediately open the uploaded image.
        std::string url = filter_url_build_http_query (resource_img_url (), "name", name);
        url = filter_url_build_http_query (url, "image", image);
        redirect_browser (webserver_request, url);
        return std::string();
      }
    } else {
      error = translate ("Nothing was uploaded");
    }
  }
  
  
  // Delete image.
  std::string remove = webserver_request.query ["delete"];
  if (remove != "") {
    std::string confirm = webserver_request.query ["confirm"];
    if (confirm == "") {
      Dialog_Yes dialog_yes = Dialog_Yes ("image", translate("Would you like to delete this image?"));
      dialog_yes.add_query ("name", name);
      dialog_yes.add_query ("delete", remove);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      database_imageresources.erase (name, remove);
      success = translate("The image was deleted.");
    }
  }
  
  
  std::vector <std::string> images = database_imageresources.get (name);
  std::string imageblock;
  for (auto & image : images) {
    imageblock.append ("<tr>");
    
    // Image.
    imageblock.append ("<td>");
    imageblock.append ("<a href=\"img?name=" + name + "&image=" + image + "\" title=\"" + translate("Edit") + "\">");
    imageblock.append (image);
    imageblock.append ("</a>");
    imageblock.append ("</td>");
    
    // Retrieve passage range for this image.
    int book1, chapter1, verse1, book2, chapter2, verse2;
    database_imageresources.get (name, image, book1, chapter1, verse1, book2, chapter2, verse2);

    imageblock.append ("<td>:</td>");

    // From passage ...
    imageblock.append ("<td>");
    if (book1) {
      imageblock.append (filter_passage_display (book1, chapter1, filter::strings::convert_to_string (verse1)));
    }
    imageblock.append ("</td>");

    imageblock.append ("<td>-</td>");

    // ... to passage.
    imageblock.append ("<td>");
    if (book2) {
      imageblock.append (filter_passage_display (book2, chapter2, filter::strings::convert_to_string (verse2)));
    }
    imageblock.append ("</td>");
    
    imageblock.append ("</tr>");
  }
  view.set_variable ("imageblock", imageblock);
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "image");
  page += assets_page::footer ();
  return page;
}

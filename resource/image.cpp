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


string resource_image_url ()
{
  return "resource/image";
}


bool resource_image_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string resource_image (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  Database_ImageResources database_imageresources;

  
  string page;
  Assets_Header header = Assets_Header (translate("Image resources"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  header.addBreadCrumb (resource_images_url (), menu_logic_resource_images_text ());
  page = header.run ();
  Assets_View view;
  string error, success;
  
  
  string name = request->query ["name"];
  view.set_variable ("name", name);
  

  // File upload.
  if (request->post.count ("upload")) {
    string folder = filter_url_tempfile ();
    filter_url_mkdir (folder);
    string file =  filter_url_create_path (folder, request->post ["filename"]);
    string data = request->post ["data"];
    if (!data.empty ()) {
      filter_url_file_put_contents (file, data);
      bool background_import = false;
      if (filter_archive_is_archive (file)) background_import = true;
      string extension = filter_url_get_extension (file);
      extension = unicode_string_casefold (extension);
      if (extension == "pdf") background_import = true;
      if (background_import) {
        tasks_logic_queue (IMPORTIMAGES, { name, file });
        success = translate("The file was uploaded and is being processed.") + " " + translate ("View the Journal for progress");
      } else {
        // Store image.
        string image = database_imageresources.store (name, file);
        // Immediately open the uploaded image.
        string url = filter_url_build_http_query (resource_img_url (), "name", name);
        url = filter_url_build_http_query (url, "image", image);
        redirect_browser (request, url);
        return "";
      }
    } else {
      error = translate ("Nothing was uploaded");
    }
  }
  
  
  // Delete image.
  string remove = request->query ["delete"];
  if (remove != "") {
    string confirm = request->query ["confirm"];
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
  
  
  vector <string> images = database_imageresources.get (name);
  string imageblock;
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
      imageblock.append (filter_passage_display (book1, chapter1, convert_to_string (verse1)));
    }
    imageblock.append ("</td>");

    imageblock.append ("<td>-</td>");

    // ... to passage.
    imageblock.append ("<td>");
    if (book2) {
      imageblock.append (filter_passage_display (book2, chapter2, convert_to_string (verse2)));
    }
    imageblock.append ("</td>");
    
    imageblock.append ("</tr>");
  }
  view.set_variable ("imageblock", imageblock);
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "image");
  page += Assets_Page::footer ();
  return page;
}

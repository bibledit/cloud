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


#include <resource/images.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <database/imageresources.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <menu/logic.h>


string resource_images_url ()
{
  return "resource/images";
}


bool resource_images_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string resource_images (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  
  string page;
  Assets_Header header = Assets_Header (translate("Image resources"), request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  string error, success;
  

  Database_ImageResources database_imageresources;
  
  
  // New image resource handler.
  if (request->query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("images", translate("Please enter a name for the new empty image resource"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("new")) {
    string resource = request->post ["entry"];
    vector <string> resources = database_imageresources.names ();
    if (in_array (resource, resources)) {
      error = translate("This image resource already exists");
    } else if (resource.empty ()) {
      error = translate("Please give a name for the image resource");
    } else {
      database_imageresources.create (resource);
      success = translate("The image resource was created");
    }
  }

  
  // Delete resource.
  string remove = request->query ["delete"];
  if (remove != "") {
    string confirm = request->query ["confirm"];
    if (confirm == "") {
      Dialog_Yes dialog_yes = Dialog_Yes ("images", translate("Would you like to delete this resource?"));
      dialog_yes.add_query ("delete", remove);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      database_imageresources.erase (remove);
      success = translate ("The resource was deleted");
    }
  }


  vector <string> resources = database_imageresources.names ();
  vector <string> resourceblock;
  for (auto & resource : resources) {
    resourceblock.push_back ("<p>");
    resourceblock.push_back ("<a href=\"image?name=" + resource + "\">");
    resourceblock.push_back (resource);
    resourceblock.push_back ("</a>");
    resourceblock.push_back ("</p>");
  }
  view.set_variable ("resourceblock", filter_string_implode (resourceblock, "\n"));

   
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "images");
  page += Assets_Page::footer ();
  return page;
}

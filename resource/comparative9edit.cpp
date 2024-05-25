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


#include <resource/comparative9edit.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <menu/logic.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <database/config/general.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <resource/comparative1edit.h>
#include <client/logic.h>


std::string resource_comparative9edit_url ()
{
  return "resource/comparative9edit";
}


bool resource_comparative9edit_acl (Webserver_Request& webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


std::string resource_comparative9edit (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Comparative resources"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  std::string error, success;
  

  // New comparative resource handler.
  if (webserver_request.query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("comparative9edit", translate("Please enter a name for the new comparative resource"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post.count ("new")) {
    // The title for the new resource as entered by the user.
    // Clean the title up and ensure it always starts with "Comparative ".
    // This word flags the comparative resource as being one of that category.
    std::string new_resource = webserver_request.post ["entry"];
    size_t pos = new_resource.find (resource_logic_comparative_resource ());
    if (pos != std::string::npos) {
      new_resource.erase (pos, resource_logic_comparative_resource ().length());
    }
    new_resource.insert (0, resource_logic_comparative_resource ());
    std::vector <std::string> titles;
    std::vector <std::string> resources = database::config::general::get_comparative_resources ();
    for (auto resource : resources) {
      std::string title;
      if (resource_logic_parse_comparative_resource (resource, &title)) {
        titles.push_back (title);
      }
    }
    if (in_array (new_resource, titles)) {
      error = translate("This comparative resource already exists");
    } else if (new_resource.empty ()) {
      error = translate("Please give a name for the comparative resource");
    } else {
      // Store the new resource in the list.
      std::string resource = resource_logic_assemble_comparative_resource (new_resource);
      resources.push_back (resource);
      database::config::general::set_comparative_resources (resources);
      success = translate("The comparative resource was created");
      // Since the default for a new resource is not to cache it,
      // add the resource to the ones not to be cached by the client.
      client_logic_no_cache_resource_add (new_resource);
      // Redirect the user to the place where to edit that new resource.
      std::string url = resource_comparative1edit_url () + "?name=" + new_resource;
      redirect_browser (webserver_request, url);
      return std::string();
    }
  }

  
  // Delete resource.
  std::string title2remove = webserver_request.query ["delete"];
  if (!title2remove.empty()) {
    std::string confirm = webserver_request.query ["confirm"];
    if (confirm == "") {
      Dialog_Yes dialog_yes = Dialog_Yes ("comparative9edit", translate("Would you like to delete this resource?"));
      dialog_yes.add_query ("delete", title2remove);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      std::vector <std::string> updated_resources;
      std::vector <std::string> existing_resources = database::config::general::get_comparative_resources ();
      for (auto resource : existing_resources) {
        std::string title;
        resource_logic_parse_comparative_resource (resource, &title);
        if (title != title2remove) updated_resources.push_back (resource);
      }
      database::config::general::set_comparative_resources (updated_resources);
      client_logic_no_cache_resource_remove (title2remove);
      success = translate ("The resource was deleted");
    }
  }


  std::vector <std::string> resources = database::config::general::get_comparative_resources ();
  std::string resourceblock;
  {
    pugi::xml_document document;
    for (auto & resource : resources) {
      std::string title;
      if (!resource_logic_parse_comparative_resource (resource, &title)) continue;
      pugi::xml_node p_node = document.append_child ("p");
      pugi::xml_node a_node = p_node.append_child("a");
      std::string href = "comparative1edit?name=" + title;
      a_node.append_attribute ("href") = href.c_str();
      title.append (" [" + translate("edit") + "]");
      a_node.text().set (title.c_str());
    }
    std::stringstream output;
    document.print (output, "", pugi::format_raw);
    resourceblock = output.str ();
  }
  view.set_variable ("resourceblock", resourceblock);

   
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "comparative9edit");
  page += assets_page::footer ();
  return page;
}

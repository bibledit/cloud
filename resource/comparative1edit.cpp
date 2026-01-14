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


#include <resource/comparative1edit.h>
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
#include <database/config/general.h>
#include <database/books.h>
#include <journal/index.h>
#include <dialog/yes.h>
#include <dialog/entry.h>
#include <dialog/select.h>
#include <tasks/logic.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <client/logic.h>


std::string resource_comparative1edit_url ()
{
  return "resource/comparative1edit";
}


bool resource_comparative1edit_acl (Webserver_Request& webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


std::string resource_comparative1edit (Webserver_Request& webserver_request)
{
  std::string page;
  Assets_Header header = Assets_Header (translate("Comparative resource"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  std::string error, success;
  
  
  std::string name = webserver_request.query ["name"];
  if (name.empty()) name = webserver_request.post_get("val1");
  view.set_variable ("name", name);

  
  std::string checkbox = webserver_request.post_get("checkbox");
  bool checked = filter::string::convert_to_bool (webserver_request.post_get("checked"));

  
  bool resource_edited = false;


  std::string title, base, update, remove, replace;
  bool diacritics = false, casefold = false, cache = false;
  {
    std::vector <std::string> resources = database::config::general::get_comparative_resources ();
    for (auto resource : resources) {
      resource_logic_parse_comparative_resource (resource, &title, &base, &update, &remove, &replace, &diacritics, &casefold, &cache);
      if (title == name) break;
    }
  }

  
  // The comparative resource's base resource.
  {
    constexpr const char* identification {"base"};
    if (webserver_request.post_count(identification)) {
      base = webserver_request.post_get(identification);
      resource_edited = true;
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = resource_logic_get_names (webserver_request, true),
      .selected = base,
      .parameters = { {"name", name} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }

  // The comparative resource's updated resource.
  {
    constexpr const char* identification {"update"};
    if (webserver_request.post_count(identification)) {
      update = webserver_request.post_get(identification);
      resource_edited = true;
    }
    dialog::select::Settings settings {
      .identification = identification,
      .values = resource_logic_get_names (webserver_request, true),
      .selected = update,
      .parameters = { {"name", name} },
    };
    dialog::select::Form form { .auto_submit = true };
    view.set_variable(identification, dialog::select::form(settings, form));
  }
  
  
  // The characters to remove from both resources before doing a comparison.
  if (webserver_request.query.count ("remove")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("comparative1edit", translate("Enter or edit the characters to remove from the resources"), remove, "remove", "");
    dialog_entry.add_query ("name", name);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post_count("remove")) {
    remove = webserver_request.post_get("entry");
    resource_edited = true;
  }

  
  // The characters to search for and replace in both resources before doing a comparison.
  if (webserver_request.query.count ("replace")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("comparative1edit", translate("Enter or edit the search and replace sets"), replace, "replace", "");
    dialog_entry.add_query ("name", name);
    page += dialog_entry.run ();
    return page;
  }
  if (webserver_request.post_count("replace")) {
    replace = webserver_request.post_get("entry");
    resource_edited = true;
  }

  
  // Whether to remove diacritics before doing the comparison.
  if (checkbox == "diacritics") {
    diacritics = checked;
    resource_edited = true;
  }

  
  // Whether to do case folding of the text before doing the comparison.
  if (checkbox == "casefold") {
    casefold = checked;
    resource_edited = true;
  }

  
  // Whether to cache the resource on client devices.
  if (checkbox == "cache") {
    cache = checked;
    resource_edited = true;
  }

  
  // If the resource was edited, then take a number of steps.
  if (resource_edited) {
    // Save the comparative resource if it was edited.
    std::vector <std::string> resources = database::config::general::get_comparative_resources ();
    error = translate ("Could not save");
    for (size_t i = 0; i < resources.size(); i++) {
      std::string title2;
      resource_logic_parse_comparative_resource (resources[i], &title2);
      if (title2 == title) {
        std::string resource = resource_logic_assemble_comparative_resource (title, base, update, remove, replace, diacritics, casefold, cache);
        resources[i] = resource;
        success = translate ("Saved");
        error.clear();
      }
    }
    database::config::general::set_comparative_resources (resources);
    // Possibly update the list of resources not to be cached on the client devices.
    if (cache) client_logic_no_cache_resource_remove(title);
    else client_logic_no_cache_resource_add(title);
    // Store the list of comparative resources for download by the client devices.
    {
      std::string path = resource_logic_comparative_resources_list_path ();
      filter_url_file_put_contents (path, filter::string::implode (resources, "\n"));
    }
  }
  

  view.set_variable ("success", success);
  view.set_variable ("error", error);
  view.set_variable ("title", title);
  view.set_variable ("remove", remove);
  view.set_variable ("replace", replace);
  view.set_variable ("diacritics", filter::string::get_checkbox_status (diacritics));
  view.set_variable ("casefold", filter::string::get_checkbox_status (casefold));
  view.set_variable ("cache", filter::string::get_checkbox_status (cache));
  page += view.render ("resource", "comparative1edit");
  page += assets_page::footer ();
  return page;
}

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


#include <xrefs/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <access/bible.h>
#include <locale/translate.h>
#include <dialog/list.h>
#include <menu/logic.h>


string xrefs_index_url ()
{
  return "xrefs/index";
}


bool xrefs_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  string page;
  Assets_Header header = Assets_Header (translate("Cross references"), webserver_request);
  header.setNavigator ();
  header.setEditorStylesheet ();
  header.addBreadCrumb (menu_logic_tools_menu (), menu_logic_tools_text ());
  page = header.run ();
  Assets_View view;
  
  
  string success;
  string error;
  
  
  if (request->query.count ("source")) {
    string source = request->query["source"];
    if (source == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to use as the source where to read the cross references from"), "", "");
      vector <string> bibles = access_bible_bibles (webserver_request);
      for (auto bible : bibles) {
        dialog_list.add_row (bible, "source", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setSourceXrefBible (source);
    }
  }
  
  
  if (request->query.count ("target")) {
    string target = request->query["target"];
    if (target == "") {
      Dialog_List dialog_list = Dialog_List ("index", translate("Select which Bible to insert the cross references into"), "", "");
      vector <string> bibles = access_bible_bibles (webserver_request);
      for (auto bible : bibles) {
        if (access_bible_write (webserver_request, bible)) {
          dialog_list.add_row (bible, "target", bible);
        }
      }
      page += dialog_list.run ();
      return page;
    } else {
      request->database_config_user()->setTargetXrefBible (target);
    }
  }
  
  
  string source = request->database_config_user()->getSourceXrefBible ();
  if (!access_bible_read (webserver_request, source)) {
    source = "";
    request->database_config_user()->setSourceXrefBible (source);
  }
  string target = request->database_config_user()->getTargetXrefBible ();
  if (!access_bible_write (webserver_request, target)) {
    target = "";
    request->database_config_user()->setTargetXrefBible (target);
  }
  if (source == "") source = "[" + translate ("select") + "]";
  if (target == "") target = "[" + translate ("select") + "]";
  view.set_variable ("source", source);
  view.set_variable ("target", target);
                                          
                                          
  view.set_variable ("success", success);
  view.set_variable ("error", error);
                                        
  
  page += view.render ("xrefs", "index");
  page += Assets_Page::footer ();
  return page;
}

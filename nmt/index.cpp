/*
Copyright (©) 2003-2021 Teus Benschop.

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


#include <nmt/index.h>
#include <assets/view.h>
#include <assets/header.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <locale/translate.h>
#include <database/volatile.h>
#include <dialog/list.h>
#include <journal/index.h>
#include <tasks/logic.h>


const char * nmt_index_url ()
{
  return "nmt/index";
}


bool nmt_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string nmt_index (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  Assets_Header header = Assets_Header (translate ("Bibledit"), webserver_request);
  
  string page = header.run ();

  Assets_View view;

  int userid = filter_string_user_identifier (webserver_request);
  
  string referencebible = Database_Volatile::getValue (userid, "nmt-ref-bible");
  string translatingbible = Database_Volatile::getValue (userid, "nmt-trans-bible");


  if (request->query.count ("reference")) {
    referencebible = request->query["reference"];
    if (referencebible.empty()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which Bible would you like to use as a reference for producing the neural machine translation suggestions?"), "", "");
      vector <string> bibles = request->database_bibles ()->getBibles ();
      bibles = filter_string_array_diff (bibles, {translatingbible});
      for (auto bible : bibles) {
        dialog_list.add_row (bible, "reference", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      Database_Volatile::setValue (userid, "nmt-ref-bible", referencebible);
    }
  }
  if (referencebible.empty()) referencebible = "[" + translate ("select") + "]";
  view.set_variable ("reference", referencebible);

  
  if (request->query.count ("translating")) {
    translatingbible = request->query["translating"];
    if (translatingbible.empty()) {
      Dialog_List dialog_list = Dialog_List ("index", translate("Which Bible would you like to use as the one now being translated for getting the neural machine translation suggestions?"), "", "");
      vector <string> bibles = request->database_bibles ()->getBibles ();
      bibles = filter_string_array_diff (bibles, {referencebible});
      for (auto bible : bibles) {
        dialog_list.add_row (bible, "translating", bible);
      }
      page += dialog_list.run ();
      return page;
    } else {
      Database_Volatile::setValue (userid, "nmt-trans-bible", translatingbible);
    }
  }
  if (translatingbible.empty()) translatingbible = "[" + translate ("select") + "]";
  view.set_variable ("translating", translatingbible);

  
  if (request->query.count ("export")) {
    tasks_logic_queue (EXPORT2NMT, {referencebible, translatingbible});
    redirect_browser (request, journal_index_url ());
    return "";
  }

  
  page += view.render ("nmt", "index");
  page += Assets_Page::footer ();
  return page;
}

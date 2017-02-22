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


#include <xrefs/help.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <webserver/request.h>
#include <locale/translate.h>


string xrefs_help_url ()
{
  return "xrefs/help";
}


bool xrefs_help_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string xrefs_help (void * webserver_request)
{
  string page;
  Assets_Header header = Assets_Header (translate("Cross references"), webserver_request);
  page = header.run ();
  Assets_View view;
  page += view.render ("xrefs", "help");
  page += Assets_Page::footer ();
  return page;
}

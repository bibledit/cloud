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


#include <public/logic.h>
#include <webserver/request.h>
#include <database/config/bible.h>


vector <string> public_logic_bibles (void * webserver_request)
{
  vector <string> public_bibles;
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  vector <string> bibles = request->database_bibles ()->getBibles ();
  for (auto & bible : bibles) {
    if (Database_Config_Bible::getPublicFeedbackEnabled (bible)) {
      public_bibles.push_back (bible);
    }
  }
  return public_bibles;
}

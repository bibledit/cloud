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


#include <codecvt>
#include <unittests/utilities.h>
#include <filter/string.h>
#include <filter/text.h>
#include <editor/html2usfm.h>
#include <editor/html2format.h>
#include <styles/logic.h>
#include <database/state.h>
#include <database/login.h>
#include <database/users.h>
#include <webserver/request.h>
#include <user/logic.h>
#include <pugixml/pugixml.hpp>
#include <html/text.h>


using namespace pugi;


void test_dev () // Todo move into place.
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  // Test extraction of all sorts of information from USFM code.
  // Test basic formatting into OpenDocument.
  {
    string bible = "bible";
    string usfm = R"(
\c 1
\p
\v 1 Verse one. \fig caption|src="filename" size="size" ref="reference"\fig*
    )";
    Filter_Text filter_text = Filter_Text (bible);
    filter_text.esword_text = new Esword_Text (bible);
    filter_text.addUsfmCode (usfm);
    filter_text.run (styles_logic_standard_sheet());
    filter_text.esword_text->finalize ();
    //filter_text.esword_text->createModule ("filename");


//      evaluate (__LINE__, __func__, 1, filter_text.runningHeaders[0].book);

  }
  exit (0); // Todo
  refresh_sandbox (true);
}

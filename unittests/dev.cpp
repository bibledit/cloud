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


void test_dev () // Todo
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);

  Html_Text html_text ("");
  html_text.have_popup_notes(); // Todo
  html_text.new_paragraph ();
  html_text.add_text ("Text1");
  html_text.add_note ("†", "");
  html_text.add_note_text ("Note1.");
  html_text.add_text (".");
  string html = html_text.get_inner_html ();
  string standard = R"(<p><span>Text1</span><a href="#note1" id="citation1" class="superscript">†<span class="popup"><span> </span><span>Note1.</span></span></a><span>.</span></p><div><p class=""><a href="#citation1" id="note1">†</a><span> </span><span>Note1.</span></p></div>)";
  evaluate (__LINE__, __func__, standard, html);
  
  refresh_sandbox (true);
  //exit(0); // Todo
}

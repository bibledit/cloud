/*
 Copyright (©) 2003-2020 Teus Benschop.
 
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
#include <styles/logic.h>


void test_dev ()
{
  trace_unit_tests (__func__);

  { // Todo move into place.
    string standard_html = R"(<p class="b-p"><span class="i-v">1</span> One<span class="i-notecall1">1</span> two.</p><p class="b-notes">&nbsp;</p><p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">117.3 </span><span class="i-fk">| key </span></p><p class="b-f"><span class="i-fk">word</span></p>)";
    Editor_Html2Usfm editor_html2usfm;
    editor_html2usfm.load (standard_html);
    editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
    editor_html2usfm.run ();
    string output_usfm = editor_html2usfm.get ();
    cout << output_usfm << endl; // Todo

  }

  refresh_sandbox (true);
}

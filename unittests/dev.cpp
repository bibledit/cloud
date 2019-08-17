/*
 Copyright (©) 2003-2019 Teus Benschop.
 
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


#include <unittests/french.h>
#include <unittests/utilities.h>
#include <checks/french.h>
#include <database/check.h>
#include <filter/string.h>
#include <filter/text.h>
#include <checks/usfm.h>
#include <editor/html2usfm.h>
#include <editor/usfm2html.h>


void test_dev ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  {
    string standard_usfm = R"(
\p
\v 1 \vp A\vp* One.
\v 2 \vp B\vp* Two.
)";
    standard_usfm = filter_string_trim (standard_usfm);
    {
      // DOM-based editor.
      string standard_html = R"(
<p class="p"><span class="v">1</span><span> </span><span class="vp">A</span><span> One.</span><span> </span><span class="v">2</span><span> </span><span class="vp">B</span><span> Two.</span></p>
)";
      Editor_Usfm2Html editor_usfm2html;
      editor_usfm2html.load (standard_usfm);
      editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
      editor_usfm2html.run ();
      string html = editor_usfm2html.get ();
      evaluate (__LINE__, __func__, filter_string_trim (standard_html), html);
      
      return; // Todo

      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      evaluate (__LINE__, __func__, standard_usfm, usfm);
    }
    {
      // Quill-based editor.
      string standard_html = R"(
<p class="b-p"><span class="i-v">1</span><span> </span><span>One.</span><span> </span><span class="i-v">2</span><span> </span><span>Two.</span></p>
)";
      Editor_Usfm2Html editor_usfm2html;
      editor_usfm2html.load (standard_usfm);
      editor_usfm2html.stylesheet (styles_logic_standard_sheet ());
      editor_usfm2html.quill ();
      editor_usfm2html.run ();
      string html = editor_usfm2html.get ();
      evaluate (__LINE__, __func__, filter_string_trim (standard_html), html);
      
      Editor_Html2Usfm editor_html2usfm;
      editor_html2usfm.load (html);
      editor_html2usfm.stylesheet (styles_logic_standard_sheet ());
      editor_html2usfm.quill ();
      editor_html2usfm.run ();
      string usfm = editor_html2usfm.get ();
      // Todo evaluate (__LINE__, __func__, standard_usfm, usfm);
    }
  }
  
}


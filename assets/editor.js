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


function assetsEditorAddNote (quill, style, caller, noteId, chapter, separator, verse)
{
  // <p class="b-f"><span class="i-notebody1">1</span> + <span class="i-fr">1.1 </span><span class="i-fk"> keyword </span><span class="i-ft"> Footnote text.</span></p>
  // Footnote uses \fr \fk \ft.
  // Cross reference uses \xo \xt.
  // As for spacing, see issue https://github.com/bibledit/cloud/issues/672
  // A vertical bar | now gets inserted before the keyword and the text for easy of editing.
  // It is about which style gets applied when typing.
  // The vertical bar makes it easier to see which style will get applied.
  var length = quill.getLength ();
  quill.insertText (length, "\n", "paragraph", style, "user");
  quill.insertText (length, caller, "character", "notebody" + noteId, "user");
  length++;
  quill.insertText (length, " + ", "character", "", "user");
  length += 3;
  var referenceText = chapter + separator + verse + " ";
  var referenceStyle = "fr";
  if (style == "x") referenceStyle = "xo";
  quill.insertText (length, referenceText, "character", referenceStyle, "user");
  length += referenceText.length;
  if (style != "x") {
    var keywordStyle = "fk";
    quill.insertText (length, "|keyword ", "character", keywordStyle, "user");
    length += 9;
  }
  var textStyle = "ft";
  if (style == "x") textStyle = "xt";
  quill.insertText (length, "|text.", "character", textStyle, "user");
}


/*
Copyright (©) 2003-2017 Teus Benschop.

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


#include <editone/logic.h>
#include <editor/usfm2html.h>
#include <editor/html2usfm.h>
#include <filter/string.h>


void editone_logic_prefix_html_stage_one (string usfm, string stylesheet, string & html, string & last_p_style)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
    // No identical id's in the same DOM.
    html = filter_string_str_replace (" id=\"notes\"", " id=\"prefixnotes\"", html);
    // The last paragraph style in this USFM fragment, the prefix to the editable fragment.
    // If the last paragraph has any content in it,
    // for correct visual representation of the editable fragment, that follows this,
    // clear that style.
    last_p_style = editor_usfm2html.currentParagraphStyle;
    if (!editor_usfm2html.currentParagraphContent.empty ()) last_p_style.clear ();
  }
}


void editone_old_logic_editable_html (string prefix_last_p_style, string usfm, string stylesheet, string & html, string & editable_last_p_style, string & editable_first_applied_p_style)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
    editable_last_p_style = editor_usfm2html.currentParagraphStyle;
  }
  
  // If the first paragraph of the editable verse does not have a paragraph style applied,
  // apply the last paragraph style of the prefix to the first paragraph of the focused verse.
  // For example, html like this:
  // <p><span class="v">7</span><span> </span><span>For Yahweh knows the way of the righteous,</span></p><p class="q2"><span>but the way of the wicked shall perish.</span></p>
  // ... becomes like this:
  // <p class="q1"><span class="v">7</span><span /><span>For Yahweh knows the way of the righteous,</span></p><p class="q2"><span>but the way of the wicked shall perish.</span></p>
  if (!html.empty ()) {
    if (!prefix_last_p_style.empty ()) {
      xml_document document;
      html = html2xml (html);
      document.load_string (html.c_str(), parse_ws_pcdata_single);
      xml_node p_node = document.first_child ();
      string p_style = p_node.attribute ("class").value ();
      if (p_style.empty ()) {
        p_node.append_attribute ("class") = prefix_last_p_style.c_str ();
        // Send the applied paragraph style to the browser,
        // for later use when it saves the modified verse text.
        editable_first_applied_p_style = prefix_last_p_style;
      }
      stringstream output;
      document.print (output, "", format_raw);
      html = output.str ();
    }
  }
}


void editone_logic_editable_html (string usfm, string stylesheet, string & html)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.quill ();
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
  }
}


void editone_logic_suffix_html (string editable_last_p_style, string usfm, string stylesheet, string & html)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
    // No identical id in the same DOM.
    html = filter_string_str_replace (" id=\"notes\"", " id=\"suffixnotes\"", html);
  }
  
  // If the first paragraph of the suffix does not have a paragraph style applied,
  // apply the last paragraph style of the focused verse to the first paragraph of the suffix.
  // For example, html like this:
  // <p><span class="v">7</span><span> </span><span>For Yahweh knows the way of the righteous,</span></p><p class="q2"><span>but the way of the wicked shall perish.</span></p>
  // ... will become like this:
  // <p class="q1"><span class="v">7</span><span /><span>For Yahweh knows the way of the righteous,</span></p><p class="q2"><span>but the way of the wicked shall perish.</span></p>
  if (!html.empty ()) {
    if (!editable_last_p_style.empty ()) {
      xml_document document;
      html = html2xml (html);
      document.load_string (html.c_str(), parse_ws_pcdata_single);
      xml_node p_node = document.first_child ();
      string p_style = p_node.attribute ("class").value ();
      if (p_style.empty ()) {
        p_node.append_attribute ("class") = editable_last_p_style.c_str ();
      }
      stringstream output;
      document.print (output, "", format_raw);
      html = output.str ();
    }
  }
}


string editone_old_logic_html_to_usfm (string stylesheet, string html, string applied_p_style)
{
  // If an initial style was added to the first paragraph, remove it again.
  if (!applied_p_style.empty ()) {
    // First do a html to xml conversion to avoid a mismatched tag error later in the save chain.
    html = html2xml (html);
    xml_document document;
    document.load_string (html.c_str(), parse_ws_pcdata_single);
    xml_node p_node = document.first_child ();
    string p_style = p_node.attribute ("class").value ();
    if (applied_p_style == p_style) {
      p_node.remove_attribute ("class");
    }
    stringstream output;
    document.print (output, "", format_raw);
    html = output.str ();
  }
  
  // Convert xml entities to normal characters.
  html = filter_string_desanitize_html (html);
  
  // Convert the html back to USFM in the special way for editing one verse.
  string usfm = editor_export_verse (stylesheet, html);
  
  return usfm;
}


string editone_logic_html_to_usfm (string stylesheet, string html)
{
  // Convert xml entities to normal characters.
  html = filter_string_desanitize_html (html);
  
  // Convert the html back to USFM in the special way for editing one verse.
  string usfm = editor_export_verse_quill (stylesheet, html);
  
  // Done.
  return usfm;
}


// Move the notes from the $prefix to the $suffix.
void editone_logic_move_notes (string & prefix, string & suffix)
{
  // No input: Ready.
  if (prefix.empty ()) return;
  
  // Do a html to xml conversion to avoid a mismatched tag error.
  prefix = html2xml (prefix);

  // Load the prefix.
  xml_document document;
  document.load_string (prefix.c_str(), parse_ws_pcdata_single);
  
  // If there's any notes, it should be at the end.
  xml_node div_node = document.last_child ();
  string id = div_node.attribute ("id").value ();
  
  // No note(s): Ready.
  if (id != "prefixnotes") return;

  // Get the note(s) leaving out the surrounding data.
  string notes_text;
  for (xml_node child : div_node.children ()) {
    if (strcmp (child.name (), "p") != 0) continue;
    stringstream ss;
    child.print (ss, "", format_raw);
    string note = ss.str ();
    notes_text.append (note);
  }
  
  // Remove the note(s) from the prefix.
  document.remove_child (div_node);
  {
    stringstream ss;
    document.print (ss, "", format_raw);
    prefix = ss.str ();
  }

  // Do a html to xml conversion to avoid a mismatched tag error.
  suffix = html2xml (suffix);
  
  // Load the suffix.
  document.load_string (suffix.c_str(), parse_ws_pcdata_single);

  // If there's any notes, it should be at the end.
  div_node = document.last_child ();
  id = div_node.attribute ("id").value ();
  
  // If there's no notes container, add it.
  if (id.empty ()) {
    div_node = document.append_child ("div");
    div_node.append_attribute ("id") = "suffixnotes";
    div_node.append_child ("hr");
  }
  
  // Contain the prefix's notes and add them to the suffix's notes container.
  notes_text.insert (0, "<div>");
  notes_text.append ("</div>");
  div_node.append_buffer (notes_text.c_str (), notes_text.size ());

  // Put the notes in the correct order, if needed.
  if (!id.empty ()) {
    xml_node hr_node = div_node.first_child ();
    xml_node new_node = div_node.last_child ();
    div_node.insert_move_after (new_node, hr_node);
  }
  
  // Convert the DOM to suffix text.
  {
    stringstream ss;
    document.print (ss, "", format_raw);
    suffix = ss.str ();
  }
}



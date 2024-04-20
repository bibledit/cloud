/*
Copyright (©) 2003-2024 Teus Benschop.

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


#include <editone2/logic.h>
#include <editor/usfm2html.h>
#include <editor/html2usfm.h>
#include <filter/string.h>
#include <filter/url.h>


void editone_logic_prefix_html (std::string usfm, std::string stylesheet, std::string& html, std::string& last_p_style)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
    // No identical id's in the same DOM.
    html = filter::strings::replace (R"( id="notes")", R"( id="prefixnotes")", html);
    // The last paragraph style in this USFM fragment, the prefix to the editable fragment.
    // If the last paragraph has any content in it,
    // for correct visual representation of the editable fragment, that follows this,
    // clear that style.
    last_p_style = editor_usfm2html.m_current_paragraph_style;
    if (!editor_usfm2html.m_current_paragraph_content.empty ()) last_p_style.clear ();
  }
}


void editone_logic_editable_html (std::string usfm, std::string stylesheet, std::string& html)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
  }
}


void editone_logic_suffix_html (std::string editable_last_p_style, std::string usfm, std::string stylesheet, std::string& html)
{
  if (!usfm.empty ()) {
    Editor_Usfm2Html editor_usfm2html;
    editor_usfm2html.load (usfm);
    editor_usfm2html.stylesheet (stylesheet);
    editor_usfm2html.run ();
    html = editor_usfm2html.get ();
    // No identical id in the same DOM.
    html = filter::strings::replace (R"( id="notes")", R"( id="suffixnotes")", html);
  }
  
  // If the first paragraph of the suffix does not have a paragraph style applied,
  // apply the last paragraph style of the focused verse to the first paragraph of the suffix.
  // For example, html like this:
  // <p><span class="v">7</span><span> </span><span>For Yahweh knows the way of the righteous,</span></p><p class="q2"><span>but the way of the wicked shall perish.</span></p>
  // ... will become like this:
  // <p class="q1"><span class="v">7</span><span /><span>For Yahweh knows the way of the righteous,</span></p><p class="q2"><span>but the way of the wicked shall perish.</span></p>
  if (!html.empty ()) {
    if (!editable_last_p_style.empty ()) {
      pugi::xml_document document;
      html = filter::strings::html2xml (html);
      document.load_string (html.c_str(), pugi::parse_ws_pcdata_single);
      pugi::xml_node p_node = document.first_child ();
      std::string p_style = p_node.attribute ("class").value ();
      if (p_style.empty ()) {
        p_node.append_attribute ("class") = editable_last_p_style.c_str ();
      }
      std::stringstream output;
      document.print (output, "", pugi::format_raw);
      html = output.str ();
    }
  }
}


std::string editone_logic_html_to_usfm (std::string stylesheet, std::string html)
{
  // It used to convert XML entities to normal characters.
  // For example, it used to convert "&lt;" to "<".
  // But doing this too early in the conversion chain led to the following problem:
  // The XML parser was taking the "<" character as part of an XML element.
  // It than didn't find the closing ">" marker, and then complained about parsing errors.
  // And it dropped whatever followed the "<" marker.
  // So it now no longer unescapes the XML special characters this early in the chain.
  // It does it much later now, before saving the USFM that the converter produces.
  
  // Convert special spaces to normal ones.
  html = filter::strings::any_space_to_standard_space (html);

  // Convert the html back to USFM in the special way for editing one verse.
  std::string usfm = editor_export_verse_quill (stylesheet, html);

  // Done.
  return usfm;
}


// Move the notes from the $prefix to the $suffix.
void editone_logic_move_notes_v2 (std::string & prefix, std::string & suffix)
{
  // No input: Ready.
  if (prefix.empty ()) return;
  
  // Do a html to xml conversion to avoid a mismatched tag error.
  prefix = filter::strings::html2xml (prefix);

  // Load the prefix.
  pugi::xml_document document;
  document.load_string (prefix.c_str(), pugi::parse_ws_pcdata_single);

  // The notes separator class.
  const char * b_notes_class = "b-notes";
  
  // Iterate over the document to find:
  // - the possible notes separator.
  // - any possible subsequent note nodes.
  bool within_notes = false;
  pugi::xml_node prefix_separator_node;
  std::vector <pugi::xml_node> prefix_note_nodes;
  for (pugi::xml_node p_node : document.children ()) {
    if (within_notes) {
      prefix_note_nodes.push_back (p_node);
    }
    std::string cls = p_node.attribute ("class").value ();
    if (cls == b_notes_class) {
      within_notes = true;
      prefix_separator_node = p_node;
    }
  }

  // No notes: Ready.
  if (prefix_note_nodes.empty()) return;

  // Get the note(s) text from the note node(s).
  // Remove the note node(s) from the prefix.
  // Remove the notes separator node from the prefix.
  std::string notes_text;
  for (pugi::xml_node p_node : prefix_note_nodes) {
    std::stringstream ss;
    p_node.print (ss, "", pugi::format_raw);
    std::string note = ss.str ();
    notes_text.append (note);
    document.remove_child (p_node);
  }
  document.remove_child (prefix_separator_node);

  // Convert the XML document back to a possibly cleaned prefix without notes.
  {
    std::stringstream ss;
    document.print (ss, "", pugi::format_raw);
    prefix = ss.str ();
  }

  // Do a html to xml conversion in the suffix to avoid a mismatched tag error.
  suffix = filter::strings::html2xml (suffix);

  // Load the suffix.
  document.load_string (suffix.c_str(), pugi::parse_ws_pcdata_single);

  // Iterate over the document to find the possible notes separator.
  pugi::xml_node suffix_separator_node;
  for (pugi::xml_node p_node : document.children ()) {
    std::string cls = p_node.attribute ("class").value ();
    if (cls == b_notes_class) {
      suffix_separator_node = p_node;
    }
  }

  // If there's no notes container, add it.
  if (!suffix_separator_node) {
    suffix_separator_node = document.append_child ("p");
    suffix_separator_node.append_attribute ("class") = b_notes_class;
    suffix_separator_node.append_child ("br");
  }

  // Contain the prefix's notes and add them to the suffix's notes container.
  suffix_separator_node.append_buffer (notes_text.c_str (), notes_text.size ());

  // Convert the DOM to suffix text.
  {
    std::stringstream ss;
    document.print (ss, "", pugi::format_raw);
    suffix = ss.str ();
  }
}



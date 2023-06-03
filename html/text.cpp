/*
Copyright (©) 2003-2023 Teus Benschop.

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


#include <html/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/url.h>
#include <database/books.h>


// Class for creating Html text documents.


HtmlText::HtmlText (const std::string& title)
{
  current_paragraph_style.clear();
  current_paragraph_content.clear();
  note_count = 0;
  
  // <html>
  xml_node root_node = document.append_child ("html");
  
  // <head>
  head_node = root_node.append_child ("head");
  
  xml_node title_node = head_node.append_child ("title");
  title_node.text ().set (title.c_str());
  
  // <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
  xml_node meta_node = head_node.append_child ("meta");
  meta_node.append_attribute ("http-equiv") = "content-type";
  meta_node.append_attribute ("content") = "text/html; charset=UTF-8";

  // <meta name="viewport" content="width=device-width, initial-scale=1.0">
  // This tag helps to make the page mobile-friendly.
  // See https://www.google.com/webmasters/tools/mobile-friendly/
  meta_node = head_node.append_child ("meta");
  meta_node.append_attribute ("name") = "viewport";
  meta_node.append_attribute ("content") = "width=device-width, initial-scale=1.0";

  // <link rel="stylesheet" type="text/css" href="stylesheet.css">
  xml_node link_node = head_node.append_child ("link");
  link_node.append_attribute ("rel") = "stylesheet";
  link_node.append_attribute ("type") = "text/css";
  link_node.append_attribute ("href") = "stylesheet.css";
  
  // <body>
  body_node = root_node.append_child ("body");
  
  // Optional for notes: <div>
  notes_div_node = body_node.append_child ("div");
}


void HtmlText::new_paragraph (const std::string& style)
{
  current_p_node = body_node.append_child ("p");
  if (!style.empty()) {
    std::string clss = style;
    if (!custom_class.empty()) {
      clss += " " + custom_class;
    }
    current_p_node.append_attribute ("class") = clss.c_str();
  }
  current_p_node_open = true;
  current_paragraph_style = style;
  current_paragraph_content.clear();
}


// This function adds text to the current paragraph.
// $text: The text to add.
void HtmlText::add_text (const std::string& text)
{
  if (!text.empty()) {
    if (!current_p_node_open) new_paragraph ();
    xml_node span = current_p_node.append_child ("span");
    span.text().set (text.c_str());
    if (!current_text_style.empty ()) {
      // Take character style(s) as specified in the object.
      span.append_attribute ("class") = filter::strings::implode (current_text_style, " ").c_str();
    }
    current_paragraph_content += text;
  }
}


// This creates a <h1> heading with contents.
// $text: Contents.
void HtmlText::new_heading1 (const std::string& text, const bool hide)
{
  new_named_heading ("h1", text, hide);
}


// This applies a page break.
void HtmlText::new_page_break ()
{
  // The style is already in the css.
  new_paragraph ("break");
  // Always clear the flag for the open paragraph,
  // because we don't want subsequent text to be added to this page break,
  // since it would be nearly invisible, and thus text would seem to be lost.
  current_p_node_open = false;
  current_paragraph_style.clear();
  current_paragraph_content.clear();
}


// This opens a text style.
// $style: the array containing the style variables.
// $note: boolean: Whether this refers to notes.
// $embed: boolean: Whether to embed the new text style in an existing text style.
//                  true: add the new style to the existing style.
//                  false: close any existing text style, and open the new style.
void HtmlText::open_text_style (const Database_Styles_Item& style, const bool note, const bool embed)
{
  std::string marker = style.marker;
  if (note) {
    if (!embed) current_note_text_style.clear();
    current_note_text_style.push_back (marker);
  } else {
    if (!embed) current_text_style.clear();
    current_text_style.push_back (marker);
  }
}


// This closes any open text style.
// $note: boolean: Whether this refers to notes.
// $embed: boolean: Whether to embed the new text style in an existing text style.
//                  true: add the new style to the existing style.
//                  false: close any existing text style, and open the new style.
void HtmlText::close_text_style (const bool note, const bool embed)
{
  if (note) {
    if (!current_note_text_style.empty()) current_note_text_style.pop_back ();
    if (!embed) current_note_text_style.clear();
  } else {
    if (!current_text_style.empty()) current_text_style.pop_back ();
    if (!embed) current_text_style.clear ();
  }
}


// This function adds a note to the current paragraph.
// $citation: The text of the note citation.
// $style: Style name for the paragraph in the note body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void HtmlText::add_note (const std::string& citation, const std::string& style, [[maybe_unused]] const bool endnote)
{
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!current_p_node_open) new_paragraph ();
  
  note_count++;
  
  // Add the link with all relevant data for the note citation.
  std::string reference = "#note" + filter::strings::convert_to_string (note_count);
  std::string identifier = "citation" + filter::strings::convert_to_string (note_count);
  add_link (current_p_node, reference, identifier, "", "superscript", citation, add_popup_notes);
  
  // Open a paragraph element for the note body.
  note_p_node = notes_div_node.append_child ("p");
  note_p_node.append_attribute ("class") = style.c_str();
  note_p_node_open = true;
  
  close_text_style (true, false);
  
  // Add the link with all relevant data for the note body.
  reference = "#citation" + filter::strings::convert_to_string (note_count);
  identifier = "note" + filter::strings::convert_to_string (note_count);
  add_link (note_p_node, reference, identifier, "", "", citation);
  
  // Add a space.
  add_note_text (" ");
}


// This function adds text to the current footnote.
// $text: The text to add.
void HtmlText::add_note_text (const std::string& text)
{
  if (text.empty()) return;
  if (!note_p_node_open) add_note ("?", "");
  xml_node span_node = note_p_node.append_child ("span");
  span_node.text().set (text.c_str());
  if (!current_note_text_style.empty ()) {
    // Take character style as specified in this object.
    span_node.append_attribute ("class") = filter::strings::implode (current_note_text_style, " ").c_str();
  }
  if (popup_node) {
    xml_node span_node_2 = popup_node.append_child ("span");
    span_node_2.text().set (text.c_str());
  }
}


// This function closes the current footnote.
void HtmlText::close_current_note ()
{
  close_text_style (true, false);
  note_p_node_open = false;
}


// This adds a link.
// $domNode: The DOM node where to add the link to.
// $reference: The link's href, e.g. where it links to.
// $identifier: The link's identifier. Others can link to it.
// $title: The link's title, to make it accessible, e.g. for screenreaders.
// $style: The link text's style.
// $text: The link's text.
void HtmlText::add_link (xml_node node,
                         const std::string& reference, const std::string& identifier,
                         const std::string& title, const std::string& style, const std::string& text,
                         const bool add_popup)
{
  xml_node a_node = node.append_child ("a");
  a_node.append_attribute ("href") = reference.c_str();
  a_node.append_attribute ("id") = identifier.c_str();
  if (!title.empty ()) a_node.append_attribute ("title") = title.c_str();
  if (!style.empty()) a_node.append_attribute ("class") = style.c_str();
  xml_node pcdata = a_node.append_child (node_pcdata);
  pcdata.set_value(text.c_str());
  // Whether to add a popup span in a note.
  if (add_popup) {
    popup_node = a_node.append_child("span");
    popup_node.append_attribute("class") = "popup";
  }
}


// This gets and then returns the html text
std::string HtmlText::get_html ()
{
  // Move any notes into place: At the end of the body.
  // Or remove empty notes container.
  if (note_count > 0) {
    body_node.append_move (notes_div_node);
  } else {
    body_node.remove_child (notes_div_node);
  }
  note_count = 0;

  // Get the html.
  std::stringstream output;
  document.print (output, "", format_raw);
  std::string html = output.str ();
  
  // Add html5 doctype.
  html.insert (0, "<!DOCTYPE html>\n");
 
  return html;
}


// Returns the html text within the <body> tags, that is, without the <head> stuff.
std::string HtmlText::get_inner_html ()
{
  std::string page = get_html ();
  size_t pos = page.find ("<body>");
  if (pos != std::string::npos) {
    page = page.substr (pos + 6);
    pos = page.find ("</body>");
    if (pos != std::string::npos) {
      page = page.substr (0, pos);
    }
  }
  return page;
}


// This saves the web page to file
// $name: the name of the file to save to.
void HtmlText::save (std::string name)
{
  const std::string html = get_html ();
  filter_url_file_put_contents (name, html);
}


// This adds a new table to the html DOM.
// Returns: The new $tableElement
xml_node HtmlText::new_table ()
{
  // Adding subsequent text should create a new paragraph.
  current_p_node_open = false;
  current_paragraph_style.clear();
  current_paragraph_content.clear();
  // Append the table.
  xml_node tableElement = body_node.append_child ("table");
  tableElement.append_attribute ("width") = "100%";
  return tableElement;
}


// This adds a new row to an existing $tableElement.
// Returns: The new $tableRowElement.
xml_node HtmlText::new_table_row (xml_node tableElement)
{
  xml_node tableRowElement = tableElement.append_child ("tr");
  return tableRowElement;
}


// This adds a new data cell to an existing $tableRowElement.
// Returns: The new $tableDataElement.
xml_node HtmlText::new_table_data (xml_node tableRowElement, const bool alignRight)
{
  xml_node tableDataElement = tableRowElement.append_child ("td");
  if (alignRight) tableDataElement.append_attribute ("align") = "right";
  return tableDataElement;
}


// This creates a heading with styled content.
// $style: A style name.
// $text: Content.
void HtmlText::new_named_heading (const std::string& style, const std::string& text, [[maybe_unused]] const bool hide)
{
  xml_node textHDomElement = body_node.append_child (style.c_str());
  textHDomElement.text ().set (text.c_str());
  // Make paragraph null, so that adding subsequent text creates a new paragraph.
  current_p_node_open = false;
  current_paragraph_style.clear ();
  current_paragraph_content.clear ();
}


// Whether to add pop-up notes as well.
void HtmlText::have_popup_notes ()
{
  add_popup_notes = true;
}


// Add an image to the html.
void HtmlText::add_image (const std::string& style, const std::string& alt, const std::string& src, const std::string& caption)
{
  xml_node img_node = body_node.append_child ("img");
  img_node.append_attribute("alt") = alt.c_str();
  img_node.append_attribute("src") = src.c_str();
  img_node.append_attribute ("width") = "100%";
  // Add the caption if it is given.
  if (!caption.empty()) {
    new_paragraph(style);
    add_text(caption);
  }
  // Close the paragraph so that adding subsequent text creates a new paragraph.
  current_p_node_open = false;
  current_paragraph_style.clear ();
  current_paragraph_content.clear ();
}

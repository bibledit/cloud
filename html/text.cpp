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


#include <html/text.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/url.h>
#include <database/books.h>


// Class for creating Html text documents.


Html_Text::Html_Text (string title)
{
  currentParagraphStyle.clear();
  currentParagraphContent.clear();
  noteCount = 0;
  
  // <html>
  xml_node root_node = htmlDom.append_child ("html");
  
  // <head>
  headDomNode = root_node.append_child ("head");
  
  xml_node title_node = headDomNode.append_child ("title");
  title_node.text ().set (title.c_str());
  
  // <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
  xml_node meta_node = headDomNode.append_child ("meta");
  meta_node.append_attribute ("http-equiv") = "content-type";
  meta_node.append_attribute ("content") = "text/html; charset=UTF-8";

  // <meta name="viewport" content="width=device-width, initial-scale=1.0">
  // This tag helps to make the page mobile-friendly.
  // See https://www.google.com/webmasters/tools/mobile-friendly/
  meta_node = headDomNode.append_child ("meta");
  meta_node.append_attribute ("name") = "viewport";
  meta_node.append_attribute ("content") = "width=device-width, initial-scale=1.0";

  // <link rel="stylesheet" type="text/css" href="stylesheet.css">
  xml_node link_node = headDomNode.append_child ("link");
  link_node.append_attribute ("rel") = "stylesheet";
  link_node.append_attribute ("type") = "text/css";
  link_node.append_attribute ("href") = "stylesheet.css";
  
  // <body>
  bodyDomNode = root_node.append_child ("body");
  
  // Optional for notes: <div>
  notesDivDomNode = bodyDomNode.append_child ("div");
}


void Html_Text::newParagraph (string style)
{
  currentPDomElement = bodyDomNode.append_child ("p");
  if (style != "") {
    string clss = style;
    if (customClass != "") {
      clss += " " + customClass;
    }
    currentPDomElement.append_attribute ("class") = clss.c_str();
  }
  current_p_node_open = true;
  currentParagraphStyle = style;
  currentParagraphContent.clear();
}


// This function adds text to the current paragraph.
// $text: The text to add.
void Html_Text::addText (string text)
{
  if (text != "") {
    if (!current_p_node_open) newParagraph ();
    xml_node span = currentPDomElement.append_child ("span");
    span.text().set (text.c_str());
    if (!currentTextStyle.empty ()) {
      // Take character style(s) as specified in the object.
      span.append_attribute ("class") = filter_string_implode (currentTextStyle, " ").c_str();
    }
    currentParagraphContent += text;
  }
}


// This creates a <h1> heading with contents.
// $text: Contents.
void Html_Text::newHeading1 (string text, bool hide)
{
  newNamedHeading ("h1", text, hide);
}


// This applies a page break.
void Html_Text::newPageBreak ()
{
  // The style is already in the css.
  newParagraph ("break");
  // Always clear the flag for the open paragraph,
  // because we don't want subsequent text to be added to this page break,
  // since it would be nearly invisible, and thus text would seem to be lost.
  current_p_node_open = false;
  currentParagraphStyle.clear();
  currentParagraphContent.clear();
}


// This opens a text style.
// $style: the array containing the style variables.
// $note: boolean: Whether this refers to notes.
// $embed: boolean: Whether to embed the new text style in an existing text style.
//                  true: add the new style to the existing style.
//                  false: close any existing text style, and open the new style.
void Html_Text::openTextStyle (Database_Styles_Item style, bool note, bool embed)
{
  string marker = style.marker;
  if (note) {
    if (!embed) currentNoteTextStyle.clear();
    currentNoteTextStyle.push_back (marker);
  } else {
    if (!embed) currentTextStyle.clear();
    currentTextStyle.push_back (marker);
  }
}


// This closes any open text style.
// $note: boolean: Whether this refers to notes.
// $embed: boolean: Whether to embed the new text style in an existing text style.
//                  true: add the new style to the existing style.
//                  false: close any existing text style, and open the new style.
void Html_Text::closeTextStyle (bool note, bool embed)
{
  if (note) {
    if (!currentNoteTextStyle.empty()) currentNoteTextStyle.pop_back ();
    if (!embed) currentNoteTextStyle.clear();
  } else {
    if (!currentTextStyle.empty()) currentTextStyle.pop_back ();
    if (!embed) currentTextStyle.clear ();
  }
}


// This function adds a note to the current paragraph.
// $citation: The text of the note citation.
// $style: Style name for the paragraph in the note body.
// $endnote: Whether this is a footnote and cross reference (false), or an endnote (true).
void Html_Text::addNote (string citation, string style, bool endnote)
{
  // Ensure that a paragraph is open, so that the note can be added to it.
  if (!current_p_node_open) newParagraph ();
  
  if (endnote) {};
  
  noteCount++;
  
  // Add the link with all relevant data for the note citation.
  string reference = "#note" + convert_to_string (noteCount);
  string identifier = "citation" + convert_to_string (noteCount);
  addLink (currentPDomElement, reference, identifier, "", "superscript", citation);
  
  // Open a paragraph element for the note body.
  notePDomElement = notesDivDomNode.append_child ("p");
  notePDomElement.append_attribute ("class") = style.c_str();
  note_p_node_open = true;
  
  closeTextStyle (true, false);
  
  // Add the link with all relevant data for the note body.
  reference = "#citation" + convert_to_string (noteCount);
  identifier = "note" + convert_to_string (noteCount);
  addLink (notePDomElement, reference, identifier, "", "", citation);
  
  // Add a space.
  addNoteText (" ");
}


// This function adds text to the current footnote.
// $text: The text to add.
void Html_Text::addNoteText (string text)
{
  if (text != "") {
    if (!note_p_node_open) addNote ("?", "");
    xml_node spanDomElement = notePDomElement.append_child ("span");
    spanDomElement.text().set (text.c_str());
    if (!currentNoteTextStyle.empty ()) {
      // Take character style as specified in this object.
      spanDomElement.append_attribute ("class") = filter_string_implode (currentNoteTextStyle, " ").c_str();
    }
  }
}


// This function closes the current footnote.
void Html_Text::closeCurrentNote ()
{
  closeTextStyle (true, false);
  note_p_node_open = false;
}


// This adds a link.
// $domNode: The DOM node where to add the link to.
// $reference: The link's href, e.g. where it links to.
// $identifier: The link's identifier. Others can link to it.
// $title: The link's title, to make it accessible, e.g. for screenreaders.
// $style: The link text's style.
// $text: The link's text.
void Html_Text::addLink (xml_node domNode, string reference, string identifier, string title, string style, string text)
{
  xml_node aDomElement = domNode.append_child ("a");
  aDomElement.append_attribute ("href") = reference.c_str();
  aDomElement.append_attribute ("id") = identifier.c_str();
  if (!title.empty ()) {
    aDomElement.append_attribute ("title") = title.c_str();
  }
  if (style != "") {
    aDomElement.append_attribute ("class") = style.c_str();
  }
  aDomElement.text ().set (text.c_str());
}


// This gets and then returns the html text
string Html_Text::getHtml ()
{
  // Move any notes into place: At the end of the body.
  // Or remove empty notes container.
  if (noteCount > 0) {
    bodyDomNode.append_move (notesDivDomNode);
  } else {
    bodyDomNode.remove_child (notesDivDomNode);
  }
  noteCount = 0;

  // Get the html.
  stringstream output;
  htmlDom.print (output, "", format_raw);
  string html = output.str ();
  
  // Add html5 doctype.
  html.insert (0, "<!DOCTYPE html>\n");
 
  return html;
}


// Returns the html text within the <body> tags, that is, without the <head> stuff.
string Html_Text::getInnerHtml ()
{
  string page = getHtml ();
  size_t pos = page.find ("<body>");
  if (pos != string::npos) {
    page = page.substr (pos + 6);
    pos = page.find ("</body>");
    if (pos != string::npos) {
      page = page.substr (0, pos);
    }
  }
  return page;
}


// This saves the web page to file
// $name: the name of the file to save to.
void Html_Text::save (string name)
{
  string html = getHtml ();
  filter_url_file_put_contents (name, html);
}


// This adds a new table to the html DOM.
// Returns: The new $tableElement
xml_node Html_Text::newTable ()
{
  // Adding subsequent text should create a new paragraph.
  current_p_node_open = false;
  currentParagraphStyle.clear();
  currentParagraphContent.clear();
  // Append the table.
  xml_node tableElement = bodyDomNode.append_child ("table");
  tableElement.append_attribute ("width") = "100%";
  return tableElement;
}


// This adds a new row to an existing $tableElement.
// Returns: The new $tableRowElement.
xml_node Html_Text::newTableRow (xml_node tableElement)
{
  xml_node tableRowElement = tableElement.append_child ("tr");
  return tableRowElement;
}


// This adds a new data cell to an existing $tableRowElement.
// Returns: The new $tableDataElement.
xml_node Html_Text::newTableData (xml_node tableRowElement, bool alignRight)
{
  xml_node tableDataElement = tableRowElement.append_child ("td");
  if (alignRight) tableDataElement.append_attribute ("align") = "right";
  return tableDataElement;
}


// This creates a heading with styled content.
// $style: A style name.
// $text: Content.
void Html_Text::newNamedHeading (string style, string text, bool hide)
{
  if (hide) {};
  xml_node textHDomElement = bodyDomNode.append_child (style.c_str());
  textHDomElement.text ().set (text.c_str());
  // Make paragraph null, so that adding subsequent text creates a new paragraph.
  current_p_node_open = false;
  currentParagraphStyle.clear ();
  currentParagraphContent.clear ();
}

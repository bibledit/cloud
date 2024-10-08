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


#include <editor/html2usfm.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/usfm.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logs.h>
#include <pugixml/utils.h>
#include <quill/logic.h>


void Editor_Html2Usfm::load (std::string html)
{
  // The web editor may insert non-breaking spaces. Convert them to normal spaces.
  html = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), " ", html);
  
  // The web editor produces <hr> and other elements following the HTML specs,
  // but the pugixml XML parser needs <hr/> and similar elements.
  html = filter::strings::html2xml (html);

  std::string xml = "<body>" + html + "</body>";
  // Parse document such that all whitespace is put in the DOM tree.
  // See http://pugixml.org/docs/manual.html for more information.
  // It is not enough to only parse with parse_ws_pcdata_single, it really needs parse_ws_pcdata.
  // This is significant for, for example, the space after verse numbers, among other cases.
  pugi::xml_parse_result result = document.load_string (xml.c_str(), pugi::parse_ws_pcdata);
  // Log parsing errors.
  pugixml_utils_error_logger (&result, xml);
}


void Editor_Html2Usfm::stylesheet (std::string stylesheet)
{
  styles.clear ();
  noteOpeners.clear ();
  characterStyles.clear ();
  Database_Styles database_styles;
  std::vector <std::string> markers = database_styles.getMarkers (stylesheet);
  // Load the style information into the object.
  for (std::string & marker : markers) {
    Database_Styles_Item style = database_styles.getMarkerData (stylesheet, marker);
    styles [marker] = style;
    // Get markers that should not have endmarkers.
    bool suppress = false;
    int type = style.type;
    int subtype = style.subtype;
    if (type == StyleTypeVerseNumber) suppress = true;
    if (type == StyleTypeFootEndNote) {
      suppress = true;
      if (subtype == FootEndNoteSubtypeFootnote) noteOpeners.insert (marker);
      if (subtype == FootEndNoteSubtypeEndnote) noteOpeners.insert (marker);
      if (subtype == FootEndNoteSubtypeContentWithEndmarker) suppress = false;
      if (subtype == FootEndNoteSubtypeParagraph) suppress = false;
    }
    if (type == StyleTypeCrossreference) {
      suppress = true;
      if (subtype == CrossreferenceSubtypeCrossreference) noteOpeners.insert (marker);
      if (subtype == CrossreferenceSubtypeContentWithEndmarker) suppress = false;
    }
    if (type == StyleTypeTableElement) suppress = true;
    if (suppress) suppressEndMarkers.insert (marker);
  }
}


void Editor_Html2Usfm::run ()
{
  preprocess ();
  process ();
  postprocess ();
}


void Editor_Html2Usfm::process ()
{
  // Iterate over the children to retrieve the "p" elements, then process them.
  pugi::xml_node body = document.first_child ();
  for (pugi::xml_node node : body.children()) {
    // Do not process the notes <div> or <p> and beyond
    // because it is at the end of the text body,
    // and note-related data has already been extracted from it.
    std::string classs = update_quill_class (node.attribute ("class").value ());
    if (classs == "notes") break;
    // Process the node.
    processNode (node);
  }
}


std::string Editor_Html2Usfm::get ()
{
  // Generate the USFM as one string.
  std::string usfm = filter::strings::implode (output, "\n");
  
  usfm = cleanUSFM (usfm);
  
  return usfm;
}


void Editor_Html2Usfm::processNode (pugi::xml_node node)
{
  switch (node.type ()) {
    case pugi::node_element:
    {
      // Skip a note with class "ql-cursor" because that is an internal Quill node.
      // The user didn't insert it.
      std::string classs = node.attribute("class").value();
      if (classs == "ql-cursor") break;
      // Process this node.
      openElementNode (node);
      for (pugi::xml_node child : node.children()) {
        processNode (child);
      }
      closeElementNode (node);
      break;
    }
    case pugi::node_pcdata:
    {
      // Add the text to the current USFM line.
      std::string text = node.text ().get ();
      currentLine += text;
      break;
    }
    case pugi::node_null:
    case pugi::node_document:
    case pugi::node_comment:
    case pugi::node_pi:
    case pugi::node_declaration:
    case pugi::node_doctype:
    case pugi::node_cdata:
    default:
    {
      std::string nodename = node.name ();
      Database_Logs::log ("XML node " + nodename + " not handled while saving editor text");
      break;
    }
  }
}


void Editor_Html2Usfm::openElementNode (pugi::xml_node node)
{
  // The tag and class names of this element node.
  std::string tagName = node.name ();
  std::string className = update_quill_class (node.attribute ("class").value ());
  
  if (tagName == "p")
  {
    // While editing, it may occur that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (className.empty ()) className = "p";
    if (className == "mono") {
      // Class 'mono': The editor has the full USFM in the text.
      mono = true;
    } else {
      // Start the USFM line with a marker with the class name.
      currentLine += filter::usfm::get_opening_usfm (className);
    }
  }
  
  if (tagName == "span")
  {
    if (className == "v")  {
      // Handle the verse.
      flushLine ();
      openInline (className);
    }
    else if (className.empty ()) {
      // Normal text is wrapped in elements without a class attribute.
    } else if (className.substr (0, 8) == "notecall") {
      // Note in Quill-based editor.
      processNoteCitation (node);
    } else {
      // Handle remaining class attributes for inline text.
      openInline (className);
    }
  }
  
  if (tagName == "a")
  {
    processNoteCitation (node);
  }
}


void Editor_Html2Usfm::closeElementNode (pugi::xml_node node)
{
  // The tag and class names of this element node.
  std::string tagName = node.name ();
  std::string className = update_quill_class (node.attribute ("class").value ());
  
  if (tagName == "p")
  {
    // While editing it happens that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (className == "") className = "p";
    
    if (noteOpeners.find (className) != noteOpeners.end()) {
      // Deal with note closers.
      currentLine += filter::usfm::get_closing_usfm (className);
    } else {
      // Normally a p element closes the USFM line.
      flushLine ();
      mono = false;
      // Clear active character styles.
      characterStyles.clear();
    }
  }
  
  if (tagName == "span")
  {
    // Do nothing for monospace elements, because the USFM would be the text nodes only.
    if (mono) return;
    // Do nothing without a class.
    if (className.empty()) return;
    // Do nothing with a note caller.
    if (className.substr (0, 8) == "notecall") return;
    // Do nothing if no endmarkers are supposed to be produced.
    if (suppressEndMarkers.find (className) != suppressEndMarkers.end()) return;
    // Add closing USFM, optionally closing embedded tags in reverse order.
    char separator = '0';
    std::vector <std::string> classes = filter::strings::explode (className, separator);
    characterStyles = filter::strings::array_diff (characterStyles, classes);
    reverse (classes.begin(), classes.end());
    for (unsigned int offset = 0; offset < classes.size(); offset++) {
      bool embedded = (classes.size () > 1) && (offset == 0);
      if (!characterStyles.empty ()) embedded = true;
      currentLine += filter::usfm::get_closing_usfm (classes [offset], embedded);
      lastNoteStyle.clear();
    }
  }
  
  if (tagName == "a")
  {
    // Do nothing for note citations in the text.
  }
}


void Editor_Html2Usfm::openInline (std::string className)
{
  // It has been observed that the <span> elements of the character styles may be embedded, like so:
  // The <span class="add">
  //   <span class="nd">Lord God</span>
  // is calling</span> you</span><span>.</span>
  char separator = '0';
  std::vector <std::string> classes = filter::strings::explode (className, separator);
  for (unsigned int offset = 0; offset < classes.size(); offset++) {
    bool embedded = (characterStyles.size () + offset) > 0;
    std::string marker = classes[offset];
    bool add_opener = true;
    if (processingNote) {
      // If the style within the note has already been opened before,
      // do not open the same style again.
      // https://github.com/bibledit/cloud/issues/353
      if (marker == lastNoteStyle) add_opener = false;
      lastNoteStyle = marker;
    } else {
      lastNoteStyle.clear ();
    }
    if (add_opener)
      currentLine += filter::usfm::get_opening_usfm (marker, embedded);
  }
  // Store active character styles in some cases.
  bool store = true;
  if (suppressEndMarkers.find (className) != suppressEndMarkers.end ()) store = false;
  if (processingNote) store = false;
  if (store) {
    characterStyles.insert (characterStyles.end(), classes.begin(), classes.end());
  }
}


void Editor_Html2Usfm::processNoteCitation (pugi::xml_node node)
{
  // Remove the note citation from the main text body.
  // It means that this:
  //   <span class="i-notecall1">1</span>
  // becomes this:
  //   <span class="i-notecall1" />
  pugi::xml_node child = node.first_child ();
  node.remove_child (child);

  // Get more information about the note to retrieve.
  // <span class="i-notecall1" />
  std::string id = node.attribute ("class").value ();
  id = filter::strings::replace ("call", "body", id);

  // Sample footnote body.
  // <p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">notetext</span></p>
  // Retrieve the <a> element from it.
  // This was initially done through an XPath expression:
  // http://www.grinninglizard.com/tinyxml2docs/index.html
  // But XPath crashed on Android with libxml2.
  // Therefore now it iterates over all the nodes to find the required element.
  // After moving to pugixml, the XPath expression could have been used again, but this was not done.
  pugi::xml_node note_p_element = get_note_pointer (document.first_child (), id);
  if (note_p_element) {

    // It now has the <p>.
    // Remove the first <span> element.
    // So we remain with:
    // <p class="x"><span> </span><span>+ 2 Joh. 1.1</span></p>
    {
      pugi::xml_node node2 = note_p_element.first_child();
      std::string name = node2.name ();
      if (name != "span") {
        // Normally the <span> is the first child in the <p> that is a note.
        // But the user may have typed some text there.
        // If so, then the <span> is the second child of the <p>.
        // This code cares for that situation.
        node2 = node2.next_sibling();
        name = node2.name();
      }
      note_p_element.remove_child (node2);
    }

    // Preserve active character styles in the main text, and reset them for the note.
    std::vector <std::string> preservedCharacterStyles = characterStyles;
    characterStyles.clear();
    
    // Process this 'p' element.
    processingNote = true;
    processNode (note_p_element);
    processingNote = false;
    
    // Restore the active character styles for the main text.
    characterStyles = preservedCharacterStyles;
    
    // Remove this element so it can't be processed again.
    pugi::xml_node parent = note_p_element.parent ();
    parent.remove_child (note_p_element);

  } else {
    Database_Logs::log ("Discarding note with id " + id);
  }
}


std::string Editor_Html2Usfm::cleanUSFM (std::string usfm)
{
  // Replace a double space after a note opener.
  for (std::string noteOpener : noteOpeners) {
    std::string opener = filter::usfm::get_opening_usfm (noteOpener);
    usfm = filter::strings::replace (opener + " ", opener, usfm);
  }
  
  // Unescape special XML characters.
  usfm = filter::strings::unescape_special_xml_characters (usfm);

  // Done.
  return usfm;
}


void Editor_Html2Usfm::preprocess ()
{
  output.clear ();
  currentLine.clear ();
  mono = false;
}


void Editor_Html2Usfm::flushLine ()
{
  if (!currentLine.empty ()) {
    // Trim so that '\p ' becomes '\p', for example.
    currentLine = filter::strings::trim (currentLine);
    // No longer doing the above
    // because it would remove a space intentionally added to the end of a line.
    // Instead it now only does a left trim instead of the full trim.
    // currentLine = filter::strings::ltrim (currentLine);
    output.push_back (currentLine);
    currentLine.clear ();
  }
}


void Editor_Html2Usfm::postprocess ()
{
  // Flush any last USFM line being built.
  flushLine ();
}


// Retrieves a pointer to a relevant footnote element in the XML.
pugi::xml_node Editor_Html2Usfm::get_note_pointer (pugi::xml_node body, std::string id)
{
  // The note wrapper node to look for.
  pugi::xml_node p_note_wrapper;

  // Check that there's a node to start with.
  if (!body) return p_note_wrapper;

  // Assert that the <body> node is given.
  if (std::string(body.name ()) != "body") return p_note_wrapper;

  // Some of the children of the <body> node will be the note wrappers.
  // Consider this XML:
  // <body>
  //   <p class="b-p">text<span class="i-notecall1" /></p>
  //   <p class="b-notes" />
  //   <p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">foot</span></p>
  //   <p class="b-f"><span class="i-ft">note</span></p>
  // </body>
  // There is node <p class="b-notes" />.
  // Any children of <body> following the above node are all note wrappers.
  // Here the code is going to get the correct note wrapper node.
  // In addition to finding the correct p node, it also looks for subsequent p nodes
  // that belong to the main p node.
  // See issue https://github.com/bibledit/cloud/issues/444.
  // It handles a situation that the user presses <Enter> while in a note.
  // The solution is to include the next p node too if it belongs to the correct note wrapper p node.
  bool within_matching_p_node = false;
  for (pugi::xml_node p_body_child : body.children ()) {
    pugi::xml_node span_notebody = p_body_child.first_child();
    std::string name = span_notebody.name ();
    if (name != "span") {
      // Normally the <span> is the first child in the <p> that is a note.
      // But the user may have typed some text there.
      // If so, then the <span> is the second child of the <p>.
      // This code cares for that situation.
      span_notebody = span_notebody.next_sibling();
      name = span_notebody.name();
    }
    if (name == "span") {
      std::string classs = span_notebody.attribute ("class").value ();
      if (classs.substr (0, 10) == id.substr(0, 10)) {
        if (classs == id) {
          within_matching_p_node = true;
        } else {
          within_matching_p_node = false;
        }
      }
    }
    if (within_matching_p_node) {
      if (!p_note_wrapper) {
        p_note_wrapper = p_body_child;
      } else {
        for (pugi::xml_node child = p_body_child.first_child(); child; child = child.next_sibling()) {
          p_note_wrapper.append_copy(child);
        }
      }
    }
  }

  // If a note wrapper was found, return that.
  // If nothing was found, the note wrapper is null.
  return p_note_wrapper;
}


std::string Editor_Html2Usfm::update_quill_class (std::string classname)
{
  classname = filter::strings::replace (quill_logic_class_prefix_block (), "", classname);
  classname = filter::strings::replace (quill_logic_class_prefix_inline (), "", classname);
  return classname;
}


// This function takes the html from a Quill-based editor that edits one verse,
// and converts it to USFM.
// It properly deals with cases when a verse does not start a new paragraph.
std::string editor_export_verse_quill (std::string stylesheet, std::string html)
{
  // When the $html starts with a paragraph without a style,
  // put a recognizable style there.
  std::string style = "oneversestyle";
  size_t pos = html.find ("<p>");
  if (pos == 0) {
    html.insert (2, R"( class=")" + quill_logic_class_prefix_block () + style + R"(")");
  }

  // Convert html to USFM.
  Editor_Html2Usfm editor_export;
  editor_export.load (html);
  editor_export.stylesheet (stylesheet);
  editor_export.run ();
  std::string usfm = editor_export.get ();
  
  // Remove that recognizable style converted to USFM.
  usfm = filter::strings::replace (R"(\)" + style, std::string(), usfm);
  usfm = filter::strings::trim (usfm);

  return usfm;
}

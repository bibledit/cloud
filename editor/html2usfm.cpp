/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


// This function returns a pair of:
// 1. Normal classes, like "add", "nd", and so on.
// 2. A string with the word-level attributes class, like "wla2" for example
static std::pair<std::vector<std::string>, std::string>const get_standard_classes_and_wla_class (const std::string& class_name)
{
  constexpr char separator = '0';
  std::vector <std::string> classes = filter::strings::explode (class_name, separator);
  std::string wla{};
  for (auto iter = classes.cbegin(); iter != classes.cend(); iter++) {
    if (iter->find(quill_word_level_attribute_class_prefix) == 0) {
      wla = *iter;
      classes.erase(iter);
      break;
    }
  }
  return {classes, wla};
};


void Editor_Html2Usfm::load (std::string html)
{
  // The web editor may insert non-breaking spaces. Convert them to normal spaces.
  html = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), " ", html);
  
  // The web editor produces <hr> and other elements following the HTML specs,
  // but the pugixml XML parser needs <hr/> and similar elements.
  html = filter::strings::html2xml (html);

  const std::string xml = "<body>" + std::move(html) + "</body>";
  // Parse document such that all whitespace is put in the DOM tree.
  // See http://pugixml.org/docs/manual.html for more information.
  // It is not enough to only parse with parse_ws_pcdata_single, it really needs parse_ws_pcdata.
  // This is significant for, for example, the space after verse numbers, among other cases.
  pugi::xml_parse_result result = document.load_string (xml.c_str(), pugi::parse_ws_pcdata);
  // Log parsing errors.
  pugixml_utils_error_logger (&result, xml);
}


void Editor_Html2Usfm::stylesheet (const std::string& stylesheet)
{
  styles.clear ();
  note_openers.clear ();
  character_styles.clear ();
  const std::vector <std::string> markers = database::styles1::get_markers (stylesheet);
  // Load the style information into the object.
  for (const std::string& marker : markers) {
    database::styles1::Item style = database::styles1::get_marker_data (stylesheet, marker);
    styles [marker] = style;
    // Get markers that should not have endmarkers.
    bool suppress = false;
    const int type = style.type;
    const int subtype = style.subtype;
    if (type == StyleTypeVerseNumber)
      suppress = true;
    if (type == StyleTypeFootEndNote) {
      suppress = true;
      if (subtype == FootEndNoteSubtypeFootnote)
        note_openers.insert (marker);
      if (subtype == FootEndNoteSubtypeEndnote)
        note_openers.insert (marker);
      if (subtype == FootEndNoteSubtypeContentWithEndmarker)
        suppress = false;
      if (subtype == FootEndNoteSubtypeParagraph)
        suppress = false;
    }
    if (type == StyleTypeCrossreference) {
      suppress = true;
      if (subtype == CrossreferenceSubtypeCrossreference)
        note_openers.insert (marker);
      if (subtype == CrossreferenceSubtypeContentWithEndmarker)
        suppress = false;
    }
    if (type == StyleTypeTableElement)
      suppress = true;
    if (suppress)
      suppress_end_markers.insert (marker);
  }
}


void Editor_Html2Usfm::run ()
{
  pre_process ();
  main_process ();
  post_process ();
}


void Editor_Html2Usfm::main_process ()
{
  // Iterate over the children to retrieve the "p" elements, then process them.
  const pugi::xml_node body = document.first_child ();
  for (pugi::xml_node& node : body.children()) {
    // Do not process the notes <p> and beyond
    // because it is at the end of the text body,
    // and note-related data has already been extracted from it.
    const std::string classs = update_quill_class (node.attribute ("class").value ());
    if (classs == quill_notes_class) {
      break;
    }
    // Process the node.
    process_node(node);
  }
}


std::string Editor_Html2Usfm::get ()
{
  // Generate the USFM as one string.
  const std::string usfm = filter::strings::implode (output, "\n");
  return clean_usfm (usfm);
}


void Editor_Html2Usfm::process_node(pugi::xml_node& node)
{
  switch (node.type ()) {
    case pugi::node_element:
    {
      const std::string classs = node.attribute("class").value();
      // Skip a node with class "ql-cursor" because that is an internal Quill node.
      // The user didn't insert it.
      if (classs == quill_caret_class)
        break;
      // Skip a node that is for the word-level attributes since it is just there for visual appearance.
      // The user didn't insert it.
      if (classs.find(quill_word_level_attributes_class) != std::string::npos)
        break;
      // Process this node.
      open_element_node (node);
      for (pugi::xml_node& child : node.children()) {
        process_node(child);
      }
      close_element_node (node);
      break;
    }
    case pugi::node_pcdata:
    {
      // Add the text to the current USFM line.
      m_last_added_text_fragment = node.text ().get ();
      current_line.append(m_last_added_text_fragment);
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
      Database_Logs::log ("XML node " + std::string(node.name ()) + " not handled while saving editor text");
      break;
    }
  }
}


void Editor_Html2Usfm::open_element_node (pugi::xml_node& node)
{
  // The tag and class names of this element node.
  const std::string tag_name = node.name ();
  std::string class_name = update_quill_class (node.attribute ("class").value ());
  
  if (tag_name == "p")
  {
    // While editing, it may occur that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (class_name.empty ())
      class_name = "p";
    if (class_name == "mono") {
      // Class 'mono': The editor has the full USFM in the text.
      mono = true;
    } else {
      // Start the USFM line with a marker with the class name.
      current_line.append(filter::usfm::get_opening_usfm (class_name));
    }
  }
  
  if (tag_name == "span")
  {
    if (class_name == "v")  {
      // Handle the verse.
      flush_line ();
      open_inline (class_name);
    }
    else if (class_name.empty ()) {
      // Normal text is wrapped in elements without a class attribute.
    }
    else if (class_name.substr (0, quill_note_caller_class.size()) == quill_note_caller_class) {
      // Note in Quill-based editor.
      process_note_citation (node);
    }
    else {
      // Handle remaining class attributes for inline text.
      open_inline (class_name);
    }
  }
  
  if (tag_name == "a")
  {
    process_note_citation (node);
  }
}


void Editor_Html2Usfm::close_element_node (const pugi::xml_node& node)
{
  // Get the tag name of this node.
  const std::string tag_name = node.name ();
  // Get the class names of this node.
  // Get the word-level attributes class name / identifier.
  std::string class_name = update_quill_class (node.attribute ("class").value ());
  auto [classes, wla_class] = get_standard_classes_and_wla_class (class_name);

  if (tag_name == "p")
  {
    // While editing it happens that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (class_name.empty())
      class_name = "p";
    
    if (note_openers.find (class_name) != note_openers.end()) {
      // Deal with note closers.
      current_line.append(filter::usfm::get_closing_usfm (class_name));
    } else {
      // Normally a p element closes the USFM line.
      flush_line ();
      mono = false;
      // Clear active character styles.
      character_styles.clear();
    }
  }
  
  if (tag_name == "span")
  {
    // Do nothing for monospace elements, because the USFM would be the text nodes only.
    if (mono)
      return;
    // Do nothing without a class.
    if (class_name.empty())
      return;
    // Do nothing with a note caller.
    if (class_name.substr (0, quill_note_caller_class.size()) == quill_note_caller_class)
      return;
    // Do nothing if no endmarkers are supposed to be produced.
    // There's two exceptions:
    // 1. If a word-level attributes ID was found: This needs an endmarker.
    // 2. If the last added text fragment contains the vertical bar, as that indicates word-level attributes.
    if (suppress_end_markers.find (class_name) != suppress_end_markers.end()) {
      if ((wla_class.empty()) && (m_last_added_text_fragment.find("|") == std::string::npos))
        return;
    }
    // Check for and get and handle word-level attributes.
    if (!wla_class.empty()) {
      const std::string contents = m_word_level_attributes[wla_class];
      m_word_level_attributes.erase(wla_class);
      if (!contents.empty()) {
        // The vertical bar separates the canonical word from the attribute(s) following it.
        current_line.append("|");
        current_line.append(contents);
      }
    }
    // Add closing USFM, optionally closing embedded tags in reverse order.
    character_styles = filter::strings::array_diff (character_styles, classes);
    reverse (classes.begin(), classes.end());
    for (unsigned int offset = 0; offset < classes.size(); offset++) {
      bool embedded = (classes.size () > 1) && (offset == 0);
      if (!character_styles.empty ()) embedded = true;
      current_line.append (filter::usfm::get_closing_usfm (classes [offset], embedded));
      last_note_style.clear();
    }
  }
  
  if (tag_name == "a")
  {
    // Do nothing for note citations in the text.
  }
}


void Editor_Html2Usfm::open_inline (const std::string& class_name)
{
  // It has been observed that the <span> elements of the character styles may be embedded, like so:
  // The <span class="add">
  //   <span class="nd">Lord God</span>
  // is calling</span> you</span><span>.</span>
  const auto [classes, wla] = get_standard_classes_and_wla_class(class_name);
  for (unsigned int offset = 0; offset < classes.size(); offset++) {
    const bool embedded = (character_styles.size () + offset) > 0;
    const std::string marker = classes[offset];
    bool add_opener = true;
    if (processing_note) {
      // If the style within the note has already been opened before,
      // do not open the same style again.
      // https://github.com/bibledit/cloud/issues/353
      if (marker == last_note_style) add_opener = false;
      last_note_style = marker;
    } else {
      last_note_style.clear ();
    }
    if (add_opener) {
      current_line.append (filter::usfm::get_opening_usfm (marker, embedded));
    }
  }
  // Store active character styles in some cases.
  bool store = true;
  if (suppress_end_markers.find (class_name) != suppress_end_markers.end ())
    store = false;
  if (processing_note)
    store = false;
  if (store) {
    character_styles.insert (character_styles.end(), classes.begin(), classes.end());
  }
}


void Editor_Html2Usfm::process_note_citation (pugi::xml_node& node)
{
  // Remove the note citation from the main text body.
  // It means that this:
  //   <span class="i-notecall1">1</span>
  // becomes this:
  //   <span class="i-notecall1" />
  const pugi::xml_node child = node.first_child ();
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
    std::vector <std::string> preserved_character_styles = std::move(character_styles);
    character_styles.clear();
    
    // Process this 'p' element.
    processing_note = true;
    process_node(note_p_element);
    processing_note = false;
    
    // Restore the active character styles for the main text.
    character_styles = std::move(preserved_character_styles);
    
    // Remove this element so it can't be processed again.
    pugi::xml_node parent = note_p_element.parent ();
    parent.remove_child (note_p_element);

  } else {
    Database_Logs::log ("Discarding note with id " + id);
  }
}


std::string Editor_Html2Usfm::clean_usfm (std::string usfm)
{
  // Replace a double space after a note opener.
  for (const std::string& noteOpener : note_openers) {
    const std::string opener = filter::usfm::get_opening_usfm (noteOpener);
    usfm = filter::strings::replace (opener + " ", opener, usfm);
  }
  
  // Unescape special XML characters.
  usfm = filter::strings::unescape_special_xml_characters (usfm);

  // Done.
  return usfm;
}


void Editor_Html2Usfm::pre_process ()
{
  output.clear ();
  current_line.clear ();
  mono = false;
  m_word_level_attributes.clear();
  
  // Remove the node for word-level attributes that was there only for visual appearance in the editors.
  // Store the word-level attributes themsselves in a container ready for use.
  // And remove those nodes too from the XML tree.
  std::vector<pugi::xml_node> delete_nodes{};
  pugi::xml_node body = document.first_child ();
  for (pugi::xml_node& node : body.children()) {
    const std::string classs = update_quill_class (node.attribute ("class").value ());
    if (classs == quill_word_level_attributes_class) {
      delete_nodes.push_back(node);
    }
    // The classs could be "wla1", and the prefix is "wla", so check on the match.
    if (classs.find(quill_word_level_attribute_class_prefix) == 0) {
      delete_nodes.push_back(node);
      const std::string text = node.text().get();
      m_word_level_attributes.insert({classs, text});
    }
  }
  for (auto& node : delete_nodes) {
    body.remove_child(node);
  }
}


void Editor_Html2Usfm::flush_line ()
{
  if (!current_line.empty ()) {
    // Trim so that '\p ' becomes '\p', for example.
    current_line = filter::strings::trim (std::move(current_line));
    // No longer doing the above
    // because it would remove a space intentionally added to the end of a line.
    // Instead it now only does a left trim instead of the full trim.
    // current_line = filter::strings::ltrim (current_line);
    output.push_back (std::move(current_line));
    current_line.clear ();
  }
}


void Editor_Html2Usfm::post_process ()
{
  // Flush any last USFM line being built.
  flush_line ();
  
  // Log any word-level attributes that have not been integrated into the USFM.
  for (const auto& element : m_word_level_attributes) {
    Database_Logs::log ("Discarding unprocessed word-level attribute with key: " + element.first + " and value: " + element.second);
  }
}


// Retrieves a pointer to a relevant footnote element in the XML.
pugi::xml_node Editor_Html2Usfm::get_note_pointer (const pugi::xml_node& body, const std::string& id)
{
  // The note wrapper node to look for.
  pugi::xml_node p_note_wrapper;

  // Check that there's a node to start with.
  if (!body)
    return p_note_wrapper;

  // Assert that the <body> node is given.
  if (std::string(body.name ()) != "body")
    return p_note_wrapper;

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
      const std::string classs = span_notebody.attribute ("class").value ();
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
  classname = filter::strings::replace (quill_class_prefix_block, std::string(), classname);
  classname = filter::strings::replace (quill_class_prefix_inline, std::string(), classname);
  return classname;
}


// This function takes the html from a Quill-based editor that edits one verse,
// and converts it to USFM.
// It properly deals with cases when a verse does not start a new paragraph.
std::string editor_export_verse_quill (const std::string& stylesheet, std::string html)
{
  // When the $html starts with a paragraph without a style,
  // put a recognizable style there.
  const std::string one_verse_style = "oneversestyle";
  const size_t pos = html.find ("<p>");
  if (pos == 0) {
    html.insert (2, R"( class=")" + std::string(quill_class_prefix_block) + one_verse_style + R"(")");
  }

  // Convert html to USFM.
  Editor_Html2Usfm editor_export;
  editor_export.load (html);
  editor_export.stylesheet (stylesheet);
  editor_export.run ();
  std::string usfm = editor_export.get ();
  
  // Remove that recognizable style converted to USFM.
  usfm = filter::strings::replace (R"(\)" + one_verse_style, std::string(), usfm);
  usfm = filter::strings::trim (usfm);

  return usfm;
}

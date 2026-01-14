/*
 Copyright (©) 2003-2026 Teus Benschop.
 
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
#include <filter/quill.h>


// This function returns a struct of:
// 1. Normal classes, like "add", "nd", and so on.
// 2. A string with the word-level attributes class, like "wla2" for example.
// 3. A string with the milestone attributes class, like "mls2" for example.
struct std_wla_mls {
  std::vector<std::string> standard_classes;
  std::string word_level_attributes_class;
  std::string milestone_attributes_class;
};
static std_wla_mls get_standard_classes_and_wla_or_mls_class (const std::string& class_name)
{
  std_wla_mls std_wla_mls;
  constexpr char separator = '0';
  std::vector <std::string> classes = filter::string::explode (class_name, separator);
  std::string wla{};
  for (auto iter = classes.cbegin(); iter != classes.cend(); iter++) {
    if (iter->find(quill::word_level_attribute_class_prefix) == 0) {
      std_wla_mls.word_level_attributes_class = *iter;
      classes.erase(iter);
      break;
    }
    if (iter->find(quill::milestone_attribute_class_prefix) == 0) {
      std_wla_mls.milestone_attributes_class = *iter;
      classes.erase(iter);
      break;
    }
  }
  std_wla_mls.standard_classes = std::move(classes);
  return std_wla_mls;
}


void Editor_Html2Usfm::load (std::string html)
{
  // The web editor may insert non-breaking spaces. Convert them to normal spaces.
  html = filter::string::replace (filter::string::unicode_non_breaking_space_entity (), " ", html);
  
  // The web editor produces <hr> and other elements following the HTML specs,
  // but the pugixml XML parser needs <hr/> and similar elements.
  html = filter::string::html2xml (html);

  const std::string xml = "<body>" + std::move(html) + "</body>";
  // Parse document such that all whitespace is put in the DOM tree.
  // See https://pugixml.org/docs/manual.html for more information.
  // It is not enough to only parse with parse_ws_pcdata_single, it really needs parse_ws_pcdata.
  // This is significant for, for example, the space after verse numbers, among other cases.
  pugi::xml_parse_result result = m_document.load_string (xml.c_str(), pugi::parse_ws_pcdata);
  // Log parsing errors.
  pugixml_utils_error_logger (&result, xml);
}


void Editor_Html2Usfm::stylesheet (const std::string& stylesheet)
{
  m_note_openers.clear();
  m_suppress_end_markers.clear();
  m_force_end_markers.clear();
  m_character_styles.clear();
  const std::list<stylesv2::Style> styles = database::styles::get_styles (stylesheet);
  for (const auto& style : styles) {
    // Paragraph styles normally don't have a closing USFM marker.
    // But there's exceptions to this rule.
    // Gather the markers that need a closing USFM marker.
    if (stylesv2::get_parameter<bool>(&style, stylesv2::Property::has_endmarker))
      m_force_end_markers.insert(style.marker);
    // Get markers that should not have endmarkers.
    // Gather the note openers.
    bool suppress_endmarker = false;
    if (style.type == stylesv2::Type::verse)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::table_row)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::table_heading)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::table_cell)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::footnote_wrapper) {
      suppress_endmarker = true;
      m_note_openers.insert (style.marker);
    }
    if (style.type == stylesv2::Type::endnote_wrapper) {
      suppress_endmarker = true;
      m_note_openers.insert (style.marker);
    }
    if (style.type == stylesv2::Type::note_standard_content)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::note_content)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::note_paragraph)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::crossreference_wrapper) {
      suppress_endmarker = true;
      m_note_openers.insert (style.marker);
    }
    if (style.type == stylesv2::Type::crossreference_standard_content)
      suppress_endmarker = true;
    if (style.type == stylesv2::Type::crossreference_content)
      suppress_endmarker = true;
    if (suppress_endmarker)
      m_suppress_end_markers.insert (style.marker);
    if (style.type == stylesv2::Type::milestone)
      m_milestone_markers.insert (style.marker);
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
  const pugi::xml_node body = m_document.first_child ();
  for (pugi::xml_node& node : body.children()) {
    // Do not process the notes <p> and beyond
    // because it is at the end of the text body,
    // and note-related data has already been extracted from it.
    const std::string classs = update_quill_class (node.attribute ("class").value ());
    if (classs == quill::notes_class) {
      break;
    }
    // Process the node.
    process_node(node);
  }
}


std::string Editor_Html2Usfm::get ()
{
  // Generate the USFM as one string.
  const std::string usfm = filter::string::implode (m_output, "\n");
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
      if (classs == quill::caret_class)
        break;
      // Skip a node that is for the word-level attributes since it is just there for visual appearance.
      // The user didn't insert it.
      if (classs.find(quill::word_level_attributes_class) != std::string::npos)
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
      m_current_line.append(m_last_added_text_fragment);
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
      m_mono = true;
    } else {
      // Start the USFM line with a marker with the class name.
      m_current_line.append(filter::usfm::get_opening_usfm (class_name));
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
    else if (class_name.substr (0, quill::note_caller_class.size()) == quill::note_caller_class) {
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
  // Get the milestone attributes class name / identifier.
  std::string class_name = update_quill_class (node.attribute ("class").value ());
  auto std_wla_mls = get_standard_classes_and_wla_or_mls_class (class_name);

  if (tag_name == "p")
  {
    // While editing it happens that the p element does not have a class.
    // Use the 'p' class in such cases.
    if (class_name.empty())
      class_name = "p";
    
    if (m_note_openers.find (class_name) != m_note_openers.cend()) {
      // Deal with note closers.
      m_current_line.append(filter::usfm::get_closing_usfm (class_name));
    } else {
      // If this style should add an endmarker, handle that here.
      if (m_force_end_markers.find(class_name) != m_force_end_markers.cend()) {
        m_current_line.append(filter::usfm::get_closing_usfm (class_name));
      }
      // Normally a p element closes the USFM line.
      flush_line ();
      m_mono = false;
      // Clear active character styles.
      m_character_styles.clear();
    }
  }
  
  if (tag_name == "span")
  {
    // Do nothing for monospace elements, because the USFM would be the text nodes only.
    if (m_mono)
      return;
    // Do nothing without a class.
    if (class_name.empty())
      return;
    // Do nothing with a note caller.
    if (class_name.substr (0, quill::note_caller_class.size()) == quill::note_caller_class)
      return;

    // Check for and get and handle word-level attributes.
    if (!std_wla_mls.word_level_attributes_class.empty()) {
      if (!m_word_level_attributes[std_wla_mls.word_level_attributes_class].empty()) {
        // The vertical bar separates the canonical word from the attribute(s) following it.
        m_current_line.append("|");
        m_current_line.append(std::move(m_word_level_attributes[std_wla_mls.word_level_attributes_class]));
      }
      // Clean up.
      m_word_level_attributes.erase(std_wla_mls.word_level_attributes_class);
    }

    // Function for detecting whether closing a milestone class.
    const auto is_milestone = [this, &std_wla_mls, &class_name]() {
      if (!std_wla_mls.milestone_attributes_class.empty())
        return true;
      if (m_milestone_markers.find (class_name) != m_milestone_markers.end())
        return true;
      return false;
    };
    
    // If this is a milestone, then it means that the current line contains the milestone emoji.
    // This emoji is put in the editor for visual appearance only, but should not be part of the USFM generated.
    // Remove it here.
    if (is_milestone())
      m_current_line = filter::string::replace (quill::milestone_emoji, std::string(), std::move(m_current_line));

    // Check for and get and handle milestone attributes.
    if (is_milestone()) {
      if (!m_milestone_attributes[std_wla_mls.milestone_attributes_class].empty()) {
        // The vertical bar separates the opening marker from the attribute(s) following it.
        m_current_line.append("|");
        m_current_line.append(m_milestone_attributes[std_wla_mls.milestone_attributes_class]);
      }
      // Clean up.
      m_milestone_attributes.erase(std_wla_mls.milestone_attributes_class);
    }

    // Do nothing more if no endmarkers are supposed to be produced.
    // There's an exception in the case of linking attributes.
    if (m_suppress_end_markers.find (class_name) != m_suppress_end_markers.end()) {
      // Linking attributes are used in this: \xt 1|GEN 2:1\xt*
      // It means the signature is that the last added text fragment contains the vertical bar,
      // but not right at the first position.
      // This distinguishes it from the vertical bar that is used when inserting a note in the Bible editor.
      // See https://github.com/bibledit/cloud/issues/1030
      const auto linking_attribute = [this] () {
        const std::size_t pos = m_last_added_text_fragment.find("|");
        if (pos == std::string::npos)
          return false;
        if (pos != 0)
          return true;
        return false;
      };
      if (!linking_attribute())
        return;
    }
    
    // Add closing USFM, optionally closing embedded tags in reverse order.
    m_character_styles = filter::string::array_diff (m_character_styles, std_wla_mls.standard_classes);
    reverse (std_wla_mls.standard_classes.begin(), std_wla_mls.standard_classes.end());
    for (unsigned int offset = 0; offset < std_wla_mls.standard_classes.size(); offset++) {
      bool embedded = (std_wla_mls.standard_classes.size () > 1) && (offset == 0);
      if (!m_character_styles.empty ())
        embedded = true;
      if (is_milestone()) {
        // Adding a milestone endmarker: \*
        // If the last character of the current line is a space,
        // it is assumed that this is the space from the milestone opening marker.
        // If the opener is followed by the closer, without attributes between them,
        // then in USFM it is customary to remove that last space.
        // The resulting fragment then looks like e.g. this: \qt-e\*
        if (!m_current_line.empty())
          if (m_current_line.back() == ' ')
            m_current_line.pop_back();
        m_current_line.append (filter::usfm::get_closing_usfm (std::string(), false));
      }
      else
        // Normal endmarker: \marker*
        m_current_line.append (filter::usfm::get_closing_usfm (std_wla_mls.standard_classes [offset], embedded));
      m_last_note_style.clear();
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
  const auto std_wla_mls = get_standard_classes_and_wla_or_mls_class(class_name);
  for (unsigned int offset = 0; offset < std_wla_mls.standard_classes.size(); offset++) {
    const bool embedded = (m_character_styles.size () + offset) > 0;
    std::string marker = std_wla_mls.standard_classes.at(offset);
    // Milestones have an underscore due to requirement of Quill editor: Change to hyphen.
    if (!std_wla_mls.milestone_attributes_class.empty())
      marker = quill::underscore_to_hyphen (std::move(marker));
    bool add_opener = true;
    if (m_processing_note) {
      // If the style within the note has already been opened before,
      // do not open the same style again.
      // https://github.com/bibledit/cloud/issues/353
      if (marker == m_last_note_style)
        add_opener = false;
      m_last_note_style = marker;
    } else {
      m_last_note_style.clear ();
    }
    if (add_opener) {
      m_current_line.append (filter::usfm::get_opening_usfm (marker, embedded));
    }
  }
  // Store active character styles in some cases.
  bool store = true;
  if (m_suppress_end_markers.find (class_name) != m_suppress_end_markers.end ())
    store = false;
  if (m_processing_note)
    store = false;
  if (store) {
    m_character_styles.insert (m_character_styles.end(), std_wla_mls.standard_classes.begin(), std_wla_mls.standard_classes.end());
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
  id = filter::string::replace ("call", "body", id);

  // Sample footnote body.
  // <p class="b-f"><span class="i-notebody1">1</span> + <span class="i-ft">notetext</span></p>
  // Retrieve the <a> element from it.
  // This was initially done through an XPath expression:
  // https://www.grinninglizard.com/tinyxml2docs/index.html
  // But XPath crashed on Android with libxml2.
  // Therefore now it iterates over all the nodes to find the required element.
  // After moving to pugixml, the XPath expression could have been used again, but this was not done.
  pugi::xml_node note_p_element = get_note_pointer (m_document.first_child (), id);
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
    std::vector <std::string> preserved_character_styles = std::move(m_character_styles);
    m_character_styles.clear();
    
    // Process this 'p' element.
    m_processing_note = true;
    process_node(note_p_element);
    m_processing_note = false;
    
    // Restore the active character styles for the main text.
    m_character_styles = std::move(preserved_character_styles);
    
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
  for (const std::string& noteOpener : m_note_openers) {
    const std::string opener = filter::usfm::get_opening_usfm (noteOpener);
    usfm = filter::string::replace (opener + " ", opener, usfm);
  }
  
  // Unescape special XML characters.
  usfm = filter::string::unescape_special_xml_characters (usfm);

  // Done.
  return usfm;
}


void Editor_Html2Usfm::pre_process ()
{
  m_output.clear ();
  m_current_line.clear ();
  m_mono = false;
  m_word_level_attributes.clear();
  m_milestone_attributes.clear();
  
  // Remove the node for word-level attributes that was there only for visual appearance in the editors.
  // Store the word-level attributes themselves in a container ready for use.
  // And remove those nodes from the XML tree.
  {
    std::vector<pugi::xml_node> delete_nodes{};
    pugi::xml_node body = m_document.first_child ();
    for (pugi::xml_node& node : body.children()) {
      const std::string classs = update_quill_class (node.attribute ("class").value ());
      if (classs == quill::word_level_attributes_class) {
        delete_nodes.push_back(node);
      }
      // The classs could be "wla1", and the prefix is "wla", so check on the match.
      if (classs.find(quill::word_level_attribute_class_prefix) == 0) {
        delete_nodes.push_back(node);
        const std::string text = node.text().get();
        m_word_level_attributes.insert({classs, text});
      }
    }
    for (auto& node : delete_nodes) {
      body.remove_child(node);
    }
  }

  // Remove the node for milestone attributes that was there only for visual appearance in the editors.
  // Store the milestone attributes themselves in a container ready for use.
  // And remove those nodes from the XML tree.
  {
    std::vector<pugi::xml_node> delete_nodes{};
    pugi::xml_node body = m_document.first_child ();
    for (pugi::xml_node& node : body.children()) {
      const std::string classs = update_quill_class (node.attribute ("class").value ());
      if (classs == quill::milestone_attributes_class) {
        delete_nodes.push_back(node);
      }
      // The classs could be "mls1", and the prefix is "mls", so check on the match.
      if (classs.find(quill::milestone_attribute_class_prefix) == 0) {
        delete_nodes.push_back(node);
        const std::string text = node.text().get();
        m_milestone_attributes.insert({classs, text});
      }
    }
    for (auto& node : delete_nodes) {
      body.remove_child(node);
    }
  }
}


void Editor_Html2Usfm::flush_line ()
{
  if (!m_current_line.empty ()) {
    // Trim so that '\p ' becomes '\p', for example.
    m_current_line = filter::string::trim (std::move(m_current_line));
    // No longer doing the above
    // because it would remove a space intentionally added to the end of a line.
    // Instead it now only does a left trim instead of the full trim.
    // current_line = filter::string::ltrim (current_line);
    m_output.push_back (std::move(m_current_line));
    m_current_line.clear ();
  }
}


void Editor_Html2Usfm::post_process ()
{
  // Flush any last USFM line being built.
  flush_line ();
  
  // Log any word-level and milestone attributes that have not been integrated into the USFM.
  for (const auto& element : m_word_level_attributes) {
    Database_Logs::log ("Discarding unprocessed word-level attribute with key: " + element.first + " and value: " + element.second);
  }
  for (const auto& element : m_milestone_attributes) {
    Database_Logs::log ("Discarding unprocessed milestone attribute with key: " + element.first + " and value: " + element.second);
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
  classname = filter::string::replace (quill::class_prefix_block, std::string(), std::move(classname));
  classname = filter::string::replace (quill::class_prefix_inline, std::string(), std::move(classname));
  classname = quill::underscore_to_hyphen (std::move(classname));
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
    html.insert (2, R"( class=")" + std::string(quill::class_prefix_block) + one_verse_style + R"(")");
  }

  // Convert html to USFM.
  Editor_Html2Usfm editor_export;
  editor_export.load (html);
  editor_export.stylesheet (stylesheet);
  editor_export.run ();
  std::string usfm = editor_export.get ();
  
  // Remove that recognizable style converted to USFM.
  usfm = filter::string::replace (R"(\)" + one_verse_style, std::string(), usfm);
  usfm = filter::string::trim (usfm);

  return usfm;
}

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


#include <styles/css.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/css.h>
#include <config/globals.h>
#include <webserver/request.h>
#include <styles/logic.h>
#include <database/config/bible.h>
#include <fonts/logic.h>
#include <quill/logic.h>


Styles_Css::Styles_Css (const std::string& stylesheet)
{
  m_stylesheet = stylesheet;
}


// Call this method to add styles suitable for the Bible editor.
void Styles_Css::editor ()
{
  editor_enabled = true;
}


// Call this method to add styles suitable for exported Bible.
void Styles_Css::exports ()
{
  exports_enabled = true;
}


// Generates the CSS.
void Styles_Css::generate ()
{
  m_code.push_back (".superscript, [class^='i-note'] { font-size: x-small; vertical-align: super; }");
  if (exports_enabled) {
    add_exports_styles ();
  }
  if (editor_enabled) {
    add_editor_styles ();
  }
  const std::vector <std::string> markers = database::styles::get_markers (m_stylesheet);
  for (const auto& marker : markers) {
    const stylesv2::Style* style = database::styles::get_marker_data (m_stylesheet, marker);
    evaluate_v2 (style);
  }
}


// Evaluates the style so as to decide how it should look.
void Styles_Css::evaluate_v2 (const stylesv2::Style* style)
{
  using namespace stylesv2;
  switch (style->type) {
    case Type::starting_boundary:
    case Type::none:
    case Type::book_id:
    case Type::usfm_version:
    case Type::file_encoding:
    case Type::remark:
    case Type::running_header:
    case Type::long_toc_text:
    case Type::short_toc_text:
    case Type::book_abbrev:
      break;
    case Type::introduction_end:
      break;
    case Type::title:
    case Type::heading:
      add_v2 (style, true, true);
      break;
    case Type::paragraph:
      add_v2 (style, true, false);
      break;
    case Type::chapter:
      add_v2 (style, true, false);
      break;
    case Type::chapter_label:
    case Type::published_chapter_marker:
    case Type::alternate_chapter_number:
      break;
    case Type::verse:
      add_v2 (style, false, false);
      break;
    case Type::published_verse_marker:
    case Type::alternate_verse_marker:
      break;
    case Type::table_row:
    case Type::table_heading:
    case Type::table_cell:
      break;
    case Type::footnote_wrapper:
    case Type::endnote_wrapper:
      add_v2 (style, true, false);
      break;
    case Type::note_standard_content:
    case Type::note_content:
    case Type::note_content_with_endmarker:
    case Type::note_paragraph:
      add_v2 (style, false, false);
      break;
    case Type::crossreference_wrapper:
      add_v2 (style, true, false);
      break;
    case Type::crossreference_standard_content:
    case Type::crossreference_content:
    case Type::crossreference_content_with_endmarker:
      add_v2 (style, false, false);
      break;
    case Type::character:
      add_v2 (style, false, false);
      break;
    case Type::page_break:
      break;
    case Type::figure:
      add_v2 (style, true, false);
      break;
    case Type::word_list:
      break;
    case Type::sidebar_begin:
    case Type::sidebar_end:
    case Type::peripheral:
      break;
    case Type::stopping_boundary:
    default:
      break;
  }
}


// This adds a style to the internal CSS.
// $style: Array with the Bibledit style information.
// $paragraph: True: Is paragraph. False: Is inline text.
// $keepwithnext: Keep text in this style together with the next paragraph.
void Styles_Css::add_v2 (const stylesv2::Style* style, const bool paragraph, const bool keep_with_next)
{
  using namespace stylesv2;
  
  std::string class_name {style->marker};
  
  // The name of the class as used in a Quill-based editor.
  std::string quill_class {", ."};
  if (paragraph) {
    quill_class.append (quill_class_prefix_block);
  } else {
    quill_class.append (quill_class_prefix_inline);
  }
  quill_class.append (class_name);
  
  // Start with the class. Notice the dot.
  m_code.push_back ("." + class_name + quill_class + " {");
  
  // Font size.
  // Since it is html and not pdf for output on paper, a font size of 12pt is considered to be equal to 100%.
  if (paragraph && style->paragraph) {
    const float points {static_cast<float>(style->paragraph.value().font_size)};
    const float percents {points * 100 / 12};
    const int fontsize = filter::strings::convert_to_int (percents);
    if (fontsize != 100) {
      m_code.push_back ("font-size: " + std::to_string (fontsize) + "%;");
    }
  }
  
  // Italics, bold, underline, small caps can be either off or on for a paragraph.
  if (paragraph && style->paragraph) {
    const auto& sp = style->paragraph.value();
    if (sp.italic != stylesv2::TwoState::off)
      m_code.push_back ("font-style: italic;");
    if (sp.bold != stylesv2::TwoState::off)
      m_code.push_back ("font-weight: bold;");
    if (sp.underline != stylesv2::TwoState::off)
      m_code.push_back ("text-decoration: underline;");
    if (sp.smallcaps != stylesv2::TwoState::off)
      m_code.push_back ("font-variant: small-caps;");
  }
  
  // For inline text.
  // Italics, bold, underline, small caps can be ooitOff or ooitOn or ooitInherit or ooitToggle.
  // Not all features have been implemented.
  if (!paragraph) {
    if (style->character) {
      if (const FourState state = style->character.value().italic;
          (state == FourState::on) || (state == FourState::toggle)) {
        m_code.push_back ("font-style: italic;");
      }
      if (const FourState state = style->character.value().bold;
          (state == FourState::on) || (state == FourState::toggle)) {
        m_code.push_back ("font-weight: bold;");
      }
      if (const FourState state = style->character.value().underline;
          (state == FourState::on) || (state == FourState::toggle)) {
        m_code.push_back ("text-decoration: underline;");
      }
      if (const FourState state = style->character.value().smallcaps;
          (state == FourState::on) || (state == FourState::toggle)) {
        m_code.push_back ("font-variant: small-caps;");
      }
    }
  }
  
  // Paragraph layout properties.
  if (paragraph && style->paragraph) {
    const auto& sp = style->paragraph.value();
    
    // Text alignment options.
    std::string alignment {};
    switch (sp.text_alignment) {
      case stylesv2::TextAlignment::left: alignment.clear(); break;
      case stylesv2::TextAlignment::center: alignment = "center"; break;
      case stylesv2::TextAlignment::right: alignment = "right"; break;
      case stylesv2::TextAlignment::justify: alignment = "justify"; break;
      default: break;
    }
    if (!alignment.empty()) {
      m_code.push_back ("text-align: " + alignment + ";");
    }
    
    // Paragraph measurements; given in mm.
    // If a value is like 1.2 then return "1.2".
    // If the value is like 1.0, then return "1" leaving out the dot and what follows.
    const auto to_float_precision_01 = [](const float value) {
      std::string result = filter::strings::convert_to_string(value, 1);
      const size_t pos = result.find(".0");
      if (pos != std::string::npos)
        result.erase(pos);
      return result;
    };
    if (static_cast<bool>(sp.space_before))
      m_code.push_back ("margin-top: " + to_float_precision_01(sp.space_before) + "mm;");
    if (static_cast<bool>(sp.space_after))
      m_code.push_back ("margin-bottom: " + to_float_precision_01(sp.space_after) + "mm;");
    if (static_cast<bool>(sp.left_margin))
      m_code.push_back ("margin-left: " + to_float_precision_01(sp.left_margin) + "mm;");
    if (static_cast<bool>(sp.right_margin))
      m_code.push_back ("margin-right: " + to_float_precision_01(sp.right_margin) + "mm;");
    if (static_cast<bool>(sp.first_line_indent))
      m_code.push_back ("text-indent: " + to_float_precision_01(sp.first_line_indent) + "mm;");
    
    // Columns have not yet been implemented.
    //bool spancolumns = style->spancolumns;
    
    // Drop caps have not yet been implemented.
    //bool dropcaps = false;
    
    // Keeping text with the next paragraph.
    if (keep_with_next) {
      m_code.push_back ("page-break-inside: avoid;");
    }
  }
  
  // Superscript and colors for inline text.
  if (!paragraph) {
    if (style->character) {
      if (style->character.value().superscript == TwoState::on) {
        m_code.push_back ("font-size: x-small;");
        m_code.push_back ("vertical-align: super;");
      }
      if (style->character.value().foreground_color != "#000000") {
        m_code.push_back ("color: " + style->character.value().foreground_color + ";");
      }
      if (style->character.value().background_color != "#FFFFFF") {
        m_code.push_back ("background-color: " + style->character.value().background_color + ";");
      }
    }
  }
  
  // Close style.
  m_code.push_back ("}");
}


// Deliver the created CSS.
// $path: If given, it saves the CSS to $path.
// The function returns the CSS as a string.
std::string Styles_Css::css (std::string path)
{
  std::string css = filter::strings::implode (m_code, "\n");
  if (!path.empty()) {
    filter_url_file_put_contents (path, css);
  }
  return css;
}


// This adds the styles for the exports.
void Styles_Css::add_exports_styles ()
{
  m_code.push_back ("body { }");
  m_code.push_back ("p { margin-top: 0; margin-bottom: 0; }");
  m_code.push_back ("p.page { page-break-after: always; }");
  // Old way of having dropcaps in the html export.
  // But this has problem with negative indent like in poetry, visible in many Psalms.
  // See issue https://github.com/bibledit/cloud/issues/905
  // m_code.push_back ("span.dropcaps { float: left; font-size: 300%; line-height: 0.85em; margin-right: 0.03em; margin-bottom:-0.25em; }");
  // New way of having dropcaps.
  // Property "vertical-align: top;" could have been added, but then without it, it looks more beautify.
  m_code.push_back (".dropcaps:nth-of-type(1) { font-size: 300%; line-height: 0.85em; margin-right: 0.03em; margin-bottom:-0.25em; }");
  m_code.push_back ("a { text-decoration: none; background: none; }");
  m_code.push_back ("a:visited { color: #5a3696; }");
  m_code.push_back ("a:active { color: #faa700; }");
  m_code.push_back ("a:hover { text-decoration: underline; }");
  m_code.push_back (".breadcrumbs { font-size: normal; }");
  m_code.push_back (".navigationbar { font-size: normal; }");
  m_code.push_back (".popup { position:absolute; display:none; background-color:lightyellow; border:.1em solid; width:15em; height:auto; padding:1em; font-size:1.5em; text-indent:0em; margin 0.5em 0.5em 0.5em 0.5em; }");
  m_code.push_back ("a:hover .popup { display:block; }");
}


// This adds the styles for the Bible text editor.
void Styles_Css::add_editor_styles ()
{
}


void Styles_Css::customize (const std::string& bible)
{
  std::string cls = Filter_Css::getClass (bible);
  std::string font = fonts::logic::get_text_font (bible);
  bool uploaded_font = fonts::logic::font_exists (font);
  font = fonts::logic::get_font_path (font);
  int direction = database::config::bible::get_text_direction (bible);
  std::string css = Filter_Css::get_css (cls, font, direction);
  if (uploaded_font) css = filter::strings::replace ("../fonts/", "", css);
  m_code.push_back (css);
}



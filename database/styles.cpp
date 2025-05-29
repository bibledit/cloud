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


#include <database/styles.h>
#include <database/sqlite.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/number.h>
#include <locale/translate.h>
#include <styles/logic.h>
#include <database/logic.h>


// This is the database for the styles.
// Resilience: It is hardly written to. 
// Chances of corruption are very low.
// All default data is stored in the code in memory, not in a database on disk.


// Internal functions, variables and constants.


namespace database::styles {


constexpr const auto database_name {"styles"};
static std::string databasefolder ();
static std::string sheetfolder (const std::string& sheet);

// Styles cache and lock.
std::map<std::string,std::list<stylesv2::Style>> sheet_cache;
std::mutex cache_mutex;
// Style file suffix.
constexpr const char* style_file_suffix {"conf"};
// Style file keys.
constexpr const std::string_view delete_key {"delete"};
constexpr const std::string_view type_key {"type"};
constexpr const std::string_view name_key {"name"};
constexpr const std::string_view info_key {"info"};
constexpr const std::string_view capability_key {"capability"};
constexpr const std::string_view paragraph_fontsize_key {"paragraphfontsize"};
constexpr const std::string_view paragraph_italic_key {"paragraphitalic"};
constexpr const std::string_view paragraph_bold_key {"paragraphbold"};
constexpr const std::string_view paragraph_underline_key {"paragraphunderline"};
constexpr const std::string_view paragraph_smallcaps_key {"paragraphsmallcaps"};
constexpr const std::string_view paragraph_textalignment_key {"paragraphtextalignment"};
constexpr const std::string_view paragraph_spacebefore_key {"paragraphspacebefore"};
constexpr const std::string_view paragraph_spaceafter_key {"paragraphspaceafter"};
constexpr const std::string_view paragraph_leftmargin_key {"paragraphleftmargin"};
constexpr const std::string_view paragraph_rightmargin_key {"paragraphrightmargin"};
constexpr const std::string_view paragraph_firstlineindent_key {"paragraphfirstlineindent"};
constexpr const std::string_view character_italic_key {"characteritalic"};
constexpr const std::string_view character_bold_key {"characterbold"};
constexpr const std::string_view character_underline_key {"characterunderline"};
constexpr const std::string_view character_smallcaps_key {"charactersmallcaps"};
constexpr const std::string_view character_superscript_key {"charactersuperscript"};
constexpr const std::string_view character_foreground_color_key {"characterforegroundcolor"};
constexpr const std::string_view character_background_color_key {"characterbackgroundcolor"};
constexpr const std::string_view doc_key {"doc"};
constexpr const std::string_view category_key {"category"};


// Forward declarations of local functions.
static std::string style_file (const std::string& sheet, const std::string& marker);
static void ensure_sheet_in_cache(const std::string& sheet);
static std::string add_space(const std::string_view key);
static std::vector<std::string> get_updated_markers (const std::string& sheet);


static std::string databasefolder ()
{
  return filter_url_create_root_path ({database_logic_databases (), "styles"});
}


static std::string sheetfolder (const std::string& sheet)
{
  return filter_url_create_path ({databasefolder (), sheet});
}


void create_database ()
{
  // Create database.
  SqliteDatabase sql (database_name);
  sql.set_sql ("CREATE TABLE IF NOT EXISTS users ("
               "user text,"
               "sheet text"
               ");");
  sql.execute ();
}


// Creates a stylesheet.
// It gets a filling for styles v1.
// For styles v2 the idea is that it gets no filling yet after the create action as everything is default still.
void create_sheet (const std::string& sheet)
{
  // Folder for storing the stylesheet.
  filter_url_mkdir (sheetfolder (sheet));
}


// Returns a list with the available stylesheets.
std::vector <std::string> get_sheets ()
{
  std::vector <std::string> sheets = filter_url_scandir (databasefolder ());
  if (find (sheets.begin (), sheets.end (), stylesv2::standard_sheet ()) == sheets.end ()) {
    sheets.push_back (stylesv2::standard_sheet ());
  }
  std::sort (sheets.begin(), sheets.end());
  return sheets;
}


// Deletes a stylesheet.
void delete_sheet (const std::string& sheet)
{
  if (!sheet.empty ())
    filter_url_rmdir (sheetfolder (sheet));
}


// Grant $user write access to stylesheet $sheet.
void grant_write_access (const std::string& user, const std::string& sheet)
{
  SqliteDatabase sql (database_name);
  sql.add ("INSERT INTO users VALUES (");
  sql.add (user);
  sql.add (",");
  sql.add (sheet);
  sql.add (");");
  sql.execute ();
}


// Revoke a $user's write access to stylesheet $sheet.
// If the $user is empty, then revoke write access of anybody to that $sheet.
void revoke_write_access (const std::string& user, const std::string& sheet)
{
  SqliteDatabase sql (database_name);
  sql.add ("DELETE FROM users WHERE");
  if (!user.empty ()) {
    sql.add ("user =");
    sql.add (user);
    sql.add ("AND");
  }
  sql.add ("sheet =");
  sql.add (sheet);
  sql.add (";");
  sql.execute ();
}


// Returns true or false depending on whether $user has write access to $sheet.
bool has_write_access (const std::string& user, const std::string& sheet)
{
  SqliteDatabase sql (database_name);
  sql.add ("SELECT rowid FROM users WHERE user =");
  sql.add (user);
  sql.add ("AND sheet =");
  sql.add (sheet);
  sql.add (";");
  std::map <std::string, std::vector <std::string> > result = sql.query ();
  return !result["rowid"].empty ();
}


static std::string style_file (const std::string& sheet, const std::string& marker)
{
  return filter_url_create_path ({styles::sheetfolder(sheet), marker + std::string(".") + style_file_suffix});
}


// Ensure sheet cache has been filled.
static void ensure_sheet_in_cache(const std::string& sheet)
{
  // Check whether the requested stylesheet is already in the cache.
  // If so: Ready.
  {
    const auto iter = sheet_cache.find(sheet);
    if (iter != sheet_cache.cend())
      return;
  }
  // The sheet is not in the cache at this point.
  
  // Create enmpty cache for the sheet.
  sheet_cache[sheet].clear();
  std::list<stylesv2::Style>& cache {sheet_cache.at(sheet)};
  
  // Copy the hard-coded default stylesheet to the cache.
  for (const auto& style : stylesv2::styles)
    cache.push_back(style);
  
  // Update the cache with any updated or deleted or added styles.
  const std::vector<std::string> updated_markers {get_updated_markers (sheet)};
  for (const auto& marker : updated_markers) {
    // Since there's an update, remove the existing style.
    auto iter = std::find(cache.cbegin(), cache.cend(), marker);
    if (iter != cache.cend())
      cache.erase(iter);
    // Get the updated style. If one is given, add it to the cache.
    std::optional<stylesv2::Style> style = load_style(sheet, marker);
    if (style)
      cache.push_back(std::move(style.value()));
  }
}


// Add a space to the key and return it.
static std::string add_space(const std::string_view key)
{
  std::string result (key);
  result.append(" ");
  return result;
}


// Function to get the base style to use for comparison.
static stylesv2::Style get_base_style (const std::string& marker) {
  // Search the default styles for the marker.
  // If the style is found among the default ones, return that as the base style.
  const auto iter = std::find(stylesv2::styles.cbegin(), stylesv2::styles.cend(), marker);
  if (iter != stylesv2::styles.cend())
    return *iter;
  // The marker is not among the default ones, return a default constructed style object.
  return stylesv2::Style();
}


// Adds a marker to the stylesheet.
void add_marker (const std::string& sheet, const std::string& marker, const std::string& base)
{
  // Get the base style and set the marker.
  auto style {get_base_style(base)};
  style.marker = marker;
  // If some properties are not given, then set some now.
  // This is to ensure something will be written to file.
  if (style.name.empty())
    style.name = "name";
  if (style.info.empty())
    style.info = "info";
  // Write it to file.
  save_style(sheet, style);
}


// Deletes a marker from a stylesheet.
void delete_marker (const std::string& sheet, const std::string& marker)
{
  // Check if the marker to delete is among the hard-coded default styles.
  const auto iter = std::find(stylesv2::styles.cbegin(), stylesv2::styles.cend(), marker);
  // If the marker is among the hard-coded ones,
  // deleting a marker means to write a style to file as deleted.
  if (iter != stylesv2::styles.cend()) {
    const std::string filename = style_file (sheet, marker);
    filter_url_file_put_contents (filename, add_space(delete_key));
  }
  // If the marker is not among the hard-coded ones,
  // deleting a marker is equal to resetting it.
  if (iter == stylesv2::styles.cend()) {
    reset_marker(sheet, marker);
  }
  // Clear the sheet cache.
  std::unique_lock lock (cache_mutex);
  sheet_cache.clear();
}


// Remove all changes to a marker and reset it to default.
void reset_marker (const std::string& sheet, const std::string& marker)
{
  // Remove the file for this marker: This means the marker does not have changes compared to the default style.
  const std::string filename = style_file (sheet, marker);
  filter_url_unlink(filename);
  // Clear the sheet cache.
  std::unique_lock lock (cache_mutex);
  sheet_cache.clear();
}


const std::list<stylesv2::Style>& get_styles(const std::string& sheet)
{
  // If the standard stylesheet is requested, return a reference to the standard hard-coded stylesheet.
  if (sheet == stylesv2::standard_sheet())
    return stylesv2::styles;

  // Make sure the stylesheet has been cached.
  ensure_sheet_in_cache (sheet);

  // Return a reference to tne sheet from the cache.
  return sheet_cache.at(sheet);
}


// Returns a list with all the markers of the styles in the stylesheet.
std::vector <std::string> get_markers (const std::string& sheet)
{
  std::vector <std::string> markers;
  const std::list<stylesv2::Style>& styles {get_styles(sheet)};
  std::transform(styles.begin(), styles.end(), std::back_inserter(markers), [](const stylesv2::Style& style) {
    return style.marker;
  });
  return markers;
}


// Returns a map with all the markers and the names of the styles in the stylesheet.
std::map <std::string, std::string> get_markers_and_names (const std::string& sheet)
{
  std::map <std::string, std::string> markers_names;
  const std::list<stylesv2::Style>& styles {get_styles(sheet)};
  std::transform(styles.begin(), styles.end(), std::inserter(markers_names, markers_names.end()), [](const stylesv2::Style& style) {
    return std::make_pair(style.marker, style.name);
  });
  return markers_names;
}


// Returns a pointer to a style object with all data belonging to a marker.
const stylesv2::Style* get_marker_data (const std::string& sheet, const std::string& marker)
{
  // If the requested marker is among the styles in the requested sheet, return that.
  const std::list<stylesv2::Style>& styles {get_styles(sheet)};
  const auto iter = std::find(styles.cbegin(), styles.cend(), marker);
  if (iter != styles.cend())
    return std::addressof(*iter);
  // Style not found.
  return nullptr;
}


// Gets the markers from a given stylesheet that have updates compared to the standard stylesheet.
std::vector<std::string> get_updated_markers (const std::string& sheet)
{
  const std::vector <std::string> files {filter_url_scandir (styles::sheetfolder (sheet))};
  std::vector <std::string> markers{};
  for (const auto& file : files) {
    if (filter_url_get_extension (file) == style_file_suffix) {
      std::string marker = file.substr(0, file.length() - strlen(style_file_suffix) - 1);
      markers.push_back(std::move(marker));
    }
  }
  return markers;
}


// Save a style to file for those parts that differ from the base style.
void save_style(const std::string& sheet, const stylesv2::Style& style)
{
  // The base style to use for finding differences between the style to save and the base style.
  const auto base_style = get_base_style(style.marker);
  
  // The lines container with the differences to write to file.
  std::vector<std::string> lines {};
  
  // Handle style type.
  if (style.type != base_style.type)
    lines.push_back (add_space(type_key) + type_enum_to_value(style.type));
  
  // Handle style name.
  if (style.name != base_style.name)
    lines.push_back (add_space(name_key) + style.name);
  
  // Handle style info.
  if (style.info != base_style.info)
    lines.push_back (add_space(info_key) + style.info);
  
  // Handle saving paragraph properties.
  // Save only if properties differ.
  // Note: If the base styles does not have a paragraph set, comparison of properties would lead to a crash.
  //       A special mechanism using a "logical or" works around this.
  if (style.paragraph) {
    const auto save = [&base_style]() {
      if (!base_style.paragraph)
        return true;
      return false;
    };
    const auto& paragraph = style.paragraph.value();
    if (save() || (paragraph.font_size) != base_style.paragraph.value().font_size)
      lines.push_back (add_space(paragraph_fontsize_key) + std::to_string(paragraph.font_size));
    if (save() || (paragraph.italic) != base_style.paragraph.value().italic)
      lines.push_back (add_space(paragraph_italic_key) + twostate_enum_to_value(paragraph.italic));
    if (save() || (paragraph.bold) != base_style.paragraph.value().bold)
      lines.push_back (add_space(paragraph_bold_key) + twostate_enum_to_value(paragraph.bold));
    if (save() || (paragraph.underline) != base_style.paragraph.value().underline)
      lines.push_back (add_space(paragraph_underline_key) + twostate_enum_to_value(paragraph.underline));
    if (save() || (paragraph.smallcaps) != base_style.paragraph.value().smallcaps)
      lines.push_back (add_space(paragraph_smallcaps_key) + twostate_enum_to_value(paragraph.smallcaps));
    if (save() || (paragraph.text_alignment) != base_style.paragraph.value().text_alignment)
      lines.push_back (add_space(paragraph_textalignment_key) + textalignment_enum_to_value(paragraph.text_alignment));
    if (save() || !filter::number::float_equal(paragraph.space_before, base_style.paragraph.value().space_before))
      lines.push_back (add_space(paragraph_spacebefore_key) + std::to_string(paragraph.space_before));
    if (save() || !filter::number::float_equal(paragraph.space_after, base_style.paragraph.value().space_after))
      lines.push_back (add_space(paragraph_spaceafter_key) + std::to_string(paragraph.space_after));
    if (save() || !filter::number::float_equal(paragraph.left_margin, base_style.paragraph.value().left_margin))
      lines.push_back (add_space(paragraph_leftmargin_key) + std::to_string(paragraph.left_margin));
    if (save() || !filter::number::float_equal(paragraph.right_margin, base_style.paragraph.value().right_margin))
      lines.push_back (add_space(paragraph_rightmargin_key) + std::to_string(paragraph.right_margin));
    if (save() || !filter::number::float_equal(paragraph.first_line_indent, base_style.paragraph.value().first_line_indent))
      lines.push_back (add_space(paragraph_firstlineindent_key) + std::to_string(paragraph.first_line_indent));
  }
  
  // Handle saving character properties.
  // Save only if properties differ.
  // Note: If the base styles does not have a character set, comparison of properties would lead to a crash.
  //       A special mechanism using a "logical or" works around this.
  if (style.character) {
    const auto save = [&base_style]() {
      if (!base_style.character)
        return true;
      return false;
    };
    const auto& character = style.character.value();
    if (save() || character.italic != base_style.character.value().italic)
      lines.push_back (add_space(character_italic_key) + fourstate_enum_to_value(character.italic));
    if (save() || character.bold != base_style.character.value().bold)
      lines.push_back (add_space(character_bold_key) + fourstate_enum_to_value(character.bold));
    if (save() || character.underline != base_style.character.value().underline)
      lines.push_back (add_space(character_underline_key) + fourstate_enum_to_value(character.underline));
    if (save() || character.smallcaps != base_style.character.value().smallcaps)
      lines.push_back (add_space(character_smallcaps_key) + fourstate_enum_to_value(character.smallcaps));
    if (save() || character.superscript != base_style.character.value().superscript)
      lines.push_back (add_space(character_superscript_key) + twostate_enum_to_value(character.superscript));
    if (save() || character.foreground_color != base_style.character.value().foreground_color)
      lines.push_back (add_space(character_foreground_color_key) + character.foreground_color);
    if (save() || character.background_color != base_style.character.value().background_color)
      lines.push_back (add_space(character_background_color_key) + character.background_color);
  }
  
  // Iterate over the parameters in the style.
  // If the parameter in the style is not found in the base style,
  // or the value is different from the value in the base style,
  // then set this parameter in the data to save.
  for (const auto& [property, value] : style.properties) {
    std::string base_value{};
    std::string style_value{};
    const auto get_string_value = [] (const stylesv2::Parameter& parameter, std::string& output) {
      if (output.empty()) {
        using namespace stylesv2;
        std::stringstream ss{};
        ss << parameter;
        output = std::move(ss).str();
      }
    };
    bool write_parameter {false};
    if (!base_style.properties.count(property)) {
      write_parameter = true;
    } else {
      get_string_value (base_style.properties.at(property), base_value);
      get_string_value (value, style_value);
      write_parameter = (style_value != base_value);
    }
    if (write_parameter) {
      std::string line {add_space(capability_key)};
      line.append(add_space(property_enum_to_value(property)));
      get_string_value (value, style_value);
      line.append(style_value);
      lines.push_back(std::move(line));
    }
  }

  // Handle documentation string.
  if (style.doc != base_style.doc)
    lines.push_back (add_space(doc_key) + style.doc);
  
  // Handle category string.
  if (style.category != base_style.category) {
    std::stringstream ss {};
    ss << add_space(category_key);
    ss << style.category;
    lines.push_back (std::move(ss).str());
  }
  
  // If there's no differences between the style to save and the base style,
  // then remove the style file.
  // If there's differences, then save the data to the style file.
  const std::string filename = style_file (sheet, style.marker);
  if (lines.empty())
    filter_url_unlink(filename);
  else
    filter_url_file_put_contents (filename, filter::strings::implode (lines, "\n"));

  // A style was saved: Clear the cache.
  std::unique_lock lock (cache_mutex);
  sheet_cache.clear();
}


// Load a style from file, combining the default style with the updated properties from file.
// Loading the style may also result in no style to be returned.
std::optional<stylesv2::Style> load_style(const std::string& sheet, const std::string& marker)
{
  // The style struct to start off with.
  stylesv2::Style style = get_base_style(marker);
  
  // Set the marker correct in the style if not yet done.
  style.marker = marker;

  // Load the style data from file.
  const std::string filename = style_file (sheet, marker);
  const std::string contents = filter_url_file_get_contents (filename);
  std::vector<std::string> lines = filter::strings::explode (contents, "\n");

  // Iterate over the loaded lines and extract the changes from it as compared to the base style.
  size_t pos{};
  for (auto& line : lines) {
    // If the style was deleted return a null optional, and be done with it.
    pos = line.find(add_space(delete_key));
    if (pos == 0)
      return std::nullopt;
    // Check on or set an updated style type.
    pos = line.find(add_space(type_key));
    if (pos == 0)
      style.type = stylesv2::type_value_to_enum(line.substr(type_key.length()+1));
    // Check on or set an updated style name.
    pos = line.find(add_space(name_key));
    if (pos == 0)
      style.name = line.substr(name_key.length()+1);
    // Check on or set updated style info.
    pos = line.find(add_space(info_key));
    if (pos == 0)
      style.info = line.substr(info_key.length()+1);
    // Function to ensure that the paragraph field is not a nullopt.
    const auto ensure_paragraph_is_set = [](stylesv2::Style& style) {
      if (!style.paragraph)
        style.paragraph = stylesv2::Paragraph();
    };
    // Check / set paragraph font size.
    pos = line.find(add_space(paragraph_fontsize_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      try {
        style.paragraph.value().font_size = std::stoi(line.substr(paragraph_fontsize_key.length()+1));
      } catch (...) {}
    }
    // Check / set paragraph italic.
    pos = line.find(add_space(paragraph_italic_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().italic = stylesv2::twostate_value_to_enum(line.substr(paragraph_italic_key.length()+1));
    }
    // Check / set paragraph bold.
    pos = line.find(add_space(paragraph_bold_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().bold = stylesv2::twostate_value_to_enum(line.substr(paragraph_bold_key.length()+1));
    }
    // Check / set paragraph underline.
    pos = line.find(add_space(paragraph_underline_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().underline = stylesv2::twostate_value_to_enum(line.substr(paragraph_underline_key.length()+1));
    }
    // Check / set paragraph smallcaps.
    pos = line.find(add_space(paragraph_smallcaps_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().smallcaps = stylesv2::twostate_value_to_enum(line.substr(paragraph_smallcaps_key.length()+1));
    }
    // Check / set paragraph text alignment.
    pos = line.find(add_space(paragraph_textalignment_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().text_alignment = stylesv2::textalignment_value_to_enum(line.substr(paragraph_textalignment_key.length()+1));
    }

    // Check / set paragraph space before.
    pos = line.find(add_space(paragraph_spacebefore_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().space_before = filter::strings::convert_to_float(line.substr(paragraph_spacebefore_key.length()+1));
    }

    // Check / set paragraph space after.
    pos = line.find(add_space(paragraph_spaceafter_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().space_after = filter::strings::convert_to_float(line.substr(paragraph_spaceafter_key.length()+1));
    }

    // Check / set paragraph left margin.
    pos = line.find(add_space(paragraph_leftmargin_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().left_margin = filter::strings::convert_to_float(line.substr(paragraph_leftmargin_key.length()+1));
    }

    // Check / set paragraph right margin.
    pos = line.find(add_space(paragraph_rightmargin_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().right_margin = filter::strings::convert_to_float(line.substr(paragraph_rightmargin_key.length()+1));
    }

    // Check / set paragraph first line indent.
    pos = line.find(add_space(paragraph_firstlineindent_key));
    if (pos == 0) {
      ensure_paragraph_is_set(style);
      style.paragraph.value().first_line_indent = filter::strings::convert_to_float(line.substr(paragraph_firstlineindent_key.length()+1));
    }
    
    // Function to ensure that the character field is not a nullopt.
    const auto ensure_character_is_set = [](stylesv2::Style& style) {
      if (!style.character)
        style.character = stylesv2::Character();
    };
    // Check / set character italic.
    pos = line.find(add_space(character_italic_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().italic = stylesv2::fourstate_value_to_enum(line.substr(character_italic_key.length()+1));
    }
    // Check / set character bold.
    pos = line.find(add_space(character_bold_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().bold = stylesv2::fourstate_value_to_enum(line.substr(character_bold_key.length()+1));
    }
    // Check / set character underline.
    pos = line.find(add_space(character_underline_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().underline = stylesv2::fourstate_value_to_enum(line.substr(character_underline_key.length()+1));
    }
    // Check / set character smallcaps.
    pos = line.find(add_space(character_smallcaps_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().smallcaps = stylesv2::fourstate_value_to_enum(line.substr(character_smallcaps_key.length()+1));
    }
    // Check / set character superscript.
    pos = line.find(add_space(character_superscript_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().superscript = stylesv2::twostate_value_to_enum(line.substr(character_superscript_key.length()+1));
    }
    // Check / set character foreground color.
    pos = line.find(add_space(character_foreground_color_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().foreground_color = line.substr(character_foreground_color_key.length()+1);
    }
    // Check / set character background color.
    pos = line.find(add_space(character_background_color_key));
    if (pos == 0) {
      ensure_character_is_set(style);
      style.character.value().background_color = line.substr(character_background_color_key.length()+1);
    }
    // Check on or set a capability as compared to the base style.
    // Such a line looks like this:
    //   capability starts_new_page 0
    // Or:
    //   capability starts_new_page 1
    pos = line.find(add_space(capability_key));
    if (pos == 0) {
      line.erase(0, capability_key.length()+1);
      pos = line.find(" ");
      if (pos == std::string::npos)
        continue;
      const stylesv2::Property property = stylesv2::property_value_to_enum(line.substr(0, pos));
      line.erase(0, pos+1);
      const stylesv2::Variant variant {stylesv2::property_to_variant(property)};
      switch (variant) {
        case stylesv2::Variant::boolean:
          style.properties[property] = filter::strings::convert_to_bool(line);
          break;
        case stylesv2::Variant::number:
          style.properties[property] = filter::strings::convert_to_int(line);
          break;
        case stylesv2::Variant::text:
          style.properties[property] = line;
          break;
        case stylesv2::Variant::none:
          style.properties[property] = std::monostate();
          break;
        default:
          break;
      }
    }
    
    // Check on or set an updated style documentation.
    pos = line.find(add_space(doc_key));
    if (pos == 0)
      style.doc = line.substr(doc_key.length() + 1);
    
    // Check on or set an updated style category.
    pos = line.find(add_space(category_key));
    if (pos == 0)
      style.category = stylesv2::category_value_to_enum (line.substr(category_key.length() + 1));
  }

  // Ready.
  return style;
}


} // Namespace

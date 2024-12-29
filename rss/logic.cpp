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


#include <rss/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/diff.h>
#include <filter/date.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <locale/translate.h>
#include <tasks/logic.h>
#include <rss/feed.h>


#ifdef HAVE_CLOUD


void rss_logic_feed_on_off ()
{
  // See whether there's Bibles that send their changes to the RSS feed.
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  bool rss_enabled = false;
  for (auto bible : bibles) {
    if (database::config::bible::get_send_changes_to_rss (bible)) {
      rss_enabled = true;
    }
  }
  if (rss_enabled) {
    // The RSS feed is enabled: Ensure the feed exists right now.
    rss_logic_update_xml ({}, {}, {});
  } else {
    // The RSS feed is disabled: Remove the last trace of it entirely.
    std::string path = rss_logic_xml_path ();
    if (file_or_dir_exists (path)) {
      filter_url_unlink (path);
    }
  }
}


std::string rss_logic_new_line ()
{
  return "rss_new_line";
}


void rss_logic_schedule_update (std::string user, std::string bible, int book, int chapter,
                                std::string oldusfm, std::string newusfm)
{
  // If the RSS feed system is off, bail out.
  if (!database::config::bible::get_send_changes_to_rss (bible)) return;
  
  // Mould the USFM into one line.
  oldusfm = filter::strings::replace ("\n", rss_logic_new_line (), oldusfm);
  newusfm = filter::strings::replace ("\n", rss_logic_new_line (), newusfm);
  
  // Schedule it.
  std::vector <std::string> parameters;
  parameters.push_back (user);
  parameters.push_back (bible);
  parameters.push_back (std::to_string (book));
  parameters.push_back (std::to_string (chapter));
  parameters.push_back (oldusfm);
  parameters.push_back (newusfm);
  tasks_logic_queue (task::rss_feed_update_chapter, parameters);
}


void rss_logic_execute_update (std::string user, std::string bible, int book, int chapter,
                               std::string oldusfm, std::string newusfm)
{
  // Bail out if there's no changes.
  if (oldusfm == newusfm) return;

  // Mould the USFM back into its original format with new lines.
  oldusfm = filter::strings::replace (rss_logic_new_line (), "\n", oldusfm);
  newusfm = filter::strings::replace (rss_logic_new_line (), "\n", newusfm);

  // Whether to include the author in the RSS feed.
  // For security reasons this can be set off.
  // This way the author does not get exposed,
  // and no information is revealed that facilitates a brute-force login attack.
  bool include_author = database::config::general::get_author_in_rss_feed ();
  if (!include_author) user.clear ();
  
  // Storage for the feed update.
  std::vector <std::string> titles, authors, descriptions;
  
  const std::string stylesheet = database::config::bible::get_export_stylesheet (bible);

  // Get the combined verse numbers in old and new USFM.
  std::vector <int> old_verse_numbers = filter::usfm::get_verse_numbers (oldusfm);
  std::vector <int> new_verse_numbers = filter::usfm::get_verse_numbers (newusfm);
  std::vector <int> verses = old_verse_numbers;
  verses.insert (verses.end (), new_verse_numbers.begin (), new_verse_numbers.end ());
  verses = filter::strings::array_unique (verses);
  sort (verses.begin(), verses.end());

  for (auto verse : verses) {
    std::string old_verse_usfm = filter::usfm::get_verse_text (oldusfm, verse);
    std::string new_verse_usfm = filter::usfm::get_verse_text (newusfm, verse);
    if (old_verse_usfm != new_verse_usfm) {
      Filter_Text filter_text_old = Filter_Text (bible);
      Filter_Text filter_text_new = Filter_Text (bible);
      filter_text_old.text_text = new Text_Text ();
      filter_text_new.text_text = new Text_Text ();
      filter_text_old.add_usfm_code (old_verse_usfm);
      filter_text_new.add_usfm_code (new_verse_usfm);
      filter_text_old.run (stylesheet);
      filter_text_new.run (stylesheet);
      std::string old_text = filter_text_old.text_text->get ();
      std::string new_text = filter_text_new.text_text->get ();
      if (old_text != new_text) {
        std::string modification = filter_diff_diff (old_text, new_text);
        titles.push_back (filter_passage_display (book, chapter, std::to_string (verse)));
        authors.push_back (user);
        descriptions.push_back ("<div>" + modification + "</div>");
      }
    }
  }
  
  // Update the feed.
  rss_logic_update_xml (titles, authors, descriptions);
}


std::string rss_logic_xml_path ()
{
  return filter_url_create_root_path ({"rss", "feed.xml"});
}


void rss_logic_update_xml (std::vector <std::string> titles, std::vector <std::string> authors, std::vector <std::string> descriptions)
{
  int seconds = filter::date::seconds_since_epoch ();
  std::string rfc822time = filter::date::rfc822 (seconds);
  std::string guid = std::to_string (seconds);
  bool document_updated = false;
  pugi::xml_document document;
  std::string path = rss_logic_xml_path ();
  document.load_file (path.c_str());
  pugi::xml_node rss_node = document.first_child ();
  if (strcmp (rss_node.name (), "rss") != 0) {
    // RSS node.
    rss_node = document.append_child ("rss");
    rss_node.append_attribute ("version") = "2.0";
    rss_node.append_attribute ("xmlns:atom") = "http://www.w3.org/2005/Atom";
    pugi::xml_node channel = rss_node.append_child ("channel");
    // Title.
    pugi::xml_node node = channel.append_child ("title");
    node.text () = translate ("Bibledit").c_str();
    // Link to website.
    node = channel.append_child ("link");
    node.text () = database::config::general::get_site_url().c_str();
    // Description.
    node = channel.append_child ("description");
    node.text () = translate ("Recent changes in the Bible texts").c_str ();
    // Feed's URL.
    node = channel.append_child ("atom:link");
    std::string link = database::config::general::get_site_url() + rss_feed_url ();
    node.append_attribute ("href") = link.c_str();
    node.append_attribute ("rel") = "self";
    node.append_attribute ("type") = "application/rss+xml";
    // Updated.
    document_updated = true;
  }
  pugi::xml_node channel = rss_node.child ("channel");
  for (size_t i = 0; i < titles.size(); i++) {
    pugi::xml_node item = channel.append_child ("item");
    std::string guid2 = guid + std::to_string (i);
    pugi::xml_node guid_node = item.append_child ("guid");
    guid_node.append_attribute ("isPermaLink") = "false";
    guid_node.text () = guid2.c_str();
    // Many readers do not display the 'author' field.
    // To cater for those readers, the author gets included with the title.
    if (!authors[i].empty ()) titles [i].append (" " + authors[i]);
    item.append_child ("title").text () = titles[i].c_str();
    if (!authors[i].empty ()) authors[i].append ("@site.org (" + authors[i] + ")");
    item.append_child ("author").text () = authors[i].c_str();
    item.append_child ("pubDate").text () = rfc822time.c_str();
    item.append_child ("description").text () = descriptions [i].c_str();
    document_updated = true;
  }
  int rss_size = 100;
  int count = static_cast<int>(std::distance (channel.children ().begin (), channel.children ().end ()));
  count -= 3;
  count -= rss_size;
  while (count > 0) {
    pugi::xml_node node = channel.child ("item");
    channel.remove_child (node);
    document_updated = true;
    count--;
  }
  if (document_updated) {
    pugi::xml_node decl = document.prepend_child (pugi::node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    std::stringstream output;
    document.print (output, " ", pugi::format_default);
    filter_url_file_put_contents (path, output.str ());
  }
}


#endif

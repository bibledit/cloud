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


#include <rss/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/diff.h>
#include <filter/date.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <pugixml/pugixml.hpp>
#include <locale/translate.h>
#include <tasks/logic.h>
#include <rss/feed.h>


using namespace pugi;


#ifdef HAVE_CLOUD


string rss_logic_new_line ()
{
  return "rss_new_line";
}


void rss_logic_schedule_update (string user, string bible, int book, int chapter,
                                string oldusfm, string newusfm)
{
  // If the RSS feed system is off, bail out.
  int size = Database_Config_General::getMaxRssFeedItems ();
  if (size == 0) return;
  
  // Mould the USFM into one line.
  oldusfm = filter_string_str_replace ("\n", rss_logic_new_line (), oldusfm);
  newusfm = filter_string_str_replace ("\n", rss_logic_new_line (), newusfm);
  
  // Schedule it.
  vector <string> parameters;
  parameters.push_back (user);
  parameters.push_back (bible);
  parameters.push_back (convert_to_string (book));
  parameters.push_back (convert_to_string (chapter));
  parameters.push_back (oldusfm);
  parameters.push_back (newusfm);
  tasks_logic_queue (RSSFEEDUPDATECHAPTER, parameters);
}


void rss_logic_execute_update (string user, string bible, int book, int chapter,
                               string oldusfm, string newusfm)
{
  // Bail out if there's no changes.
  if (oldusfm == newusfm) return;

  // Mould the USFM back into its original format with new lines.
  oldusfm = filter_string_str_replace (rss_logic_new_line (), "\n", oldusfm);
  newusfm = filter_string_str_replace (rss_logic_new_line (), "\n", newusfm);
  
  // Storage for the feed update.
  vector <string> titles, authors, descriptions;
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);

  // Get the combined verse numbers in old and new USFM.
  vector <int> old_verse_numbers = usfm_get_verse_numbers (oldusfm);
  vector <int> new_verse_numbers = usfm_get_verse_numbers (newusfm);
  vector <int> verses = old_verse_numbers;
  verses.insert (verses.end (), new_verse_numbers.begin (), new_verse_numbers.end ());
  verses = array_unique (verses);
  sort (verses.begin(), verses.end());

  for (auto verse : verses) {
    string old_verse_usfm = usfm_get_verse_text (oldusfm, verse);
    string new_verse_usfm = usfm_get_verse_text (newusfm, verse);
    if (old_verse_usfm != new_verse_usfm) {
      Filter_Text filter_text_old = Filter_Text (bible);
      Filter_Text filter_text_new = Filter_Text (bible);
      filter_text_old.text_text = new Text_Text ();
      filter_text_new.text_text = new Text_Text ();
      filter_text_old.addUsfmCode (old_verse_usfm);
      filter_text_new.addUsfmCode (new_verse_usfm);
      filter_text_old.run (stylesheet);
      filter_text_new.run (stylesheet);
      string old_text = filter_text_old.text_text->get ();
      string new_text = filter_text_new.text_text->get ();
      if (old_text != new_text) {
        string modification = filter_diff_diff (old_text, new_text);
        titles.push_back (filter_passage_display (book, chapter, convert_to_string (verse)));
        authors.push_back (user);
        descriptions.push_back ("<div>" + modification + "</div>");
      }
    }
  }
  
  // Update the feed.
  rss_logic_update_xml (titles, authors, descriptions);
}


string rss_logic_xml_path ()
{
  return filter_url_create_root_path ("rss", "feed.xml");
}


void rss_logic_update_xml (vector <string> titles, vector <string> authors, vector <string> descriptions)
{
  string path = rss_logic_xml_path ();
  int size = Database_Config_General::getMaxRssFeedItems ();
  if (size == 0) {
    if (file_or_dir_exists (path)) filter_url_unlink (path);
    return;
  }
  int seconds = filter_date_seconds_since_epoch ();
  string rfc822time = filter_date_rfc822 (seconds);
  string guid = convert_to_string (seconds);
  bool document_updated = false;
  xml_document document;
  document.load_file (path.c_str());
  xml_node rss_node = document.first_child ();
  if (strcmp (rss_node.name (), "rss") != 0) {
    // RSS node.
    rss_node = document.append_child ("rss");
    rss_node.append_attribute ("version") = "2.0";
    rss_node.append_attribute ("xmlns:atom") = "http://www.w3.org/2005/Atom";
    xml_node channel = rss_node.append_child ("channel");
    // Title.
    xml_node node = channel.append_child ("title");
    node.text () = "Bibledit";
    // Link to website.
    node = channel.append_child ("link");
    node.text () = Database_Config_General::getSiteURL().c_str();
    // Description.
    node = channel.append_child ("description");
    node.text () = translate ("Recent changes in the Bible text").c_str ();
    // Feed's URL.
    node = channel.append_child ("atom:link");
    string link = Database_Config_General::getSiteURL() + rss_feed_url ();
    node.append_attribute ("href") = link.c_str();
    node.append_attribute ("rel") = "self";
    node.append_attribute ("type") = "application/rss+xml";
    // Updated.
    document_updated = true;
  }
  xml_node channel = rss_node.child ("channel");
  for (size_t i = 0; i < titles.size(); i++) {
    xml_node item = channel.append_child ("item");
    string guid2 = guid + convert_to_string (i);
    xml_node guid_node = item.append_child ("guid");
    guid_node.append_attribute ("isPermaLink") = "false";
    guid_node.text () = guid2.c_str();
    item.append_child ("title").text () = titles [i].c_str();
    string author = authors[i] + "@site.org (" + authors[i] + ")";
    item.append_child ("author").text () = author.c_str();
    item.append_child ("pubDate").text () = rfc822time.c_str();
    item.append_child ("description").text () = descriptions [i].c_str();
    document_updated = true;
  }
  int count = distance (channel.children ().begin (), channel.children ().end ());
  count -= 3;
  count -= size;
  while (count > 0) {
    xml_node node = channel.child ("item");
    channel.remove_child (node);
    document_updated = true;
    count--;
  }
  if (document_updated) {
    xml_node decl = document.prepend_child (node_declaration);
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    stringstream output;
    document.print (output, " ", format_default);
    filter_url_file_put_contents (path, output.str ());
  }
}


#endif

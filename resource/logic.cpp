/*
 Copyright (©) 2003-2022 Teus Benschop.
 
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


#include <resource/logic.h>
#include <webserver/request.h>
#include <access/bible.h>
#include <database/usfmresources.h>
#include <database/imageresources.h>
#include <database/mappings.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <database/cache.h>
#include <database/books.h>
#include <database/versifications.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/shell.h>
#include <filter/roles.h>
#include <filter/diff.h>
#include <resource/external.h>
#include <locale/translate.h>
#include <client/logic.h>
#include <lexicon/logic.h>
#include <sword/logic.h>
#include <demo/logic.h>
#include <sync/resources.h>
#include <tasks/logic.h>
#include <related/logic.h>
#include <developer/logic.h>
#include <database/logic.h>


/*

Stages to retrieve resource content and serve it.
 
 * fetch http: Fetch raw page from the internet through http.
 * fetch sword: Fetch a verse from a SWORD module.
 * fetch cloud: Fetch content from the dedicated or demo Bibledit Cloud.
 * check cache: Fetch http or sword content from the cache.
 * store cache: Store http or sword content in the cache.
 * extract: Extract the desired snipped from the larger page content.
 * postprocess: Postprocessing of the content to fine-tune it.
 * display: Display content to the user.
 * serve: Serve content to the client.
 
 A Bibledit client uses the following sequence to display a resource to the user:
 * check cache or fetch cloud -> store cache -> postprocess -> display.
 
 Bibledit Cloud uses the following sequence to display a resource to the user:
 * fetch http or fetch sword or check cache -> store cache -> extract -> postprocess - display.

 A Bibledit client uses the following sequence to install a resource:
 * check cache -> fetch cloud -> store cache.
 
 Bibledit Cloud uses the following sequence to serve a resource to a client:
 * fetch http or fetch sword or check cache -> store cache -> extract -> serve.
 
 In earlier versions of Bibledit,
 the client would download external resources straight from the Internet.
 To also be able to download content via https (secure http),
 the client needs the cURL library.
 And libcurl has not yet been compiled for all operating systems Bibledit runs on.
 In the current version of Bibledit, it works differently.
 It now requests all external content from Bibledit Cloud.
 An important advantage of this method is that this minimizes data transfer on the Bibledit Client.
 The client no longer fetches the full web page. Bibledit Cloud does that.
 And the Cloud then extracts the small relevant snipped from the web page,
 and serves that to the Client.
 
*/


vector <string> resource_logic_get_names (void * webserver_request, bool bibles_only)
{
  vector <string> names;
  
  // Bibles the user has read access to.
  vector <string> bibles = AccessBible::Bibles (webserver_request);
  names.insert (names.end(), bibles.begin (), bibles.end());
  
  // USFM resources.
  Database_UsfmResources database_usfmresources;
  vector <string> usfm_resources = database_usfmresources.getResources ();
  names.insert (names.end(), usfm_resources.begin(), usfm_resources.end());
  
  // External resources.
  vector <string> external_resources = resource_external_names ();
  names.insert (names.end (), external_resources.begin(), external_resources.end());
  
  // Image resources.
  if (!bibles_only) {
    Database_ImageResources database_imageresources;
    vector <string> image_resources = database_imageresources.names ();
    names.insert (names.end (), image_resources.begin(), image_resources.end());
  }
  
  // Lexicon resources.
  if (!bibles_only) {
    vector <string> lexicon_resources = lexicon_logic_resource_names ();
    names.insert (names.end (), lexicon_resources.begin(), lexicon_resources.end());
  }
  
  // SWORD resources
  vector <string> sword_resources = sword_logic_get_available ();
  names.insert (names.end (), sword_resources.begin(), sword_resources.end());
  
  sort (names.begin(), names.end());
  
  return names;
}


string resource_logic_get_html (void * webserver_request,
                                string resource, int book, int chapter, int verse,
                                bool add_verse_numbers)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  string html;

  // Determine the type of the resource.
  bool isBible = resource_logic_is_bible (resource);
  bool isUsfm = resource_logic_is_usfm (resource);
  bool isExternal = resource_logic_is_external (resource);
  bool isImage = resource_logic_is_image (resource);
  bool isLexicon = resource_logic_is_lexicon (resource);
  bool isSword = resource_logic_is_sword (resource);
  bool isBibleGateway = resource_logic_is_biblegateway (resource);
  bool isStudyLight = resource_logic_is_studylight (resource);
  bool isComparative = resource_logic_is_comparative (resource);

  // Handle a comparative resource.
  // This type of resource is special.
  // It is not one resource, but made out of two resources.
  // It fetches data from two resources and combines that into one.
  if (isComparative) {
#ifdef HAVE_CLOUD
    html = resource_logic_cloud_get_comparison (webserver_request, resource, book, chapter, verse, add_verse_numbers);
#endif
#ifdef HAVE_CLIENT
    html = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif
    return html;
  }

  Database_Mappings database_mappings;

  // Retrieve versification system of the active Bible.
  string bible = request->database_config_user ()->getBible ();
  string bible_versification = Database_Config_Bible::getVersificationSystem (bible);

  // Determine the versification system of the current resource.
  string resource_versification;
  if (isBible || isUsfm) {
    resource_versification = Database_Config_Bible::getVersificationSystem (bible);
  } else if (isExternal) {
    resource_versification = resource_external_mapping (resource);
  } else if (isImage) {
  } else if (isLexicon) {
    resource_versification = database_mappings.original ();
    if (resource == KJV_LEXICON_NAME) resource_versification = english ();
  } else if (isSword) {
    resource_versification = english ();
  } else if (isBibleGateway) {
    resource_versification = english ();
  } else if (isStudyLight) {
    resource_versification = english ();
  } else {
  }

  // If the resource versification system differs from the Bible's versification system,
  // map the focused verse of the Bible to a verse in the Resource.
  // There are resources without versification system: Do nothing about them.
  vector <Passage> passages;
  if ((bible_versification != resource_versification) && !resource_versification.empty ()) {
    passages = database_mappings.translate (bible_versification, resource_versification, book, chapter, verse);
  } else {
    passages.push_back (Passage ("", book, chapter, convert_to_string (verse)));
  }

  // If there's been a mapping, the resource should include the verse number for clarity.
  if (passages.size () != 1) add_verse_numbers = true;
  for (auto passage : passages) {
    if (verse != convert_to_int (passage.verse)) {
      add_verse_numbers = true;
    }
  }
  
  // Flag for whether to add the full passage (e.g. Matthew 1:1) to the text of that passage.
  bool add_passages_in_full = false;

  // Deal with user's preference whether to include related passages.
  if (request->database_config_user ()->getIncludeRelatedPassages ()) {
    
    // Take the Bible's active passage and mapping, and translate that to the original mapping.
    vector <Passage> related_passages = database_mappings.translate (bible_versification, database_mappings.original (), book, chapter, verse);
    
    // Look for related passages.
    related_passages = related_logic_get_verses (related_passages);
    
    add_passages_in_full = !related_passages.empty ();
    
    // If there's any related passages, map them to the resource's versification system.
    if (!related_passages.empty ()) {
      if (!resource_versification.empty ()) {
        if (resource_versification != database_mappings.original ()) {
          passages.clear ();
          for (auto & related_passage : related_passages) {
            vector <Passage> mapped_passages = database_mappings.translate (database_mappings.original (), resource_versification, related_passage.book, related_passage.chapter, convert_to_int (related_passage.verse));
            passages.insert (passages.end (), mapped_passages.begin (), mapped_passages.end ());
          }
        }
      }
    }
  }
  
  for (auto passage : passages) {
    string possible_included_passage;
    if (add_verse_numbers) possible_included_passage = passage.verse + " ";
    if (add_passages_in_full) possible_included_passage = filter_passage_display (passage.book, passage.chapter, passage.verse) + " ";
    if (isImage) possible_included_passage.clear ();
    html.append (possible_included_passage);
    html.append (resource_logic_get_verse (webserver_request, resource, passage.book, passage.chapter, convert_to_int (passage.verse)));
  }
  
  return html;
}


// This is the most basic version that fetches the text of a $resource.
// It works on server and on client.
// It uses the cache.
string resource_logic_get_verse (void * webserver_request, string resource, int book, int chapter, int verse)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  string data;

  // Determine the type of the current resource.
  bool isBible = resource_logic_is_bible (resource);
  Database_UsfmResources database_usfmresources;
  vector <string> local_usfms = database_usfmresources.getResources ();
  bool isLocalUsfm = in_array (resource, local_usfms);
  vector <string> remote_usfms;
#ifdef HAVE_CLIENT
  remote_usfms = client_logic_usfm_resources_get ();
#endif
  bool isRemoteUsfm = in_array (resource, remote_usfms);
  bool isExternal = resource_logic_is_external (resource);
  bool isImage = resource_logic_is_image (resource);
  bool isLexicon = resource_logic_is_lexicon (resource);
  bool isSword = resource_logic_is_sword (resource);
  bool isBibleGateway = resource_logic_is_biblegateway (resource);
  bool isStudyLight = resource_logic_is_studylight (resource);
  
  if (isBible || isLocalUsfm) {
    string chapter_usfm;
    if (isBible) chapter_usfm = request->database_bibles()->getChapter (resource, book, chapter);
    if (isLocalUsfm) chapter_usfm = database_usfmresources.getUsfm (resource, book, chapter);
    string verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
    string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (resource);
    filter_text.html_text_standard = new Html_Text ("");
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    data = filter_text.html_text_standard->get_inner_html ();
  } else if (isRemoteUsfm) {
    data = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
  } else if (isExternal) {
#ifdef HAVE_CLIENT
    // A client fetches it from the cache or from the Cloud.
    data = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#else
    // The server fetches it from the web, via the http cache.
    data.append (resource_external_cloud_fetch_cache_extract (resource, book, chapter, verse));
#endif
  } else if (isImage) {
    Database_ImageResources database_imageresources;
    vector <string> images = database_imageresources.get (resource, book, chapter, verse);
    for (auto & image : images) {
      data.append ("<div><img src=\"/resource/imagefetch?name=" + resource + "&image=" + image + "\" alt=\"Image resource\" style=\"width:100%\"></div>");
    }
  } else if (isLexicon) {
    data = lexicon_logic_get_html (request, resource, book, chapter, verse);
  } else if (isSword) {
    string sword_module = sword_logic_get_remote_module (resource);
    string sword_source = sword_logic_get_source (resource);
    data = sword_logic_get_text (sword_source, sword_module, book, chapter, verse);
  } else if (isBibleGateway) {
    data = resource_logic_bible_gateway_get (resource, book, chapter, verse);
  } else if (isStudyLight) {
    data = resource_logic_study_light_get (resource, book, chapter, verse);
  } else {
    // Nothing found.
  }
  
  // Any font size given in a paragraph style may interfere with the font size setting for the resources
  // as given in Bibledit. For that reason remove the class name from a paragraph style.
  for (unsigned int i = 0; i < 5; i++) {
    string fragment = "p class=\"";
    size_t pos = data.find (fragment);
    if (pos != string::npos) {
      size_t pos2 = data.find ("\"", pos + fragment.length () + 1);
      if (pos2 != string::npos) {
        data.erase (pos + 1, pos2 - pos);
      }
    }
  }
  
  // NET Bible updates.
  data = filter_string_str_replace ("<span class=\"s ", "<span class=\"", data);

  return data;
}


string resource_logic_cloud_get_comparison (void * webserver_request,
                                            string resource, int book, int chapter, int verse,
                                            bool add_verse_numbers)
{
  // This function gets passed the resource title only.
  // So get all resources and look for the one with this title.
  // And then get the additional properties belonging to this resource.
  string title, base, update, remove, replace;
  bool diacritics = false, casefold = false;
  vector <string> resources = Database_Config_General::getComparativeResources ();
  for (auto s : resources) {
    resource_logic_parse_comparative_resource (s, &title, &base, &update, &remove, &replace, &diacritics, &casefold);
    if (title == resource) break;
  }

  // Get the html of both resources to compare.
  base = resource_logic_get_html (webserver_request, base, book, chapter, verse, add_verse_numbers);
  update = resource_logic_get_html (webserver_request, update, book, chapter, verse, add_verse_numbers);
  
  // Clean all html elements away from the text to get a better and cleaner comparison.
  base = filter_string_html2text (base);
  update = filter_string_html2text(update);
  
  // It has been seen that one resource had a normal space, and the updated resource a non-breaking space.
  // In such a situation there was highlighting of differences between the two resources.
  // But with the eye one could not find any differences.
  // So the question would come up like:
  // Why does it highlight a difference while there seems to be no difference?
  // The solution is to convert types of non-breaking spaces to normal ones.
  base = any_space_to_standard_space (base);
  update = any_space_to_standard_space(update);

  // If characters are given to remove from the resources, handle that situation now.
  if (!remove.empty()) {
    vector<string> bits = filter_string_explode(remove, ' ');
    for (auto rem : bits) {
      if (rem.empty()) continue;
      base = filter_string_str_replace(rem, "", base);
      update = filter_string_str_replace(rem, "", update);
    }
  }
  
  // If search-and-replace sets are given to be applied to the resources, handle that now.
  if (!replace.empty()) {
    vector<string> search_replace_sets = filter_string_explode(replace, ' ');
    for (auto search_replace_set : search_replace_sets) {
      vector <string> search_replace = filter_string_explode(search_replace_set, '=');
      if (search_replace.size() == 2) {
        string search = search_replace[0];
        if (search.empty()) continue;
        string replace = search_replace[1];
        base = filter_string_str_replace(search, replace, base);
        update = filter_string_str_replace(search, replace, update);
      }
    }
  }

  // When showing the difference between two Greek New Testaments,
  // one with diacritics and the other without diacritics.
  // there's a lot of flagging of difference, just because of the diacritics.
  // To handle such a situation, remove the diacritics.
  // Similarly to not mark small letters versus capitals as a difference, do case folding.
#ifdef HAVE_ICU
  base = icu_string_normalize (base, diacritics, casefold);
  update = icu_string_normalize (update, diacritics, casefold);
#endif

  // Find the differences.
  string html = filter_diff_diff (base, update);
  return html;
}


// This runs on the server.
// It gets the html or text contents for a $resource for serving it to a client.
string resource_logic_get_contents_for_client (string resource, int book, int chapter, int verse)
{
  // Determine the type of the current resource.
  bool isExternal = resource_logic_is_external (resource);
  bool isUsfm = resource_logic_is_usfm (resource);
  bool isSword = resource_logic_is_sword (resource);
  bool isBibleGateway = resource_logic_is_biblegateway (resource);
  bool isStudyLight = resource_logic_is_studylight (resource);
  bool isComparative = resource_logic_is_comparative (resource);

  if (isExternal) {
    // The server fetches it from the web.
    return resource_external_cloud_fetch_cache_extract (resource, book, chapter, verse);
  }
  
  if (isUsfm) {
    // Fetch from database and convert to html.
    Database_UsfmResources database_usfmresources;
    string chapter_usfm = database_usfmresources.getUsfm (resource, book, chapter);
    string verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
    string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (resource);
    filter_text.html_text_standard = new Html_Text ("");
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    return filter_text.html_text_standard->get_inner_html ();
  }
  
  if (isSword) {
    // Fetch it from a SWORD module.
    string sword_module = sword_logic_get_remote_module (resource);
    string sword_source = sword_logic_get_source (resource);
    return sword_logic_get_text (sword_source, sword_module, book, chapter, verse);
  }

  if (isBibleGateway) {
    // The server fetches it from the web.
    return resource_logic_bible_gateway_get (resource, book, chapter, verse);
  }

  if (isStudyLight) {
    // The server fetches it from the web.
    return resource_logic_study_light_get (resource, book, chapter, verse);
  }

  if (isComparative) {
    // Handle a comparative resource.
    // This type of resource is special.
    // It is not one resource, but made out of two resources.
    // It fetches data from two resources and combines that into one.
    Webserver_Request request;
    return resource_logic_cloud_get_comparison (&request, resource, book, chapter, verse, false);
  }
  
  // Nothing found.
  return translate ("Bibledit Cloud could not localize this resource");
}


// The client runs this function to fetch a general resource $name from the Cloud,
// or from its local cache,
// and to update the local cache with the fetched content, if needed,
// and to return the requested content.
string resource_logic_client_fetch_cache_from_cloud (string resource, int book, int chapter, int verse)
{
  // Whether the client should cache this resource.
  bool cache = !in_array(resource, client_logic_no_cache_resources_get ());
  
  // Ensure that the cache for this resource exists on the client.
  if (cache && !Database_Cache::exists (resource, book)) {
    Database_Cache::create (resource, book);
  }
  
  // If content is to be cached and the content exists in the cache, return that content.
  if (cache && Database_Cache::exists (resource, book, chapter, verse)) {
    return Database_Cache::retrieve (resource, book, chapter, verse);
  }
  
  // Fetch this resource from Bibledit Cloud.
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  if (!client_logic_client_enabled ()) {
    // If the client has not been connected to a cloud instance,
    // fetch the resource from the Bibledit Cloud demo.
    address = demo_address ();
    port = demo_port ();
  }
  
  string url = client_logic_url (address, port, sync_resources_url ());
  url = filter_url_build_http_query (url, "r", filter_url_urlencode (resource));
  url = filter_url_build_http_query (url, "b", convert_to_string (book));
  url = filter_url_build_http_query (url, "c", convert_to_string (chapter));
  url = filter_url_build_http_query (url, "v", convert_to_string (verse));
  string error;
  string content = filter_url_http_get (url, error, false);
  
  // Cache the content under circumstances.
  if (cache) {
    if (database_cache_can_cache (error, content)) {
      Database_Cache::cache (resource, book, chapter, verse, content);
    }
  }
  if (!error.empty ()) {
    // Error: Log it, and add it to the contents.
    Database_Logs::log (resource + ": " + error);
    content.append (error);
  }

  // Done.
  return content;
}


// Imports the file at $path into $resource.
void resource_logic_import_images (string resource, string path)
{
  Database_ImageResources database_imageresources;
  
  Database_Logs::log ("Importing: " + filter_url_basename_cpp17 (path));
  
  // To begin with, add the path to the main file to the list of paths to be processed.
  vector <string> paths = {path};
  
  while (!paths.empty ()) {
  
    // Take and remove the first path from the container.
    path = paths[0];
    paths.erase (paths.begin());
    string basename = filter_url_basename_cpp17 (path);
    string extension = filter_url_get_extension (path);
    extension = unicode_string_casefold (extension);

    if (extension == "pdf") {
      
      Database_Logs::log ("Processing PDF: " + basename);
      
      // Retrieve PDF information.
      filter_shell_run ("", "pdfinfo", {path}, NULL, NULL);

      // Convert the PDF file to separate images.
      string folder = filter_url_tempfile ();
      filter_url_mkdir (folder);
      filter_shell_run (folder, "pdftocairo", {"-jpeg", path}, NULL, NULL);
      // Add the images to the ones to be processed.
      filter_url_recursive_scandir (folder, paths);
      
    } else if (filter_archive_is_archive (path)) {
      
      Database_Logs::log ("Unpacking archive: " + basename);
      string folder = filter_archive_uncompress (path);
      filter_url_recursive_scandir (folder, paths);
      
    } else {

      if (!extension.empty ()) {
        basename = database_imageresources.store (resource, path);
        Database_Logs::log ("Storing image " + basename );
      }
      
    }
  }

  Database_Logs::log ("Ready importing images");
}


string resource_logic_yellow_divider ()
{
  return "Yellow Divider";
}


string resource_logic_green_divider ()
{
  return "Green Divider";
}


string resource_logic_blue_divider ()
{
  return "Blue Divider";
}


string resource_logic_violet_divider ()
{
  return "Violet Divider";
}


string resource_logic_red_divider ()
{
  return "Red Divider";
}


string resource_logic_orange_divider ()
{
  return "Orange Divider";
}


string resource_logic_rich_divider ()
{
  return "Rich Divider";
}


bool resource_logic_parse_rich_divider (string input, string & title, string & link,
                                      string & foreground, string & background)
{
  title.clear();
  link.clear();
  foreground.clear();
  background.clear();
  vector <string> bits = filter_string_explode(input, '|');
  if (bits.size() != 5) return false;
  if (bits[0] != resource_logic_rich_divider()) return false;
  title = bits[1];
  link = bits[2];
  foreground = bits[3];
  background = bits[4];
  return true;
}


string resource_logic_assemble_rich_divider (string title, string link,
                                             string foreground, string background)
{
  vector <string> bits = {resource_logic_rich_divider(), title, link, foreground, background};
  return filter_string_implode(bits, "|");
}


string resource_logic_get_divider (string resource)
{
  string title;
  string link;
  string foreground;
  string background;
  if (resource_logic_parse_rich_divider (resource, title, link, foreground, background)) {
    // Trim whitespace.
    title = filter_string_trim(title);
    link = filter_string_trim(link);
    // Render a rich divider.
    if (title.empty ()) title = link;
    // The $ influences the resource's embedding through Javascript.
    string html = "$";
    html.append (R"(<div class="width100 center" style="background-color:)");
    html.append (background);
    html.append (R"(;color:)");
    html.append (foreground);
    html.append (R"(;)");
    html.append (R"(">)");
    html.append (R"(<a href=")");
    html.append (link);
    html.append (R"(" target="_blank">)");
    html.append (title);
    html.append (R"(</a>)");
    html.append (R"()");
    html.append (R"()");
    html.append (R"(</div>)");
    return html;
  } else {
    // Render the standard fixed dividers.
    vector <string> bits = filter_string_explode (resource, ' ');
    string colour = unicode_string_casefold (bits [0]);
    // The $ influences the resource's embedding through Javascript.
    string html = R"($<div class="divider" style="background-color:)" + colour + R"(">&nbsp;</div>)";
    return html;
  }

}


// In Cloud mode, this function wraps around http GET.
// It fetches existing content from the cache, and caches new content.
string resource_logic_web_or_cache_get (string url, string & error)
{
#ifndef HAVE_CLIENT
  // On the Cloud, check if the URL is in the cache.
  if (database_filebased_cache_exists (url)) {
    return database_filebased_cache_get (url);
  }
#endif

  // Fetch the URL from the network.
  error.clear ();
  string html = filter_url_http_get (url, error, false);

#ifdef HAVE_CLOUD
  // In the Cloud, cache the response based on certain criteria.
  bool cache = database_cache_can_cache (error, html);
  if (cache) {
    database_filebased_cache_put (url, html);
  }
#endif

  // Done.
  return html;
}


// Returns the page type for the resource selector.
string resource_logic_selector_page (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  string page = request->query["page"];
  return page;
}


// Returns the page which called the resource selector.
string resource_logic_selector_caller (void * webserver_request)
{
  string caller = resource_logic_selector_page (webserver_request);
  if (caller == "view") caller = "organize";
  if (caller == "consistency") caller = "../consistency/index";
  if (caller == "print") caller = "print";
  return caller;
}


string resource_logic_default_user_url ()
{
  return "http://bibledit.org/resource-[book]-[chapter]-[verse].html";
}


// This creates a resource database cache and runs in the Cloud.
void resource_logic_create_cache ()
{
  // Because clients usually request caches in a quick sequence,
  // the Cloud would start to cache several books in parallel.
  // This is undesired.
  // Here's some logic to ensure there's only one book at a time being cached.
  static bool resource_logic_create_cache_running = false;
  if (resource_logic_create_cache_running) return;
  resource_logic_create_cache_running = true;
  
  // If there's nothing to cache, bail out.
  vector <string> signatures = Database_Config_General::getResourcesToCache ();
  if (signatures.empty ()) return;

  // Resource and book to cache.
  string signature = signatures [0];
  signatures.erase (signatures.begin ());
  Database_Config_General::setResourcesToCache (signatures);
  size_t pos = signature.find_last_of (" ");
  string resource = signature.substr (0, pos);
  int book = convert_to_int (signature.substr (pos++));
  string bookname = Database_Books::getEnglishFromId (book);
  
  // Whether it's a SWORD module.
  string sword_module = sword_logic_get_remote_module (resource);
  string sword_source = sword_logic_get_source (resource);
  bool is_sword_module = (!sword_source.empty () && !sword_module.empty ());

  // In case of a  SWORD module, ensure it has been installed.
  if (is_sword_module) {
    vector <string> modules = sword_logic_get_installed ();
    bool already_installed = false;
    for (auto & installed_module : modules) {
      if (installed_module.find ("[" + sword_module + "]") != string::npos) {
        already_installed = true;
      }
    }
    if (!already_installed) {
      sword_logic_install_module (sword_source, sword_module);
    }
  }
  
  // Database layout is per book: Create a database for this book.
  Database_Cache::remove (resource, book);
  Database_Cache::create (resource, book);
  
  Database_Versifications database_versifications;
  vector <int> chapters = database_versifications.getMaximumChapters (book);
  for (auto & chapter : chapters) {

    Database_Logs::log ("Caching " + resource + " " + bookname + " " + convert_to_string (chapter), Filter_Roles::consultant ());

    // The verse numbers in the chapter.
    vector <int> verses = database_versifications.getMaximumVerses (book, chapter);
    
    // In case of a SWORD module, fetch the texts of all verses in bulk.
    // This is because calling vfork once per verse to run diatheke stops working in the Cloud after some time.
    // Forking once per chapter is much better, also for the performance.
    map <int, string> sword_texts;
    if (is_sword_module) {
      sword_texts = sword_logic_get_bulk_text (sword_module, book, chapter, verses);
    }
    
    // Iterate over the verses.
    for (auto & verse : verses) {

      // Fetch the text for the passage.
      bool server_is_installing_module = false;
      bool server_is_updating = false;
      bool server_is_unavailable = false;
      int wait_iterations = 0;
      string html, error;
      do {
        // Fetch the resource data.
        if (is_sword_module) html = sword_texts [verse];
        else html = resource_logic_get_contents_for_client (resource, book, chapter, verse);
        // Check on errors.
        server_is_installing_module = (html == sword_logic_installing_module_text ());
        if (server_is_installing_module) {
          Database_Logs::log ("Waiting while installing SWORD module: " + resource);
        }
        server_is_updating = html.find ("... upgrading ...") != string::npos;
        if (server_is_updating) {
          Database_Logs::log ("Waiting while Cloud is upgrading itself");
        }
        // In case of server unavailability, wait a while, then try again.
        server_is_unavailable = server_is_installing_module || server_is_updating;
        if (server_is_unavailable) {
          this_thread::sleep_for (chrono::seconds (60));
          wait_iterations++;
        }
      } while (server_is_unavailable && (wait_iterations < 5));

      // Cache the verse data, even if there's an error.
      // If it were not cached at, say, Leviticus, then the caching mechanism,
      // after restart, would always continue from that same book, from Leviticus,
      // and never finish. Therefore something should be cached, even if it's an empty string.
      if (server_is_installing_module) html.clear ();
      Database_Cache::cache (resource, book, chapter, verse, html);
    }
  }

  // Done.
  Database_Cache::ready (resource, book, true);
  Database_Logs::log ("Completed caching " + resource + " " + bookname, Filter_Roles::consultant ());
  resource_logic_create_cache_running = false;
  
  // If there's another resource database waiting to be cached, schedule it for caching.
  if (!signatures.empty ()) tasks_logic_queue (CACHERESOURCES);
}


// Returns true if the resource can be installed locally.
bool resource_logic_can_cache (string resource)
{
  // Bibles are local already, cannot be installed.
  if (resource_logic_is_bible (resource)) return false;

  // Lexicons are local already, cannot be installed.
  if (resource_logic_is_lexicon (resource)) return false;

  // Dividers are local already, cannot be installed.
  if (resource_logic_is_divider (resource)) return false;
  
  // Remaining resources can be installed locally.
  return true;
}


// The path to the list of BibleGateway resources.
// It is stored in the client files area.
// Clients will download it from there.
string resource_logic_bible_gateway_module_list_path ()
{
  return filter_url_create_root_path_cpp17 ({database_logic_databases (), "client", "bible_gateway_modules.txt"});
}


// Refreshes the list of resources available from BibleGateway.
string resource_logic_bible_gateway_module_list_refresh ()
{
  Database_Logs::log ("Refresh BibleGateway resources");
  string path = resource_logic_bible_gateway_module_list_path ();
  string error;
  string html = filter_url_http_get ("https://www.biblegateway.com/versions/", error, false);
  if (error.empty ()) {
    vector <string> resources;
    html = filter_text_html_get_element (html, "select");
    xml_document document;
    document.load_string (html.c_str());
    xml_node select_node = document.first_child ();
    for (xml_node option_node : select_node.children()) {
      string cls = option_node.attribute ("class").value ();
      if (cls == "lang") continue;
      if (cls == "spacer") continue;
      string name = option_node.text ().get ();
      resources.push_back (name);
    }
    filter_url_file_put_contents (path, filter_string_implode (resources, "\n"));
    Database_Logs::log ("Modules: " + convert_to_string (resources.size ()));
  } else {
    Database_Logs::log (error);
  }
  return error;
}


// Get the list of BibleGateway resources.
vector <string> resource_logic_bible_gateway_module_list_get ()
{
  string path = resource_logic_bible_gateway_module_list_path ();
  string contents = filter_url_file_get_contents (path);
  return filter_string_explode (contents, '\n');
}


string resource_logic_bible_gateway_book (int book)
{
  // Map Bibledit books to biblegateway.com books as used at the web service.
  map <int, string> mapping = {
    pair (1, "Genesis"),
    pair (2, "Exodus"),
    pair (3, "Leviticus"),
    pair (4, "Numbers"),
    pair (5, "Deuteronomy"),
    pair (6, "Joshua"),
    pair (7, "Judges"),
    pair (8, "Ruth"),
    pair (9, "1 Samuel"),
    pair (10, "2 Samuel"),
    pair (11, "1 Kings"),
    pair (12, "2 Kings"),
    pair (13, "1 Chronicles"),
    pair (14, "2 Chronicles"),
    pair (15, "Ezra"),
    pair (16, "Nehemiah"),
    pair (17, "Esther"),
    pair (18, "Job"),
    pair (19, "Psalms"),
    pair (20, "Proverbs"),
    pair (21, "Ecclesiastes"),
    pair (22, "Song of Solomon"),
    pair (23, "Isaiah"),
    pair (24, "Jeremiah"),
    pair (25, "Lamentations"),
    pair (26, "Ezekiel"),
    pair (27, "Daniel"),
    pair (28, "Hosea"),
    pair (29, "Joel"),
    pair (30, "Amos"),
    pair (31, "Obadiah"),
    pair (32, "Jonah"),
    pair (33, "Micah"),
    pair (34, "Nahum"),
    pair (35, "Habakkuk"),
    pair (36, "Zephaniah"),
    pair (37, "Haggai"),
    pair (38, "Zechariah"),
    pair (39, "Malachi"),
    pair (40, "Matthew"),
    pair (41, "Mark"),
    pair (42, "Luke"),
    pair (43, "John"),
    pair (44, "Acts"),
    pair (45, "Romans"),
    pair (46, "1 Corinthians"),
    pair (47, "2 Corinthians"),
    pair (48, "Galatians"),
    pair (49, "Ephesians"),
    pair (50, "Philippians"),
    pair (51, "Colossians"),
    pair (52, "1 Thessalonians"),
    pair (53, "2 Thessalonians"),
    pair (54, "1 Timothy"),
    pair (55, "2 Timothy"),
    pair (56, "Titus"),
    pair (57, "Philemon"),
    pair (58, "Hebrews"),
    pair (59, "James"),
    pair (60, "1 Peter"),
    pair (61, "2 Peter"),
    pair (62, "1 John"),
    pair (63, "2 John"),
    pair (64, "3 John"),
    pair (65, "Jude"),
    pair (66, "Revelation")
  };
  return filter_url_urlencode (mapping [book]);
}


string resource_external_convert_book_studylight (int book)
{
  // Map Bibledit books to biblehub.com books.
  map <int, string> mapping = {
    pair (1, "genesis"),
    pair (2, "exodus"),
    pair (3, "leviticus"),
    pair (4, "numbers"),
    pair (5, "deuteronomy"),
    pair (6, "joshua"),
    pair (7, "judges"),
    pair (8, "ruth"),
    pair (9, "1-samuel"),
    pair (10, "2-samuel"),
    pair (11, "1-kings"),
    pair (12, "2-kings"),
    pair (13, "1-chronicles"),
    pair (14, "2-chronicles"),
    pair (15, "ezra"),
    pair (16, "nehemiah"),
    pair (17, "esther"),
    pair (18, "job"),
    pair (19, "psalms"),
    pair (20, "proverbs"),
    pair (21, "ecclesiastes"),
    pair (22, "song-of-solomon"),
    pair (23, "isaiah"),
    pair (24, "jeremiah"),
    pair (25, "lamentations"),
    pair (26, "ezekiel"),
    pair (27, "daniel"),
    pair (28, "hosea"),
    pair (29, "joel"),
    pair (30, "amos"),
    pair (31, "obadiah"),
    pair (32, "jonah"),
    pair (33, "micah"),
    pair (34, "nahum"),
    pair (35, "habakkuk"),
    pair (36, "zephaniah"),
    pair (37, "haggai"),
    pair (38, "zechariah"),
    pair (39, "malachi"),
    pair (40, "matthew"),
    pair (41, "mark"),
    pair (42, "luke"),
    pair (43, "john"),
    pair (44, "acts"),
    pair (45, "romans"),
    pair (46, "1-corinthians"),
    pair (47, "2-corinthians"),
    pair (48, "galatians"),
    pair (49, "ephesians"),
    pair (50, "philippians"),
    pair (51, "colossians"),
    pair (52, "1-thessalonians"),
    pair (53, "2-thessalonians"),
    pair (54, "1-timothy"),
    pair (55, "2-timothy"),
    pair (56, "titus"),
    pair (57, "philemon"),
    pair (58, "hebrews"),
    pair (59, "james"),
    pair (60, "1-peter"),
    pair (61, "2-peter"),
    pair (62, "1-john"),
    pair (63, "2-john"),
    pair (64, "3-john"),
    pair (65, "jude"),
    pair (66, "revelation")
  };
  return mapping [book];
}


struct bible_gateway_walker: xml_tree_walker
{
  bool skip_next_text = false;
  bool parsing = true;
  string text;
  vector <string> footnotes;

  virtual bool for_each (xml_node& node)
  {
    // Details of the current node.
    string classname = node.attribute ("class").value ();
    string nodename = node.name ();

    // At the end of the verse, there's this:
    // Read full chapter.
    if (classname == "full-chap-link") {
      parsing = false;
      return true;
    }

    // Do not include the verse number with the Bible text.
    if (classname == "versenum") {
      skip_next_text = true;
      return true;
    }

    // Add spaces instead of new lines.
    if (nodename == "p") if (parsing) text.append (" ");
    if (nodename == "br") if (parsing) text.append (" ");

    // Include node's text content.
    if (parsing) {
      if (!skip_next_text) {
        text.append (node.value ());
      }
      skip_next_text = false;
    }
    
    // Fetch the foonote(s) relevant to this verse.
    // <sup data-fn='#fen-TLB-20531a' class='footnote' data-link=......
    if (parsing && (classname == "footnote")) {
      string data_fn = node.attribute ("data-fn").value ();
      footnotes.push_back(data_fn);
    }
    
    // Continue parsing.
    return true;
  }
};


// Get the clean text of a passage of a BibleGateway resource.
string resource_logic_bible_gateway_get (string resource, int book, int chapter, int verse)
{
  string result;
#ifdef HAVE_CLOUD
  // Convert the resource name to a resource abbreviation for biblegateway.com.
  size_t pos = resource.find_last_of ("(");
  if (pos != string::npos) {
    pos++;
    resource.erase (0, pos);
    pos = resource.find_last_of (")");
    if (pos != string::npos) {
      resource.erase (pos);
      // Assemble the URL to fetch the chapter.
      string bookname = resource_logic_bible_gateway_book (book);
      string url = "https://www.biblegateway.com/passage/?search=" + bookname + "+" + convert_to_string (chapter) + ":" + convert_to_string(verse) + "&version=" + resource;
      // Fetch the html.
      string error;
      string html = resource_logic_web_or_cache_get (url, error);
      // Remove the html that precedes the relevant verses content.
      pos = html.find ("<div class=\"passage-text\">");
      if (pos != string::npos) {
        html.erase (0, pos);
        // Load the remaining html into the XML parser.
        // The parser will given an error about Start-end tags mismatch.
        // The parser will also give the location where this mismatch occurs first.
        // The location where the mismatch occurs indicates the end of the relevant verses content.
        {
          xml_document document;
          xml_parse_result result = document.load_string (html.c_str(), parse_default | parse_fragment);
          if (result.offset > 10) {
            size_t pos = result.offset - 2;
            html.erase (pos);
          }
        }
        // Parse the html fragment into a DOM.
        string verse_s = convert_to_string (verse);
        xml_document document;
        document.load_string (html.c_str());
        // There can be cross references in the html.
        // These result in e.g. "A" or "B" scattered through the final text.
        // So remove these.
        // Sample cross reference XML:
        // <sup class='crossreference' data-cr='#cen-NASB-30388A'  data-link='(&lt;a href=&quot;#cen-NASB-30388A&quot; title=&quot;See cross-reference A&quot;&gt;A&lt;/a&gt;)'>(<a href="#cen-NASB-30388A" title="See cross-reference A">A</a>)</sup>
        {
          string selector = "//sup[@class='crossreference']";
          xpath_node_set nodeset = document.select_nodes(selector.c_str());
          for (auto xrefnode: nodeset) xrefnode.node().parent().remove_child(xrefnode.node());
        }
        // Start parsing for actual text.
        xml_node passage_text_node = document.first_child ();
        xml_node passage_wrap_node = passage_text_node.first_child ();
        xml_node passage_content_node = passage_wrap_node.first_child ();
        bible_gateway_walker walker;
        passage_content_node.traverse (walker);
        result.append (walker.text);
        // Adding text of the footnote(s) if any.
        for (auto footnote_id : walker.footnotes) {
          if (footnote_id.empty()) continue;
          // Example footnote ID is: #fen-TLB-20531a
          // Remove the #.
          footnote_id.erase (0, 1);
          // XPath selector.
          // <li id="fen-TLB-20531a"><a href="#en-TLB-20531" title="Go to Matthew 1:17">Matthew 1:17</a> <span class='footnote-text'><i>These are fourteen,</i> literally, “So all the generations from Abraham unto David are fourteen.”</span></li>
          string selector = "//li[@id='" + footnote_id + "']/span[@class='footnote-text']";
          xpath_node xpath = document.select_node(selector.c_str());
          if (xpath) {
            stringstream ss;
            xpath.node().print (ss, "", format_raw);
            string html = ss.str ();
            string footnote_text = filter_string_html2text (html);
            result.append ("<br>Note: ");
            result.append (footnote_text);
          }
        }
      }
    }
  }
  result = filter_string_str_replace (unicode_non_breaking_space_entity (), " ", result);
  result = filter_string_str_replace (" ", " ", result); // Make special space visible.
  result = filter_string_collapse_whitespace (result);
  result = filter_string_trim (result);
#endif
#ifdef HAVE_CLIENT
  result = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif
  return result;
}


// The path to the list of StudyLight resources.
// It is stored in the client files area.
// Clients will download it from there.
string resource_logic_study_light_module_list_path ()
{
  return filter_url_create_root_path_cpp17 ({database_logic_databases (), "client", "study_light_modules.txt"});
}


// Refreshes the list of resources available from StudyLight.
string resource_logic_study_light_module_list_refresh ()
{
  Database_Logs::log ("Refresh StudyLight resources");
  string path = resource_logic_study_light_module_list_path ();
  string error;
  string html = filter_url_http_get ("http://www.studylight.org/commentaries", error, false);
  if (error.empty ()) {
    vector <string> resources;
    // Example commentary fragment:
    // <h3><a class="emphasis" href="//www.studylight.org/commentaries/gsb.html">Geneva Study Bible</a></h3>
    do {
      // <a class="emphasis" ...
      string fragment = R"(<a class="emphasis")";
      // New fragment on updated website:
      fragment = R"(<a class="fg-darkgrey")";
      size_t pos = html.find (fragment);
      if (pos == string::npos) break;
      html.erase (0, pos + fragment.size ());
      fragment = "commentaries";
      pos = html.find (fragment);
      if (pos == string::npos) break;
      html.erase (0, pos + fragment.size () + 1);
      fragment = ".html";
      pos = html.find (fragment);
      if (pos == string::npos) break;
      string abbreviation = html.substr (0, pos);
      html.erase (0, pos + fragment.size () + 2);
      pos = html.find ("</a>");
      if (pos == string::npos) break;
      string name = html.substr (0, pos);
      string resource = name + " (studylight-" + abbreviation + ")";
      resources.push_back (resource);
    } while (true);
    // Store the resources in a file.
    filter_url_file_put_contents (path, filter_string_implode (resources, "\n"));
    // Done.
    Database_Logs::log ("Modules: " + convert_to_string (resources.size ()));
  } else {
    Database_Logs::log (error);
  }
  return error;
}


// Get the list of StudyLight resources.
vector <string> resource_logic_study_light_module_list_get ()
{
  string path = resource_logic_study_light_module_list_path ();
  string contents = filter_url_file_get_contents (path);
  return filter_string_explode (contents, '\n');
}


// Get the slightly formatted of a passage of a StudyLight resource.
string resource_logic_study_light_get (string resource, int book, int chapter, int verse)
{
  string result;

#ifdef HAVE_CLOUD
  // Transform the full name to the abbreviation for the website, e.g.:
  // "Adam Clarke Commentary (acc)" becomes "acc".
  size_t pos = resource.find_last_of ("(");
  if (pos == string::npos) return "";
  resource.erase (0, pos + 1);
  pos = resource.find (")");
  if (pos == string::npos) return "";
  resource.erase (pos);
  
  // The resource abbreviation might look like this:
  // studylight-eng/bnb
  // Also remove that "studylight-" bit.
  resource.erase (0, 11);
  
  // Example URL: https://www.studylight.org/commentaries/eng/acc/revelation-1.html
  string url;
  url.append ("http://www.studylight.org/commentaries/");
  url.append (resource + "/");
  url.append (resource_external_convert_book_studylight (book));
  url.append ("-" + convert_to_string (chapter) + ".html");
  
  // Get the html from the server.
  string error;
  string html = resource_logic_web_or_cache_get (url, error);

  // It appears that the html from this website is not well-formed.
  // It cannot be loaded as an XML document without errors and missing text.
  // Therefore the html is tidied first.
  html = filter_string_tidy_invalid_html (html);

  // Parse the html into a DOM.
  string verse_s = convert_to_string (verse);
  xml_document document;
  document.load_string (html.c_str());

  // Example verse indicator within the XML:
  // <a name="verses-2-10"></a>
  // <a name="verse-2"></a>
  string selector1 = "//a[contains(@name,'verses-" + convert_to_string (verse) + "-')]";
  string selector2 = "//a[@name='verse-" + convert_to_string (verse) + "']";
  string selector = selector1 + "|" + selector2;
  xpath_node_set nodeset = document.select_nodes(selector.c_str());
  nodeset.sort();
  for (xpath_node xpathnode : nodeset) {
    xml_node h3_node = xpathnode.node().parent();
    xml_node div_node = h3_node.parent();
    stringstream ss;
    div_node.print (ss, "", format_raw);
    result.append(ss.str ());
  }
#endif

#ifdef HAVE_CLIENT
  result = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif

  return result;
}


string resource_logic_easy_english_bible_name ()
{
  return "Easy English Bible Commentary";
}


// Given a book number and a chapter,
// this returns 0 to 2 parts of the URL that will contain the relevant text.
vector <string> resource_logic_easy_english_bible_pages (int book, int chapter)
{
  switch (book) {
    case 1: return { "genesis-lbw", "genesis-mwks-lbw" }; // Genesis.
    case 2: // Exodus.
      if (chapter <= 11) return { "exodus-1-13-im-lbw", "exodus-1-18-lbw" };
      if (chapter <= 18) return { "exodus-14-18-im-lbw", "exodus-1-18-lbw" };
      if (chapter <= 31) return { "exodus-19-31-im-lbw", "exodus-19-40-lbw" };
      if (chapter <= 40) return { "exodus-32-40-im-lbw", "exodus-19-40-lbw" };
      break;
    case 3:  return { "leviticus-lbw" }; // Leviticus.
    case 4:  return { "numbers-lbw" }; // Numbers.
    case 5:  return { "deuteronomy-im-lbw", "deuteronomy-lbw" }; // Deuteronomy.
    case 6:  return { "joshua-lbw" }; // Joshua.
    case 7:  return { "judges-lbw" }; // Judges.
    case 8:  return { "ruth-law", "ruth-lbw" }; // Ruth.
    case 9:  return { "1sam-lbw" }; // 1 Samuel.
    case 10: return { "2samuel-lbw" }; // 2 Samuel.
    case 11: return { "1kings-lbw" }; // 1 Kings.
    case 12: return { "2kings-lbw" }; // 2 Kings.
    case 13: return { "1chronicles-lbw" }; // 1 Chronicles.
    case 14: // 2 Chronicles.
      if (chapter <= 9) return { "2chronicles-1-9-lbw" };
      return { "2chronicles-10-36-lbw" };
    case 15: return { "ezra-lbw" }; // Ezra.
    case 16: return { "nehemiah-lbw" }; // Nehemiah.
    case 17: return { "esther-lbw" }; // Esther.
    case 18: return { "job-lbw" }; // Job.
    case 19: // Psalms.
    {
      string number = filter_string_fill (convert_to_string (chapter), 3, '0');
      return { "psalm" + number + "-taw" };
    }
    case 20: return { "proverbs-lbw" }; // Proverbs.
    case 21: return { "ecclesiastes-lbw" }; // Ecclesiastes.
    case 22: return {
      "song-of-songs-lbw",
      "songsolomon-lbw",
      "songsolomon-mk-lbw",
      "song-of-songs-sr-law"
    }; // Song of Solomon.
    case 23: // Isaiah.
      if (chapter <= 6)  return { "isaiah1-6-gc-lbw",   "isaiah1-9-lbw-nh"   };
      if (chapter <= 9)  return { "isaiah7-12-gc-lbw",  "isaiah1-9-lbw-nh"   };
      if (chapter <= 12) return { "isaiah7-12-gc-lbw",  "isaiah10-20-lbw-nh" };
      if (chapter <= 20) return { "isaiah13-23-gc-lbw", "isaiah10-20-lbw-nh" };
      if (chapter <= 23) return { "isaiah13-23-gc-lbw", "isaiah21-30-lbw-nh" };
      if (chapter <= 27) return { "isaiah24-27-gc-lbw", "isaiah21-30-lbw-nh" };
      if (chapter <= 30) return { "isaiah28-33-gc-lbw", "isaiah21-30-lbw-nh" };
      if (chapter <= 33) return { "isaiah28-33-gc-lbw", "isaiah31-39-lbw-nh" };
      if (chapter <= 39) return { "isaiah34-40-gc-lbw", "isaiah31-39-lbw-nh" };
      if (chapter <= 40) return { "isaiah34-40-gc-lbw", "isaiah40-48-lbw-nh" };
      if (chapter <= 48) return { "isaiah41-55-gc-lbw", "isaiah40-48-lbw-nh" };
      if (chapter <= 55) return { "isaiah41-55-gc-lbw", "isaiah49-57-lbw-nh" };
      if (chapter <= 57) return { "isaiah56-66-gc-lbw", "isaiah49-57-lbw-nh" };
      return { "isaiah56-66-gc-lbw", "isaiah58-66-lbw-nh" };
    case 24: // Jeremiah.
      if (chapter <= 10) return { "jeremiah1-10-lbw"  };
      if (chapter <= 20) return { "jeremiah11-20-lbw" };
      if (chapter <= 33) return { "jeremiah21-33-lbw" };
      if (chapter <= 39) return { "jeremiah34-39-lbw" };
      if (chapter <= 44) return { "jeremiah40-44-lbw" };
      return { "jeremiah45-52-lbw" };
    case 25: return { "lam-lbw" }; // Lamentations.
    case 26: // Ezekiel.
      if (chapter <= 24) return { "ezekiel1-24-lbw" };
      if (chapter <= 39) return { "ezekiel25-39-lbw" };
      return { "ezekiel40-48-lbw", "ezekiel40-48-ks-lbw" };
    case 27: return { "dan-lbw" }; // Daniel.
    case 28: return { "hosea-lbw" }; // Hosea.
    case 29: return { "joel-lbw" }; // Joel.
    case 30: return { "amos-lbw" }; // Amos.
    case 31: return { "obad-lbw" }; // Obadiah.
    case 32: return { "jonah-lbw" }; // Jonah.
    case 33: return { "micah-lbw" }; // Micah.
    case 34: return { "nahum-lbw" }; // Nahum.
    case 35: return { "habakkuk-gc" }; // Habakkuk.
    case 36: return { "zephaniah-gc" }; // Zephaniah.
    case 37: return { "haggai-law" }; // Haggai.
    case 38: return { "zechariah-lbw" }; // Zechariah.
    case 39: return { "malachi-lbw" }; // Malachi.
    case 40: // Matthew.
      if (chapter <= 4)  return { "matthew1-4-im-lbw",   "matthew-lbw" };
      if (chapter <= 13) return { "matthew4-13-im-lbw",  "matthew-lbw" };
      if (chapter <= 20) return { "matthew14-20-im-lbw", "matthew-lbw" };
      return { "matthew21-28-im-lbw", "matthew-lbw" };
    case 41: return { "mark-ks", "mark-lbw" }; // Mark.
    case 42: // Luke.
      if (chapter <=  4) return { "luke1-4-im-lbw",   "luke4-9-im-lbw",   "luke-lbw" };
      if (chapter <=  9) return { "luke4-9-im-lbw",   "luke9-19-im-lbw",  "luke-lbw" };
      if (chapter <= 19) return { "luke9-19-im-lbw",  "luke19-21-im-lbw", "luke-lbw" };
      if (chapter <= 21) return { "luke19-21-im-lbw", "luke-lbw" };
      return { "luke22-24-im-lbw", "luke-lbw" };
    case 43: return { "john-ma-lbw" }; // John.
    case 44: return { "acts-lbw" }; // Acts.
    case 45: return { "romans-lbw" }; // Romans.
    case 46: return { "1-corinthians-lbw" }; // 1 Corinthians.
    case 47: return { "2corinthians-lbw" }; // 2 Corinthians.
    case 48: return { "galatians-rr-lbw", "galatians-lbw" }; // Galatians.
    case 49: return { "eph-lbw" }; // Ephesians.
    case 50: return { "philippians-lbw" }; // Philippians.
    case 51: return { "col-lbw" }; // Colossians.
    case 52: return { "1thess-lbw" }; // 1 Thessalonians.
    case 53: return { "2thess-lbw" }; // 2 Thessalonians.
    case 54: return { "1tim-lbw" }; // 1 Timothy.
    case 55: return { "2tim-lbw" }; // 2 Timothy.
    case 56: return { "titus-lbw" }; // Titus.
    case 57: return { "phm-lbw" }; // Philemon.
    case 58: return { "hebrews-lbw" }; // Hebrews.
    case 59: return { "james-lbw" }; // James.
    case 60: return { "1peter-lbw" }; // 1 Peter.
    case 61: return { "2peter-lbw" }; // 2 Peter.
    case 62: return { "1john-lbw" }; // 1 John.
    case 63: return { "2john-lbw" }; // 2 John.
    case 64: return { "3john-lbw" }; // 3 John.
    case 65: return { "jude-lbw", "jude-nh-lbw" }; // Jude.
    case 66: return { "revelation-lbw" }; // Revelation.
  }
  return {};
}


struct easy_english_bible_walker: xml_tree_walker
{
  string text;

  virtual bool for_each (xml_node& node)
  {
    // Handle this node if it's a text node.
    if (node.type() == pugi::node_pcdata) {
      string fragment = node.value();
      fragment = filter_string_str_replace ("\n", " ", fragment);
      text.append (fragment);
    }
    // Continue parsing.
    return true;
  }
};


// Get the slightly formatted of a passage of a Easy English Bible Commentary.
string resource_logic_easy_english_bible_get (int book, int chapter, int verse)
{
  // First handle the easier part:
  // The client will fetch the data from the Cloud.
#ifdef HAVE_CLIENT
  string resource = resource_logic_easy_english_bible_name ();
  return resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif

  // Now handle the Cloud part: Fetch and parse the text.
#ifdef HAVE_CLOUD

  // The combined text result taken from the commentary.
  string result;

  // This resource may have one or more commentaries per book.
  // This means that the URLs may be one or more.
  vector <string> urls;
  {
    vector <string> pages = resource_logic_easy_english_bible_pages (book, chapter);
    for (auto page : pages) {
      // Example URL: https://www.easyenglish.bible/bible-commentary/matthew-lbw.htm
      string url = "https://www.easyenglish.bible/bible-commentary/";
      // Handle the special URL for the Psalms:
      if (book == 19) url = "https://www.easyenglish.bible/psalms/";
      url.append (page);
      url.append (".htm");
      urls.push_back(url);
    }
  }
  
  // Handle the possible URLs.
  for (auto url : urls) {
    
    // Flag for whether the current paragraph contains the desired passage.
    bool near_passage = false;
    // Flag for whether the current paragraph is for the exact verse number.
    bool at_passage = false;
    
    // Get the html from the server.
    string error;
    string html = resource_logic_web_or_cache_get (url, error);

    // It appears that the html from this website is not well-formed.
    // It cannot be loaded as an XML document without errors and missing text.
    // Therefore the html is tidied first.
    html = filter_string_tidy_invalid_html (html);

    // Parse the html into a DOM.
    string verse_s = convert_to_string (verse);
    xml_document document;
    document.load_string (html.c_str());

    // The document has one main div like this:
    // <div class="Section1">
    // Or: <div class="WordSection1"> like in Exodus.
    // That secion has many children all containing one paragraph of text.
    string selector = "//div[contains(@class, 'Section1')]";
    xpath_node xnode = document.select_node(selector.c_str());
    xml_node div_node = xnode.node();

    // Iterate over the paragraphs of text and process them.
    for (auto paragraph_node : div_node.children()) {

      // Assemble the text by iterating over all child text nodes.
      easy_english_bible_walker tree_walker;
      paragraph_node.traverse(tree_walker);

      // Clean the text and skip empty text.
      string paragraph = filter_string_trim (tree_walker.text);
      if (paragraph.empty()) continue;

      // Check for whether the chapter indicates that
      // the parser is now at the chapter that may contain the passage to look for.

      // Special case for the Psalms: There's one chapter per URL.
      // So it's always "near the passage" so to say.
      // Same for the one-chapter books.
      if ((book == 19) || (book == 57) || (book == 63) || (book == 64) || (book == 65)) {
        near_passage = true;

      } else {
        
        // Handle e.g. "Chapter 1" all on one line.
        if (resource_logic_easy_english_bible_handle_chapter_heading (paragraph, chapter, near_passage, at_passage)) {
          // It was handled, skip any further processing of this line.
          continue;
        }

        // Handle a heading that contains the passages it covers.
        if (resource_logic_easy_english_bible_handle_passage_heading (paragraph, chapter, verse, near_passage, at_passage)) {
          // It was handled, skip any further processing of this line.
          continue;
        }

      }
      
      // Handle the situation that this paragraph contains the passage to be looked for.
      if (near_passage) {

        // Check whether the parser is right at the desired passage.
        resource_logic_easy_english_bible_handle_verse_marker (paragraph, verse, at_passage);
        
        // If at the correct verse, check whether the paragraph starts with "v".
        // Like in "v20".
        // Such text is canonical text, and is not part of the commentary.
        if (at_passage) {
          if (paragraph.find ("v") == 0) at_passage = false;
        }
        
        // Another situation occurs in Judges and likely other books.
        // That book does not have paragraphs starting with "Verse".
        // In that case the verse is found if e.g. "v12" is in the text.
        // But in Psalms this confuses things again.
        if (!at_passage) {
          if (book != 19) {
            string tag = "v" + convert_to_string(verse);
            if (paragraph.find(tag) != string::npos) {
              at_passage = true;
              continue;
            }
          }
        }

        // If still at the correct verse, add the paragraph text.
        if (at_passage) {
          result.append ("<p>");
          result.append (paragraph);
          result.append ("</p>");
        }
      }
    }
  }

  // Done.
  return result;

#endif
}



bool resource_logic_easy_english_bible_handle_chapter_heading (const string & paragraph,
                                                               int chapter,
                                                               bool & near_passage,
                                                               bool & at_passage)
{
  // Handle one type of commentary structure.
  // An example is Genesis.
  // That book is divided up into chapters.
  // It has headings like "Chapter 1", and so on.
  // There may be paragraph with normal text that also have "Chapter " at the start.
  // Skip those as these are not the desired markers.
  // In the book of Proverbs, the chapters are marked like this:
  // Proverbs chapter 30
  // The above is 19 characters long, so set the limit slightly higher.
  if (paragraph.length() <= 25) {
    if (paragraph.find ("Proverbs chapter") == 0) {
      string tag = "Proverbs chapter " + convert_to_string(chapter);
      near_passage = (paragraph == tag);
      if (near_passage) {
        // If this paragraph contains a passage, it likely is a heading.
        // Skip those, do not include them.
        // And clear any flag that the exact current verse is there.
        at_passage = false;
        // The heading was handled.
        return true;
      }
    }
    if (paragraph.find ("Chapter ") == 0) {
      string tag = "Chapter " + convert_to_string(chapter);
      near_passage = (paragraph == tag);
      if (near_passage) {
        // If this paragraph contains a passage, it likely is a heading.
        // Skip those, do not include them.
        // And clear any flag that the exact current verse is there.
        at_passage = false;
        // The heading was handled.
        return true;
      }
    }
  }
  // The heading was not handled.
  return false;
}


bool resource_logic_easy_english_bible_handle_passage_heading (const string & paragraph,
                                                               int chapter, int verse,
                                                               bool & near_passage,
                                                               bool & at_passage)
{
  // A heading contains information about chapter(s) and verses.
  // Possible formats:
  // 1 Jesus, son of God, greater than all 1:1-2:18
  // The greatness of the son 1:1-3
  // So look for the last complete word in the line.
  // That last word consists of digits, one or two colons, and one hyphen.
  // No other characters are in that last word.
  size_t space_pos = paragraph.find_last_of(" ");
  if (space_pos == string::npos) return false;
  string last_word = paragraph.substr(space_pos + 1);
  
  // There's also situations that the last bit is surrounded by round bracket.
  // Example: Greetings from Paul to Timothy (1:1-2)
  last_word = filter_string_str_replace ("(", string(), last_word);
  last_word = filter_string_str_replace (")", string(), last_word);
  
  // Look for the first colon and the first hyphen.
  // This will obtain the starting chapter and verse numbers.
  size_t colon_pos = last_word.find(":");
  if (colon_pos == string::npos) return false;
  size_t hyphen_pos = last_word.find ("-");
  if (hyphen_pos == string::npos) return false;
  string ch_fragment = last_word.substr(0, colon_pos);
  int starting_chapter = convert_to_int(ch_fragment);
  string check = convert_to_string(starting_chapter);
  if (ch_fragment != check) return false;

  // Look for the first hyphen.
  // This will provide the starting verse number.
  string vs_fragment = last_word.substr(colon_pos + 1, hyphen_pos - colon_pos - 1);
  int starting_verse = convert_to_int(vs_fragment);
  check = convert_to_string(starting_verse);
  if (vs_fragment != check) return false;
  last_word.erase (0, hyphen_pos + 1);
  
  // A second chapter number can be given, or can be omitted.
  // In this example it is given:
  // 1:1-2:18
  // If a second colon can be found in the fragment,
  // it means that the figure following the hyphen and before the colon,
  // is that last chapter number.
  int ending_chapter = starting_chapter;
  colon_pos = last_word.find(":");
  if (colon_pos != string::npos) {
    string ch_fragment = last_word.substr(0, colon_pos);
    ending_chapter = convert_to_int(ch_fragment);
    string check = convert_to_string(ending_chapter);
    if (ch_fragment != check) return false;
    last_word.erase(0, colon_pos + 1);
  }

  // The last bit of the fragment will now be the second verse number.
  int ending_verse = convert_to_int(last_word);
  check = convert_to_string(ending_verse);
  if (check != last_word) return false;

  // Set a flag if the passage that is to be obtained is within the current lines of text.
  near_passage = ((chapter >= starting_chapter)
                  && (chapter <= ending_chapter)
                  && (verse >= starting_verse)
                  && (verse <= ending_verse));

  // If this paragraph contains a passage, it likely is a heading.
  // Skip those, do not include them.
  // Clear any flag that the exact current verse is there.
  at_passage = false;

  // Heading parsed.
  return true;
}


void resource_logic_easy_english_bible_handle_verse_marker (const string & paragraph,
                                                            int verse,
                                                            bool & at_passage)
{
  // If the paragraph starts with "Verse" or with "Verses",
  // then investigate whether this paragraph belongs to the current verse,
  // that it is now looking for.
  // This way of working makes it possible to handle a situation
  // where a verse contains multiple paragraphs.
  // Because the flag for whether the current verse is found,
  // will only be affected by paragraph starting with this "Verse" tag.
  if (paragraph.find("Verse") != 0) return;
  
  // Look for e.g. "Verse 13 " at the start of the paragraph.
  // The space at the end is to prevent it from matching more verses.
  // Like when looking for "Verse 1", it would be found in "Verse 10" too.
  // Hence the space.
  string tag = "Verse " + convert_to_string(verse) + " ";
  at_passage = paragraph.find(tag) == 0;
  // If it's at the passage, then it's done parsing.
  if (at_passage) return;

  // If no verse is found yet, look for e.g. "Verse 13:".
  tag = "Verse " + convert_to_string(verse) + ":";
  at_passage = paragraph.find(tag) == 0;
  //If it's at the passage, then it's done parsing.
  if (at_passage) return;

  // If no verse is found yet, look for the same tag but without the space at the end.
  // Then the entire paragraph should consist of this tag.
  // This occurs in Genesis 1 for example.
  tag = "Verse " + convert_to_string(verse);
  at_passage = (paragraph == tag);
  //If it's at the passage, then it's done parsing.
  if (at_passage) return;

  // If no verse is found yet, then look for a variation of the markup that occurs too.
  // Example: Verse 8-11 ...
  tag = "Verse ";
  if (paragraph.find(tag) == 0) {
    string fragment = paragraph.substr(tag.size(), 10);
    vector<string> bits = filter_string_explode(fragment, '-');
    if (bits.size() >= 2) {
      int begin = convert_to_int(bits[0]);
      int end = convert_to_int(bits[1]);
      at_passage = (verse >= begin) && (verse <= end);
    }
  }
  
  // If no verse is found yet, then look for the type of markup as below.
  // Example: Verses 15-17 ...
  // Example: Verses 3 4: ...
  // Example: Verses 3 – 4: ...
  // The above case is from content that is in an unknown encoding:
  // $ file -bi out.txt
  // text/html; charset=unknown-8bit
  // So the special hyphen (–) is rendered as a space or another character.
  // So use another technique: To iterate over the string to find numeric characters.
  // And to dedice the starting and ending verse from that.
  tag = "Verses ";
  if (paragraph.find(tag) == 0) {
    string fragment = paragraph.substr(tag.size(), 10);
    vector <int> digits;
    int digit = 0;
    for (size_t i = 0; i < fragment.size(); i++) {
      int d = convert_to_int(fragment.substr(i, 1));
      if (d) {
        digit *= 10;
        digit += d;
      } else {
        if (digit) digits.push_back(digit);
        digit = 0;
      }
    }
    if (digits.size() >= 2) {
      at_passage = (verse >= digits[0]) && (verse <= digits[1]);
    }
  }
  if (at_passage) return;
}


bool resource_logic_is_bible (string resource)
{
  Database_Bibles database_bibles;
  vector <string> names = database_bibles.getBibles ();
  return in_array (resource, names);
}


bool resource_logic_is_usfm (string resource)
{
  vector <string> names;
#ifdef HAVE_CLIENT
  names = client_logic_usfm_resources_get ();
#else
  Database_UsfmResources database_usfmresources;
  names = database_usfmresources.getResources ();
#endif
  return in_array (resource, names);
}


bool resource_logic_is_external (string resource)
{
  vector <string> names = resource_external_names ();
  return in_array (resource, names);
}


bool resource_logic_is_image (string resource)
{
  Database_ImageResources database_imageresources;
  vector <string> names = database_imageresources.names ();
  return in_array (resource, names);
}


bool resource_logic_is_lexicon (string resource)
{
  vector <string> names = lexicon_logic_resource_names ();
  return in_array (resource, names);
}


bool resource_logic_is_sword (string resource)
{
  string module = sword_logic_get_remote_module (resource);
  string source = sword_logic_get_source (resource);
  return (!source.empty () && !module.empty ());
}


bool resource_logic_is_divider (string resource)
{
  if (resource == resource_logic_yellow_divider ()) return true;
  if (resource == resource_logic_green_divider ()) return true;
  if (resource == resource_logic_blue_divider ()) return true;
  if (resource == resource_logic_violet_divider ()) return true;
  if (resource == resource_logic_red_divider ()) return true;
  if (resource == resource_logic_orange_divider ()) return true;
  if (resource.find (resource_logic_rich_divider ()) == 0) return true;
  return false;
}


bool resource_logic_is_biblegateway (string resource)
{
  vector <string> names = resource_logic_bible_gateway_module_list_get ();
  return in_array (resource, names);
}


bool resource_logic_is_studylight (string resource)
{
  vector <string> names = resource_logic_study_light_module_list_get ();
  return in_array (resource, names);
}


bool resource_logic_is_comparative (string resource)
{
  return resource_logic_parse_comparative_resource(resource);
}


string resource_logic_comparative_resource ()
{
  return "Comparative ";
}


bool resource_logic_parse_comparative_resource (string input,
                                                string * title,
                                                string * base,
                                                string * update,
                                                string * remove,
                                                string * replace,
                                                bool * diacritics,
                                                bool * casefold,
                                                bool * cache)
{
  // The definite check whether this is a comparative resource
  // is to check that "Comparative " is the first part of the input.
  if (input.find(resource_logic_comparative_resource()) != 0) return false;

  // Do a forgiving parsing of the properties of this resource.
  if (title) title->clear();
  if (base) base->clear();
  if (update) update->clear();
  if (remove) remove->clear();
  if (replace) replace->clear();
  if (diacritics) * diacritics = false;
  if (casefold) * casefold = false;
  if (cache) * cache = false;
  vector <string> bits = filter_string_explode(input, '|');
  if (bits.size() > 0) if (title) title->assign (bits[0]);
  if (bits.size() > 1) if (base) base->assign(bits[1]);
  if (bits.size() > 2) if (update) update->assign(bits[2]);
  if (bits.size() > 3) if (remove) remove->assign(bits[3]);
  if (bits.size() > 4) if (replace) replace->assign(bits[4]);
  if (bits.size() > 5) if (diacritics) * diacritics = convert_to_bool(bits[5]);
  if (bits.size() > 6) if (casefold) * casefold = convert_to_bool(bits[6]);
  if (bits.size() > 7) if (cache) * cache = convert_to_bool(bits[7]);

  // Done.
  return true;
}


string resource_logic_assemble_comparative_resource (string title,
                                                     string base,
                                                     string update,
                                                     string remove,
                                                     string replace,
                                                     bool diacritics,
                                                     bool casefold,
                                                     bool cache)
{
  // Check whether the "Comparative " flag already is included in the given $title.
  size_t pos = title.find (resource_logic_comparative_resource ());
  if (pos != string::npos) {
    title.erase (pos, resource_logic_comparative_resource ().length());
  }
  // Ensure the "Comparative " flag is always included right at the start.
  vector <string> bits = {resource_logic_comparative_resource() + title, base, update, remove, replace, convert_to_true_false(diacritics), convert_to_true_false(casefold), convert_to_true_false(cache)};
  return filter_string_implode(bits, "|");
}

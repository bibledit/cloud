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
#include <filter/google.h>
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
#include <pugixml/utils.h>


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


std::vector <std::string> resource_logic_get_names (Webserver_Request& webserver_request, bool bibles_only)
{
  std::vector <std::string> names {};
  
  // Bibles the user has read access to.
  std::vector <std::string> bibles = access_bible::bibles (webserver_request);
  names.insert (names.end(), bibles.begin (), bibles.end());
  
  // USFM resources.
  Database_UsfmResources database_usfmresources {};
  std::vector <std::string> usfm_resources = database_usfmresources.getResources ();
  names.insert (names.end(), usfm_resources.begin(), usfm_resources.end());
  
  // External resources.
  std::vector <std::string> external_resources = resource_external_names ();
  names.insert (names.end (), external_resources.begin(), external_resources.end());
  
  // Image resources.
  if (!bibles_only) {
    Database_ImageResources database_imageresources {};
    std::vector <std::string> image_resources = database_imageresources.names ();
    names.insert (names.end (), image_resources.begin(), image_resources.end());
  }
  
  // Lexicon resources.
  if (!bibles_only) {
    std::vector <std::string> lexicon_resources = lexicon_logic_resource_names ();
    names.insert (names.end (), lexicon_resources.begin(), lexicon_resources.end());
  }
  
  // SWORD resources.
  std::vector <std::string> sword_resources = sword_logic_get_available ();
  names.insert (names.end (), sword_resources.begin(), sword_resources.end());
  
  // Bible Gateway resources.
  std::vector <std::string> bible_gateway_resources = resource_logic_bible_gateway_module_list_get ();
  names.insert (names.end (), bible_gateway_resources.begin(), bible_gateway_resources.end());
  
  sort (names.begin(), names.end());
  
  return names;
}


std::string resource_logic_get_html (Webserver_Request& webserver_request,
                                     std::string resource, int book, int chapter, int verse,
                                     bool add_verse_numbers)
{
  // Determine the type of the resource.
  bool is_bible = resource_logic_is_bible (resource);
  bool is_usfm = resource_logic_is_usfm (resource);
  bool is_external = resource_logic_is_external (resource);
  bool is_image = resource_logic_is_image (resource);
  bool is_lexicon = resource_logic_is_lexicon (resource);
  bool is_sword = resource_logic_is_sword (resource);
  bool is_bible_gateway = resource_logic_is_biblegateway (resource);
  bool is_study_light = resource_logic_is_studylight (resource);
  bool is_comparative = resource_logic_is_comparative (resource);
  bool is_translated = resource_logic_is_translated(resource);

  // Handle a comparative resource.
  // This type of resource is special.
  // It is not one resource, but made out of two resources.
  // It fetches data from two resources and combines that into one.
  if (is_comparative) {
#ifdef HAVE_CLOUD
    return resource_logic_cloud_get_comparison (webserver_request, resource, book, chapter, verse, add_verse_numbers);
#endif
#ifdef HAVE_CLIENT
    return resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif
  }

  // Handle a translated resource.
  // This type of resource is special.
  // It consists of any of the other types of resources, as the base resource.
  // It gets that data, and then has that translated.
  if (is_translated) {
#ifdef HAVE_CLOUD
    return resource_logic_cloud_get_translation (webserver_request, resource, book, chapter, verse, add_verse_numbers);
#endif
#ifdef HAVE_CLIENT
    return resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif
  }
  
  Database_Mappings database_mappings;

  // Retrieve versification system of the active Bible.
  std::string bible = webserver_request.database_config_user ()->getBible ();
  std::string bible_versification = database::config::bible::get_versification_system (bible);

  // Determine the versification system of the current resource.
  std::string resource_versification;
  if (is_bible || is_usfm) {
    resource_versification = database::config::bible::get_versification_system (bible);
  } else if (is_external) {
    resource_versification = resource_external_mapping (resource);
  } else if (is_image) {
  } else if (is_lexicon) {
    resource_versification = database_mappings.original ();
    if (resource == KJV_LEXICON_NAME) resource_versification = filter::strings::english ();
  } else if (is_sword) {
    resource_versification = filter::strings::english ();
  } else if (is_bible_gateway) {
    resource_versification = filter::strings::english ();
  } else if (is_study_light) {
    resource_versification = filter::strings::english ();
  } else {
  }

  // If the resource versification system differs from the Bible's versification system,
  // map the focused verse of the Bible to a verse in the Resource.
  // There are resources without versification system: Do nothing about them.
  std::vector <Passage> passages;
  if ((bible_versification != resource_versification) && !resource_versification.empty ()) {
    passages = database_mappings.translate (bible_versification, resource_versification, book, chapter, verse);
  } else {
    passages.push_back (Passage ("", book, chapter, std::to_string (verse)));
  }

  // If there's been a mapping, the resource should include the verse number for clarity.
  if (passages.size () != 1) add_verse_numbers = true;
  for (auto passage : passages) {
    if (verse != filter::strings::convert_to_int (passage.m_verse)) {
      add_verse_numbers = true;
    }
  }
  
  // Flag for whether to add the full passage (e.g. Matthew 1:1) to the text of that passage.
  bool add_passages_in_full = false;

  // Deal with user's preference whether to include related passages.
  if (webserver_request.database_config_user ()->getIncludeRelatedPassages ()) {
    
    // Take the Bible's active passage and mapping, and translate that to the original mapping.
    std::vector <Passage> related_passages = database_mappings.translate (bible_versification, database_mappings.original (), book, chapter, verse);
    
    // Look for related passages.
    related_passages = related_logic_get_verses (related_passages);
    
    add_passages_in_full = !related_passages.empty ();
    
    // If there's any related passages, map them to the resource's versification system.
    if (!related_passages.empty ()) {
      if (!resource_versification.empty ()) {
        if (resource_versification != database_mappings.original ()) {
          passages.clear ();
          for (auto & related_passage : related_passages) {
            std::vector <Passage> mapped_passages = database_mappings.translate (database_mappings.original (), resource_versification, related_passage.m_book, related_passage.m_chapter, filter::strings::convert_to_int (related_passage.m_verse));
            passages.insert (passages.end (), mapped_passages.begin (), mapped_passages.end ());
          }
        }
      }
    }
  }

  std::string html {};

  for (auto passage : passages) {
    std::string possible_included_passage;
    if (add_verse_numbers) possible_included_passage = passage.m_verse + " ";
    if (add_passages_in_full) possible_included_passage = filter_passage_display (passage.m_book, passage.m_chapter, passage.m_verse) + " ";
    if (is_image) possible_included_passage.clear ();
    html.append (possible_included_passage);
    html.append (resource_logic_get_verse (webserver_request, resource, passage.m_book, passage.m_chapter, filter::strings::convert_to_int (passage.m_verse)));
  }
  
  return html;
}


// This is the most basic version that fetches the text of a $resource.
// It works on server and on client.
// It uses the cache.
std::string resource_logic_get_verse (Webserver_Request& webserver_request, std::string resource, int book, int chapter, int verse)
{
  std::string data {};

  // Determine the type of the current resource.
  bool isBible = resource_logic_is_bible (resource);
  Database_UsfmResources database_usfmresources;
  std::vector <std::string> local_usfms {database_usfmresources.getResources ()};
  bool isLocalUsfm = in_array (resource, local_usfms);
  std::vector <std::string> remote_usfms {};
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
    std::string chapter_usfm {};
    if (isBible) 
      chapter_usfm = database::bibles::get_chapter (resource, book, chapter);
    if (isLocalUsfm) chapter_usfm = database_usfmresources.getUsfm (resource, book, chapter);
    std::string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
    std::string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (resource);
    filter_text.html_text_standard = new HtmlText ("");
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
    const std::vector <std::string> images = database_imageresources.get (resource, book, chapter, verse);
    for (const auto& image : images) {
      data.append ("<div><img src=\"/resource/imagefetch?name=" + resource + "&image=" + image + "\" alt=\"Image resource\" style=\"width:100%\"></div>");
    }
  } else if (isLexicon) {
    data = lexicon_logic_get_html (webserver_request, resource, book, chapter, verse);
  } else if (isSword) {
    const std::string sword_module = sword_logic_get_remote_module (resource);
    const std::string sword_source = sword_logic_get_source (resource);
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
    std::string fragment = "p class=\"";
    size_t pos = data.find (fragment);
    if (pos != std::string::npos) {
      size_t pos2 = data.find ("\"", pos + fragment.length () + 1);
      if (pos2 != std::string::npos) {
        data.erase (pos + 1, pos2 - pos);
      }
    }
  }
  
  // NET Bible updates.
  data = filter::strings::replace ("<span class=\"s ", "<span class=\"", data);

  return data;
}


std::string resource_logic_cloud_get_comparison (Webserver_Request& webserver_request,
                                                 std::string resource, int book, int chapter, int verse,
                                                 bool add_verse_numbers)
{
  // This function gets passed the resource title only.
  // So get all resources and look for the one with this title.
  // And then get the additional properties belonging to this resource.
  std::string title, base, update, remove, replace;
  bool diacritics = false, casefold = false;
  std::vector <std::string> resources = database::config::general::get_comparative_resources ();
  for (const auto& s : resources) {
    resource_logic_parse_comparative_resource (s, &title, &base, &update, &remove, &replace, &diacritics, &casefold);
    if (title == resource) break;
  }

  // Get the html of both resources to compare.
  base = resource_logic_get_html (webserver_request, base, book, chapter, verse, add_verse_numbers);
  update = resource_logic_get_html (webserver_request, update, book, chapter, verse, add_verse_numbers);
  
  // Clean all html elements away from the text to get a better and cleaner comparison.
  base = filter::strings::html2text (base);
  update = filter::strings::html2text(update);
  
  // It has been seen that one resource had a normal space, and the updated resource a non-breaking space.
  // In such a situation there was highlighting of differences between the two resources.
  // But with the eye one could not find any differences.
  // So the question would come up like:
  // Why does it highlight a difference while there seems to be no difference?
  // The solution is to convert types of non-breaking spaces to normal ones.
  base = filter::strings::any_space_to_standard_space (base);
  update = filter::strings::any_space_to_standard_space(update);

  // If characters are given to remove from the resources, handle that situation now.
  if (!remove.empty()) {
    const std::vector<std::string> bits = filter::strings::explode(remove, ' ');
    for (const auto& rem : bits) {
      if (rem.empty()) continue;
      base = filter::strings::replace(rem, "", base);
      update = filter::strings::replace(rem, "", update);
    }
  }
  
  // If search-and-replace sets are given to be applied to the resources, handle that now.
  if (!replace.empty()) {
    std::vector<std::string> search_replace_sets = filter::strings::explode(replace, ' ');
    for (auto search_replace_set : search_replace_sets) {
      std::vector <std::string> search_replace = filter::strings::explode(search_replace_set, '=');
      if (search_replace.size() == 2) {
        std::string search_item = search_replace[0];
        if (search_item.empty()) continue;
        std::string replace_item = search_replace[1];
        base = filter::strings::replace(search_item, replace_item, base);
        update = filter::strings::replace(search_item, replace_item, update);
      }
    }
  }

  // When showing the difference between two Greek New Testaments,
  // one with diacritics and the other without diacritics.
  // there's a lot of flagging of difference, just because of the diacritics.
  // To handle such a situation, remove the diacritics.
  // Similarly to not mark small letters versus capitals as a difference, do case folding.
#ifdef HAVE_ICU
  base = filter::strings::icu_string_normalize (base, diacritics, casefold);
  update = filter::strings::icu_string_normalize (update, diacritics, casefold);
#endif

  // Find the differences.
  std::string html = filter_diff_diff (base, update);
  return html;
}


std::string resource_logic_cloud_get_translation (Webserver_Request& webserver_request,
                                                  const std::string& resource,
                                                  int book, int chapter, int verse,
                                                  bool add_verse_numbers)
{
  // This function gets passed the resource title only.
  // So get all defined translated resources and look for the one with this title.
  // And then get the additional properties belonging to this resource.
  std::string title, original_resource, source_language, target_language;
  std::vector <std::string> resources = database::config::general::get_translated_resources ();
  for (const auto& input : resources) {
    resource_logic_parse_translated_resource (input, &title, &original_resource, &source_language, &target_language);
    if (title == resource) break;
  }
  
  // Get the html of the resources to be translated.
  std::string original_text = resource_logic_get_html (webserver_request, original_resource, book, chapter, verse, add_verse_numbers);
  // Clean all html elements away from the text to get a better and cleaner translation.
  original_text = filter::strings::html2text (original_text);
  
  // If the original text is empty, do not even send it to Google Translate, for saving resources.
  if (original_text.empty()) return std::string();

  // Run it through Google Translate.
  auto [ translation_success, translated_text, translation_error] = filter::google::translate (original_text, source_language.c_str(), target_language.c_str());

  // Done.
  if (translation_success) return translated_text;
  Database_Logs::log (translation_error);
  return "Failed to translate";
}


// This runs on the server.
// It gets the html or text contents for a $resource for serving it to a client.
std::string resource_logic_get_contents_for_client (std::string resource, int book, int chapter, int verse)
{
  // Determine the type of the current resource.
  bool is_external = resource_logic_is_external (resource);
  bool is_usfm = resource_logic_is_usfm (resource);
  bool is_sword = resource_logic_is_sword (resource);
  bool is_bible_gateway = resource_logic_is_biblegateway (resource);
  bool is_study_light = resource_logic_is_studylight (resource);
  bool is_comparative = resource_logic_is_comparative (resource);
  bool is_translated = resource_logic_is_translated(resource);

  if (is_external) {
    // The server fetches it from the web.
    return resource_external_cloud_fetch_cache_extract (resource, book, chapter, verse);
  }
  
  if (is_usfm) {
    // Fetch from database and convert to html.
    Database_UsfmResources database_usfmresources;
    std::string chapter_usfm = database_usfmresources.getUsfm (resource, book, chapter);
    std::string verse_usfm = filter::usfm::get_verse_text (chapter_usfm, verse);
    std::string stylesheet = styles_logic_standard_sheet ();
    Filter_Text filter_text = Filter_Text (resource);
    filter_text.html_text_standard = new HtmlText ("");
    filter_text.add_usfm_code (verse_usfm);
    filter_text.run (stylesheet);
    return filter_text.html_text_standard->get_inner_html ();
  }
  
  if (is_sword) {
    // Fetch it from a SWORD module.
    std::string sword_module = sword_logic_get_remote_module (resource);
    std::string sword_source = sword_logic_get_source (resource);
    return sword_logic_get_text (sword_source, sword_module, book, chapter, verse);
  }

  if (is_bible_gateway) {
    // The server fetches it from the web.
    return resource_logic_bible_gateway_get (resource, book, chapter, verse);
  }

  if (is_study_light) {
    // The server fetches it from the web.
    return resource_logic_study_light_get (resource, book, chapter, verse);
  }

  if (is_comparative) {
    // Handle a comparative resource.
    // This type of resource is special.
    // It is not one resource, but made out of two resources.
    // It fetches data from two resources and combines that into one.
    Webserver_Request webserver_request {};
    return resource_logic_cloud_get_comparison (webserver_request, resource, book, chapter, verse, false);
  }
  
  if (is_translated) {
    // Handle a translated resource.
    // This passes the resource title only
    Webserver_Request webserver_request;
    return resource_logic_cloud_get_translation (webserver_request, resource, book, chapter, verse, false);
  }
  
  // Nothing found.
  return translate ("Bibledit Cloud could not locate this resource");
}


// The client runs this function to fetch a general resource $name from the Cloud,
// or from its local cache,
// and to update the local cache with the fetched content, if needed,
// and to return the requested content.
std::string resource_logic_client_fetch_cache_from_cloud (std::string resource, int book, int chapter, int verse)
{
  // Whether the client should cache this resource.
  bool cache = !in_array(resource, client_logic_no_cache_resources_get ());
  
  // Ensure that the cache for this resource exists on the client.
  if (cache && !database::cache::sql::exists (resource, book)) {
    database::cache::sql::create (resource, book);
  }
  
  // If content is to be cached and the content exists in the cache, return that content.
  if (cache && database::cache::sql::exists (resource, book, chapter, verse)) {
    return database::cache::sql::retrieve (resource, book, chapter, verse);
  }
  
  // Fetch this resource from Bibledit Cloud.
  std::string address = database::config::general::get_server_address ();
  int port = database::config::general::get_server_port ();
  if (!client_logic_client_enabled ()) {
    // If the client has not been connected to a cloud instance,
    // fetch the resource from the Bibledit Cloud demo.
    address = demo_address ();
    port = demo_port ();
  }
  
  std::string url = client_logic_url (address, port, sync_resources_url ());
  url = filter_url_build_http_query (url, "r", filter_url_urlencode (resource));
  url = filter_url_build_http_query (url, "b", std::to_string (book));
  url = filter_url_build_http_query (url, "c", std::to_string (chapter));
  url = filter_url_build_http_query (url, "v", std::to_string (verse));
  std::string error {};
  std::string content = filter_url_http_get (url, error, false);
  
  // Cache the content under circumstances.
  if (cache) {
    if (database::cache::can_cache (error, content)) {
      database::cache::sql::cache (resource, book, chapter, verse, content);
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
void resource_logic_import_images (std::string resource, std::string path)
{
  Database_ImageResources database_imageresources;
  
  Database_Logs::log ("Importing: " + filter_url_basename (path));
  
  // To begin with, add the path to the main file to the list of paths to be processed.
  std::vector <std::string> paths = {path};
  
  while (!paths.empty ()) {
  
    // Take and remove the first path from the container.
    path = paths[0];
    paths.erase (paths.begin());
    std::string basename = filter_url_basename (path);
    std::string extension = filter_url_get_extension (path);
    extension = filter::strings::unicode_string_casefold (extension);

    if (extension == "pdf") {
      
      Database_Logs::log ("Processing PDF: " + basename);
      
      // Retrieve PDF information.
      filter_shell_run ("", "pdfinfo", {path}, nullptr, nullptr);

      // Convert the PDF file to separate images.
      std::string folder = filter_url_tempfile ();
      filter_url_mkdir (folder);
      filter_shell_run (folder, "pdftocairo", {"-jpeg", path}, nullptr, nullptr);
      // Add the images to the ones to be processed.
      filter_url_recursive_scandir (folder, paths);
      
    } else if (filter_archive_is_archive (path)) {
      
      Database_Logs::log ("Unpacking archive: " + basename);
      std::string folder = filter_archive_uncompress (path);
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


std::string resource_logic_yellow_divider ()
{
  return "Yellow Divider";
}


std::string resource_logic_green_divider ()
{
  return "Green Divider";
}


std::string resource_logic_blue_divider ()
{
  return "Blue Divider";
}


std::string resource_logic_violet_divider ()
{
  return "Violet Divider";
}


std::string resource_logic_red_divider ()
{
  return "Red Divider";
}


std::string resource_logic_orange_divider ()
{
  return "Orange Divider";
}


std::string resource_logic_rich_divider ()
{
  return "Rich Divider";
}


bool resource_logic_parse_rich_divider (std::string input, std::string& title, std::string& link, std::string& foreground, std::string& background)
{
  title.clear();
  link.clear();
  foreground.clear();
  background.clear();
  std::vector <std::string> bits = filter::strings::explode(input, '|');
  if (bits.size() != 5) return false;
  if (bits[0] != resource_logic_rich_divider()) return false;
  title = bits[1];
  link = bits[2];
  foreground = bits[3];
  background = bits[4];
  return true;
}


std::string resource_logic_assemble_rich_divider (std::string title, std::string link, std::string foreground, std::string background)
{
  std::vector <std::string> bits = {resource_logic_rich_divider(), title, link, foreground, background};
  return filter::strings::implode(bits, "|");
}


std::string resource_logic_get_divider (std::string resource)
{
  std::string title {};
  std::string link {};
  std::string foreground {};
  std::string background {};
  if (resource_logic_parse_rich_divider (resource, title, link, foreground, background)) {
    // Trim whitespace.
    title = filter::strings::trim(title);
    link = filter::strings::trim(link);
    // Render a rich divider.
    if (title.empty ()) title = link;
    // The $ influences the resource's embedding through Javascript.
    std::string html = "$";
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
    std::vector <std::string> bits = filter::strings::explode (resource, ' ');
    std::string colour = filter::strings::unicode_string_casefold (bits [0]);
    // The $ influences the resource's embedding through Javascript.
    std::string html = R"($<div class="divider" style="background-color:)" + colour + R"(">&nbsp;</div>)";
    return html;
  }

}


// In Cloud mode, this function wraps around http GET.
// It fetches existing content from the cache, and caches new content.
std::string resource_logic_web_or_cache_get (std::string url, std::string& error)
{
#ifndef HAVE_CLIENT
  // On the Cloud, check if the URL is in the cache.
  if (database::cache::file::exists (url)) {
    return database::cache::file::get (url);
  }
#endif

  // Fetch the URL from the network.
  error.clear ();
  std::string html = filter_url_http_get (url, error, false);

#ifdef HAVE_CLOUD
  // In the Cloud, cache the response based on certain criteria.
  const bool cache = database::cache::can_cache (error, html);
  if (cache) {
    database::cache::file::put (url, html);
  }
#endif

  // Done.
  return html;
}


// Returns the page type for the resource selector.
std::string resource_logic_selector_page (Webserver_Request& webserver_request)
{
  std::string page {webserver_request.query["page"]};
  return page;
}


// Returns the page which called the resource selector.
std::string resource_logic_selector_caller (Webserver_Request& webserver_request)
{
  std::string caller = resource_logic_selector_page (webserver_request);
  if (caller == "view") caller = "organize";
  if (caller == "consistency") caller = "../consistency/index";
  if (caller == "print") caller = "print";
  return caller;
}


std::string resource_logic_default_user_url ()
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
  
  // Get the signatures of the resources to cache.
  std::vector <std::string> signatures = database::config::general::get_resources_to_cache ();
  // If there's nothing to cache, bail out.
  if (signatures.empty ()) return;

  // A signature is the resource title, then a space, and then the book number.
  // Remove this signature and store the remainder back into the configuration.
  std::string signature = signatures [0];
  signatures.erase (signatures.begin ());
  database::config::general::set_resources_to_cache (signatures);
  size_t pos = signature.find_last_of (" ");
  std::string resource = signature.substr (0, pos);
  int book = filter::strings::convert_to_int (signature.substr (pos++));
  std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  
  // Whether it's a SWORD module.
  std::string sword_module = sword_logic_get_remote_module (resource);
  std::string sword_source = sword_logic_get_source (resource);
  bool is_sword_module = (!sword_source.empty () && !sword_module.empty ());

  // In case of a  SWORD module, ensure it has been installed.
  if (is_sword_module) {
    std::vector <std::string> modules = sword_logic_get_installed ();
    bool already_installed = false;
    for (const auto& installed_module : modules) {
      if (installed_module.find ("[" + sword_module + "]") != std::string::npos) {
        already_installed = true;
      }
    }
    if (!already_installed) {
      sword_logic_install_module (sword_source, sword_module);
    }
  }
  
  // Database layout is per book: Create a database for this book.
  database::cache::sql::remove (resource, book);
  database::cache::sql::create (resource, book);
  
  Database_Versifications database_versifications;
  std::vector <int> chapters = database_versifications.getMaximumChapters (book);
  for (const auto& chapter : chapters) {

    Database_Logs::log ("Caching " + resource + " " + bookname + " " + std::to_string (chapter), Filter_Roles::consultant ());

    // The verse numbers in the chapter.
    std::vector <int> verses = database_versifications.getMaximumVerses (book, chapter);
    
    // In case of a SWORD module, fetch the texts of all verses in bulk.
    // This is because calling vfork once per verse to run diatheke stops working in the Cloud after some time.
    // Forking once per chapter is much better, also for the performance.
    std::map <int, std::string> sword_texts {};
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
      std::string html, error;
      do {
        // Fetch the resource data.
        if (is_sword_module) html = sword_texts [verse];
        else html = resource_logic_get_contents_for_client (resource, book, chapter, verse);
        // Check on errors.
        server_is_installing_module = (html == sword_logic_installing_module_text ());
        if (server_is_installing_module) {
          Database_Logs::log ("Waiting while installing SWORD module: " + resource);
        }
        server_is_updating = html.find ("... upgrading ...") != std::string::npos;
        if (server_is_updating) {
          Database_Logs::log ("Waiting while Cloud is upgrading itself");
        }
        // In case of server unavailability, wait a while, then try again.
        server_is_unavailable = server_is_installing_module || server_is_updating;
        if (server_is_unavailable) {
          std::this_thread::sleep_for (std::chrono::seconds (60));
          wait_iterations++;
        }
      } while (server_is_unavailable && (wait_iterations < 5));

      // Cache the verse data, even if there's an error.
      // If it were not cached at, say, Leviticus, then the caching mechanism,
      // after restart, would always continue from that same book, from Leviticus,
      // and never finish. Therefore something should be cached, even if it's an empty string.
      if (server_is_installing_module) html.clear ();
      database::cache::sql::cache (resource, book, chapter, verse, html);
    }
  }

  // Done.
  database::cache::sql::ready (resource, book, true);
  Database_Logs::log ("Completed caching " + resource + " " + bookname, Filter_Roles::consultant ());
  resource_logic_create_cache_running = false;
  
  // If there's another resource database waiting to be cached, schedule it for caching.
  if (!signatures.empty ()) tasks_logic_queue (task::cache_resources);
}


// Returns true if the resource can be installed locally.
bool resource_logic_can_cache (std::string resource)
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
std::string resource_logic_bible_gateway_module_list_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "bible_gateway_modules.txt"});
}


// Refreshes the list of resources available from BibleGateway.
std::string resource_logic_bible_gateway_module_list_refresh ()
{
  Database_Logs::log ("Refresh BibleGateway resources");
  std::string path = resource_logic_bible_gateway_module_list_path ();
  std::string error {};
  std::string html = filter_url_http_get ("https://www.biblegateway.com/versions/", error, false);
  if (error.empty ()) {
    std::vector <std::string> resources {};
    html =  filter::strings::html_get_element (html, "select");
    pugi::xml_document document;
    document.load_string (html.c_str());
    pugi::xml_node select_node = document.first_child ();
    for (pugi::xml_node option_node : select_node.children()) {
      std::string cls = option_node.attribute ("class").value ();
      if (cls == "lang") continue;
      if (cls == "spacer") continue;
      std::string name = option_node.text ().get ();
      resources.push_back (name);
    }
    filter_url_file_put_contents (path, filter::strings::implode (resources, "\n"));
    Database_Logs::log ("Modules: " + std::to_string (resources.size ()));
  } else {
    Database_Logs::log (error);
  }
  return error;
}


// Get the list of BibleGateway resources.
std::vector <std::string> resource_logic_bible_gateway_module_list_get ()
{
  std::string path = resource_logic_bible_gateway_module_list_path ();
  std::string contents = filter_url_file_get_contents (path);
  return filter::strings::explode (contents, '\n');
}


std::string resource_logic_bible_gateway_book (int book)
{
  // Map Bibledit books to biblegateway.com books as used at the web service.
  std::map <int, std::string> mapping = {
    std::pair (1, "Genesis"),
    std::pair (2, "Exodus"),
    std::pair (3, "Leviticus"),
    std::pair (4, "Numbers"),
    std::pair (5, "Deuteronomy"),
    std::pair (6, "Joshua"),
    std::pair (7, "Judges"),
    std::pair (8, "Ruth"),
    std::pair (9, "1 Samuel"),
    std::pair (10, "2 Samuel"),
    std::pair (11, "1 Kings"),
    std::pair (12, "2 Kings"),
    std::pair (13, "1 Chronicles"),
    std::pair (14, "2 Chronicles"),
    std::pair (15, "Ezra"),
    std::pair (1, "Genesis"),
    std::pair (1, "Genesis"),
    std::pair (1, "Genesis"),
    std::pair (16, "Nehemiah"),
    std::pair (17, "Esther"),
    std::pair (18, "Job"),
    std::pair (19, "Psalms"),
    std::pair (20, "Proverbs"),
    std::pair (21, "Ecclesiastes"),
    std::pair (22, "Song of Solomon"),
    std::pair (23, "Isaiah"),
    std::pair (24, "Jeremiah"),
    std::pair (25, "Lamentations"),
    std::pair (26, "Ezekiel"),
    std::pair (27, "Daniel"),
    std::pair (28, "Hosea"),
    std::pair (29, "Joel"),
    std::pair (30, "Amos"),
    std::pair (31, "Obadiah"),
    std::pair (32, "Jonah"),
    std::pair (33, "Micah"),
    std::pair (34, "Nahum"),
    std::pair (35, "Habakkuk"),
    std::pair (36, "Zephaniah"),
    std::pair (37, "Haggai"),
    std::pair (38, "Zechariah"),
    std::pair (39, "Malachi"),
    std::pair (40, "Matthew"),
    std::pair (41, "Mark"),
    std::pair (42, "Luke"),
    std::pair (43, "John"),
    std::pair (44, "Acts"),
    std::pair (45, "Romans"),
    std::pair (46, "1 Corinthians"),
    std::pair (47, "2 Corinthians"),
    std::pair (48, "Galatians"),
    std::pair (49, "Ephesians"),
    std::pair (50, "Philippians"),
    std::pair (51, "Colossians"),
    std::pair (52, "1 Thessalonians"),
    std::pair (53, "2 Thessalonians"),
    std::pair (54, "1 Timothy"),
    std::pair (55, "2 Timothy"),
    std::pair (56, "Titus"),
    std::pair (57, "Philemon"),
    std::pair (58, "Hebrews"),
    std::pair (59, "James"),
    std::pair (60, "1 Peter"),
    std::pair (61, "2 Peter"),
    std::pair (62, "1 John"),
    std::pair (63, "2 John"),
    std::pair (64, "3 John"),
    std::pair (65, "Jude"),
    std::pair (66, "Revelation")
  };
  return filter_url_urlencode (mapping [book]);
}


std::string resource_external_convert_book_studylight (int book)
{
  // Map Bibledit books to biblehub.com books.
  std::map <int, std::string> mapping = {
    std::pair (1, "genesis"),
    std::pair (2, "exodus"),
    std::pair (3, "leviticus"),
    std::pair (4, "numbers"),
    std::pair (5, "deuteronomy"),
    std::pair (6, "joshua"),
    std::pair (7, "judges"),
    std::pair (8, "ruth"),
    std::pair (9, "1-samuel"),
    std::pair (10, "2-samuel"),
    std::pair (11, "1-kings"),
    std::pair (12, "2-kings"),
    std::pair (13, "1-chronicles"),
    std::pair (14, "2-chronicles"),
    std::pair (15, "ezra"),
    std::pair (16, "nehemiah"),
    std::pair (17, "esther"),
    std::pair (18, "job"),
    std::pair (19, "psalms"),
    std::pair (20, "proverbs"),
    std::pair (21, "ecclesiastes"),
    std::pair (22, "song-of-solomon"),
    std::pair (23, "isaiah"),
    std::pair (24, "jeremiah"),
    std::pair (25, "lamentations"),
    std::pair (26, "ezekiel"),
    std::pair (27, "daniel"),
    std::pair (28, "hosea"),
    std::pair (29, "joel"),
    std::pair (30, "amos"),
    std::pair (31, "obadiah"),
    std::pair (32, "jonah"),
    std::pair (33, "micah"),
    std::pair (34, "nahum"),
    std::pair (35, "habakkuk"),
    std::pair (36, "zephaniah"),
    std::pair (37, "haggai"),
    std::pair (38, "zechariah"),
    std::pair (39, "malachi"),
    std::pair (40, "matthew"),
    std::pair (41, "mark"),
    std::pair (42, "luke"),
    std::pair (43, "john"),
    std::pair (44, "acts"),
    std::pair (45, "romans"),
    std::pair (46, "1-corinthians"),
    std::pair (47, "2-corinthians"),
    std::pair (48, "galatians"),
    std::pair (49, "ephesians"),
    std::pair (50, "philippians"),
    std::pair (51, "colossians"),
    std::pair (52, "1-thessalonians"),
    std::pair (53, "2-thessalonians"),
    std::pair (54, "1-timothy"),
    std::pair (55, "2-timothy"),
    std::pair (56, "titus"),
    std::pair (57, "philemon"),
    std::pair (58, "hebrews"),
    std::pair (59, "james"),
    std::pair (60, "1-peter"),
    std::pair (61, "2-peter"),
    std::pair (62, "1-john"),
    std::pair (63, "2-john"),
    std::pair (64, "3-john"),
    std::pair (65, "jude"),
    std::pair (66, "revelation")
  };
  return mapping [book];
}


struct bible_gateway_walker: pugi::xml_tree_walker
{
  bool skip_next_text = false;
  bool parsing = true;
  std::string text {};
  std::vector <std::string> footnotes {};

  virtual bool for_each (pugi::xml_node& node) override
  {
    // Details of the current node.
    std::string classname = node.attribute ("class").value ();
    std::string nodename = node.name ();

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
      std::string data_fn = node.attribute ("data-fn").value ();
      footnotes.push_back(data_fn);
    }
    
    // Continue parsing.
    return true;
  }
};


// Get the clean text of a passage of a BibleGateway resource.
std::string resource_logic_bible_gateway_get (std::string resource, int book, int chapter, int verse)
{
  std::string result {};
#ifdef HAVE_CLOUD
  // Convert the resource name to a resource abbreviation for biblegateway.com.
  size_t pos = resource.find_last_of ("(");
  if (pos != std::string::npos) {
    pos++;
    resource.erase (0, pos);
    pos = resource.find_last_of (")");
    if (pos != std::string::npos) {
      resource.erase (pos);
      // Assemble the URL to fetch the chapter.
      std::string bookname = resource_logic_bible_gateway_book (book);
      std::string url = "https://www.biblegateway.com/passage/?search=" + bookname + "+" + std::to_string (chapter) + ":" + std::to_string(verse) + "&version=" + resource;
      // Fetch the html.
      std::string error {};
      std::string html = resource_logic_web_or_cache_get (url, error);
      // Remove the html that precedes the relevant verses content.
      pos = html.find ("<div class=\"passage-text\">");
      if (pos != std::string::npos) {
        html.erase (0, pos);
        // Load the remaining html into the XML parser.
        // The parser will given an error about Start-end tags mismatch.
        // The parser will also give the location where this mismatch occurs first.
        // The location where the mismatch occurs indicates the end of the relevant verses content.
        {
          pugi::xml_document document;
          pugi::xml_parse_result parse_result = document.load_string (html.c_str(), pugi::parse_default | pugi::parse_fragment);
          if (parse_result.offset > 10) {
            size_t pos2 = static_cast<size_t>(parse_result.offset - 2);
            html.erase (pos2);
          }
        }
        // Parse the html fragment into a DOM.
        std::string verse_s = std::to_string (verse);
        pugi::xml_document document;
        document.load_string (html.c_str());
        // There can be cross references in the html.
        // These result in e.g. "A" or "B" scattered through the final text.
        // So remove these.
        // Sample cross reference XML:
        // <sup class='crossreference' data-cr='#cen-NASB-30388A'  data-link='(&lt;a href=&quot;#cen-NASB-30388A&quot; title=&quot;See cross-reference A&quot;&gt;A&lt;/a&gt;)'>(<a href="#cen-NASB-30388A" title="See cross-reference A">A</a>)</sup>
        {
          std::string selector = "//sup[@class='crossreference']";
          pugi::xpath_node_set nodeset = document.select_nodes(selector.c_str());
          for (auto xrefnode: nodeset) xrefnode.node().parent().remove_child(xrefnode.node());
        }
        // Start parsing for actual text.
        pugi::xml_node passage_text_node = document.first_child ();
        pugi::xml_node passage_wrap_node = passage_text_node.first_child ();
        pugi::xml_node passage_content_node = passage_wrap_node.first_child ();
        bible_gateway_walker walker {};
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
          std::string selector = "//li[@id='" + footnote_id + "']/span[@class='footnote-text']";
          pugi::xpath_node xpath = document.select_node(selector.c_str());
          if (xpath) {
            std::stringstream ss {};
            xpath.node().print (ss, "", pugi::format_raw);
            std::string selected_html = ss.str ();
            std::string footnote_text = filter::strings::html2text (selected_html);
            result.append ("<br>Note: ");
            result.append (footnote_text);
          }
        }
      }
    }
  }
  result = filter::strings::replace (filter::strings::unicode_non_breaking_space_entity (), " ", result);
  result = filter::strings::replace (" ", " ", result); // Make special space visible.
  result = filter::strings::collapse_whitespace (result);
  result = filter::strings::trim (result);
#endif
#ifdef HAVE_CLIENT
  result = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif
  return result;
}


// The path to the list of StudyLight resources.
// It is stored in the client files area.
// Clients will download it from there.
std::string resource_logic_study_light_module_list_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "study_light_modules.txt"});
}


// Refreshes the list of resources available from StudyLight.
std::string resource_logic_study_light_module_list_refresh ()
{
  Database_Logs::log ("Refresh StudyLight resources");
  std::string path {resource_logic_study_light_module_list_path ()};
  std::string error {};
  std::string html = filter_url_http_get ("http://www.studylight.org/commentaries", error, false);
  if (error.empty ()) {
    std::vector <std::string> resources;
    // Example commentary fragment:
    // <h3><a class="emphasis" href="//www.studylight.org/commentaries/gsb.html">Geneva Study Bible</a></h3>
    do {
      // <a class="emphasis" ...
      // std::string fragment = R"(<a class="emphasis")";
      // New fragment on updated website:
      // std::string fragment = R"(<a class="fg-darkgrey")";
      // New fragent on updated website:
      std::string fragment = R"(<a class="fg-dark")";
      size_t pos = html.find (fragment);
      if (pos == std::string::npos) break;
      html.erase (0, pos + fragment.size ());
      fragment = "commentaries";
      pos = html.find (fragment);
      if (pos == std::string::npos) break;
      html.erase (0, pos + fragment.size () + 1);
      fragment = ".html";
      pos = html.find (fragment);
      if (pos == std::string::npos) break;
      std::string abbreviation = html.substr (0, pos);
      html.erase (0, pos + fragment.size () + 2);
      pos = html.find ("</a>");
      if (pos == std::string::npos) break;
      std::string name = html.substr (0, pos);
      filter::strings::replace_between(name, "<desktop>", "</desktop>", std::string());
      name = filter::strings::replace("<mobile>", std::string(), name);
      name = filter::strings::replace("</mobile>", std::string(), name);
      std::string resource = name + " (studylight-" + abbreviation + ")";
      resources.push_back (resource);
    } while (true);
    // Store the resources in a file.
    filter_url_file_put_contents (path, filter::strings::implode (resources, "\n"));
    // Done.
    Database_Logs::log ("Modules: " + std::to_string (resources.size ()));
  } else {
    Database_Logs::log (error);
  }
  return error;
}


// Get the list of StudyLight resources.
std::vector <std::string> resource_logic_study_light_module_list_get ()
{
  std::string path = resource_logic_study_light_module_list_path ();
  std::string contents = filter_url_file_get_contents (path);
  return filter::strings::explode (contents, '\n');
}


// Get the slightly formatted of a passage of a StudyLight resource.
std::string resource_logic_study_light_get (std::string resource, int book, int chapter, int verse)
{
  std::string result {};

#ifdef HAVE_CLOUD
  // Transform the full name to the abbreviation for the website, e.g.:
  // "Adam Clarke Commentary (acc)" becomes "acc".
  size_t pos = resource.find_last_of ("(");
  if (pos == std::string::npos) return std::string();
  resource.erase (0, pos + 1);
  pos = resource.find (")");
  if (pos == std::string::npos) return std::string();
  resource.erase (pos);
  
  // The resource abbreviation might look like this:
  // studylight-eng/bnb
  // Also remove that "studylight-" bit.
  resource.erase (0, 11);
  
  // Example URL: https://www.studylight.org/commentaries/eng/acc/revelation-1.html
  std::string url {};
  url.append ("http://www.studylight.org/commentaries/");
  url.append (resource + "/");
  url.append (resource_external_convert_book_studylight (book));
  url.append ("-" + std::to_string (chapter) + ".html");
  
  // Get the html from the server.
  std::string error {};
  std::string html = resource_logic_web_or_cache_get (url, error);

  // It appears that the html from this website is not well-formed.
  // It cannot be loaded as an XML document without errors and missing text.
  // Therefore the html is tidied first.
//  html = filter::strings::fix_invalid_html_gumbo (html);
  html = filter::strings::fix_invalid_html_tidy(html);

  // Where to start (updated over time).
//  std::string start_key = R"(<div class="ptb10">)";
//  start_key = R"(<h3 class="commentaries-entry-number">)";
//  start_key = R"(<body )";
//  const std::vector <int> class_lightgrey_book {
//    23, // Isaiah
//    27, // Daniel
//    52, // 1 Thessalonians
//    53, // 2 Thessalonians
//    54, // 1 Timothy
//    58, // Hebrews
//  };
//  if (in_array(book, class_lightgrey_book)) {
//    start_key = R"(<div class="tl-lightgrey ptb10">)";
//  }
//  pos = html.find(start_key);
//  if (pos != std::string::npos) html.erase (0, pos);

  // Parse the html into a DOM.
  std::string verse_s {std::to_string (verse)};
  pugi::xml_document document {};
  pugi::xml_parse_result parse_result = document.load_string (html.c_str());
  pugixml_utils_error_logger (&parse_result, html);

  // Example verse indicator within the XML:
  // <a name="verses-2-10"></a>
  // <a name="verse-2"></a>
  std::string selector1 = "//a[contains(@name,'verses-" + std::to_string (verse) + "-')]";
  std::string selector2 = "//a[@name='verse-" + std::to_string (verse) + "']";
  std::string selector = selector1 + "|" + selector2;
  pugi::xpath_node_set nodeset = document.select_nodes(selector.c_str());
  nodeset.sort();
  for (pugi::xpath_node xpathnode : nodeset) {
    pugi::xml_node h3_node = xpathnode.node().parent();
    pugi::xml_node div_node = h3_node.parent();
    std::stringstream ss {};
    div_node.print (ss, "", pugi::format_raw);
    result.append(ss.str ());
  }
#endif

#ifdef HAVE_CLIENT
  result = resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif

  return result;
}


std::string resource_logic_easy_english_bible_name ()
{
  return "Easy English Bible Commentary";
}


// Given a book number and a chapter,
// this returns 0 to 2 parts of the URL that will contain the relevant text.
std::vector <std::string> resource_logic_easy_english_bible_pages (int book, int chapter)
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
      std::string number = filter::strings::fill (std::to_string (chapter), 3, '0');
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
    default: return {};
  }
  return {};
}


struct easy_english_bible_walker: pugi::xml_tree_walker
{
  std::string text {};

  virtual bool for_each (pugi::xml_node& node) override
  {
    // Handle this node if it's a text node.
    if (node.type() == pugi::node_pcdata) {
      std::string fragment = node.value();
      fragment = filter::strings::replace ("\n", " ", fragment);
      text.append (fragment);
    }
    // Continue parsing.
    return true;
  }
};


// Get the slightly formatted of a passage of a Easy English Bible Commentary.
std::string resource_logic_easy_english_bible_get (int book, int chapter, int verse)
{
  // First handle the easier part:
  // The client will fetch the data from the Cloud.
#ifdef HAVE_CLIENT
  std::string resource = resource_logic_easy_english_bible_name ();
  return resource_logic_client_fetch_cache_from_cloud (resource, book, chapter, verse);
#endif

  // Now handle the Cloud part: Fetch and parse the text.
#ifdef HAVE_CLOUD

  // The combined text result taken from the commentary.
  std::string result {};

  // This resource may have one or more commentaries per book.
  // This means that the URLs may be one or more.
  std::vector <std::string> urls {};
  {
    std::vector <std::string> pages = resource_logic_easy_english_bible_pages (book, chapter);
    for (auto page : pages) {
      // Example URL: https://www.easyenglish.bible/bible-commentary/matthew-lbw.htm
      std::string url = "https://www.easyenglish.bible/bible-commentary/";
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
    bool near_passage {false};
    // Flag for whether the current paragraph is for the exact verse number.
    bool at_passage {false};
    
    // Get the html from the server.
    std::string error {};
    std::string html = resource_logic_web_or_cache_get (url, error);

    // It appears that the html from this website is not well-formed.
    // It cannot be loaded as an XML document without errors and missing text.
    // Therefore the html is tidied first.
    html = filter::strings::fix_invalid_html_gumbo (html);

    // The html from this website requires further cleaning.
    // One issue is that instead of  class="Section1"  it has  class=Section1
    // Notice the missing quotes around the class name.
    // Call libtidy to further tidy this heml up.
    html = filter::strings::fix_invalid_html_tidy (html);

    // The document has one main div like this:
    // <div class="Section1">
    // Or: <div class="WordSection1"> like in Exodus.
    size_t pos = html.find(R"(<div class="Section1">)");
    if (pos == std::string::npos) {
      pos = html.find(R"(<div class="WordSection1">)");
    }
    if (pos != std::string::npos) html.erase (0, pos);
    
    // Parse the html into a DOM.
    pugi::xml_document document {};
    document.load_string (html.c_str());
    
    // The document has one main div like this:
    // <div class="Section1">
    // Or: <div class="WordSection1"> like in Exodus.
    // That secion has many children all containing one paragraph of text.
    std::string selector = "//div[contains(@class, 'Section1')]";
    pugi::xpath_node xnode = document.select_node(selector.c_str());
    pugi::xml_node div_node = xnode.node();

    // Iterate over the paragraphs of text and process them.
    for (auto paragraph_node : div_node.children()) {

      // Assemble the text by iterating over all child text nodes.
      easy_english_bible_walker tree_walker {};
      paragraph_node.traverse(tree_walker);

      // Clean the text and skip empty text.
      std::string paragraph = filter::strings::trim (tree_walker.text);
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
            std::string tag = "v" + std::to_string(verse);
            if (paragraph.find(tag) != std::string::npos) {
              at_passage = true;
              continue;
            }
          }
        }

        // If still at the correct verse, add the contents.
        if (at_passage) {
          // The html from this website is encoded as charset=windows-1252.
          // Convert that to UTF0-8.
          paragraph = filter::strings::convert_windows1252_to_utf8 (paragraph);
          // Add this paragraph to the resulting text.
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



bool resource_logic_easy_english_bible_handle_chapter_heading (const std::string& paragraph,
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
      std::string tag = "Proverbs chapter " + std::to_string(chapter);
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
      std::string tag = "Chapter " + std::to_string(chapter);
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


bool resource_logic_easy_english_bible_handle_passage_heading (const std::string& paragraph,
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
  if (space_pos == std::string::npos) return false;
  std::string last_word = paragraph.substr(space_pos + 1);
  
  // There's also situations that the last bit is surrounded by round bracket.
  // Example: Greetings from Paul to Timothy (1:1-2)
  last_word = filter::strings::replace ("(", std::string(), last_word);
  last_word = filter::strings::replace (")", std::string(), last_word);
  
  // Look for the first colon and the first hyphen.
  // This will obtain the starting chapter and verse numbers.
  size_t colon_pos = last_word.find(":");
  if (colon_pos == std::string::npos) return false;
  size_t hyphen_pos = last_word.find ("-");
  if (hyphen_pos == std::string::npos) return false;
  std::string ch_fragment = last_word.substr(0, colon_pos);
  int starting_chapter = filter::strings::convert_to_int(ch_fragment);
  std::string check = std::to_string(starting_chapter);
  if (ch_fragment != check) return false;

  // Look for the first hyphen.
  // This will provide the starting verse number.
  std::string vs_fragment = last_word.substr(colon_pos + 1, hyphen_pos - colon_pos - 1);
  int starting_verse = filter::strings::convert_to_int(vs_fragment);
  check = std::to_string(starting_verse);
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
  if (colon_pos != std::string::npos) {
    std::string chapter_fragment = last_word.substr(0, colon_pos);
    ending_chapter = filter::strings::convert_to_int(chapter_fragment);
    check = std::to_string(ending_chapter);
    if (chapter_fragment != check) return false;
    last_word.erase(0, colon_pos + 1);
  }

  // The last bit of the fragment will now be the second verse number.
  int ending_verse = filter::strings::convert_to_int(last_word);
  check = std::to_string(ending_verse);
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


void resource_logic_easy_english_bible_handle_verse_marker (const std::string& paragraph,
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
  std::string tag = "Verse " + std::to_string(verse) + " ";
  at_passage = paragraph.find(tag) == 0;
  // If it's at the passage, then it's done parsing.
  if (at_passage) return;

  // If no verse is found yet, look for e.g. "Verse 13:".
  tag = "Verse " + std::to_string(verse) + ":";
  at_passage = paragraph.find(tag) == 0;
  //If it's at the passage, then it's done parsing.
  if (at_passage) return;

  // If no verse is found yet, look for the same tag but without the space at the end.
  // Then the entire paragraph should consist of this tag.
  // This occurs in Genesis 1 for example.
  tag = "Verse " + std::to_string(verse);
  at_passage = (paragraph == tag);
  //If it's at the passage, then it's done parsing.
  if (at_passage) return;

  // If no verse is found yet, then look for a variation of the markup that occurs too.
  // Example: Verse 8-11 ...
  tag = "Verse ";
  if (paragraph.find(tag) == 0) {
    std::string fragment = paragraph.substr(tag.size(), 10);
    std::vector<std::string> bits = filter::strings::explode(fragment, '-');
    if (bits.size() >= 2) {
      int begin = filter::strings::convert_to_int(bits[0]);
      int end = filter::strings::convert_to_int(bits[1]);
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
    std::string fragment = paragraph.substr(tag.size(), 10);
    std::vector <int> digits;
    int digit = 0;
    for (size_t i = 0; i < fragment.size(); i++) {
      int d = filter::strings::convert_to_int(fragment.substr(i, 1));
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


bool resource_logic_is_bible (std::string resource)
{
  std::vector <std::string> names = database::bibles::get_bibles ();
  return in_array (resource, names);
}


bool resource_logic_is_usfm (std::string resource)
{
  std::vector <std::string> names {};
#ifdef HAVE_CLIENT
  names = client_logic_usfm_resources_get ();
#else
  Database_UsfmResources database_usfmresources;
  names = database_usfmresources.getResources ();
#endif
  return in_array (resource, names);
}


bool resource_logic_is_external (std::string resource)
{
  std::vector <std::string> names = resource_external_names ();
  return in_array (resource, names);
}


bool resource_logic_is_image (std::string resource)
{
  Database_ImageResources database_imageresources;
  std::vector <std::string> names = database_imageresources.names ();
  return in_array (resource, names);
}


bool resource_logic_is_lexicon (std::string resource)
{
  std::vector <std::string> names = lexicon_logic_resource_names ();
  return in_array (resource, names);
}


bool resource_logic_is_sword (std::string resource)
{
  std::string module = sword_logic_get_remote_module (resource);
  std::string source = sword_logic_get_source (resource);
  return (!source.empty () && !module.empty ());
}


bool resource_logic_is_divider (std::string resource)
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


bool resource_logic_is_biblegateway (std::string resource)
{
  std::vector <std::string> names = resource_logic_bible_gateway_module_list_get ();
  return in_array (resource, names);
}


bool resource_logic_is_studylight (std::string resource)
{
  std::vector <std::string> names = resource_logic_study_light_module_list_get ();
  return in_array (resource, names);
}


bool resource_logic_is_comparative (const std::string& resource)
{
  return resource_logic_parse_comparative_resource(resource);
}


std::string resource_logic_comparative_resource ()
{
  return "Comparative ";
}


bool resource_logic_parse_comparative_resource (const std::string& input,
                                                std::string* title,
                                                std::string* base,
                                                std::string* update,
                                                std::string* remove,
                                                std::string* replace,
                                                bool* diacritics,
                                                bool* casefold,
                                                bool* cache)
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
  std::vector <std::string> bits = filter::strings::explode(input, '|');
  if (bits.size() > 0) if (title) title->assign (bits[0]);
  if (bits.size() > 1) if (base) base->assign(bits[1]);
  if (bits.size() > 2) if (update) update->assign(bits[2]);
  if (bits.size() > 3) if (remove) remove->assign(bits[3]);
  if (bits.size() > 4) if (replace) replace->assign(bits[4]);
  if (bits.size() > 5) if (diacritics) * diacritics = filter::strings::convert_to_bool(bits[5]);
  if (bits.size() > 6) if (casefold) * casefold = filter::strings::convert_to_bool(bits[6]);
  if (bits.size() > 7) if (cache) * cache = filter::strings::convert_to_bool(bits[7]);

  // Done.
  return true;
}


std::string resource_logic_assemble_comparative_resource (std::string title,
                                                          std::string base,
                                                          std::string update,
                                                          std::string remove,
                                                          std::string replace,
                                                          bool diacritics,
                                                          bool casefold,
                                                          bool cache)
{
  // Check whether the "Comparative " flag already is included in the given $title.
  size_t pos = title.find (resource_logic_comparative_resource ());
  if (pos != std::string::npos) {
    title.erase (pos, resource_logic_comparative_resource ().length());
  }
  // Ensure the "Comparative " flag is always included right at the start.
  std::vector <std::string> bits = {resource_logic_comparative_resource() + title, base, update, remove, replace, filter::strings::convert_to_true_false(diacritics), filter::strings::convert_to_true_false(casefold), filter::strings::convert_to_true_false(cache)};
  return filter::strings::implode(bits, "|");
}


std::string resource_logic_comparative_resources_list_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "comparative_resources.txt"});
}


// Get the list of comparative resources on a client device.
std::vector <std::string> resource_logic_comparative_resources_get_list_on_client ()
{
  std::string path = resource_logic_comparative_resources_list_path ();
  std::string contents = filter_url_file_get_contents (path);
  return filter::strings::explode (contents, '\n');
}


bool resource_logic_is_translated (const std::string& resource)
{
  return resource_logic_parse_translated_resource(resource);
}


std::string resource_logic_translated_resource ()
{
  return "Translated ";
}


bool resource_logic_parse_translated_resource (const std::string& input,
                                               std::string* title,
                                               std::string* original_resource,
                                               std::string* source_language,
                                               std::string* target_language,
                                               bool* cache)
{
  // The definite check whether this is a translated resource
  // is to check that "Translated " is the first part of the input.
  if (input.find(resource_logic_translated_resource()) != 0) return false;
  
  // Do a forgiving parsing of the properties of this resource.
  if (title) title->clear();
  if (original_resource) original_resource->clear();
  if (source_language) source_language->clear();
  if (target_language) target_language->clear();
  if (cache) * cache = false;
  std::vector <std::string> bits = filter::strings::explode(input, '|');
  if (bits.size() > 0) if (title) title->assign (bits[0]);
  if (bits.size() > 1) if (original_resource) original_resource->assign(bits[1]);
  if (bits.size() > 2) if (source_language) source_language->assign(bits[2]);
  if (bits.size() > 3) if (target_language) target_language->assign(bits[3]);
  if (bits.size() > 4) if (cache) * cache = filter::strings::convert_to_bool(bits[4]);
  
  // Done.
  return true;
}


std::string resource_logic_assemble_translated_resource (std::string title,
                                                         std::string original_resource,
                                                         std::string source_language,
                                                         std::string target_language,
                                                         bool cache)
{
  // Check whether the "Translated " flag already is included in the given $title.
  size_t pos = title.find (resource_logic_translated_resource ());
  if (pos != std::string::npos) {
    title.erase (pos, resource_logic_translated_resource ().length());
  }
  // Ensure the "Translated " flag is always included right at the start.
  std::vector <std::string> bits = {resource_logic_translated_resource() + title, original_resource, source_language, target_language, filter::strings::convert_to_true_false(cache)};
  return filter::strings::implode(bits, "|");
}


std::string resource_logic_translated_resources_list_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "translated_resources.txt"});
}


// Get the list of translated resources on a client device.
std::vector <std::string> resource_logic_translated_resources_get_list_on_client ()
{
  std::string path = resource_logic_translated_resources_list_path ();
  std::string contents = filter_url_file_get_contents (path);
  return filter::strings::explode (contents, '\n');
}

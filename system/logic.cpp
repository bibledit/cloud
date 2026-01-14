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


#include <system/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/usfm.h>
#include <filter/roles.h>
#include <filter/date.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/jobs.h>
#include <database/cache.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/privileges.h>
#include <database/confirm.h>
#include <html/text.h>
#include <styles/logic.h>
#include <tasks/logic.h>
#include <database/logic.h>
#include <email/send.h>
#include <search/logic.h>
#include <user/logic.h>


std::string system_logic_bibles_file_name ()
{
  return filter_url_create_path ({filter_url_temp_dir (), "bibles.tar"});
}


// This produes a tarball with all installed Bibles.
void system_logic_produce_bibles_file (int jobid)
{
  Database_Jobs database_jobs;

  
  // Generate the initial page.
  {
    HtmlText html_text ("");
    html_text.new_paragraph ();
    html_text.add_text (translate ("Generating a file with the Bibles."));
    html_text.new_paragraph ();
    html_text.add_text (translate ("In progress..."));
    database_jobs.set_start (jobid, html_text.get_inner_html ());
  }

  
  // The location of the tarball to generate.
  std::string tarball = filter_url_create_root_path ({system_logic_bibles_file_name ()});
  
  
  // The directory where the exported Bibles will be put.
  std::string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);

  
  // The files in the tarball.
  std::vector <std::string> files;
  

  // Iterate over the Bibles, the books, the chapters.
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  for (auto bible : bibles) {
    std::vector <int> books = database::bibles::get_books (bible);
    for (auto book : books) {
      std::string book_usfm;
      std::vector <int> chapters = database::bibles::get_chapters (bible, book);
      for (auto chapter : chapters) {
        std::string usfm = database::bibles::get_chapter (bible, book, chapter);
        book_usfm.append (filter::string::trim (usfm));
        book_usfm.append ("\n");
      }
      std::string file = bible + "_" + std::to_string (book) + ".usfm";
      std::string path = filter_url_create_path ({directory, file});
      filter_url_file_put_contents (path, book_usfm);
      files.push_back (file);
    }
  }
  
  
  // Pack the contents of all the Bibles into one tarball.
  std::string error = filter_archive_microtar_pack (tarball, directory, files);
  
  
  // Ready, provide info about how to download the file, or about the error.
  {
    HtmlText html_text ("");
    html_text.new_paragraph ();
    if (error.empty ()) {
      html_text.add_text (translate ("The file with Bibles is ready."));
      html_text.new_paragraph ();
      html_text.add_link (html_text.current_p_node, "/" + system_logic_bibles_file_name (), "", "", "", translate ("Get it."));
      html_text.new_paragraph ();
      html_text.add_text (translate ("The file can be imported in another Bibledit client."));
      html_text.add_text (" ");
      html_text.add_text (translate ("This will create the same Bibles in that other client."));
    } else {
      html_text.add_text (translate ("It failed to create the file with Bibles."));
      html_text.new_paragraph ();
      html_text.add_text (error);
    }
    database_jobs.set_result (jobid, html_text.get_inner_html ());
  }
}


void system_logic_import_bibles_file (std::string tarball)
{
  Database_Logs::log ("Importing Bibles from " + tarball);

  // Unpack the tarball into a directory.
  std::string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);
  std::string error= filter_archive_microtar_unpack (tarball, directory);
  if (!error.empty ()) {
    Database_Logs::log ("Importing Bibles failure: " + error);
    return;
  }

  // Iterate over all the files of the tarball.
  std::vector <std::string> files = filter_url_scandir (directory);
  for (auto file : files) {
    
    // Get the file's contents for import.
    Database_Logs::log ("Importing from file " + file);
    std::string path = filter_url_create_path ({directory, file});
    std::string data = filter_url_file_get_contents (path);
    
    // The name of the Bible this file is to be imported into.
    std::string bible (file);
    size_t pos = bible.find_last_of ("_");
    if (pos != std::string::npos) bible.erase (pos);
    
    // Get details about the USFM to import.
    const std::string stylesheet = stylesv2::standard_sheet ();
    std::vector <filter::usfm::BookChapterData> book_chapter_text = filter::usfm::usfm_import (data, stylesheet);
    for (auto & book_chapter_data : book_chapter_text) {
      if (book_chapter_data.m_book > 0) {
        // Store the data and log it.
        // This does not trigger the client to send it to the Cloud.
        // Reason is that the Cloud is authoritative,
        // so importing outdated Bibles would not affect the authoritative copy in the Cloud.
        database::bibles::store_chapter (bible, book_chapter_data.m_book, book_chapter_data.m_chapter, book_chapter_data.m_data);
        std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book_chapter_data.m_book));
        Database_Logs::log ("Imported " + bible + " " + bookname + " " + std::to_string (book_chapter_data.m_chapter));
      } else {
        // Import error.
        Database_Logs::log ("Could not import this file: " + file);
      }
    }
  }
  
  // Clean up.
  filter_url_rmdir (directory);
  filter_url_unlink (tarball);

  // Since new Bibles may have been imported, index them all.
  database::config::general::set_index_bibles (true);
  tasks_logic_queue (task::reindex_bibles, {"1"});

  // Ready, hallelujah!
  Database_Logs::log ("Importing Bibles ready");
}


std::string system_logic_notes_file_name ()
{
  return filter_url_create_path ({filter_url_temp_dir (), "notes.tar"});
}


// This produes a tarball with all local Consultation Notes.
void system_logic_produce_notes_file (int jobid)
{
  Database_Jobs database_jobs;
 
  
  // Generate the initial page.
  {
    HtmlText html_text ("");
    html_text.new_paragraph ();
    html_text.add_text (translate ("Generating a file with the Consultation Notes."));
    html_text.new_paragraph ();
    html_text.add_text (translate ("In progress..."));
    database_jobs.set_start (jobid, html_text.get_inner_html ());
  }
  
  
  // The location of the tarball to generate.
  std::string tarball = filter_url_create_root_path ({system_logic_notes_file_name ()});
  
  
  // The database directory where the consultation notes reside.
  std::string directory = filter_url_create_root_path ({"consultations"});

  
  // The files to include in the tarball.
  std::vector <std::string> files;
  filter_url_recursive_scandir (directory, files);
  for (std::string & file : files) {
    file.erase (0, directory.length () + 1);
  }
  

  // Pack the contents of all the Bibles into one tarball.
  std::string error = filter_archive_microtar_pack (tarball, directory, files);

  
  // Ready, provide info about how to download the file, or about the error.
  {
    HtmlText html_text ("");
    html_text.new_paragraph ();
    if (error.empty ()) {
      html_text.add_text (translate ("The file with Consultation Notes is ready."));
      html_text.new_paragraph ();
      html_text.add_link (html_text.current_p_node, "/" + system_logic_notes_file_name (), "", "", "", translate ("Get it."));
      html_text.new_paragraph ();
      html_text.add_text (translate ("The file can be imported in another Bibledit client."));
      html_text.add_text (" ");
      html_text.add_text (translate ("This will create the same Consultation Notes in that other client."));
    } else {
      html_text.add_text (translate ("It failed to create the file with Consultation Notes."));
      html_text.new_paragraph ();
      html_text.add_text (error);
    }
    database_jobs.set_result (jobid, html_text.get_inner_html ());
  }
}


void system_logic_import_notes_file (std::string tarball)
{
  Database_Logs::log ("Importing Consultation Notes from " + tarball);
  
  // The database directory where the consultation notes reside.
  std::string directory = filter_url_create_root_path ({"consultations"});
  
  // Unpack the tarball into the directory.
  std::string error= filter_archive_microtar_unpack (tarball, directory);
  if (!error.empty ()) {
    Database_Logs::log ("Importing Consultation Notes failure: " + error);
    return;
  }
  
  // Clean up.
  filter_url_unlink (tarball);

  // Since notes may have been imported or updated, index them all.
  database::config::general::setIndexNotes (true);
  tasks_logic_queue (task::reindex_notes);

  // Ready, hallelujah!
  Database_Logs::log ("Importing Consultation Notes ready");
}


std::string system_logic_resources_file_name (std::string resourcename)
{
  if (!resourcename.empty ()) resourcename.append ("_");
  return filter_url_create_path ({filter_url_temp_dir (), resourcename + "resources.tar"});
}


// This produces a tarball with all installed resources.
void system_logic_produce_resources_file (int jobid)
{
  Database_Jobs database_jobs;
  
  
  // Generate the initial page.
  {
    HtmlText html_text ("");
    html_text.new_paragraph ();
    html_text.add_text (translate ("Generating a file with the resources."));
    html_text.new_paragraph ();
    html_text.add_text (translate ("In progress..."));
    database_jobs.set_start (jobid, html_text.get_inner_html ());
  }
  
  // The location of the single tarball to generate.
  std::string tarball = filter_url_create_root_path ({system_logic_resources_file_name ()});
  
  
  // The database directory where the cached resources reside.
  std::string directory = filter_url_create_root_path ({database_logic_databases ()});
  
  
  // The filenames of the cached resources.
  std::vector <std::string> resources;
  std::vector <std::string> rawfiles = filter_url_scandir (directory);
  for (auto filename : rawfiles) {
    if (filename.find (database::cache::sql::fragment()) != std::string::npos) {
      resources.push_back (filename);
    }
  }
  
  
  // Get the filenames to pack the resources, one resource per tarball.
  // This is to generate smaller tarballs which can be handled on devices with limited internal memory.
  // Such devices fail to have sufficient memory to handle one tarball with logs and logs of resources.
  // It fails to allocate enough memory on such devices.
  // So that's the reason for doing one resource per tarball.
  std::map <std::string, std::vector <std::string> > single_resources;
  for (auto filename : rawfiles) {
    // Sample filename: cache_resource_[CrossWire][LXX]_62.sqlite
    // Look for the last underscore.
    // This indicates which resource it is, by leaving the book number out.
    if (filename.find (database::cache::sql::fragment()) != std::string::npos) {
      size_t pos = filename.find_last_of ("_");
      if (pos != std::string::npos) {
        std::string resource = filename.substr (0, pos);
        single_resources[resource].push_back (filename);
      }
    }
  }

  
  // Progress bar data: How many tarballs to create.
  int tarball_count = static_cast<int>(1 + single_resources.size ());
  int tarball_counter = 0;

  
  // Pack the resources into one tarball.
  // It does not compress the files (as could be done).
  // It just puts them in a tarball.
  // Compression is not needed because the file is transferred locally,
  // so the size of the file is not very important.
  // Not compressing speeds things up a great lot.
  tarball_counter++;
  database_jobs.set_percentage (jobid, 100 * tarball_counter / tarball_count);
  database_jobs.set_progress (jobid, translate ("All"));
  std::string error = filter_archive_microtar_pack (tarball, directory, resources);

  
  // Create one tarball per resource.
  for (auto element : single_resources) {
    tarball_counter++;
    std::string resource_name = element.first;
    std::vector <std::string> v_resources = element.second;
    database_jobs.set_percentage (jobid, 100 * tarball_counter / tarball_count);
    database_jobs.set_progress (jobid, resource_name);
    std::string resource_tarball = filter_url_create_root_path ({system_logic_resources_file_name (resource_name)});
    error += filter_archive_microtar_pack (resource_tarball, directory, v_resources);
  }
  
  
  // Ready, provide info about how to download the file or about the error.
  {
    HtmlText html_text ("");
    html_text.new_paragraph ();
    if (!resources.empty ()) {
      if (error.empty ()) {
        html_text.add_text (translate ("The file with all of the installed resources is ready."));
        html_text.add_text (" ");
        html_text.add_text (translate ("Amount of resources:"));
        html_text.add_text (" ");
        html_text.add_text (std::to_string (single_resources.size()));
        html_text.add_text (".");
        html_text.new_paragraph ();
        html_text.add_link (html_text.current_p_node, "/" + system_logic_resources_file_name (), "", "", "", translate ("Download the archive with all installed resources."));
        html_text.new_paragraph ();
        html_text.add_text (translate ("The file can be imported in another Bibledit client."));
        html_text.add_text (" ");
        html_text.add_text (translate ("This will create the same resources in that other client."));
        html_text.new_paragraph ();
        html_text.add_text (translate ("The above file may be huge in case there is lots of installed resources."));
        html_text.add_text (" ");
        html_text.add_text (translate ("For that reason there are alternative files with individual resources below."));
        html_text.add_text (" ");
        html_text.add_text (translate ("These are smaller in size."));
        for (auto element : single_resources) {
          std::string resource_name = element.first;
          html_text.new_paragraph ();
          html_text.add_link (html_text.current_p_node, "/" + system_logic_resources_file_name (resource_name), "", "", "", translate ("Download") + " " + resource_name);
        }
      } else {
        html_text.add_text (translate ("It failed to create the file with resources."));
        html_text.new_paragraph ();
        html_text.add_text (error);
      }
    } else {
      html_text.add_text (translate ("There were no installed resources to make an archive from."));
      html_text.add_text (" ");
      html_text.add_text (translate ("Install some resources on the device, and try again."));
    }
    database_jobs.set_result (jobid, html_text.get_inner_html ());
  }
}


void system_logic_import_resources_file (std::string tarball)
{
  Database_Logs::log ("Importing Resources from " + tarball);
  
  // Unpack the tarball into a directory.
  std::string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);
  std::string error= filter_archive_microtar_unpack (tarball, directory);
  if (!error.empty ()) {
    Database_Logs::log ("Importing Resources failure: " + error);
    return;
  }
  
  // Iterate over all the files of the tarball.
  std::vector <std::string> files = filter_url_scandir (directory);
  for (auto file : files) {

    // Get the file's contents for import.
    Database_Logs::log ("Importing " + file);
    std::string path = filter_url_create_path ({directory, file});
    std::string data = filter_url_file_get_contents (path);

    // Store the resource into place.
    path = filter_url_create_root_path ({database_logic_databases (), file});
    filter_url_file_put_contents (path, data);
  }

  // Clean up.
  filter_url_rmdir (directory);
  filter_url_unlink (tarball);

  // Ready, hallelujah!
  Database_Logs::log ("Importing Resources ready");
}



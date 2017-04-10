/*
 Copyright (©) 2003-2017 Teus Benschop.
 
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
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/jobs.h>
#include <database/cache.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <html/text.h>
#include <styles/logic.h>


string system_logic_bibles_file_name ()
{
  return filter_url_create_path (filter_url_temp_dir (), "bibles.tar");
}


// This produes a tarball with all installed Bibles.
void system_logic_produce_bibles_file (int jobid)
{
  Database_Jobs database_jobs;
  Database_Bibles database_bibles;

  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.newParagraph ();
    html_text.addText (translate ("Generating a file with the Bibles."));
    html_text.newParagraph ();
    html_text.addText (translate ("In progress..."));
    database_jobs.setStart (jobid, html_text.getInnerHtml ());
  }

  
  // The location of the tarball to generate.
  string tarball = filter_url_create_root_path (system_logic_bibles_file_name ());
  
  
  // The database directory where the exported Bibles will be put.
  string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);

  
  // The files in the tarball.
  vector <string> files;
  

  // Iterate over the Bibles, the books, the chapters.
  vector <string> bibles = database_bibles.getBibles ();
  for (auto bible : bibles) {
    vector <int> books = database_bibles.getBooks (bible);
    for (auto book : books) {
      string book_usfm;
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto chapter : chapters) {
        string usfm = database_bibles.getChapter (bible, book, chapter);
        book_usfm.append (filter_string_trim (usfm));
        book_usfm.append ("\n");
      }
      string file = bible + "_" + convert_to_string (book) + ".usfm";
      string path = filter_url_create_path (directory, file);
      filter_url_file_put_contents (path, book_usfm);
      files.push_back (file);
    }
  }
  
  
  // Pack the contents of all the Bibles into one tarball.
  string error = filter_archive_microtar_pack (tarball, directory, files);
  
  
  // Ready, provide info about how to download the file, or about the error.
  {
    Html_Text html_text ("");
    html_text.newParagraph ();
    if (error.empty ()) {
      html_text.addText (translate ("The file with Bibles is ready."));
      html_text.newParagraph ();
      html_text.addLink (html_text.currentPDomElement, "/" + system_logic_bibles_file_name (), "", "", "", translate ("Get it."));
      html_text.newParagraph ();
      html_text.addText (translate ("The file can be imported in another Bibledit client."));
      html_text.addText (" ");
      html_text.addText (translate ("This will create the same Bibles in that other client."));
    } else {
      html_text.addText (translate ("It failed to create the file with Bibles."));
      html_text.newParagraph ();
      html_text.addText (error);
    }
    database_jobs.setResult (jobid, html_text.getInnerHtml ());
  }
}


void system_logic_import_bibles_file (string tarball)
{
  Database_Logs::log ("Importing Bibles from " + tarball);

  Database_Bibles database_bibles;
  
  // Unpack the tarball into a directory.
  string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);
  string error= filter_archive_microtar_unpack (tarball, directory);
  if (!error.empty ()) {
    Database_Logs::log ("Importing Bibles failure: " + error);
    return;
  }

  // Iterate over all the files of the tarball.
  vector <string> files = filter_url_scandir (directory);
  for (auto file : files) {
    
    // Get the file's contents for import.
    Database_Logs::log ("Importing from file " + file);
    string path = filter_url_create_path (directory, file);
    string data = filter_url_file_get_contents (path);
    
    // The name of the Bible this file is to be imported into.
    string bible (file);
    size_t pos = bible.find_last_of ("_");
    if (pos != string::npos) bible.erase (pos);
    
    // Get details about the USFM to import.
    string stylesheet = styles_logic_standard_sheet ();
    vector <BookChapterData> book_chapter_text = usfm_import (data, stylesheet);
    for (auto & data : book_chapter_text) {
      if (data.book > 0) {
        // Store the data and log it.
        // This does not trigger the client to send it to the Cloud.
        // Reason is that the Cloud is authoritative,
        // so importing outdated Bibles would not affect the authoritative copy in the Cloud.
        database_bibles.storeChapter (bible, data.book, data.chapter, data.data);
        string bookname = Database_Books::getEnglishFromId (data.book);
        Database_Logs::log ("Imported " + bible + " " + bookname + " " + convert_to_string (data.chapter));
      } else {
        // Import error.
        Database_Logs::log ("Could not import this file: " + file);
      }
    }
  }
  
  // Clean up.
  filter_url_rmdir (directory);
  filter_url_unlink (tarball);

  // Ready, hallelujah!
  Database_Logs::log ("Importing Bibles ready");
}


string system_logic_resources_file_name ()
{
  return filter_url_create_path (filter_url_temp_dir (), "resources.tar");
}


// This produces a tarball with all installed resources.
void system_logic_produce_resources_file (int jobid)
{
  Database_Jobs database_jobs;
  
  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.newParagraph ();
    html_text.addText (translate ("Generating a file with the resources."));
    html_text.newParagraph ();
    html_text.addText (translate ("In progress..."));
    database_jobs.setStart (jobid, html_text.getInnerHtml ());
  }
  
  // The location of the tarball to generate.
  string tarball = filter_url_create_root_path (system_logic_resources_file_name ());
  
  
  // The database directory where the cached resources reside.
  string directory = filter_url_create_root_path ("databases");
  
  
  // The filenames of the cached resources.
  vector <string> resources;
  vector <string> rawfiles = filter_url_scandir (directory);
  for (auto filename : rawfiles) {
    if (filename.find (Database_Cache::fragment()) != string::npos) {
      resources.push_back (filename);
    }
  }
  
  
  // Pack the resources into one tarball.
  // It does not compress the files (as could be done).
  // It just puts them in a tarball.
  // Compression is not needed because the file is transferred locally,
  // so the size of the file is not so important.
  // Not compressing speeds things up a great lot.
  string error = filter_archive_microtar_pack (tarball, directory, resources);
  
  
  // Ready, provide info about how to download the file or about the error.
  {
    Html_Text html_text ("");
    html_text.newParagraph ();
    if (error.empty ()) {
      html_text.addText (translate ("The file with the installed resources is ready."));
      html_text.newParagraph ();
      html_text.addLink (html_text.currentPDomElement, "/" + system_logic_resources_file_name (), "", "", "", translate ("Get it."));
      html_text.newParagraph ();
      html_text.addText (translate ("The file can be imported in another Bibledit client."));
      html_text.addText (" ");
      html_text.addText (translate ("This will create the same resources in that other client."));
    } else {
      html_text.addText (translate ("It failed to create the file with resources."));
      html_text.newParagraph ();
      html_text.addText (error);
    }
    database_jobs.setResult (jobid, html_text.getInnerHtml ());
  }
}


void system_logic_import_resources_file (string tarball)
{
  Database_Logs::log ("Importing Resources from " + tarball);
  
  // Unpack the tarball into a directory.
  string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);
  string error= filter_archive_microtar_unpack (tarball, directory);
  if (!error.empty ()) {
    Database_Logs::log ("Importing Resources failure: " + error);
    return;
  }
  
  // Iterate over all the files of the tarball.
  vector <string> files = filter_url_scandir (directory);
  for (auto file : files) {

    // Get the file's contents for import.
    Database_Logs::log ("Importing " + file);
    string path = filter_url_create_path (directory, file);
    string data = filter_url_file_get_contents (path);

    // Store the resource into place.
    path = filter_url_create_root_path ("databases", file);
    filter_url_file_put_contents (path, data);
  }

  // Clean up.
  filter_url_rmdir (directory);
  filter_url_unlink (tarball);

  // Ready, hallelujah!
  Database_Logs::log ("Importing Resources ready");
}

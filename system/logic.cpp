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
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/jobs.h>
#include <database/cache.h>
#include <database/bibles.h>
#include <html/text.h>


string system_logic_resources_file_name ()
{
  return filter_url_create_path (filter_url_temp_dir (), "resources.tar");
}


void system_logic_produce_resources_file (int jobid)
{
  Database_Jobs database_jobs;

  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.newParagraph ();
    html_text.addText (translate ("Generating file with resources for download."));
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
      html_text.addLink (html_text.currentPDomElement, "/" + system_logic_resources_file_name (), "", "", "", translate ("Download it."));
    } else {
      html_text.addText (translate ("It failed to create the file with resources."));
      html_text.newParagraph ();
      html_text.addText (error);
    }
    database_jobs.setResult (jobid, html_text.getInnerHtml ());
  }
}


string system_logic_bibles_file_name ()
{
  return filter_url_create_path (filter_url_temp_dir (), "bibles.tar");
}


void system_logic_produce_bibles_file (int jobid)
{
  Database_Jobs database_jobs;
  Database_Bibles database_bibles;

  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.newParagraph ();
    html_text.addText (translate ("Generating file with Bibles for download."));
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
      html_text.addText (translate ("The file with Bibles is ready for download."));
      html_text.newParagraph ();
      html_text.addLink (html_text.currentPDomElement, "/" + system_logic_bibles_file_name (), "", "", "", translate ("Download it."));
    } else {
      html_text.addText (translate ("It failed to create the file with Bibles."));
      html_text.newParagraph ();
      html_text.addText (error);
    }
    database_jobs.setResult (jobid, html_text.getInnerHtml ());
  }
}

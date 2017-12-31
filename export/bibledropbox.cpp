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


#include <export/bibledropbox.h>
#include <export/logic.h>
#include <tasks/logic.h>
#include <database/bibles.h>
#include <database/logs.h>
#include <database/users.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/archive.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <webserver/request.h>
#include <email/send.h>


void export_bibledropbox (string user, string bible)
{
  Webserver_Request request;
  Database_Bibles database_bibles;
  Database_Users database_users;

  
  string tag = translate ("Submit to the Bible Drop Box") + ": ";
  Database_Logs::log (tag + bible, Filter_Roles::translator ());

  
  // Temporal USFM directory.
  string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);
  

  // Take the USFM from the Bible database.
  // Generate one USFM file per book.
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    
    
    // The USFM data of the current book.
    string bookdata;
    
    
    // Collect the USFM for all chapters in this book.
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {
      // Get the USFM code for the current chapter.
      string usfm = database_bibles.getChapter (bible, book, chapter);
      usfm = filter_string_trim (usfm);
      // Add the chapter USFM code to the book's USFM code.
      bookdata.append (usfm);
      bookdata.append ("\n");
    }
    
    
    // The filename for the USFM for this book.
    string filename = Export_Logic::baseBookFileName (bible, book);
    string path = filter_url_create_path (directory, filename + ".usfm");
    
    
    // Save.
    filter_url_file_put_contents (path, bookdata);
  }
  
  
  // Compress USFM files into one zip file.
  string zipfile = filter_url_create_path (directory, Export_Logic::baseBookFileName ("", 0) + ".zip");
  
  string archive = filter_archive_zip_folder (directory);
  filter_url_rename (archive, zipfile);
  
  /*
  <form action="SubmitAction.phtml" method="post" enctype="multipart/form-data">
  <p>Your name: <input type="text" name="nameLine" size="40" /> </p>
  <p>Your email address: <input type="text" name="emailLine" size="50" /></p>
  <p>Language/Project name: <input type="text" name="projectLine" size="35" /></p>
  <p>I have the authority to submit this data: <input type="checkbox" name="permission" value="Yes">
  What is your main goal here (optional): <input type="text" name="goalLine" size="80" />
  <input type="checkbox" name="photoBible" value="Yes">
  OpenDocument <input type="checkbox" name="odfs" value="Yes">
  PDF <input type="checkbox" name="pdfs" value="Yes">
  Zip file containing your Bible file(s) <input type="file" name="uploadedZipFile" size="50" />
  <input type="hidden" name="MAX_FILE_SIZE" value="5000000" />
  <input type="file" name="uploadedMetadataFile" size="50" />
  <input type="submit" name="submit" value="Submit" />
  </form>
  */
  
  
  // Bible Drop Box submission URL.
  string url = "http://freely-given.org/Software/BibleDropBox/SubmitAction.phtml";
  
  
  // Form values to POST.
  map <string, string> post;
  post ["nameLine"] = user;
  post ["emailLine"] = database_users.get_email (user);
  post ["projectLine"] = bible;
  post ["permission"] = "Yes";
  post ["goalLine"] = translate ("Bible translation through Bibledit");
  // Just one request: Is it possible to make the Bibledit submission system default to turning off the three check-boxes for the tasks that take heavy processing on The Bible Drop Box: PhotoBible, ODFs using the Python interface to LibreOffice (which is slow compared to the Pathway method of creating the XML files directly), and PDF exports (via TeX). If the user is only after, say a Sword module, it's quite a heavy cost to wastefully produce these other exports.
  //post ["photoBible"] = "Yes";
  //post ["odfs"] = "Yes";
  //post ["pdfs"] = "Yes";
  post ["submit"] = "Submit";
  
  // Submission and response.
  string error;
  string response = filter_url_http_upload (url, post, zipfile, error);
  if (!error.empty ()) {
    Database_Logs::log (tag + error, Filter_Roles::translator ());
    email_schedule (user, "Error submitting to the Bible Drop Box", error);
  }
  size_t pos = response.find ("<head>");
  if (pos != string::npos) {
    response.insert (pos + 6, "<base href=\"http://freely-given.org/Software/BibleDropBox/\">");
  }
  email_schedule (user, "Result of your submission to the Bible Drop Box", response);

  
  // Done.
  Database_Logs::log (tag + translate("Ready"), Filter_Roles::translator ());
}

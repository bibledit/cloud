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
    string filename = Export_Logic::baseBookFileName (book);
    string path = filter_url_create_path ({directory, filename + ".usfm"});
    
    
    // Save.
    filter_url_file_put_contents (path, bookdata);
  }
  
  
  // Compress USFM files into one zip file.
  string zipfile = filter_url_create_path ({directory, Export_Logic::baseBookFileName (0) + ".zip"});
  
  string archive = filter_archive_zip_folder (directory);
  filter_url_rename (archive, zipfile);
  
  // Here is the submission form as of July 2018:
  /*
   <form action="SubmitAction.phtml" method="post" enctype="multipart/form-data">
   <p>Your name: <input type="text" name="nameLine" size="40" /> </p>
   <p>Your email address: <input type="text" name="emailLine" size="50" /></p>
   <p>Language/Project name: <input type="text" name="projectLine" size="35" /></p>
   <p>Main task (please check one or both):</p>
   <p><input type="checkbox" name="doChecks" value="Yes"> (STILL COMING) Thoroughly CHECK the submitted Bible</p> <!-- Use checked="checked" to make it default -->
   <p><small>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The following help the software to know your expectations (optional):
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;New Testament is finished:
   <input type="checkbox" name="NTfinished" value="Yes">
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Old Testament is finished:
   <input type="checkbox" name="OTfinished" value="Yes">
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Deuterocanon books are finished:
   <input type="checkbox" name="DCfinished" value="Yes">
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;All submitted books are finished:
   <input type="checkbox" name="ALLfinished" value="Yes">
   </small></p>
   <p><input type="checkbox" name="doExports" value="Yes" checked="checked"> CONVERT/EXPORT the submitted Bible <!-- Used checked="checked" to make it default -->
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;e.g., to USFM, USX, OSIS, Sword module, HTML, etc., etc.</p>
   <p><small>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The following exports use heavy processing so have to be explicitly requested:
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="https://lingtran.net/JPEG+Scripture+Publishing+for+Simple+Phones" target="_blank">PhotoBible</a>
   export (for cheap, non-Java cellphones):
   <input type="checkbox" name="photoBible" value="Yes">
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="https://en.wikipedia.org/wiki/OpenDocument" target="_blank">ODF</a>
   exports (for LibreOffice, OpenOffice, etc.):
   <input type="checkbox" name="odfs" value="Yes">
   <br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="https://en.wikipedia.org/wiki/Portable_Document_Format" target="_blank">PDF</a>
   exports (via TeX or lout):
   <input type="checkbox" name="pdfs" value="Yes">
   </small></p>
   <p>What is your main goal here (optional, but helps us):<center><input type="text" name="goalLine" size="80" /></center></p>
   <p>Zip file containing your Bible file(s) (preferably in UTF-8 encoding):<center><!--<input type="hidden" name="MAX_FILE_SIZE" value="5000000" />--><input type="file" name="uploadedZipFile" size="50" /></center></p>
   <p>Text file containing your Bible <a href="Metadata.html">metadata</a> (recommended):<center><input type="file" name="uploadedMetadataFile" size="50" /></center></p>
   <p>I have the authority to submit this data: <input type="checkbox" name="permission" value="Yes"></p>
   <p><center><input type="submit" name="submit" value="Submit" /></center></p>
   </form>
  */
  
  
  // Bible Drop Box submission URL.
  string url = "http://freely-given.org/Software/BibleDropBox/SubmitAction.phtml";
  
  
  // Form values to POST.
  map <string, string> post;
  post ["nameLine"] = user + " through " PACKAGE_STRING;
  post ["emailLine"] = database_users.get_email (user);
  post ["projectLine"] = bible;
  post ["permission"] = "Yes";
  post ["goalLine"] = translate ("Bible translation through Bibledit");
  post ["doExports"] = "Yes";
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
    response.insert (pos + 6, R"(<base href="http://freely-given.org/Software/BibleDropBox/">)");
  }
  email_schedule (user, "Result of your submission to the Bible Drop Box", response);

  
  // Done.
  Database_Logs::log (tag + translate("Ready"), Filter_Roles::translator ());
}

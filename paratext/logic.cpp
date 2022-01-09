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


// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.
#include <paratext/logic.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/roles.h>
#include <filter/usfm.h>
#include <filter/merge.h>
#include <filter/shell.h>
#ifndef HAVE_WINDOWS
#include <pwd.h>
#endif
#include <database/books.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <database/bibles.h>
#include <tasks/logic.h>
#include <locale/translate.h>
#include <bb/logic.h>
#include <client/logic.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>


string Paratext_Logic::searchProjectsFolder ()
{
  const char *homedir;

  // Try Linux.
  if ((homedir = getenv("HOME")) == NULL) {
#ifndef HAVE_WINDOWS
    homedir = getpwuid(getuid())->pw_dir;
#endif
  }
  if (homedir) {
    vector <string> files = filter_url_scandir (homedir);
    for (auto file : files) {
      if (file.find ("Paratext") != string::npos) {
        return filter_url_create_path (homedir, file);
      }
    }
  }
  
#ifdef HAVE_WINDOWS
  // Try Windows.
  homedir = "C:\\";
  vector <string> files = filter_url_scandir (homedir);
  for (auto file : files) {
    if (file.find ("Paratext") != string::npos) {
      string path = filter_url_create_path (homedir, file);
      path = filter_string_str_replace ("\\\\", "\\", path);
      return path;
    }
  }
#endif

  // No Paratext projects folder found.
  return "";
}


vector <string> Paratext_Logic::searchProjects (string projects_folder)
{
  vector <string> projects;
  vector <string> folders = filter_url_scandir (projects_folder);
  for (auto folder : folders) {
    string path = filter_url_create_path (projects_folder, folder);
    if (filter_url_is_dir (path)) {
      map <int, string> books = searchBooks (path);
      if (!books.empty ()) projects.push_back (folder);
    }
  }
  return projects;
}


map <int, string> Paratext_Logic::searchBooks (string project_path)
{
  map <int, string> books;
  vector <string> files = filter_url_scandir (project_path);
  for (auto file : files) {
    if (file.find (".BAK") != string::npos) continue;
    if (file.find ("~") != string::npos) continue;
    string path = filter_url_create_path (project_path, file);
    int id = getBook (path);
    if (id) books [id] = file;
  }
  return books;
}


int Paratext_Logic::getBook (string filename)
{
  // A USFM file should not be larger than 4 Mb and not be smaller than 7 bytes.
  int filesize = filter_url_filesize (filename);
  if (filesize < 7) return 0;
  if (filesize > 4000000) return 0;
  
  // Read a small portion of the file for higher speed.
  ifstream fin (filename);
  fin.seekg (0);
  char buffer [128];
  fin.read (buffer, 7);
  buffer [7] = 0;
  string fragment (buffer);

  // Check for "\id "
  if (fragment.find ("\\id ") == string::npos) return 0;
  fragment.erase (0, 4);
  
  // Get book from the USFM id.
  int id = Database_Books::getIdFromUsfm (fragment);
  return id;
}


void Paratext_Logic::setup (string bible, string master)
{
  if (bible.empty ()) {
    Database_Logs::log ("No Bible given for Paratext link setup.");
    return;
  }
  if (master == "bibledit") {
    copyBibledit2Paratext (bible);
    Database_Config_Bible::setParatextCollaborationEnabled (bible, true);
  } else if (master == "paratext") {
    copyParatext2Bibledit (bible);
    Database_Config_Bible::setParatextCollaborationEnabled (bible, true);
  } else {
    Database_Logs::log ("Unknown master copy for Paratext link setup.");
  }
}


void Paratext_Logic::copyBibledit2Paratext (string bible)
{
  Database_Bibles database_bibles;
  
  Database_Logs::log (translate ("Copying Bible from Bibledit to a Paratext project."));

  string paratext_project_folder = projectFolder (bible);

  Database_Logs::log (translate ("Bibledit Bible:") + " " + bible);
  Database_Logs::log (translate ("Paratext project:") + " " + paratext_project_folder);

  map <int, string> paratext_books = searchBooks (paratext_project_folder);
  
  vector <int> bibledit_books = database_bibles.getBooks (bible);
  for (int book : bibledit_books) {

    string bookname = Database_Books::getEnglishFromId (book);

    string paratext_book = paratext_books [book];

    string usfm;
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (int chapter : chapters) {
      usfm.append (database_bibles.getChapter (bible, book, chapter));
      // Add a new line after each chapter.
      usfm.append ("\n");
    }
    
    if (!paratext_book.empty ()) {

      string path = filter_url_create_path (paratext_project_folder, paratext_book);
      Database_Logs::log (bookname + ": " "Saving to:" " " + path);
      // Paratext on Windows and on Linux store the line ending with carriage return and line feed.
      filter_url_file_put_contents (path, lf2crlf (usfm));
      
      paratext_books [book].clear ();
    
    } else {

      Database_Logs::log (bookname + ": " "It could not be stored because the Paratext project does not have this book." " " "Create it, then retry.");
    
    }

    // Ancestor data needed for future merge.
    ancestor (bible, book, usfm);
  }
  
  for (auto element : paratext_books) {
    string paratext_book = element.second;
    if (paratext_book.empty ()) continue;
    Database_Logs::log (paratext_book + ": " "This Paratext project file was not affected.");
  }
}


void Paratext_Logic::copyParatext2Bibledit (string bible)
{
  Database_Bibles database_bibles;

  Database_Logs::log (translate ("Copying Paratext project to a Bible in Bibledit."));
  
  string project_folder = projectFolder (bible);
  
  Database_Logs::log (translate ("Paratext project:") + " " + project_folder);
  Database_Logs::log (translate ("Bibledit Bible:") + " " + bible);

  vector <int> bibledit_books = database_bibles.getBooks (bible);

  map <int, string> paratext_books = searchBooks (project_folder);
  for (auto element : paratext_books) {

    int book = element.first;
    string bookname = Database_Books::getEnglishFromId (book);

    string paratext_book = element.second;
    string path = filter_url_create_path (projectFolder (bible), paratext_book);

    Database_Logs::log (bookname + ": " "Scheduling import from:" " " + path);

    // It is easiest to schedule an import task.
    // The task will take care of everything, including recording what to send to the Cloud.
    tasks_logic_queue (IMPORTBIBLE, { path, bible });

    // Ancestor data needed for future merge.
    // The Paratext files have cr+lf at the end, and the ancestor data should only have lf at the end of each line.
    string usfm = crlf2lf (filter_url_file_get_contents (path));
    ancestor (bible, book, usfm);
  }
}


string Paratext_Logic::projectFolder (string bible)
{
  return filter_url_create_path (Database_Config_General::getParatextProjectsFolder (), Database_Config_Bible::getParatextProject (bible));
}


void Paratext_Logic::ancestor (string bible, int book, string usfm)
{
  string path = ancestorPath (bible, book);
  filter_url_file_put_contents (path, usfm);
}


string Paratext_Logic::ancestor (string bible, int book)
{
  string path = ancestorPath (bible, book);
  return filter_url_file_get_contents (path);
}


string Paratext_Logic::ancestorPath (string bible, int book)
{
  string path = filter_url_create_root_path ("paratext", "ancestors", bible);
  if (!file_or_dir_exists (path)) filter_url_mkdir (path);
  if (book) path = filter_url_create_path (path, convert_to_string (book));
  return path;
}


vector <string> Paratext_Logic::enabledBibles ()
{
  vector <string> enabled;
  Database_Bibles database_bibles;
  vector <string> bibles = database_bibles.getBibles ();
  for (auto bible : bibles) {
    if (Database_Config_Bible::getParatextCollaborationEnabled (bible)) {
      enabled.push_back (bible);
    }
  }
  return enabled;
}


void Paratext_Logic::synchronize ()
{
  // The Bibles for which Paratext synchronization has been enabled.
  vector <string> bibles = enabledBibles ();
  if (bibles.empty ()) return;

  
  Database_Bibles database_bibles;

  
  Database_Logs::log (synchronizeStartText (), Filter_Roles::translator ());
  
  
  string username = client_logic_get_username ();

  
  // When Bibledit writes changes to Paratext's USFM files, 
  // Paratext does not reload those changed USFM files. 
  // Thus Bibledit may overwrite changes made by others in the loaded chapter in Paratext.
  // Therefore only update the USFM files when Paratext does not run.
  bool paratext_running = false;
  vector <string> processes = filter_shell_active_processes ();
  for (auto p : processes) {
    if (p.find ("Paratext") != string::npos)
      paratext_running = true;
  }
  if (paratext_running) {
    Database_Logs::log ("Cannot synchronize while Paratext is running", Filter_Roles::translator ());
    return;
  }
  
  
  // Go through each Bible.
  for (auto bible : bibles) {
  
    
    // The Paratext project folder for the current Bible.
    string project_folder = projectFolder (bible);
    if (!file_or_dir_exists (project_folder)) {
      Database_Logs::log ("Cannot find Paratext project folder:" " " + project_folder, Filter_Roles::translator ());
      continue;
    }

    
    vector <int> bibledit_books = database_bibles.getBooks (bible);
    map <int, string> paratext_books = searchBooks (project_folder);

    
    for (auto book : bibledit_books) {

      
      // Check whether the book exists in the Paratext project, if not, skip it.
      string paratext_book = paratext_books [book];
      if (paratext_book.empty ()) {
        Database_Logs::log (journalTag (bible, book, -1) + "The Paratext project does not have this book", Filter_Roles::translator ());
        Database_Logs::log (journalTag (bible, book, -1) + "Looked for a file with " + usfm_get_opening_usfm("id") + Database_Books::getUsfmFromId (book) + " on the first line", Filter_Roles::translator ());
        continue;
      }
      

      // Ancestor USFM per chapter.
      map <int, string> ancestor_usfm;
      {
        string book_usfm = ancestor (bible, book);
        vector <int> chapters = usfm_get_chapter_numbers (book_usfm);
        for (auto chapter : chapters) {
          string chapter_usfm = usfm_get_chapter_text (book_usfm, chapter);
          ancestor_usfm [chapter] = chapter_usfm;
        }
      }


      // Paratext USFM per chapter.
      // Remove the carriage return that Paratext stores on both Windows and Linux.
      map <int, string> paratext_usfm;
      {
        string path = filter_url_create_path (projectFolder (bible), paratext_book);
        string book_usfm = crlf2lf (filter_url_file_get_contents (path));
        // Paratext on Linux has been seen adding empty lines right after \c (chapter).
        // It does that after syncing with Bibledit and editing the chapter in Paratext.
        // This looks like a bug in Paratext. Remove those empty lines.
        book_usfm = filter_string_str_replace ("\n\n", "\n", book_usfm);
        vector <int> chapters = usfm_get_chapter_numbers (book_usfm);
        for (auto chapter : chapters) {
          string chapter_usfm = usfm_get_chapter_text (book_usfm, chapter);
          paratext_usfm [chapter] = chapter_usfm;
        }
      }

      
      // Assemble the available chapters in this book
      // by combining the available chapters in the Bible in Bibledit
      // with the available chapters in the relevant Paratext project.
      vector <int> chapters = database_bibles.getChapters (bible, book);
      for (auto element : paratext_usfm) {
        chapters.push_back (element.first);
      }
      chapters = array_unique (chapters);
      sort (chapters.begin(), chapters.end());
      

      bool book_is_updated = false;
      
      
      for (int chapter : chapters) {
        
        string usfm;
        string ancestor = ancestor_usfm [chapter];
        string bibledit = database_bibles.getChapter (bible, book, chapter);
        string paratext = paratext_usfm [chapter];

        vector <string> messages;
        bool merged;
        vector <Merge_Conflict> conflicts;

        // Run the synchronizer.
        usfm = synchronize (ancestor, bibledit, paratext, messages, merged, conflicts);
        
        // If there was a result of syncing, set the ancestor and paratext data.
        if (!usfm.empty ()) {
          ancestor_usfm [chapter] = usfm;
          paratext_usfm [chapter] = usfm;
        }
        
        // Messages for the logbook.
        for (auto message : messages) {
          Database_Logs::log (journalTag (bible, book, chapter) + message, Filter_Roles::translator ());
        }

        // Log the change due to a merge.
        if (merged) {
          bible_logic_log_change (bible, book, chapter, usfm, "", "Paratext", true);
        }

        // If there's any conflicts, email full details about the conflict to the user.
        // This may enable the user to resolve conflicts manually.
        filter_merge_add_book_chapter (conflicts, book, chapter);
        bible_logic_merge_irregularity_mail ({ username }, conflicts);
        
        // Store the updated chapter in Bibledit.
        if (!usfm.empty ()) {
          // Set flag for saving to Paratext.
          book_is_updated = true;
          // Store it only in case the Bibledit data was updated.
          // https://github.com/bibledit/cloud/issues/339
          if (usfm != bibledit) {
            bible_logic_store_chapter (bible, book, chapter, usfm);
          }
        }

      }

      
      // The whole book has now been dealt with here at this point.
      // The Bibledit data is already up to date or has been updated.
      // In case of any updates made in this book, update the ancestor data and the Paratext data.
      if (book_is_updated) {
        
        string usfm;
        for (auto element : ancestor_usfm) {
          string data = element.second;
          if (!data.empty ()) {
            if (!usfm.empty ()) usfm.append ("\n");
            usfm.append (filter_string_trim (data));
          }
        }
        ancestor (bible, book, usfm);
        
        usfm.clear ();
        for (auto element : paratext_usfm) {
          string data = element.second;
          if (!data.empty ()) {
            if (!usfm.empty ()) usfm.append ("\n");
            usfm.append (filter_string_trim (data));
          }
        }
        string path = filter_url_create_path (projectFolder (bible), paratext_book);
        filter_url_file_put_contents (path, lf2crlf (usfm));
      }
    }
  }
  
  Database_Logs::log (synchronizeReadyText (), Filter_Roles::translator ());
}


string Paratext_Logic::synchronize (string ancestor, string bibledit, string paratext,
                                    vector <string> & messages,
                                    bool & merged, vector <Merge_Conflict> & conflicts)
{
  string resulting_usfm;

  messages.clear ();
  merged = false;
  conflicts.clear ();
  
  // If Bibledit has the chapter, and Paratext does not, take the Bibledit chapter.
  if (!bibledit.empty () && paratext.empty ()) {
    resulting_usfm = bibledit;
    messages.push_back (translate ("Copy Bibledit to Paratext"));
  }

  // If Paratext has the chapter, and Bibledit does not, take the Paratext chapter.
  else if (bibledit.empty () && !paratext.empty ()) {
    resulting_usfm = paratext;
    messages.push_back (translate ("Copy Paratext to Bibledit"));
  }

  // Bibledit and Paratext are the same: Do nothing.
  else if (filter_string_trim (bibledit) == filter_string_trim (paratext)) {
  }

  // If the chapter in Bibledit is much larger than the chapter in Paratext,
  // it probably means that the chapter in Paratext was created,
  // and that the Paratext chapter has the outline only, without any text.
  // So in this case take the chapter from Bibledit.
  else if (bibledit.size () > (paratext.size() * 3)) {
    resulting_usfm = bibledit;
    messages.push_back (translate ("Copy larger Bibledit chapter to smaller Paratext chapter"));
  }

  // If ancestor data exists, and Bibledit and Paratext differ,
  // merge both chapters, giving preference to Paratext,
  // as Paratext is more likely to contain the preferred version,
  // since it is assumed that perhaps a Manager runs Paratext,
  // and perhaps Translators run Bibledit.
  // But this assumption may be wrong.
  // Nevertheless preference must be given to some data anyway.
  else if (!ancestor.empty ()) {
    resulting_usfm = filter_merge_run (ancestor, bibledit, paratext, true, conflicts);
    messages.push_back (translate ("Chapter merged"));
    merged = true;
  }
  
  // Cannot merge the two.
  else {
    messages.push_back (translate("Cannot merge chapter due to missing parent data"));
  }

  return resulting_usfm;
}


string Paratext_Logic::synchronizeStartText ()
{
  return translate ("Paratext: Send/receive");
}


string Paratext_Logic::synchronizeReadyText ()
{
  return translate ("Paratext: Up to date");
}


// Create tag for the journal.
// If chapter is negative, it is left out from the tag.
string Paratext_Logic::journalTag (string bible, int book, int chapter)
{
  string bookname = Database_Books::getEnglishFromId (book);
  string project = Database_Config_Bible::getParatextProject (bible);
  string fragment = bible + " <> " + project + " " + bookname;
  if (chapter >= 0) fragment.append (" " + convert_to_string (chapter));
  fragment.append (": ");
  return fragment;
}

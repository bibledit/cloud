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


std::string Paratext_Logic::searchProjectsFolder ()
{
  const char* homedir {nullptr};

  // Try Linux.
  if ((homedir = getenv("HOME")) == nullptr) {
#ifndef HAVE_WINDOWS
    homedir = getpwuid(getuid())->pw_dir;
#endif
  }
  if (homedir) {
    std::vector <std::string> files = filter_url_scandir (homedir);
    for (auto file : files) {
      if (file.find ("Paratext") != std::string::npos) {
        return filter_url_create_path ({homedir, file});
      }
    }
  }
  
#ifdef HAVE_WINDOWS
  // Try Windows.
  homedir = "C:\\";
  std::vector <std::string> files = filter_url_scandir (homedir);
  for (auto file : files) {
    if (file.find ("Paratext") != std::string::npos) {
      std::string path = filter_url_create_path ({homedir, file});
      path = filter::string::replace ("\\\\", "\\", path);
      return path;
    }
  }
#endif

  // No Paratext projects folder found.
  return std::string();
}


std::vector <std::string> Paratext_Logic::searchProjects (std::string projects_folder)
{
  std::vector <std::string> projects;
  std::vector <std::string> folders = filter_url_scandir (projects_folder);
  for (auto folder : folders) {
    std::string path = filter_url_create_path ({projects_folder, folder});
    if (filter_url_is_dir (path)) {
      std::map <int, std::string> books = searchBooks (path);
      if (!books.empty ()) projects.push_back (folder);
    }
  }
  return projects;
}


std::map <int, std::string> Paratext_Logic::searchBooks (std::string project_path)
{
  std::map <int, std::string> books;
  std::vector <std::string> files = filter_url_scandir (project_path);
  for (auto file : files) {
    if (file.find (".BAK") != std::string::npos) continue;
    if (file.find ("~") != std::string::npos) continue;
    std::string path = filter_url_create_path ({project_path, file});
    int id = getBook (path);
    if (id) books [id] = file;
  }
  return books;
}


int Paratext_Logic::getBook (std::string filename)
{
  // A USFM file should not be larger than 4 Mb and not be smaller than 7 bytes.
  int filesize = filter_url_filesize (filename);
  if (filesize < 7) return 0;
  if (filesize > 4000000) return 0;
  
  // Read a small portion of the file for higher speed.
  std::ifstream fin (filename);
  fin.seekg (0);
  char buffer [128];
  fin.read (buffer, 7);
  buffer [7] = 0;
  std::string fragment (buffer);

  // Check for "\id "
  if (fragment.find ("\\id ") == std::string::npos) return 0;
  fragment.erase (0, 4);
  
  // Get book from the USFM id.
  book_id id = database::books::get_id_from_usfm (fragment);
  return static_cast<int>(id);
}


void Paratext_Logic::setup (std::string bible, std::string master)
{
  if (bible.empty ()) {
    Database_Logs::log ("No Bible given for Paratext link setup.");
    return;
  }
  if (master == "bibledit") {
    copyBibledit2Paratext (bible);
    database::config::bible::set_paratext_collaboration_enabled (bible, true);
  } else if (master == "paratext") {
    copyParatext2Bibledit (bible);
    database::config::bible::set_paratext_collaboration_enabled (bible, true);
  } else {
    Database_Logs::log ("Unknown master copy for Paratext link setup.");
  }
}


void Paratext_Logic::copyBibledit2Paratext (std::string bible)
{
  Database_Logs::log (translate ("Copying Bible from Bibledit to a Paratext project."));

  std::string paratext_project_folder = projectFolder (bible);

  Database_Logs::log (translate ("Bibledit Bible:") + " " + bible);
  Database_Logs::log (translate ("Paratext project:") + " " + paratext_project_folder);

  std::map <int, std::string> paratext_books = searchBooks (paratext_project_folder);
  
  std::vector <int> bibledit_books = database::bibles::get_books (bible);
  for (int book : bibledit_books) {

    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));

    std::string paratext_book = paratext_books [book];

    std::string usfm;
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (int chapter : chapters) {
      usfm.append (database::bibles::get_chapter (bible, book, chapter));
      // Add a new line after each chapter.
      usfm.append ("\n");
    }
    
    if (!paratext_book.empty ()) {

      std::string path = filter_url_create_path ({paratext_project_folder, paratext_book});
      Database_Logs::log (bookname + ": " "Saving to:" " " + path);
      // Paratext on Windows and on Linux store the line ending with carriage return and line feed.
      filter_url_file_put_contents (path, filter::string::lf2crlf (usfm));
      
      paratext_books [book].clear ();
    
    } else {

      Database_Logs::log (bookname + ": " "It could not be stored because the Paratext project does not have this book." " " "Create it, then retry.");
    
    }

    // Ancestor data needed for future merge.
    ancestor (bible, book, usfm);
  }
  
  for (auto element : paratext_books) {
    std::string paratext_book = element.second;
    if (paratext_book.empty ()) continue;
    Database_Logs::log (paratext_book + ": " "This Paratext project file was not affected.");
  }
}


void Paratext_Logic::copyParatext2Bibledit (std::string bible)
{
  Database_Logs::log (translate ("Copying Paratext project to a Bible in Bibledit."));
  
  std::string project_folder = projectFolder (bible);
  
  Database_Logs::log (translate ("Paratext project:") + " " + project_folder);
  Database_Logs::log (translate ("Bibledit Bible:") + " " + bible);

  std::vector <int> bibledit_books = database::bibles::get_books (bible);

  std::map <int, std::string> paratext_books = searchBooks (project_folder);
  for (auto element : paratext_books) {

    int book = element.first;
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));

    std::string paratext_book = element.second;
    std::string path = filter_url_create_path ({projectFolder (bible), paratext_book});

    Database_Logs::log (bookname + ": " "Scheduling import from:" " " + path);

    // It is easiest to schedule an import task.
    // The task will take care of everything, including recording what to send to the Cloud.
    tasks_logic_queue (task::import_bible, { path, bible });

    // Ancestor data needed for future merge.
    // The Paratext files have cr+lf at the end, and the ancestor data should only have lf at the end of each line.
    std::string usfm = filter::string::crlf2lf (filter_url_file_get_contents (path));
    ancestor (bible, book, usfm);
  }
}


std::string Paratext_Logic::projectFolder (std::string bible)
{
  return filter_url_create_path ({database::config::general::get_paratext_projects_folder (), database::config::bible::get_paratext_project (bible)});
}


void Paratext_Logic::ancestor (std::string bible, int book, std::string usfm)
{
  std::string path = ancestorPath (bible, book);
  filter_url_file_put_contents (path, usfm);
}


std::string Paratext_Logic::ancestor (std::string bible, int book)
{
  std::string path = ancestorPath (bible, book);
  return filter_url_file_get_contents (path);
}


std::string Paratext_Logic::ancestorPath (std::string bible, int book)
{
  std::string path = filter_url_create_root_path ({"paratext", "ancestors", bible});
  if (!file_or_dir_exists (path)) filter_url_mkdir (path);
  if (book) path = filter_url_create_path ({path, std::to_string (book)});
  return path;
}


std::vector <std::string> Paratext_Logic::enabledBibles ()
{
  std::vector <std::string> enabled;
  std::vector <std::string> bibles = database::bibles::get_bibles ();
  for (auto bible : bibles) {
    if (database::config::bible::get_paratext_collaboration_enabled (bible)) {
      enabled.push_back (bible);
    }
  }
  return enabled;
}


void Paratext_Logic::synchronize (tasks::enums::paratext_sync method)
{
  // The Bibles for which Paratext synchronization has been enabled.
  std::vector <std::string> bibles = enabledBibles ();
  if (bibles.empty ()) return;

  
  Database_Logs::log (synchronizeStartText (), roles::translator);
  
  
  const std::string username = client_logic_get_username ();

  
  // When Bibledit writes changes to Paratext's USFM files, 
  // Paratext does not reload those changed USFM files. 
  // Thus Bibledit may overwrite changes made by others in the loaded chapter in Paratext.
  // Therefore only update the USFM files when Paratext does not run.
  bool paratext_running = false;
  std::vector <std::string> processes = filter::shell::active_processes ();
  for (auto p : processes) {
    if (p.find ("Paratext") != std::string::npos)
      paratext_running = true;
  }
  if (paratext_running) {
    Database_Logs::log ("Cannot synchronize while Paratext is running", roles::translator);
    return;
  }
  
  
  // Go through each Bible.
  for (auto bible : bibles) {
  
    
    // The Paratext project folder for the current Bible.
    std::string project_folder = projectFolder (bible);
    if (!file_or_dir_exists (project_folder)) {
      Database_Logs::log ("Cannot find Paratext project folder:" " " + project_folder, roles::translator);
      continue;
    }

    
    std::vector <int> bibledit_books = database::bibles::get_books (bible);
    std::map <int, std::string> paratext_books = searchBooks (project_folder);

    
    for (auto book : bibledit_books) {

      
      // Check whether the book exists in the Paratext project, if not, skip it.
      std::string paratext_book = paratext_books [book];
      if (paratext_book.empty ()) {
        Database_Logs::log (journalTag (bible, book, -1) + "The Paratext project does not have this book", roles::translator);
        Database_Logs::log (journalTag (bible, book, -1) + "Looked for a file with " + filter::usfm::get_opening_usfm("id") + database::books::get_usfm_from_id (static_cast<book_id>(book)) + " on the first line", roles::translator);
        continue;
      }
      

      // Ancestor USFM per chapter.
      std::map <int, std::string> ancestor_usfm;
      {
        std::string book_usfm = ancestor (bible, book);
        std::vector <int> chapters = filter::usfm::get_chapter_numbers (book_usfm);
        for (auto chapter : chapters) {
          std::string chapter_usfm = filter::usfm::get_chapter_text (book_usfm, chapter);
          ancestor_usfm [chapter] = chapter_usfm;
        }
      }


      // Paratext USFM per chapter.
      // Remove the carriage return that Paratext stores on both Windows and Linux.
      std::map <int, std::string> paratext_usfm;
      {
        std::string path = filter_url_create_path ({projectFolder (bible), paratext_book});
        std::string book_usfm = filter::string::crlf2lf (filter_url_file_get_contents (path));
        // Paratext on Linux has been seen adding empty lines right after \c (chapter).
        // It does that after syncing with Bibledit and editing the chapter in Paratext.
        // This looks like a bug in Paratext. Remove those empty lines.
        book_usfm = filter::string::replace ("\n\n", "\n", book_usfm);
        std::vector <int> chapters = filter::usfm::get_chapter_numbers (book_usfm);
        for (auto chapter : chapters) {
          std::string chapter_usfm = filter::usfm::get_chapter_text (book_usfm, chapter);
          paratext_usfm [chapter] = chapter_usfm;
        }
      }

      
      // Assemble the available chapters in this book
      // by combining the available chapters in the Bible in Bibledit
      // with the available chapters in the relevant Paratext project.
      std::vector <int> chapters = database::bibles::get_chapters (bible, book);
      for (auto element : paratext_usfm) {
        chapters.push_back (element.first);
      }
      chapters = filter::string::array_unique (chapters);
      sort (chapters.begin(), chapters.end());
      

      bool book_is_updated = false;
      
      
      for (int chapter : chapters) {
        
        std::string ancestor = ancestor_usfm [chapter];
        std::string bibledit = database::bibles::get_chapter (bible, book, chapter);
        std::string paratext = paratext_usfm [chapter];

        // Results of the merge or copy operations.
        std::vector <std::string> messages;
        std::vector <Merge_Conflict> conflicts;
        std::string updated_usfm;
        
        // Handle the synchronization method.
        switch (method) {
          case tasks::enums::paratext_sync::none:
          {
            break;
          }
          case tasks::enums::paratext_sync::bi_directional:
          {
            // Run the merge synchronizer.
            updated_usfm = synchronize (ancestor, bibledit, paratext, messages, conflicts);
            break;
          }
          case tasks::enums::paratext_sync::bibledit_to_paratext:
          {
            // If there's a change between Bibledit and Paratext, take the Bibledit data.
            if (bibledit != paratext) {
              updated_usfm = bibledit;
              messages.push_back (translate ("Copy Bibledit to Paratext"));
            }
            break;
          }
          case tasks::enums::paratext_sync::paratext_to_bibledit:
          {
            // If there's a change between Paratext and Bibledit, take the Paratext data.
            if (paratext != bibledit) {
              updated_usfm = paratext;
              messages.push_back (translate ("Copy Paratext to Bibledit"));
            }
            break;
          }
          default:
            break;
        }

        // If there was a result of syncing or copying, set the ancestor and paratext data.
        if (!updated_usfm.empty ()) {
          ancestor_usfm [chapter] = updated_usfm;
          paratext_usfm [chapter] = updated_usfm;
        }
        
        // Messages for the logbook.
        for (auto message : messages) {
          Database_Logs::log (journalTag (bible, book, chapter) + message, roles::translator);
        }

        // Log the change due to a merge or copy.
        if (!updated_usfm.empty ()) {
          bible_logic::log_change (bible, book, chapter, updated_usfm, "", "Paratext", true);
        }

        // If there's any conflicts, email full details about the conflict to the user.
        // This may enable the user to resolve conflicts manually.
        filter_merge_add_book_chapter (conflicts, book, chapter);
        bible_logic::merge_irregularity_mail ({ username }, conflicts);
        
        // Store the updated chapter in Bibledit.
        if (!updated_usfm.empty ()) {
          // Set flag for saving to Paratext.
          book_is_updated = true;
          // Store it only in case the Bibledit data was updated.
          // https://github.com/bibledit/cloud/issues/339
          if (updated_usfm != bibledit) {
            bible_logic::store_chapter (bible, book, chapter, updated_usfm);
          }
        }

      }

      
      // The whole book has now been dealt with here at this point.
      // The Bibledit data is already up to date or has been updated.
      // In case of any updates made in this book, update the ancestor data and the Paratext data.
      if (book_is_updated) {
        
        std::string usfm;
        for (auto element : ancestor_usfm) {
          std::string data = element.second;
          if (!data.empty ()) {
            if (!usfm.empty ()) usfm.append ("\n");
            usfm.append (filter::string::trim (data));
          }
        }
        ancestor (bible, book, usfm);
        
        usfm.clear ();
        for (auto element : paratext_usfm) {
          std::string data = element.second;
          if (!data.empty ()) {
            if (!usfm.empty ()) usfm.append ("\n");
            usfm.append (filter::string::trim (data));
          }
        }
        std::string path = filter_url_create_path ({projectFolder (bible), paratext_book});
        filter_url_file_put_contents (path, filter::string::lf2crlf (usfm));
      }
    }
  }
  
  Database_Logs::log (synchronizeReadyText (), roles::translator);
}


std::string Paratext_Logic::synchronize (std::string ancestor, std::string bibledit, std::string paratext,
                                         std::vector <std::string> & messages,
                                         std::vector <Merge_Conflict> & conflicts)
{
  std::string resulting_usfm;

  messages.clear ();
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
  else if (filter::string::trim (bibledit) == filter::string::trim (paratext)) {
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
  }
  
  // Cannot merge the two.
  else {
    messages.push_back (translate("Cannot merge chapter due to missing parent data"));
  }

  return resulting_usfm;
}


std::string Paratext_Logic::synchronizeStartText ()
{
  return translate ("Paratext: Send/receive");
}


std::string Paratext_Logic::synchronizeReadyText ()
{
  return translate ("Paratext: Up to date");
}


// Create tag for the journal.
// If chapter is negative, it is left out from the tag.
std::string Paratext_Logic::journalTag (std::string bible, int book, int chapter)
{
  std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  std::string project = database::config::bible::get_paratext_project (bible);
  std::string fragment = bible + " <> " + project + " " + bookname;
  if (chapter >= 0) fragment.append (" " + std::to_string (chapter));
  fragment.append (": ");
  return fragment;
}

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


#include <filter/git.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/shell.h>
#include <filter/merge.h>
#include <database/logs.h>
#include <database/books.h>
#include <database/jobs.h>
#include <database/git.h>
#include <database/config/general.h>
#include <bb/logic.h>
#include <locale/translate.h>
#include <rss/logic.h>


#ifdef HAVE_CLOUD


// This function returns the directory of the git repository belonging to $object.
std::string filter_git_directory (std::string object)
{
  return filter_url_create_root_path ({"git", object});
}


void filter_git_check_error (std::string data)
{
  std::vector <std::string> lines = filter::strings::explode (data, '\n');
  for (auto & line : lines) Database_Logs::log (line);
}


// Runs the equivalent of "git init".
bool filter_git_init (std::string directory, bool bare)
{
  std::vector <std::string> parameters = {"init"};
  if (bare) parameters.push_back ("--bare");
  std::string output, error;
  int result = filter_shell_run (directory, "git", parameters, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  return (result == 0);
}


// Internal function that commits a user-generated change to the git repository.
void filter_git_commit_modification_to_git (std::string repository, std::string user, int book, int chapter,
                                            std::string & oldusfm, std::string & newusfm)
{
  std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  std::string bookdir = filter_url_create_path ({repository, bookname});
  std::string chapterdir = filter_url_create_path ({bookdir, std::to_string (chapter)});
  if (!file_or_dir_exists (chapterdir)) filter_url_mkdir (chapterdir);
  std::string datafile = filter_url_create_path ({chapterdir, "data"});
  std::string contents = filter_url_file_get_contents (datafile);
  if (contents != oldusfm) {
    filter_url_file_put_contents (datafile, oldusfm);
    std::string error;
    filter_git_add_remove_all (repository, error);
    std::vector <std::string> messages;
    filter_git_commit (repository, "", "System-generated to clearly display user modification in next commit", messages, error);
  }
  filter_url_file_put_contents (datafile, newusfm);
  std::string error;
  filter_git_add_remove_all (repository, error);
  std::vector <std::string> messages;
  filter_git_commit (repository, user, "User modification", messages, error);
}


// This filter stores the changes made by users on $bible in $repository.
// This puts commits in the repository, where the author is the user who made the changes.
// This information in the git repository can then be used for statistical or other purposes.
void filter_git_sync_modifications_to_git (std::string bible, std::string repository)
{
  // Go through all the users who saved data to this Bible.
  std::vector <std::string> users = database::git::get_users (bible);
  for (auto & user : users) {
    
    bool iteration_initialized = false;
    std::string overall_old_usfm, overall_new_usfm;
    int overall_book = 0, overall_chapter = 0;
    
    // Go through all the rowids for the user and the Bible.
    std::vector <int> rowids = database::git::get_rowids (user, bible);
    for (auto rowid : rowids) {

      std::string s;
      std::string oldusfm, newusfm;
      int book, chapter;
      database::git::get_chapter (rowid, s, s, book, chapter, oldusfm, newusfm);
      
      if (iteration_initialized) {
        // Look at the sequences of old and new USFM, and join the matching changes together,
        // to make one large change that contains all sequential small changes.
        if (oldusfm == overall_new_usfm) {
          overall_new_usfm = newusfm;
        } else {
          filter_git_commit_modification_to_git (repository, user, overall_book, overall_chapter,
                                                 overall_old_usfm, overall_new_usfm);
          iteration_initialized = false;
        }
      }
      
      if (!iteration_initialized) {
        // Initialize the large overall book/chapter/USFM from the first change set.
        overall_book = book;
        overall_chapter = chapter;
        overall_old_usfm = oldusfm;
        overall_new_usfm = newusfm;
        iteration_initialized = true;
      }

      // This record has been processed.
      database::git::erase_rowid (rowid);

    }
    
    if (iteration_initialized) {
      // Commit the final overall modification.
      filter_git_commit_modification_to_git (repository, user, overall_book, overall_chapter,
                                             overall_old_usfm, overall_new_usfm);
    }
  
  }
}


// This filter takes the Bible data as it is stored in Bibledit's database,
// and puts this information into the layout in books and chapters
// such as is used in Bibledit-Gtk into the $git folder.
// The $git is a git repository, and may contain other data as well.
// The filter focuses on reading the data in the git repository, and only writes to it if necessary,
// This speeds up the filter.
void filter_git_sync_bible_to_git (Webserver_Request& webserver_request, std::string bible, std::string repository)
{
  // First stage.
  // Read the chapters in the git repository,
  // and check if they occur in the database.
  // If a chapter is not in the database, remove it from the repository.
  std::vector <int> books = database::bibles::get_books (bible);
  std::vector <std::string> bookfiles = filter_url_scandir (repository);
  for (auto & bookname : bookfiles) {
    std::string path = filter_url_create_path ({repository, bookname});
    if (filter_url_is_dir (path)) {
      int book = static_cast<int>(database::books::get_id_from_english (bookname));
      if (book) {
        if (in_array (book, books)) {
          // Book exists in the database: Check the chapters.
          std::vector <int> chapters = database::bibles::get_chapters (bible, book);
          std::vector <std::string> chapterfiles = filter_url_scandir (filter_url_create_path ({repository, bookname}));
          for (auto & chaptername : chapterfiles) {
            std::string chapter_path = filter_url_create_path ({repository, bookname, chaptername});
            if (filter_url_is_dir (chapter_path)) {
              if (filter::strings::is_numeric (chaptername)) {
                int chapter = filter::strings::convert_to_int (chaptername);
                std::string filename = filter_url_create_path ({repository, bookname, chaptername, "data"});
                if (file_or_dir_exists (filename)) {
                  if (!in_array (chapter, chapters)) {
                    // Chapter does not exist in the database.
                    filter_url_rmdir (filter_url_create_path ({repository, bookname, chaptername}));
                  }
                }
              }
            }
          }
        } else {
          // Book does not exist in the database: Remove it from $git.
          filter_url_rmdir (filter_url_create_path ({repository, bookname}));
        }
      }
    }
  }
  
  // Second stage.
  // Read the books / chapters from the database,
  // and check if they occur in the repository, and the data matches.
  // If necessary, save the chapter to the repository.
  books = database::bibles::get_books (bible);
  for (auto & book : books) {
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    std::string bookdir = filter_url_create_path ({repository, bookname});
    if (!file_or_dir_exists (bookdir)) filter_url_mkdir (bookdir);
    std::vector <int> chapters = database::bibles::get_chapters (bible, book);
    for (auto & chapter : chapters) {
      std::string chapterdir = filter_url_create_path ({bookdir, std::to_string (chapter)});
      if (!file_or_dir_exists (chapterdir)) filter_url_mkdir (chapterdir);
      std::string datafile = filter_url_create_path ({chapterdir, "data"});
      std::string contents = filter_url_file_get_contents (datafile);
      std::string usfm = database::bibles::get_chapter (bible, book, chapter);
      if (contents != usfm) filter_url_file_put_contents (datafile, usfm);
    }
  }
}


// This filter takes the Bible data as it is stored in the git $repository folder,
// and puts this information into Bibledit's database.
// The $repository is a git repository, and may contain other data as well.
// The filter focuses on reading the data in the git repository and the database,
// and only writes to the database if necessary,
// This speeds up the filter.
void filter_git_sync_git_to_bible (Webserver_Request& webserver_request, std::string repository, std::string bible)
{
  // Stage one:
  // Read the chapters in the git repository,
  // and check that they occur in the database.
  // If any does not occur, add the chapter to the database.
  // This stage does not check the contents of the chapters.
  std::vector <std::string> bookfiles = filter_url_scandir (repository);
  for (auto & bookname : bookfiles) {
    std::string bookpath = filter_url_create_path ({repository, bookname});
    if (filter_url_is_dir (bookpath)) {
      int book = static_cast<int>(database::books::get_id_from_english (bookname));
      if (book) {
        // Check the chapters.
        std::vector <int> chapters = database::bibles::get_chapters (bible, book);
        std::vector <std::string> chapterfiles = filter_url_scandir (bookpath);
        for (auto & chapterfile : chapterfiles) {
          std::string chapterpath = filter_url_create_path ({bookpath, chapterfile});
          if (filter_url_is_dir (chapterpath)) {
            if (filter::strings::is_numeric (chapterfile)) {
              int chapter = filter::strings::convert_to_int (chapterfile);
              std::string filename = filter_url_create_path ({chapterpath, "data"});
              if (file_or_dir_exists (filename)) {
                if (!in_array (chapter, chapters)) {
                  // Chapter does not exist in the database: Add it.
                  std::string usfm = filter_url_file_get_contents (filename);
                  bible_logic::store_chapter (bible, book, chapter, usfm);
                  // Log it.
                  std::string message = translate("A translator added chapter") + " " + bible + " " + bookname + " " + chapterfile;
                  Database_Logs::log (message);
                }
              }
            }
          }
        }
      }
    }
  }
  
  
  // Stage two:
  // Read through the chapters in the database,
  // and check that they occur in the git folder.
  // If necessary, remove a chapter from the database.
  // If a chapter matches, check that the contents of the data in the git
  // folder and the contents in the database match.
  // If necessary, update the data in the database.
  std::vector <int> books = database::bibles::get_books (bible);
  for (auto & book : books) {
    std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
    std::string bookdir = filter_url_create_path ({repository, bookname});
    if (file_or_dir_exists (bookdir)) {
      std::vector <int> chapters = database::bibles::get_chapters (bible, book);
      for (auto & chapter : chapters) {
        std::string chapterdir = filter_url_create_path ({bookdir, std::to_string (chapter)});
        if (file_or_dir_exists (chapterdir)) {
          std::string datafile = filter_url_create_path ({chapterdir, "data"});
          std::string contents = filter_url_file_get_contents (datafile);
          std::string usfm = database::bibles::get_chapter (bible, book, chapter);
          if (contents != usfm) {
            bible_logic::store_chapter (bible, book, chapter, contents);
            Database_Logs::log (translate("A translator updated chapter") + " " + bible + " " + bookname + " " + std::to_string (chapter));
            rss_logic_schedule_update ("collaborator", bible, book, chapter, usfm, contents);
          }
        } else {
          bible_logic::delete_chapter (bible, book, chapter);
          Database_Logs::log (translate("A translator deleted chapter") + " " + bible + " " + bookname + " " + std::to_string (chapter));
        }
      }
    } else {
      bible_logic::delete_book (bible, book);
      Database_Logs::log (translate("A translator deleted book") + " " + bible + " " + bookname);
    }
  }
}


std::string filter_git_disabled ()
{
  return "Git has been disabled on iOS and Android, and can be enabled on Linux, Windows and macOS";
}


// This filter takes one chapter of the Bible data as it is stored in the $git folder,
// and puts this information into Bibledit's database.
// The $git is a git repository, and may contain other data as well.
void filter_git_sync_git_chapter_to_bible (std::string repository, std::string bible, int book, int chapter)
{
  // Filename for the chapter.
  std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  std::string filename = filter_url_create_path ({repository, bookname, std::to_string (chapter), "data"});
  
  if (file_or_dir_exists (filename)) {
    
    // Store chapter in database and log it.
    std::string existing_usfm = database::bibles::get_chapter (bible, book, chapter);
    std::string usfm = filter_url_file_get_contents (filename);
    bible_logic::log_change (bible, book, chapter, usfm, "collaborator", "Chapter updated from git repository", false);
    bible_logic::store_chapter (bible, book, chapter, usfm);
    rss_logic_schedule_update ("collaborator", bible, book, chapter, existing_usfm, usfm);
    
  } else {
    
    // Delete chapter from database.
    bible_logic::delete_chapter (bible, book, chapter);
    Database_Logs::log (translate("A collaborator deleted chapter") + " " + bible + " " + bookname + " " + std::to_string (chapter));
    
  }
}


// Returns true if the git repository at "url" is online.
bool filter_git_remote_read (std::string url, std::string & error)
{
  std::string output;
  int result = filter_shell_run ("", "git", {"ls-remote", url}, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  return (result == 0);
}


bool filter_git_remote_clone (std::string url, std::string path, [[maybe_unused]] int jobid, std::string & error)
{
  // Clear a possible existing git repository directory.
  filter_url_rmdir (path);

  std::string output;
  int result = filter_shell_run ("", "git", {"clone", url, path}, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  error.clear ();
  return (result == 0);
}


bool filter_git_add_remove_all (std::string repository, std::string & error)
{
  std::string output;
  int result = filter_shell_run (repository, "git", {"add", "--all", "."}, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  return (result == 0);
}


// This function runs "git commit" through the shell.
bool filter_git_commit (std::string repository, std::string user, std::string message,
                        std::vector <std::string> & messages, std::string & error)
{
  user = filter_git_user (user);
  std::string email = filter_git_email (user);
  std::stringstream author;
  author << "--author=" << std::quoted(user + " <" + email + ">");
  std::string out, err;
  int result = filter_shell_run (repository, "git",
                                {"commit",
                                 author.str(),
                                 "-a",
                                 "-m",
                                 message
                                }, &out, &err);
  out = filter::strings::trim (out);
  err = filter::strings::trim (err);
  error = err;
  filter_git_check_error (error);
  messages = filter::strings::explode (out, '\n');
  std::vector <std::string> lines = filter::strings::explode (err, '\n');
  messages.insert (messages.end(), lines.begin(), lines.end());
  
  // In case of Your branch is up-to-date with 'origin/master'. nothing to commit, working directory clean,
  // git returns exit code 256. Yet this is not an error.
  if (out.find ("nothing to commit") != std::string::npos) result = 0;
  
  return (result == 0);
}


void filter_git_config_set_bool (std::string repository, std::string name, bool value)
{
  std::string svalue = value ? "true" : "false";
  filter_git_config_set_string (repository, name, svalue);
}


void filter_git_config_set_int (std::string repository, std::string name, int value)
{
  std::string svalue = std::to_string (value);
  filter_git_config_set_string (repository, name, svalue);
}


void filter_git_config_set_string (std::string repository, std::string name, std::string value)
{
  std::string output, error;
  filter_shell_run (repository, "git", {"config", name, value}, &output, &error);
}


// This filter takes a $line of the output of the git pull command.
// It tries to interpret it to find a passage that would have been updated.
// If a valid book and chapter are found, it returns them.
Passage filter_git_get_passage (std::string line)
{
  // Sample lines for git pull:

  // "From https://github.com/joe/test"
  // "   443579b..90dcb57  master     -> origin/master"
  // "Updating 443579b..90dcb57"
  // "Fast-forward"
  // " Genesis/1/data | 2 +-"
  // " 1 file changed, 1 insertion(+), 1 deletion(-)"
  // " delete mode 100644 Leviticus/1/data"
  // " create mode 100644 Leviticus/2/data"

  // Sample lines for git status:

  // On branch master
  // Your branch is up-to-date with 'origin/master'.
  
  // Changes not staged for commit:
  //   (use "git add <file>..." to update what will be committed)
  //   (use "git checkout -- <file>..." to discard changes in working directory)
  //      modified:   Genesis/1/data
  // no changes added to commit (use "git add" and/or "git commit -a")
  
  Passage passage;
  std::vector <std::string> bits = filter::strings::explode (line, '/');
  if (bits.size () == 3) {
    size_t pos = bits [0].find (":");
    if (pos != std::string::npos) bits [0].erase (0, pos + 1);
    std::string bookname = filter::strings::trim (bits [0]);
    int book = static_cast<int>(database::books::get_id_from_english (bookname));
    if (book) {
      if (filter::strings::is_numeric (bits [1])) {
        int chapter = filter::strings::convert_to_int (bits [1]);
        std::string data = bits [2];
        if (data.find ("data") != std::string::npos) {
          passage.m_book = book;
          passage.m_chapter = chapter;
        }
      }
    }
  }
  return passage;
}


// Reports information comparable to "git status".
// Repository: "repository".
// If $porcelain is given, it adds the --porcelain flag.
// All changed files will be returned.
std::vector <std::string> filter_git_status (std::string repository, bool porcelain)
{
  std::vector <std::string> paths;
  std::string output, error;
  std::vector <std::string> parameters = {"status"};
  if (porcelain) parameters.push_back("--porcelain");
  filter_shell_run (repository, "git", parameters, &output, &error);
  filter_git_check_error (error);
  paths = filter::strings::explode (output, '\n');
  return paths;
}


// Runs "git pull" and returns true if it ran fine.
// It puts the messages in container "messages".
bool filter_git_pull (std::string repository, std::vector <std::string> & messages)
{
  std::string out, err;
  int result = filter_shell_run (repository, "git", {"pull"}, &out, &err);
  out = filter::strings::trim (out);
  err = filter::strings::trim (err);
  messages = filter::strings::explode (out, '\n');
  std::vector <std::string> lines = filter::strings::explode (err, '\n');
  messages.insert (messages.end(), lines.begin(), lines.end());
  return (result == 0);
}


// Runs "git pull" and returns true if it ran fine.
// It puts the push messages in container "messages".
bool filter_git_push (std::string repository, std::vector <std::string> & messages, bool all)
{
  std::string out, err;
  std::vector <std::string> parameters = {"push"};
  if (all) parameters.push_back ("--all");
  int result = filter_shell_run (repository, "git", parameters, &out, &err);
  out = filter::strings::trim (out);
  err = filter::strings::trim (err);
  messages = filter::strings::explode (out, '\n');
  std::vector <std::string> lines = filter::strings::explode (err, '\n');
  messages.insert (messages.end(), lines.begin(), lines.end());
  return (result == 0);
}


// Resolves any conflicts in "repository".
// It fills "paths" with the paths to the files with the resolved merge conflicts.
// It fills "error" with any error that git generates.
// It returns true on success, that is, no errors occurred.
bool filter_git_resolve_conflicts (std::string repository, std::vector <std::string> & paths, std::string & error)
{
  int result = 0;
  paths.clear();

  // Get the unmerged paths.
  // Use the --porcelain parameter for a better API for scripting.
  std::vector <std::string> unmerged_paths;
  std::vector <std::string> lines = filter_git_status (repository, true);
  for (auto line : lines) {
    size_t pos = line.find ("UU ");
    if (pos != std::string::npos) {
      line.erase (0, 3);
      line = filter::strings::trim (line);
      unmerged_paths.push_back (line);
    }
  }

  // Deal with each unmerged path.
  for (auto & unmerged_path : unmerged_paths) {
    
    std::string common_ancestor;
    filter_shell_run (repository, "git", {"show", ":1:" + unmerged_path}, &common_ancestor, &error);

    std::string head_version;
    filter_shell_run (repository, "git", {"show", ":2:" + unmerged_path}, &head_version, &error);

    std::string merge_head_version;
    filter_shell_run (repository, "git", {"show", ":3:" + unmerged_path}, &merge_head_version, &error);

    std::string mergeBase (common_ancestor);
    std::string userData (head_version);
    std::string serverData (merge_head_version);
    
    std::vector <Merge_Conflict> conflicts;
    std::string mergedData = filter_merge_run (mergeBase, userData, serverData, true, conflicts);
    mergedData = filter::strings::trim (mergedData);
    filter_url_file_put_contents (filter_url_create_path ({repository, unmerged_path}), mergedData);
    
    paths.push_back (unmerged_path);
  }

  if (!unmerged_paths.empty ()) {
    std::vector <std::string> messages;
    std::string error2;
    filter_git_commit (repository, "", translate ("Bibledit fixed merge conflicts"), messages, error2);
  }
  
  // Done.
  return (result == 0);
}


// Configure the $repository: Make certain settings.
void filter_git_config (std::string repository)
{
  // At times there's a stale index.lock file that prevents any collaboration.
  // This is to be removed.
  std::string index_lock = filter_url_create_path ({repository, ".git", "index.lock"});
  if (file_or_dir_exists (index_lock)) {
    Database_Logs::log ("Cleaning out index lock " + index_lock);
    filter_url_unlink (index_lock);
  }

  // On some machines the mail name and address are not set properly; therefore these are set here.
  std::string user = database::config::general::get_site_mail_name ();
  if (user.empty ()) user = "Bibledit";
  filter_git_config_set_string (repository, "user.name", user);
  
  std::string mail = database::config::general::get_site_mail_address ();
  if (mail.empty ()) mail = "bibledit@bibledit.org";
  filter_git_config_set_string (repository, "user.email", mail);

  // Switch rename detection off.
  // This is for the consultation notes, since git has been seen to falsely "detect" renames.
  filter_git_config_set_int (repository, "diff.renamelimit", 0);
  filter_git_config_set_bool (repository, "diff.renames", false);

  // Current versions of git ask the user to set the default push method.
  filter_git_config_set_string (repository, "push.default", "matching");
  
  // Newer version of git do not automatically fast-forward, so set that here.
  filter_git_config_set_string (repository, "pull.ff", "yes");

}


// This checks $user, and optionally set it, to be sure it always returns a username.
std::string filter_git_user (std::string user)
{
  if (user.empty ()) {
    user = database::config::general::get_site_mail_name ();
  }
  if (user.empty ()) {
    user = "Bibledit Cloud";
  }
  return user;
}


// This takes the email address that belongs to $user,
// and optionally sets the email address to a valid value,
// and returns that email address.
std::string filter_git_email (std::string user)
{
  Database_Users database_users;
  std::string email = database_users.get_email (user);
  if (email.empty ()) {
    email = database::config::general::get_site_mail_address ();
  }
  if (email.empty ()) {
    email = "bibledit@bibledit.org";
  }
  return email;
}


#endif

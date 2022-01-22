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
string filter_git_directory (string object)
{
  return filter_url_create_root_path ({"git", object});
}


void filter_git_check_error (string data)
{
  vector <string> lines = filter_string_explode (data, '\n');
  for (auto & line : lines) Database_Logs::log (line);
}


// Runs the equivalent of "git init".
bool filter_git_init (string directory, bool bare)
{
  vector <string> parameters = {"init"};
  if (bare) parameters.push_back ("--bare");
  string output, error;
  int result = filter_shell_run (directory, "git", parameters, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  return (result == 0);
}


// Internal function that commits a user-generated change to the git repository.
void filter_git_commit_modification_to_git (string repository, string user, int book, int chapter,
                                            string & oldusfm, string & newusfm)
{
  string bookname = Database_Books::getEnglishFromId (book);
  string bookdir = filter_url_create_path ({repository, bookname});
  string chapterdir = filter_url_create_path ({bookdir, convert_to_string (chapter)});
  if (!file_or_dir_exists (chapterdir)) filter_url_mkdir (chapterdir);
  string datafile = filter_url_create_path ({chapterdir, "data"});
  string contents = filter_url_file_get_contents (datafile);
  if (contents != oldusfm) {
    filter_url_file_put_contents (datafile, oldusfm);
    string error;
    filter_git_add_remove_all (repository, error);
    vector <string> messages;
    filter_git_commit (repository, "", "System-generated to clearly display user modification in next commit", messages, error);
  }
  filter_url_file_put_contents (datafile, newusfm);
  string error;
  filter_git_add_remove_all (repository, error);
  vector <string> messages;
  filter_git_commit (repository, user, "User modification", messages, error);
}


// This filter stores the changes made by users on $bible in $repository.
// This puts commits in the repository, where the author is the user who made the changes.
// This information in the git repository can then be used for statistical or other purposes.
void filter_git_sync_modifications_to_git (string bible, string repository)
{
  // Go through all the users who saved data to this Bible.
  vector <string> users = Database_Git::get_users (bible);
  for (auto & user : users) {
    
    bool iteration_initialized = false;
    string overall_old_usfm, overall_new_usfm;
    int overall_book = 0, overall_chapter = 0;
    
    // Go through all the rowids for the user and the Bible.
    vector <int> rowids = Database_Git::get_rowids (user, bible);
    for (auto rowid : rowids) {

      string s;
      string oldusfm, newusfm;
      int book, chapter;
      Database_Git::get_chapter (rowid, s, s, book, chapter, oldusfm, newusfm);
      
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
      Database_Git::erase_rowid (rowid);

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
void filter_git_sync_bible_to_git (void * webserver_request, string bible, string repository)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  // First stage.
  // Read the chapters in the git repository,
  // and check if they occur in the database.
  // If a chapter is not in the database, remove it from the repository.
  vector <int> books = request->database_bibles()->getBooks (bible);
  vector <string> bookfiles = filter_url_scandir (repository);
  for (auto & bookname : bookfiles) {
    string path = filter_url_create_path ({repository, bookname});
    if (filter_url_is_dir (path)) {
      int book = Database_Books::getIdFromEnglish (bookname);
      if (book) {
        if (in_array (book, books)) {
          // Book exists in the database: Check the chapters.
          vector <int> chapters = request->database_bibles()->getChapters (bible, book);
          vector <string> chapterfiles = filter_url_scandir (filter_url_create_path ({repository, bookname}));
          for (auto & chaptername : chapterfiles) {
            string path = filter_url_create_path ({repository, bookname, chaptername});
            if (filter_url_is_dir (path)) {
              if (filter_string_is_numeric (chaptername)) {
                int chapter = convert_to_int (chaptername);
                string filename = filter_url_create_path ({repository, bookname, chaptername, "data"});
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
  books = request->database_bibles()->getBooks (bible);
  for (auto & book : books) {
    string bookname = Database_Books::getEnglishFromId (book);
    string bookdir = filter_url_create_path ({repository, bookname});
    if (!file_or_dir_exists (bookdir)) filter_url_mkdir (bookdir);
    vector <int> chapters = request->database_bibles()->getChapters (bible, book);
    for (auto & chapter : chapters) {
      string chapterdir = filter_url_create_path ({bookdir, convert_to_string (chapter)});
      if (!file_or_dir_exists (chapterdir)) filter_url_mkdir (chapterdir);
      string datafile = filter_url_create_path ({chapterdir, "data"});
      string contents = filter_url_file_get_contents (datafile);
      string usfm = request->database_bibles()->getChapter (bible, book, chapter);
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
void filter_git_sync_git_to_bible (void * webserver_request, string repository, string bible)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  // Stage one:
  // Read the chapters in the git repository,
  // and check that they occur in the database.
  // If any does not occur, add the chapter to the database.
  // This stage does not check the contents of the chapters.
  vector <string> bookfiles = filter_url_scandir (repository);
  for (auto & bookname : bookfiles) {
    string bookpath = filter_url_create_path ({repository, bookname});
    if (filter_url_is_dir (bookpath)) {
      int book = Database_Books::getIdFromEnglish (bookname);
      if (book) {
        // Check the chapters.
        vector <int> chapters = request->database_bibles()->getChapters (bible, book);
        vector <string> chapterfiles = filter_url_scandir (bookpath);
        for (auto & chapterfile : chapterfiles) {
          string chapterpath = filter_url_create_path ({bookpath, chapterfile});
          if (filter_url_is_dir (chapterpath)) {
            if (filter_string_is_numeric (chapterfile)) {
              int chapter = convert_to_int (chapterfile);
              string filename = filter_url_create_path ({chapterpath, "data"});
              if (file_or_dir_exists (filename)) {
                if (!in_array (chapter, chapters)) {
                  // Chapter does not exist in the database: Add it.
                  string usfm = filter_url_file_get_contents (filename);
                  bible_logic_store_chapter (bible, book, chapter, usfm);
                  // Log it.
                  string message = translate("A translator added chapter") + " " + bible + " " + bookname + " " + chapterfile;
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
  vector <int> books = request->database_bibles()->getBooks (bible);
  for (auto & book : books) {
    string bookname = Database_Books::getEnglishFromId (book);
    string bookdir = filter_url_create_path ({repository, bookname});
    if (file_or_dir_exists (bookdir)) {
      vector <int> chapters = request->database_bibles()->getChapters (bible, book);
      for (auto & chapter : chapters) {
        string chapterdir = filter_url_create_path ({bookdir, convert_to_string (chapter)});
        if (file_or_dir_exists (chapterdir)) {
          string datafile = filter_url_create_path ({chapterdir, "data"});
          string contents = filter_url_file_get_contents (datafile);
          string usfm = request->database_bibles()->getChapter (bible, book, chapter);
          if (contents != usfm) {
            bible_logic_store_chapter (bible, book, chapter, contents);
            Database_Logs::log (translate("A translator updated chapter") + " " + bible + " " + bookname + " " + convert_to_string (chapter));
            rss_logic_schedule_update ("collaborator", bible, book, chapter, usfm, contents);
          }
        } else {
          bible_logic_delete_chapter (bible, book, chapter);
          Database_Logs::log (translate("A translator deleted chapter") + " " + bible + " " + bookname + " " + convert_to_string (chapter));
        }
      }
    } else {
      bible_logic_delete_book (bible, book);
      Database_Logs::log (translate("A translator deleted book") + " " + bible + " " + bookname);
    }
  }
}


string filter_git_disabled ()
{
  return "Git has been disabled on iOS and Android, and can be enabled on Linux, Windows and macOS";
}


// This filter takes one chapter of the Bible data as it is stored in the $git folder,
// and puts this information into Bibledit's database.
// The $git is a git repository, and may contain other data as well.
void filter_git_sync_git_chapter_to_bible (string repository, string bible, int book, int chapter)
{
  // Filename for the chapter.
  string bookname = Database_Books::getEnglishFromId (book);
  string filename = filter_url_create_path ({repository, bookname, convert_to_string (chapter), "data"});
  
  if (file_or_dir_exists (filename)) {
    
    // Store chapter in database and log it.
    Database_Bibles database_bibles;
    string existing_usfm = database_bibles.getChapter (bible, book, chapter);
    string usfm = filter_url_file_get_contents (filename);
    bible_logic_log_change (bible, book, chapter, usfm, "collaborator", "Chapter updated from git repository", false);
    bible_logic_store_chapter (bible, book, chapter, usfm);
    rss_logic_schedule_update ("collaborator", bible, book, chapter, existing_usfm, usfm);
    
  } else {
    
    // Delete chapter from database.
    bible_logic_delete_chapter (bible, book, chapter);
    Database_Logs::log (translate("A collaborator deleted chapter") + " " + bible + " " + bookname + " " + convert_to_string (chapter));
    
  }
}


// Returns true if the git repository at "url" is online.
bool filter_git_remote_read (string url, string & error)
{
  string output;
  int result = filter_shell_run ("", "git", {"ls-remote", url}, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  return (result == 0);
}


bool filter_git_remote_clone (string url, string path, [[maybe_unused]] int jobid, string & error)
{
  // Clear a possible existing git repository directory.
  filter_url_rmdir (path);

  string output;
  int result = filter_shell_run ("", "git", {"clone", url, path}, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  error.clear ();
  return (result == 0);
}


bool filter_git_add_remove_all (string repository, string & error)
{
  string output;
  int result = filter_shell_run (repository, "git", {"add", "--all", "."}, &output, &error);
  filter_git_check_error (output);
  filter_git_check_error (error);
  return (result == 0);
}


// This function runs "git commit" through the shell.
bool filter_git_commit (string repository, string user, string message,
                        vector <string> & messages, string & error)
{
  user = filter_git_user (user);
  string email = filter_git_email (user);
  stringstream author;
  author << "--author=" << quoted(user + " <" + email + ">");
  string out, err;
  int result = filter_shell_run (repository, "git",
                                {"commit",
                                 author.str(),
                                 "-a",
                                 "-m",
                                 message
                                }, &out, &err);
  out = filter_string_trim (out);
  err = filter_string_trim (err);
  error = err;
  filter_git_check_error (error);
  messages = filter_string_explode (out, '\n');
  vector <string> lines = filter_string_explode (err, '\n');
  messages.insert (messages.end(), lines.begin(), lines.end());
  
  // In case of Your branch is up-to-date with 'origin/master'. nothing to commit, working directory clean,
  // git returns exit code 256. Yet this is not an error.
  if (out.find ("nothing to commit") != string::npos) result = 0;
  
  return (result == 0);
}


void filter_git_config_set_bool (string repository, string name, bool value)
{
  string svalue = value ? "true" : "false";
  filter_git_config_set_string (repository, name, svalue);
}


void filter_git_config_set_int (string repository, string name, int value)
{
  string svalue = convert_to_string (value);
  filter_git_config_set_string (repository, name, svalue);
}


void filter_git_config_set_string (string repository, string name, string value)
{
  string output, error;
  filter_shell_run (repository, "git", {"config", name, value}, &output, &error);
}


// This filter takes a $line of the output of the git pull command.
// It tries to interpret it to find a passage that would have been updated.
// If a valid book and chapter are found, it returns them.
Passage filter_git_get_passage (string line)
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
  vector <string> bits = filter_string_explode (line, '/');
  if (bits.size () == 3) {
    size_t pos = bits [0].find (":");
    if (pos != string::npos) bits [0].erase (0, pos + 1);
    string bookname = filter_string_trim (bits [0]);
    int book = Database_Books::getIdFromEnglish (bookname);
    if (book) {
      if (filter_string_is_numeric (bits [1])) {
        int chapter = convert_to_int (bits [1]);
        string data = bits [2];
        if (data.find ("data") != string::npos) {
          passage.book = book;
          passage.chapter = chapter;
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
vector <string> filter_git_status (string repository, bool porcelain)
{
  vector <string> paths;
  string output, error;
  vector <string> parameters = {"status"};
  if (porcelain) parameters.push_back("--porcelain");
  filter_shell_run (repository, "git", parameters, &output, &error);
  filter_git_check_error (error);
  paths = filter_string_explode (output, '\n');
  return paths;
}


// Runs "git pull" and returns true if it ran fine.
// It puts the messages in container "messages".
bool filter_git_pull (string repository, vector <string> & messages)
{
  string out, err;
  int result = filter_shell_run (repository, "git", {"pull"}, &out, &err);
  out = filter_string_trim (out);
  err = filter_string_trim (err);
  messages = filter_string_explode (out, '\n');
  vector <string> lines = filter_string_explode (err, '\n');
  messages.insert (messages.end(), lines.begin(), lines.end());
  return (result == 0);
}


// Runs "git pull" and returns true if it ran fine.
// It puts the push messages in container "messages".
bool filter_git_push (string repository, vector <string> & messages, bool all)
{
  string out, err;
  vector <string> parameters = {"push"};
  if (all) parameters.push_back ("--all");
  int result = filter_shell_run (repository, "git", parameters, &out, &err);
  out = filter_string_trim (out);
  err = filter_string_trim (err);
  messages = filter_string_explode (out, '\n');
  vector <string> lines = filter_string_explode (err, '\n');
  messages.insert (messages.end(), lines.begin(), lines.end());
  return (result == 0);
}


// Resolves any conflicts in "repository".
// It fills "paths" with the paths to the files with the resolved merge conflicts.
// It fills "error" with any error that git generates.
// It returns true on success, that is, no errors occurred.
bool filter_git_resolve_conflicts (string repository, vector <string> & paths, string & error)
{
  int result = 0;
  paths.clear();

  // Get the unmerged paths.
  // Use the --porcelain parameter for a better API for scripting.
  vector <string> unmerged_paths;
  vector <string> lines = filter_git_status (repository, true);
  for (auto line : lines) {
    size_t pos = line.find ("UU ");
    if (pos != string::npos) {
      line.erase (0, 3);
      line = filter_string_trim (line);
      unmerged_paths.push_back (line);
    }
  }

  // Deal with each unmerged path.
  for (auto & unmerged_path : unmerged_paths) {
    
    string common_ancestor;
    filter_shell_run (repository, "git", {"show", ":1:" + unmerged_path}, &common_ancestor, &error);

    string head_version;
    filter_shell_run (repository, "git", {"show", ":2:" + unmerged_path}, &head_version, &error);

    string merge_head_version;
    filter_shell_run (repository, "git", {"show", ":3:" + unmerged_path}, &merge_head_version, &error);

    string mergeBase (common_ancestor);
    string userData (head_version);
    string serverData (merge_head_version);
    
    vector <Merge_Conflict> conflicts;
    string mergedData = filter_merge_run (mergeBase, userData, serverData, true, conflicts);
    mergedData = filter_string_trim (mergedData);
    filter_url_file_put_contents (filter_url_create_path ({repository, unmerged_path}), mergedData);
    
    paths.push_back (unmerged_path);
  }

  if (!unmerged_paths.empty ()) {
    vector <string> messages;
    string error;
    filter_git_commit (repository, "", translate ("Bibledit fixed merge conflicts"), messages, error);
  }
  
  // Done.
  return (result == 0);
}


// Configure the $repository: Make certain settings.
void filter_git_config (string repository)
{
  // At times there's a stale index.lock file that prevents any collaboration.
  // This is to be removed.
  string index_lock = filter_url_create_path ({repository, ".git", "index.lock"});
  if (file_or_dir_exists (index_lock)) {
    Database_Logs::log ("Cleaning out index lock " + index_lock);
    filter_url_unlink (index_lock);
  }

  // On some machines the mail name and address are not set properly; therefore these are set here.
  string user = Database_Config_General::getSiteMailName ();
  if (user.empty ()) user = "Bibledit";
  filter_git_config_set_string (repository, "user.name", user);
  
  string mail = Database_Config_General::getSiteMailAddress ();
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
string filter_git_user (string user)
{
  if (user.empty ()) {
    user = Database_Config_General::getSiteMailName ();
  }
  if (user.empty ()) {
    user = "Bibledit Cloud";
  }
  return user;
}


// This takes the email address that belongs to $user,
// and optionally sets the email address to a valid value,
// and returns that email address.
string filter_git_email (string user)
{
  Database_Users database_users;
  string email = database_users.get_email (user);
  if (email.empty ()) {
    email = Database_Config_General::getSiteMailAddress ();
  }
  if (email.empty ()) {
    email = "bibledit@bibledit.org";
  }
  return email;
}


#endif

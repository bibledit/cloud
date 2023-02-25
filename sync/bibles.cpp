/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <sync/bibles.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/merge.h>
#include <tasks/logic.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/logs.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/mail.h>
#include <database/modifications.h>
#include <database/git.h>
#include <client/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <sync/logic.h>
#include <checksum/logic.h>
#include <access/bible.h>
#include <bb/logic.h>
#include <rss/logic.h>
#include <sendreceive/logic.h>
using namespace std;


string sync_bibles_url ()
{
  return "sync/bibles";
}


string sync_bibles_receive_chapter (Webserver_Request * request, string & bible, int book, int chapter)
{
  // Convert the tags to plus signs, which the client had converted to tags,
  // for safekeeping the + signs during transit.
  string oldusfm = filter_url_tag_to_plus (request->post ["o"]);
  string newusfm = filter_url_tag_to_plus (request->post ["n"]);
  string checksum = request->post ["s"];

  
  string username = request->session_logic ()->currentUser ();
  string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  
  
  // Check whether the user has write-access to the Bible book.
  if (!access_bible::book_write (request, username, bible, book)) {
    string message = "User " + username + " does not have write access to Bible " + bible;
    Database_Logs::log (message, Filter_Roles::manager ());
    // The Cloud will email the user with details about the issue.
    bible_logic::client_no_write_access_mail (bible, book, chapter, username, oldusfm, newusfm);
    // The Cloud returns the checksum so the client things the chapter was send off correcly,
    // and will not re-schedule this as a failure.
    return checksum;
  }
  
  
  // Check checksum.
  if (checksum != checksum_logic::get (oldusfm + newusfm)) {
    string message = "The received data is corrupted";
    Database_Logs::log (message, Filter_Roles::manager ());
    return message;
  }
  
  
  string serverusfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  
  // There have been cases like this:
  // The Bibledit client sends USFM with double spaces in them to the Cloud.
  // Then the Cloud would add one one more space.
  // If this chapter was edited and sent multiple times,
  // the result would be that each time extra spaces kept being added.
  // Solution / fix to this:
  // Remove all the double spaces from the USFM to be saved.
  // (Keep the reference USFM as it is: Goal: Detect the differences.
  newusfm = filter_string_collapse_whitespace (newusfm);
  

  // Gather data for recording the changes made by the user, for the change notifications.
  int old_id = request->database_bibles()->getChapterId (bible, book, chapter);
  string old_text = serverusfm;
  string new_text = newusfm;
  
  
  // Server logs the change from the client.
  bible_logic::log_change (bible, book, chapter, newusfm, username, translate ("Received from client"), false);

  
  if (serverusfm == "") {
    // If the chapter on the server is still empty, then just store the client's version on the server, and that's it.
    bible_logic::store_chapter (bible, book, chapter, newusfm);
  } else if (newusfm != serverusfm) {
    // Do a merge in case the client sends USFM that differs from what's on the server.
    vector <Merge_Conflict> conflicts;
    string mergedusfm = filter_merge_run (oldusfm, newusfm, serverusfm, true, conflicts);
    // Update the server with the new chapter data.
    bible_logic::store_chapter (bible, book, chapter, mergedusfm);
    // Check on the merge.
    filter_merge_add_book_chapter (conflicts, book, chapter);
    bible_logic::client_receive_merge_mail (bible, book, chapter, username, oldusfm, newusfm, mergedusfm);
    // Log the merge in the journal, for possible trouble shooting.
    bible_logic::log_merge (username, bible, book, chapter, oldusfm, newusfm, serverusfm, mergedusfm);
  }
  

  // If text was saved, record it as a change entered by the user.
  int new_id = request->database_bibles()->getChapterId (bible, book, chapter);
  if (new_id != old_id) {
    Database_Modifications database_modifications;
    database_modifications.recordUserSave (username, bible, book, chapter, old_id, old_text, new_id, new_text);
#ifdef HAVE_CLOUD
    if (sendreceive_git_repository_linked (bible)) {
      Database_Git::store_chapter (username, bible, book, chapter, old_text, new_text);
    }
    rss_logic_schedule_update (username, bible, book, chapter, old_text, new_text);
#endif
  }


  // Done: Return the checksum to client.
  // Returning the checksum serves as a confirmation to the client that the Cloud properly received the chapter.
  // When the client works through a sub-standard network, this checksumming is getting very important.
  // Back in the time that the Cloud didn't return a checksum, there were cases of data loss on clients,
  // due to a poor network connection.
  return checksum;
}


string sync_bibles (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Sync_Logic sync_logic = Sync_Logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    request->response_code = 426;
    return "";
  }
  
  // Check on the credentials.
  if (!sync_logic.credentials_okay ()) return "";
  
  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();
  
  // Get the relevant parameters the client may have POSTed to us, the server.
  int action = convert_to_int (request->post ["a"]);
  string bible = request->post ["b"];
  int book = convert_to_int (request->post ["bk"]);
  int chapter = convert_to_int (request->post ["c"]);
  
  switch (action) {
    case Sync_Logic::bibles_get_total_checksum:
    {
      // The server reads the credentials from the client's user,
      // checks which Bibles this user has access to,
      // calculate the checksum of all chapters in those Bibles,
      // and returns this checksum to the client.
      string username = request->session_logic ()->currentUser ();
      vector <string> bibles = access_bible::bibles (request, username);
      string server_checksum = checksum_logic::get_bibles (request, bibles);
      return server_checksum;
    }
    case Sync_Logic::bibles_get_bibles:
    {
      // The server reads the credentials from the client's user,
      // and responds with a list of Bibles this user has access to.
      string username = request->session_logic ()->currentUser ();
      vector <string> bibles = access_bible::bibles (request, username);
      string checksum = checksum_logic::get (bibles);
      string s_bibles = filter_string_implode (bibles, "\n");
      return checksum + "\n" + s_bibles;
    }
    case Sync_Logic::bibles_get_bible_checksum:
    {
      // The server responds with the checksum for the whole Bible.
      return checksum_logic::get_bible (request, bible);
    }
    case Sync_Logic::bibles_get_books:
    {
      // The server responds with a checksum and then the list of books in the Bible.
      vector <int> server_books = request->database_bibles()->getBooks (bible);
      vector <string> v_server_books;
      for (auto server_book : server_books) v_server_books.push_back (convert_to_string (server_book));
      string s_server_books = filter_string_implode (v_server_books, "\n");
      string checksum = checksum_logic::get (v_server_books);
      return checksum + "\n" + s_server_books;
    }
    case Sync_Logic::bibles_get_book_checksum:
    {
      // The server responds with the checksum of the whole book.
      return checksum_logic::get_book (request, bible, book);
    }
    case Sync_Logic::bibles_get_chapters:
    {
      // The server responds with the list of books in the Bible book.
      vector <int> server_chapters = request->database_bibles()->getChapters (bible, book);
      vector <string> v_server_chapters;
      for (auto & server_chapter : server_chapters) v_server_chapters.push_back (convert_to_string (server_chapter));
      string s_server_chapters = filter_string_implode (v_server_chapters, "\n");
      string checksum = checksum_logic::get (v_server_chapters);
      return checksum + "\n" + s_server_chapters;
    }
    case Sync_Logic::bibles_get_chapter_checksum:
    {
      // The server responds with the checksum of the whole chapter.
      return checksum_logic::get_chapter (request, bible, book, chapter);
    }
    case Sync_Logic::bibles_send_chapter:
    {
      return sync_bibles_receive_chapter (request, bible, book, chapter);
    }
    case Sync_Logic::bibles_get_chapter:
    {
      // The server responds with the USFM of the chapter, prefixed by a checksum.
      string usfm = filter_string_trim (request->database_bibles()->getChapter (bible, book, chapter));
      string checksum = checksum_logic::get (usfm);
      return checksum + "\n" + usfm;
    }
    default: {};
  }
  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  request->response_code = 400;
  return "";
}

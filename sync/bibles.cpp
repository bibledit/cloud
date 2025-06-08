/*
 Copyright (©) 2003-2025 Teus Benschop.
 
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


std::string sync_bibles_url ()
{
  return "sync/bibles";
}


std::string sync_bibles_receive_chapter (Webserver_Request& webserver_request, std::string & bible, int book, int chapter)
{
  // Convert the tags to plus signs, which the client had converted to tags,
  // for safekeeping the + signs during transit.
  std::string oldusfm = filter_url_tag_to_plus (webserver_request.post ["o"]);
  std::string newusfm = filter_url_tag_to_plus (webserver_request.post ["n"]);
  std::string checksum = webserver_request.post ["s"];

  
  const std::string& username = webserver_request.session_logic ()->get_username ();
  std::string bookname = database::books::get_english_from_id (static_cast<book_id>(book));
  
  
  // Check whether the user has write-access to the Bible book.
  if (!access_bible::book_write (webserver_request, username, bible, book)) {
    std::string message = "User " + username + " does not have write access to Bible " + bible;
    Database_Logs::log (message, roles::manager);
    // The Cloud will email the user with details about the issue.
    bible_logic::client_no_write_access_mail (bible, book, chapter, username, oldusfm, newusfm);
    // The Cloud returns the checksum so the client things the chapter was send off correcly,
    // and will not re-schedule this as a failure.
    return checksum;
  }
  
  
  // Check checksum.
  if (checksum != checksum_logic::get (oldusfm + newusfm)) {
    std::string message = "The received data is corrupted";
    Database_Logs::log (message, roles::manager);
    return message;
  }
  
  
  std::string serverusfm = database::bibles::get_chapter (bible, book, chapter);
  
  
  // There have been cases like this:
  // The Bibledit client sends USFM with double spaces in them to the Cloud.
  // Then the Cloud would add one one more space.
  // If this chapter was edited and sent multiple times,
  // the result would be that each time extra spaces kept being added.
  // Solution / fix to this:
  // Remove all the double spaces from the USFM to be saved.
  // (Keep the reference USFM as it is: Goal: Detect the differences.
  newusfm = filter::strings::collapse_whitespace (newusfm);
  

  // Gather data for recording the changes made by the user, for the change notifications.
  int old_id = database::bibles::get_chapter_id (bible, book, chapter);
  std::string old_text = serverusfm;
  std::string new_text = newusfm;
  
  
  // Server logs the change from the client.
  bible_logic::log_change (bible, book, chapter, newusfm, username, translate ("Received from client"), false);

  
  if (serverusfm == "") {
    // If the chapter on the server is still empty, then just store the client's version on the server, and that's it.
    bible_logic::store_chapter (bible, book, chapter, newusfm);
  } else if (newusfm != serverusfm) {
    // Do a merge in case the client sends USFM that differs from what's on the server.
    std::vector <Merge_Conflict> conflicts;
    std::string mergedusfm = filter_merge_run (oldusfm, newusfm, serverusfm, true, conflicts);
    // Update the server with the new chapter data.
    bible_logic::store_chapter (bible, book, chapter, mergedusfm);
    // Check on the merge.
    filter_merge_add_book_chapter (conflicts, book, chapter);
    bible_logic::client_receive_merge_mail (bible, book, chapter, username, oldusfm, newusfm, mergedusfm);
    // Log the merge in the journal, for possible trouble shooting.
    bible_logic::log_merge (username, bible, book, chapter, oldusfm, newusfm, serverusfm, mergedusfm);
  }
  

  // If text was saved, record it as a change entered by the user.
  int new_id = database::bibles::get_chapter_id (bible, book, chapter);
  if (new_id != old_id) {
    database::modifications::recordUserSave (username, bible, book, chapter, old_id, old_text, new_id, new_text);
#ifdef HAVE_CLOUD
    if (sendreceive_git_repository_linked (bible)) {
      database::git::store_chapter (username, bible, book, chapter, old_text, new_text);
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


std::string sync_bibles (Webserver_Request& webserver_request)
{
  Sync_Logic sync_logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    webserver_request.response_code = 426;
    return std::string();
  }
  
  // Check on the credentials.
  if (!sync_logic.credentials_okay ()) return std::string();
  
  // Client makes a prioritized server call: Record the client's IP address.
  sync_logic.prioritized_ip_address_record ();
  
  // Get the relevant parameters the client may have POSTed to us, the server.
  int action = filter::strings::convert_to_int (webserver_request.post ["a"]);
  std::string bible = webserver_request.post ["b"];
  int book = filter::strings::convert_to_int (webserver_request.post ["bk"]);
  int chapter = filter::strings::convert_to_int (webserver_request.post ["c"]);
  
  switch (action) {
    case Sync_Logic::bibles_get_total_checksum:
    {
      // The server reads the credentials from the client's user,
      // checks which Bibles this user has access to,
      // calculate the checksum of all chapters in those Bibles,
      // and returns this checksum to the client.
      const std::string& username = webserver_request.session_logic ()->get_username ();
      std::vector <std::string> bibles = access_bible::bibles (webserver_request, username);
      std::string server_checksum = checksum_logic::get_bibles (bibles);
      return server_checksum;
    }
    case Sync_Logic::bibles_get_bibles:
    {
      // The server reads the credentials from the client's user,
      // and responds with a list of Bibles this user has access to.
      const std::string& username = webserver_request.session_logic ()->get_username ();
      std::vector <std::string> bibles = access_bible::bibles (webserver_request, username);
      std::string checksum = checksum_logic::get (bibles);
      std::string s_bibles = filter::strings::implode (bibles, "\n");
      return checksum + "\n" + s_bibles;
    }
    case Sync_Logic::bibles_get_bible_checksum:
    {
      // The server responds with the checksum for the whole Bible.
      return checksum_logic::get_bible (bible);
    }
    case Sync_Logic::bibles_get_books:
    {
      // The server responds with a checksum and then the list of books in the Bible.
      std::vector <int> server_books = database::bibles::get_books (bible);
      std::vector <std::string> v_server_books;
      for (auto server_book : server_books) v_server_books.push_back (std::to_string (server_book));
      std::string s_server_books = filter::strings::implode (v_server_books, "\n");
      std::string checksum = checksum_logic::get (v_server_books);
      return checksum + "\n" + s_server_books;
    }
    case Sync_Logic::bibles_get_book_checksum:
    {
      // The server responds with the checksum of the whole book.
      return checksum_logic::get_book (bible, book);
    }
    case Sync_Logic::bibles_get_chapters:
    {
      // The server responds with the list of books in the Bible book.
      std::vector <int> server_chapters = database::bibles::get_chapters (bible, book);
      std::vector <std::string> v_server_chapters;
      for (auto & server_chapter : server_chapters) v_server_chapters.push_back (std::to_string (server_chapter));
      std::string s_server_chapters = filter::strings::implode (v_server_chapters, "\n");
      std::string checksum = checksum_logic::get (v_server_chapters);
      return checksum + "\n" + s_server_chapters;
    }
    case Sync_Logic::bibles_get_chapter_checksum:
    {
      // The server responds with the checksum of the whole chapter.
      return checksum_logic::get_chapter (bible, book, chapter);
    }
    case Sync_Logic::bibles_send_chapter:
    {
      return sync_bibles_receive_chapter (webserver_request, bible, book, chapter);
    }
    case Sync_Logic::bibles_get_chapter:
    {
      // The server responds with the USFM of the chapter, prefixed by a checksum.
      std::string usfm = filter::strings::trim (database::bibles::get_chapter (bible, book, chapter));
      std::string checksum = checksum_logic::get (usfm);
      return checksum + "\n" + usfm;
    }
    default: {};
  }
  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  std::this_thread::sleep_for (std::chrono::seconds (1));
  webserver_request.response_code = 400;
  return std::string();
}

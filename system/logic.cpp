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


#include <system/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/usfm.h>
#include <filter/indonesian.h>
#include <filter/roles.h>
#include <filter/date.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <database/jobs.h>
#include <database/cache.h>
#include <database/bibles.h>
#include <database/books.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/privileges.h>
#include <database/confirm.h>
#include <html/text.h>
#include <styles/logic.h>
#include <tasks/logic.h>
#include <database/logic.h>
#include <email/send.h>
#include <search/logic.h>
#include <user/logic.h>


string system_logic_bibles_file_name ()
{
  return filter_url_create_path ({filter_url_temp_dir (), "bibles.tar"});
}


// This produes a tarball with all installed Bibles.
void system_logic_produce_bibles_file (int jobid)
{
  Database_Jobs database_jobs;
  Database_Bibles database_bibles;

  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.new_paragraph ();
    html_text.add_text (translate ("Generating a file with the Bibles."));
    html_text.new_paragraph ();
    html_text.add_text (translate ("In progress..."));
    database_jobs.set_start (jobid, html_text.get_inner_html ());
  }

  
  // The location of the tarball to generate.
  string tarball = filter_url_create_root_path ({system_logic_bibles_file_name ()});
  
  
  // The directory where the exported Bibles will be put.
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
      string path = filter_url_create_path ({directory, file});
      filter_url_file_put_contents (path, book_usfm);
      files.push_back (file);
    }
  }
  
  
  // Pack the contents of all the Bibles into one tarball.
  string error = filter_archive_microtar_pack (tarball, directory, files);
  
  
  // Ready, provide info about how to download the file, or about the error.
  {
    Html_Text html_text ("");
    html_text.new_paragraph ();
    if (error.empty ()) {
      html_text.add_text (translate ("The file with Bibles is ready."));
      html_text.new_paragraph ();
      html_text.add_link (html_text.current_p_node, "/" + system_logic_bibles_file_name (), "", "", "", translate ("Get it."));
      html_text.new_paragraph ();
      html_text.add_text (translate ("The file can be imported in another Bibledit client."));
      html_text.add_text (" ");
      html_text.add_text (translate ("This will create the same Bibles in that other client."));
    } else {
      html_text.add_text (translate ("It failed to create the file with Bibles."));
      html_text.new_paragraph ();
      html_text.add_text (error);
    }
    database_jobs.set_result (jobid, html_text.get_inner_html ());
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
    string path = filter_url_create_path ({directory, file});
    string data = filter_url_file_get_contents (path);
    
    // The name of the Bible this file is to be imported into.
    string bible (file);
    size_t pos = bible.find_last_of ("_");
    if (pos != string::npos) bible.erase (pos);
    
    // Get details about the USFM to import.
    string stylesheet = styles_logic_standard_sheet ();
    vector <filter::usfm::BookChapterData> book_chapter_text = filter::usfm::usfm_import (data, stylesheet);
    for (auto & book_chapter_data : book_chapter_text) {
      if (book_chapter_data.m_book > 0) {
        // Store the data and log it.
        // This does not trigger the client to send it to the Cloud.
        // Reason is that the Cloud is authoritative,
        // so importing outdated Bibles would not affect the authoritative copy in the Cloud.
        database_bibles.storeChapter (bible, book_chapter_data.m_book, book_chapter_data.m_chapter, book_chapter_data.m_data);
        string bookname = Database_Books::getEnglishFromId (book_chapter_data.m_book);
        Database_Logs::log ("Imported " + bible + " " + bookname + " " + convert_to_string (book_chapter_data.m_chapter));
      } else {
        // Import error.
        Database_Logs::log ("Could not import this file: " + file);
      }
    }
  }
  
  // Clean up.
  filter_url_rmdir (directory);
  filter_url_unlink (tarball);

  // Since new Bibles may have been imported, index them all.
  Database_Config_General::setIndexBibles (true);
  tasks_logic_queue (REINDEXBIBLES, {"1"});

  // Ready, hallelujah!
  Database_Logs::log ("Importing Bibles ready");
}


string system_logic_notes_file_name ()
{
  return filter_url_create_path ({filter_url_temp_dir (), "notes.tar"});
}


// This produes a tarball with all local Consultation Notes.
void system_logic_produce_notes_file (int jobid)
{
  Database_Jobs database_jobs;
 
  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.new_paragraph ();
    html_text.add_text (translate ("Generating a file with the Consultation Notes."));
    html_text.new_paragraph ();
    html_text.add_text (translate ("In progress..."));
    database_jobs.set_start (jobid, html_text.get_inner_html ());
  }
  
  
  // The location of the tarball to generate.
  string tarball = filter_url_create_root_path ({system_logic_notes_file_name ()});
  
  
  // The database directory where the consultation notes reside.
  string directory = filter_url_create_root_path ({"consultations"});

  
  // The files to include in the tarball.
  vector <string> files;
  filter_url_recursive_scandir (directory, files);
  for (string & file : files) {
    file.erase (0, directory.length () + 1);
  }
  

  // Pack the contents of all the Bibles into one tarball.
  string error = filter_archive_microtar_pack (tarball, directory, files);

  
  // Ready, provide info about how to download the file, or about the error.
  {
    Html_Text html_text ("");
    html_text.new_paragraph ();
    if (error.empty ()) {
      html_text.add_text (translate ("The file with Consultation Notes is ready."));
      html_text.new_paragraph ();
      html_text.add_link (html_text.current_p_node, "/" + system_logic_notes_file_name (), "", "", "", translate ("Get it."));
      html_text.new_paragraph ();
      html_text.add_text (translate ("The file can be imported in another Bibledit client."));
      html_text.add_text (" ");
      html_text.add_text (translate ("This will create the same Consultation Notes in that other client."));
    } else {
      html_text.add_text (translate ("It failed to create the file with Consultation Notes."));
      html_text.new_paragraph ();
      html_text.add_text (error);
    }
    database_jobs.set_result (jobid, html_text.get_inner_html ());
  }
}


void system_logic_import_notes_file (string tarball)
{
  Database_Logs::log ("Importing Consultation Notes from " + tarball);
  
  // The database directory where the consultation notes reside.
  string directory = filter_url_create_root_path ({"consultations"});
  
  // Unpack the tarball into the directory.
  string error= filter_archive_microtar_unpack (tarball, directory);
  if (!error.empty ()) {
    Database_Logs::log ("Importing Consultation Notes failure: " + error);
    return;
  }
  
  // Clean up.
  filter_url_unlink (tarball);

  // Since notes may have been imported or updated, index them all.
  Database_Config_General::setIndexNotes (true);
  tasks_logic_queue (REINDEXNOTES);

  // Ready, hallelujah!
  Database_Logs::log ("Importing Consultation Notes ready");
}


string system_logic_resources_file_name (string resourcename)
{
  if (!resourcename.empty ()) resourcename.append ("_");
  return filter_url_create_path ({filter_url_temp_dir (), resourcename + "resources.tar"});
}


// This produces a tarball with all installed resources.
void system_logic_produce_resources_file (int jobid)
{
  Database_Jobs database_jobs;
  
  
  // Generate the initial page.
  {
    Html_Text html_text ("");
    html_text.new_paragraph ();
    html_text.add_text (translate ("Generating a file with the resources."));
    html_text.new_paragraph ();
    html_text.add_text (translate ("In progress..."));
    database_jobs.set_start (jobid, html_text.get_inner_html ());
  }
  
  // The location of the single tarball to generate.
  string tarball = filter_url_create_root_path ({system_logic_resources_file_name ()});
  
  
  // The database directory where the cached resources reside.
  string directory = filter_url_create_root_path ({database_logic_databases ()});
  
  
  // The filenames of the cached resources.
  vector <string> resources;
  vector <string> rawfiles = filter_url_scandir (directory);
  for (auto filename : rawfiles) {
    if (filename.find (Database_Cache::fragment()) != string::npos) {
      resources.push_back (filename);
    }
  }
  
  
  // Get the filenames to pack the resources, one resource per tarball.
  // This is to generate smaller tarballs which can be handled on devices with limited internal memory.
  // Such devices fail to have sufficient memory to handle one tarball with logs and logs of resources.
  // It fails to allocate enough memory on such devices.
  // So that's the reason for doing one resource per tarball.
  map <string, vector <string> > single_resources;
  for (auto filename : rawfiles) {
    // Sample filename: cache_resource_[CrossWire][LXX]_62.sqlite
    // Look for the last underscore.
    // This indicates which resource it is, by leaving the book number out.
    if (filename.find (Database_Cache::fragment()) != string::npos) {
      size_t pos = filename.find_last_of ("_");
      if (pos != string::npos) {
        string resource = filename.substr (0, pos);
        single_resources[resource].push_back (filename);
      }
    }
  }

  
  // Progress bar data: How many tarballs to create.
  int tarball_count = static_cast<int>(1 + single_resources.size ());
  int tarball_counter = 0;

  
  // Pack the resources into one tarball.
  // It does not compress the files (as could be done).
  // It just puts them in a tarball.
  // Compression is not needed because the file is transferred locally,
  // so the size of the file is not very important.
  // Not compressing speeds things up a great lot.
  tarball_counter++;
  database_jobs.set_percentage (jobid, 100 * tarball_counter / tarball_count);
  database_jobs.set_progress (jobid, translate ("All"));
  string error = filter_archive_microtar_pack (tarball, directory, resources);

  
  // Create one tarball per resource.
  for (auto element : single_resources) {
    tarball_counter++;
    string resource_name = element.first;
    vector <string> v_resources = element.second;
    database_jobs.set_percentage (jobid, 100 * tarball_counter / tarball_count);
    database_jobs.set_progress (jobid, resource_name);
    string resource_tarball = filter_url_create_root_path ({system_logic_resources_file_name (resource_name)});
    error += filter_archive_microtar_pack (resource_tarball, directory, v_resources);
  }
  
  
  // Ready, provide info about how to download the file or about the error.
  {
    Html_Text html_text ("");
    html_text.new_paragraph ();
    if (!resources.empty ()) {
      if (error.empty ()) {
        html_text.add_text (translate ("The file with all of the installed resources is ready."));
        html_text.add_text (" ");
        html_text.add_text (translate ("Amount of resources:"));
        html_text.add_text (" ");
        html_text.add_text (convert_to_string (single_resources.size()));
        html_text.add_text (".");
        html_text.new_paragraph ();
        html_text.add_link (html_text.current_p_node, "/" + system_logic_resources_file_name (), "", "", "", translate ("Download the archive with all installed resources."));
        html_text.new_paragraph ();
        html_text.add_text (translate ("The file can be imported in another Bibledit client."));
        html_text.add_text (" ");
        html_text.add_text (translate ("This will create the same resources in that other client."));
        html_text.new_paragraph ();
        html_text.add_text (translate ("The above file may be huge in case there is lots of installed resources."));
        html_text.add_text (" ");
        html_text.add_text (translate ("For that reason there are alternative files with individual resources below."));
        html_text.add_text (" ");
        html_text.add_text (translate ("These are smaller in size."));
        for (auto element : single_resources) {
          string resource_name = element.first;
          html_text.new_paragraph ();
          html_text.add_link (html_text.current_p_node, "/" + system_logic_resources_file_name (resource_name), "", "", "", translate ("Download") + " " + resource_name);
        }
      } else {
        html_text.add_text (translate ("It failed to create the file with resources."));
        html_text.new_paragraph ();
        html_text.add_text (error);
      }
    } else {
      html_text.add_text (translate ("There were no installed resources to make an archive from."));
      html_text.add_text (" ");
      html_text.add_text (translate ("Install some resources on the device, and try again."));
    }
    database_jobs.set_result (jobid, html_text.get_inner_html ());
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
    string path = filter_url_create_path ({directory, file});
    string data = filter_url_file_get_contents (path);

    // Store the resource into place.
    path = filter_url_create_root_path ({database_logic_databases (), file});
    filter_url_file_put_contents (path, data);
  }

  // Clean up.
  filter_url_rmdir (directory);
  filter_url_unlink (tarball);

  // Ready, hallelujah!
  Database_Logs::log ("Importing Resources ready");
}


void system_logic_indonesian_free_deletion ([[maybe_unused]] string username,
                                            [[maybe_unused]] string email)
{
#ifdef HAVE_CLOUD
  Database_Logs::log ("Starting to inform and delete user " + username + " and associated Bible");

  {
    // Create the body of the email.
    xml_document document;
    xml_node node;
    
    node = document.append_child ("p");
    node.text ().set ("Shalom Bapak/Ibu Pengguna Bibledit,");
    
    node = document.append_child ("p");
    node.text ().set ("Kami berharap Saudara sempat menggunakan Bibledit Tamu selama sebulan ini. Kami mengundang Saudara untuk mendaftar kembali sekarang, atau di saat di mana Saudara punya keperluan meneliti ayat Alkitab. Kami juga mengundang Saudara mengunjungi situs http://alkitabkita.info untuk segala informasi dari bahan penelitian Alkitab yang akan ditambahkan.");

    node = document.append_child ("p");
    node.text ().set ("Apabila Saudara sudah memasukkan ayat-ayat dalam bagian Terjemahanku pasal-pasal itu akan dikirim lewat email kepada Saudara.");

    node = document.append_child ("p");
    node.text ().set ("Klik link ini untuk membaca tentang kelebihan tingkat Bibledit Anggota https://sites.google.com/view/alkitabkita/menjadi-anggota-bibledit.");

    node = document.append_child ("p");
    node.text ().set ("Tuhan memberkati Saudara,");

    node = document.append_child ("p");
    node.text ().set ("Balazi Gulo");

    node = document.append_child ("p");
    node.text ().set ("Pengurus Albata");

    node = document.append_child ("p");
    node.text ().set ("(albata.info)");

    // Convert the document to a string.
    stringstream output;
    document.print (output, "", format_raw);
    string html = output.str ();
    
    // Schedule the mail for sending to the user.
    email_schedule (email, "Bibledit", html);
  }
  
  Database_Bibles database_bibles;
  string bible = filter::indonesian::mytranslation (username);
  vector <int> books = database_bibles.getBooks (bible);
  for (auto book : books) {
    vector <int> chapters = database_bibles.getChapters (bible, book);
    for (auto chapter : chapters) {

      // If the chapter identifier is equal to the initial ID,
      // it means the chapter was not changed by anyone.
      int chapter_id = database_bibles.getChapterId (bible, book, chapter);
      if (chapter_id == 100000001) continue;
      // If the chapter was changed, email the contents of that chapter to the user.

      // Create the body of the email.
      xml_document document;
      xml_node node;
      
      string heading = filter_passage_display (book, chapter, {});
      node = document.append_child ("h3");
      node.text ().set (heading.c_str());
      
      string explanation = translate ("Inilah hasil terjemahan Saudara:");
      node = document.append_child ("p");
      node.text ().set (explanation.c_str ());

      document.append_child ("br");
      node = document.append_child ("pre");
      string usfm = database_bibles.getChapter(bible, book, chapter);
      node.text ().set (usfm.c_str ());
      
      // Convert the document to a string.
      stringstream output;
      document.print (output, "", format_raw);
      string html = output.str ();
      
      // Schedule the mail for sending to the user.
      email_schedule (email, translate (bible + ": " + heading), html);
    }
  }
  
  // Delete the user's Bible and associated data.
  database_bibles.deleteBible(bible);
  search_logic_delete_bible (bible);
  Database_Privileges::removeBible (bible);
  Database_Config_Bible::remove (bible);
  
  Database_Logs::log ("Ready handling user and associated data");
#endif
}


void system_logic_indonesian_free_expiration ()
{
#ifdef HAVE_CLOUD
  // The first sequence is to removed expired accounts and data that belongs to it.
  Database_Logs::log ("Expiring free Indonesian Cloud accounts and associated data");
  Database_Users database_users;
  vector<string> users = database_users.get_users();
  for (auto user : users) {
    // In the free Indonesian Cloud, the relevant level is that of Consultant.
    int level = database_users.get_level(user);
    if (level != Filter_Roles::consultant()) continue;
    // Expire this account after 30 days.
    int account_creation_time = filter::date::seconds_since_epoch();
    {
      vector <string> lines = Database_Config_General::getAccountCreationTimes ();
      for (auto line : lines) {
        vector <string> bits = filter_string_explode(line, '|');
        if (bits.size() != 2) continue;
        int seconds = convert_to_int(bits[0]);
        if (user == bits[1]) account_creation_time = seconds;
      }
    }
    int seconds = filter::date::seconds_since_epoch() - account_creation_time;
    int days = seconds / (3600 * 24);
    if (days <= 30) continue;
    // Get details of this user.
    string email = database_users.get_email(user);
    // Delete the user account.
    Database_Logs::log("Deleting free user " + user + " with email " + email);
    string message;
    user_logic_delete_account (user, "free", email, message);
    system_logic_indonesian_free_deletion (user, email);
  }
  
  // The next sequence is to delete a Bible that was created for a user,
  // but the user is no longer there, or did not confirm the account.
  
  // Create a container with Indonesian Bibles that are associated to an existing or pending user.
  vector<string> valid_indonesian_bibles;
  users = database_users.get_users();
  for (auto user : users) {
    string bible = filter::indonesian::mytranslation (user);
    valid_indonesian_bibles.push_back(bible);
  }
  Database_Confirm database_confirm;
  vector <int> ids = database_confirm.get_ids();
  for (auto id : ids) {
    string username = database_confirm.get_username(static_cast<unsigned int>(id));
    string bible = filter::indonesian::mytranslation (username);
    valid_indonesian_bibles.push_back(bible);
  }
  // Get all available Bibles at the system.
  Database_Bibles database_bibles;
  vector<string> bibles = database_bibles.getBibles();
  // Get array of Bibles to delete.
  vector <string> bibles_to_delete = filter_string_array_diff(bibles, valid_indonesian_bibles);
  for (auto bible : bibles_to_delete) {
    // Skip the shared Indonesian Bible.
    if (bible == filter::indonesian::ourtranslation()) continue;
    // Delete this unassociated Bible.
    Database_Logs::log("Deleting Bible not associated with any account: " + bible);
    database_bibles.deleteBible(bible);
    search_logic_delete_bible (bible);
    Database_Privileges::removeBible (bible);
    Database_Config_Bible::remove (bible);
  }
#endif
}

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


#include <unittests/url.h>
#include <unittests/utilities.h>
#include <filter/url.h>


void test_url ()
{
  trace_unit_tests (__func__);
  
  // Test writing to and reading from files, and whether a file exists.
  {
    string filename = "/tmp/בוקר טוב";
    string contents = "בוקר טוב בוקר טוב";
    evaluate (__LINE__, __func__, false, file_or_dir_exists (filename));
    evaluate (__LINE__, __func__, false, file_or_dir_exists (filename));
    filter_url_file_put_contents (filename, contents);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (filename));
    evaluate (__LINE__, __func__, true, file_or_dir_exists (filename));
    evaluate (__LINE__, __func__, contents, filter_url_file_get_contents (filename));
    filter_url_unlink (filename);
    evaluate (__LINE__, __func__, false, file_or_dir_exists (filename));
    evaluate (__LINE__, __func__, false, file_or_dir_exists (filename));
  }

  // Test function to check existence of directory.
  {
    string folder = "/tmp/בוקר טוב";
    evaluate (__LINE__, __func__, false, file_or_dir_exists (folder));
    filter_url_mkdir (folder);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (folder));
    filter_url_rmdir (folder);
    evaluate (__LINE__, __func__, false, file_or_dir_exists (folder));
  }
  
  // Test unique filename.
  {
    string filename = "/tmp/unique";
    filter_url_file_put_contents (filename, "");
    string filename1 = filter_url_unique_path (filename);
    filter_url_file_put_contents (filename1, "");
    evaluate (__LINE__, __func__, "/tmp/unique.1", filename1);
    string filename2 = filter_url_unique_path (filename);
    filter_url_file_put_contents (filename2, "");
    evaluate (__LINE__, __func__, "/tmp/unique.2", filename2);
    filter_url_unlink (filename);
    filter_url_unlink (filename1);
    filter_url_unlink (filename2);
  }
  
  // Html export filenames.
  {
    evaluate (__LINE__, __func__, "index.html", filter_url_html_file_name_bible ());
    evaluate (__LINE__, __func__, "path/index.html", filter_url_html_file_name_bible ("path"));
    evaluate (__LINE__, __func__, "path/01-Genesis.html", filter_url_html_file_name_bible ("path", 1));
    evaluate (__LINE__, __func__, "01-Genesis.html", filter_url_html_file_name_bible ("", 1));
    evaluate (__LINE__, __func__, "path/11-1Kings.html", filter_url_html_file_name_bible ("path", 11));
    evaluate (__LINE__, __func__, "path/22-SongofSolomon-000.html", filter_url_html_file_name_bible ("path", 22, 0));
    evaluate (__LINE__, __func__, "path/33-Micah-333.html", filter_url_html_file_name_bible ("path", 33, 333));
    evaluate (__LINE__, __func__, "33-Micah-333.html", filter_url_html_file_name_bible ("", 33, 333));
  }
  
  // Testing mkdir and rmdir including parents.
  {
    // Do test a folder name that starts with a dot.
    string directory = filter_url_create_path ({testing_directory, "a", ".git"});
    filter_url_mkdir (directory);
    string path = filter_url_create_path ({directory, "c"});
    string contents = "unittest";
    filter_url_file_put_contents (path, contents);
    evaluate (__LINE__, __func__, contents, filter_url_file_get_contents (path));
    
    path = filter_url_create_path ({testing_directory, "a"});
    evaluate (__LINE__, __func__, true, file_or_dir_exists (path));
    evaluate (__LINE__, __func__, true, filter_url_is_dir (path));
    
    filter_url_rmdir (path);
    evaluate (__LINE__, __func__, false, file_or_dir_exists (path));
    evaluate (__LINE__, __func__, false, filter_url_is_dir (path));
  }
  
  // Test filter_url_escape_shell_argument.
  {
    evaluate (__LINE__, __func__, "'argument'", filter_url_escape_shell_argument ("argument"));
    evaluate (__LINE__, __func__, "'arg\\'ument'", filter_url_escape_shell_argument ("arg'ument"));
  }

  // Test URL decoder.
  {
    evaluate (__LINE__, __func__, "Store settings", filter_url_urldecode ("Store+settings"));
    evaluate (__LINE__, __func__, "test@mail", filter_url_urldecode ("test%40mail"));
    evaluate (__LINE__, __func__, "ᨀab\\d@a", filter_url_urldecode ("%E1%A8%80ab%5Cd%40a"));
    evaluate (__LINE__, __func__, "\xFF", filter_url_urldecode ("%FF"));
    evaluate (__LINE__, __func__, "\xFF", filter_url_urldecode ("%ff"));
  }
  
  // Test URL encoder.
  {
    evaluate (__LINE__, __func__, "Store%20settings", filter_url_urlencode ("Store settings"));
    evaluate (__LINE__, __func__, "test%40mail", filter_url_urlencode ("test@mail"));
    evaluate (__LINE__, __func__, "%E1%A8%80ab%5Cd%40a", filter_url_urlencode ("ᨀab\\d@a"));
    evaluate (__LINE__, __func__, "foo%3Dbar%26baz%3D", filter_url_urlencode ("foo=bar&baz="));
    evaluate (__LINE__, __func__, "%D7%91%D6%BC%D6%B0%D7%A8%D6%B5%D7%90%D7%A9%D7%81%D6%B4%D6%96%D7%99%D7%AA", filter_url_urlencode ("בְּרֵאשִׁ֖ית"));
    evaluate (__LINE__, __func__, "ABC", filter_url_urlencode ("ABC"));
    evaluate (__LINE__, __func__, "%FF", filter_url_urlencode ("\xFF"));
  }

  // Test encode and decode round trip.
  {
    string original ("\0\1\2", 3);
    string encoded ("%00%01%02");
    evaluate (__LINE__, __func__, encoded, filter_url_urlencode (original));
    evaluate (__LINE__, __func__, original, filter_url_urldecode (encoded));
  }

  // Test encode and decode unsafe chars, RFC1738.
  {
    string unsafe (" <>#{}|\\^~[]`");
    string unsafe_encoded = filter_url_urlencode (unsafe);
    evaluate (__LINE__, __func__, true, unsafe_encoded.find_first_of (unsafe) == string::npos);
    evaluate (__LINE__, __func__, unsafe, filter_url_urldecode (unsafe_encoded));
  }
  
  // Test char values used in encoding and decoding.
  {
    char one = -1;
    char two = (char)255;
    evaluate (__LINE__, __func__, one, two);
  }
    
  // Test dirname and basename functions.
  {
    evaluate (__LINE__, __func__, ".", filter_url_dirname (string()));
    evaluate (__LINE__, __func__, ".", filter_url_dirname ("/"));
    evaluate (__LINE__, __func__, ".", filter_url_dirname ("dir/"));
    evaluate (__LINE__, __func__, "/", filter_url_dirname ("/dir"));
    evaluate (__LINE__, __func__, "foo", filter_url_dirname ("foo/bar"));
    evaluate (__LINE__, __func__, "/foo", filter_url_dirname ("/foo/bar"));
    evaluate (__LINE__, __func__, "/foo", filter_url_dirname ("/foo/bar/"));
    evaluate (__LINE__, __func__, "a.txt", filter_url_basename ("/a.txt"));
    evaluate (__LINE__, __func__, "txt", filter_url_basename ("/txt/"));
    evaluate (__LINE__, __func__, "foo.bar", filter_url_basename ("/path/to/foo.bar"));
    evaluate (__LINE__, __func__, "foo.bar", filter_url_basename ("foo.bar"));
  }
  
  // Test http GET and POST
  {
    string result, error;
    result = filter_url_http_get ("http://localhost/none", error, false);
#ifndef HAVE_CLIENT
    evaluate (__LINE__, __func__, "Couldn't connect to server", error);
#endif
    evaluate (__LINE__, __func__, "", result);
    map <string, string> values = {pair ("a", "value1"), pair ("b", "value2")};
    result = filter_url_http_post ("http://localhost/none", values, error, false, false);
#ifndef HAVE_CLIENT
    evaluate (__LINE__, __func__, "Couldn't connect to server", error);
#endif
    evaluate (__LINE__, __func__, "", result);
  }
  
  // Test low-level http(s) client error for unknown host.
  {
    string result, error;
    result = filter_url_http_request_mbed ("http://unknownhost", error, {}, "", false);
    evaluate (__LINE__, __func__, "", result);
    evaluate (__LINE__, __func__, "Internet connection failure: unknownhost: nodename nor servname provided, or not known", error);
  }
  
  // Test low-level http(s) client error for closed port.
  {
    string result, error;
    result = filter_url_http_request_mbed ("http://bibledit.org:8084/non-existing", error, {}, "", false);
    evaluate (__LINE__, __func__, "", result);
    evaluate (__LINE__, __func__, "bibledit.org:8084: Connection refused | bibledit.org:8084: Connection refused", error);
  }
  
  // Test low-level http(s) client result.
  {
    string result, error;
    result = filter_url_http_request_mbed ("http://185.87.186.229", error, {}, "", false);
    evaluate (__LINE__, __func__, true, result.find ("Home") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("Ndebele Bible") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("Shona Bible") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("Downloads") != string::npos);
    evaluate (__LINE__, __func__, "", error);
  }
  {
    string result, error;
    result = filter_url_http_request_mbed ("https://bibledit.org", error, {}, "", false);
    evaluate (__LINE__, __func__, true, result.find ("Bibledit") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("Linux") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("Cloud") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("Android") != string::npos);
    evaluate (__LINE__, __func__, "", error);
  }

  // Test removing credentials from a URL.
  {
    string url = "https://username:password@github.com/username/repository.git";
    url = filter_url_remove_username_password (url);
    evaluate (__LINE__, __func__, "https://github.com/username/repository.git", url);
  }
  
  // Test recursively copying a directory.
  {
    string input = filter_url_create_root_path ({"unittests"});
    string output = "/tmp/test_copy_directory";
    filter_url_rmdir (output);
    filter_url_dir_cp (input, output);
    string path = filter_url_create_path ({output, "tests", "basic.css"});
    evaluate (__LINE__, __func__, true, file_or_dir_exists (path));
  }
  
  // Secure communications.
  {
    filter_url_ssl_tls_initialize ();
    
    string url;
    string error;
    string result;
    
    url = filter_url_set_scheme (" localhost ", false);
    evaluate (__LINE__, __func__, "http://localhost", url);
    url = filter_url_set_scheme ("httpx://localhost", false);
    evaluate (__LINE__, __func__, "http://localhost", url);
    url = filter_url_set_scheme ("http://localhost", true);
    evaluate (__LINE__, __func__, "https://localhost", url);
    
    result = filter_url_http_request_mbed ("http://www.google.nl", error, {}, "", false);
    evaluate (__LINE__, __func__, true, result.find ("google") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("search") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("background") != string::npos);
    evaluate (__LINE__, __func__, "", error);
    
    result = filter_url_http_request_mbed ("https://www.google.nl", error, {}, "", false);
    evaluate (__LINE__, __func__, true, result.find ("google") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("search") != string::npos);
    evaluate (__LINE__, __func__, true, result.find ("background") != string::npos);
    evaluate (__LINE__, __func__, "", error);
    
    result = filter_url_http_request_mbed ("https://bibledit.org:8091", error, {}, "", false);
    evaluate (__LINE__, __func__, "", result);
    evaluate (__LINE__, __func__, "Response code: 302 Found", error);
    
    filter_url_ssl_tls_finalize ();
  }
  
  // Testing is_dir.
  {
    string path = filter_url_create_root_path ({"git"});
    evaluate (__LINE__, __func__, true, filter_url_is_dir (path));
    path = filter_url_create_root_path ({"setup", "index.html"});
    evaluate (__LINE__, __func__, false, filter_url_is_dir (path));
  }
  
  // Testing checking for and setting write permissions.
  {
    string directory = filter_url_create_root_path ({filter_url_temp_dir ()});
    string file1 = filter_url_create_path ({directory, "1"});
    string file2 = filter_url_create_path ({directory, "2"});
    filter_url_file_put_contents (file1, "x");
    filter_url_file_put_contents (file2, "x");
    
    evaluate (__LINE__, __func__, true, filter_url_get_write_permission (directory));
    evaluate (__LINE__, __func__, true, filter_url_get_write_permission (file1));
    evaluate (__LINE__, __func__, true, filter_url_get_write_permission (file2));

    chmod (directory.c_str(), S_IRUSR);
    chmod (file1.c_str(), S_IRUSR);
    chmod (file2.c_str(), S_IRUSR);
    
    evaluate (__LINE__, __func__, false, filter_url_get_write_permission (directory));
    evaluate (__LINE__, __func__, false, filter_url_get_write_permission (file1));
    evaluate (__LINE__, __func__, false, filter_url_get_write_permission (file2));
    
    filter_url_set_write_permission (directory);
    filter_url_set_write_permission (file1);
    filter_url_set_write_permission (file2);
    
    evaluate (__LINE__, __func__, true, filter_url_get_write_permission (directory));
    evaluate (__LINE__, __func__, true, filter_url_get_write_permission (file1));
    evaluate (__LINE__, __func__, true, filter_url_get_write_permission (file2));
  }

  // Email address validity.
  {
    evaluate (__LINE__, __func__, true, filter_url_email_is_valid ("user@web.site"));
    evaluate (__LINE__, __func__, false, filter_url_email_is_valid ("user@website"));
    evaluate (__LINE__, __func__, false, filter_url_email_is_valid (" user@web.site"));
    evaluate (__LINE__, __func__, false, filter_url_email_is_valid ("user @ web.site"));
  }
  
  // Getting the file extension.
  {
    evaluate (__LINE__, __func__, "txt", filter_url_get_extension ("foo/bar.txt"));
    evaluate (__LINE__, __func__, "", filter_url_get_extension (".hidden"));
    evaluate (__LINE__, __func__, "", filter_url_get_extension (""));
  }
  
  refresh_sandbox (true);
}

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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "gtest/gtest.h"
#pragma GCC diagnostic pop
#include <unittests/utilities.h>
#include <filter/url.h>
#include <filter/date.h>
#include <chrono>
#include <filesystem>


class filter_url : public testing::Test {
protected:
  static void SetUpTestSuite() {
    refresh_sandbox (false);
  }
  static void TearDownTestSuite() {
    refresh_sandbox (true);
  }
  void SetUp() override { }
  void TearDown() override { }
};


TEST_F (filter_url, files_exist)
{
  // Test writing to and reading from files, and whether a file exists.
  const std::string filename = "/tmp/בוקר טוב";
  const std::string contents = "בוקר טוב בוקר טוב";
  EXPECT_EQ (false, file_or_dir_exists (filename));
  EXPECT_EQ (false, file_or_dir_exists (filename));
  filter_url_file_put_contents (filename, contents);
  EXPECT_EQ (true, file_or_dir_exists (filename));
  EXPECT_EQ (true, file_or_dir_exists (filename));
  EXPECT_EQ (contents, filter_url_file_get_contents (filename));
  filter_url_unlink (filename);
  EXPECT_EQ (false, file_or_dir_exists (filename));
  EXPECT_EQ (false, file_or_dir_exists (filename));
}


TEST_F (filter_url, dir_exists)
{
  // Test function to check existence of directory.
  const std::string folder = "/tmp/בוקר טוב";
  EXPECT_EQ (false, file_or_dir_exists (folder));
  filter_url_mkdir (folder);
  EXPECT_EQ (true, file_or_dir_exists (folder));
  filter_url_rmdir (folder);
  EXPECT_EQ (false, file_or_dir_exists (folder));
}


TEST_F (filter_url, unique)
{
  // Test unique filename.
  std::string filename = "/tmp/unique";
  filter_url_file_put_contents (filename, "");
  std::string filename1 = filter_url_unique_path (filename);
  filter_url_file_put_contents (filename1, "");
  EXPECT_EQ ("/tmp/unique.1", filename1);
  std::string filename2 = filter_url_unique_path (filename);
  filter_url_file_put_contents (filename2, "");
  EXPECT_EQ ("/tmp/unique.2", filename2);
  filter_url_unlink (filename);
  filter_url_unlink (filename1);
  filter_url_unlink (filename2);
}


TEST_F (filter_url, html_export_filenames)
{
  // Html export filenames.
  EXPECT_EQ ("index.html", filter_url_html_file_name_bible ());
  EXPECT_EQ ("path/index.html", filter_url_html_file_name_bible ("path"));
  EXPECT_EQ ("path/01-Genesis.html", filter_url_html_file_name_bible ("path", 1));
  EXPECT_EQ ("01-Genesis.html", filter_url_html_file_name_bible ("", 1));
  EXPECT_EQ ("path/11-1Kings.html", filter_url_html_file_name_bible ("path", 11));
  EXPECT_EQ ("path/22-SongofSolomon-000.html", filter_url_html_file_name_bible ("path", 22, 0));
  EXPECT_EQ ("path/33-Micah-333.html", filter_url_html_file_name_bible ("path", 33, 333));
  EXPECT_EQ ("33-Micah-333.html", filter_url_html_file_name_bible ("", 33, 333));
}


TEST_F (filter_url, mk_rm_dir)
{
  // Testing mkdir and rmdir including parents.

  // Do test a folder name that starts with a dot.
    std::string directory = filter_url_create_path ({testing_directory, "a", ".git"});
    filter_url_mkdir (directory);
    std::string path = filter_url_create_path ({directory, "c"});
    std::string contents = "unittest";
    filter_url_file_put_contents (path, contents);
    EXPECT_EQ (contents, filter_url_file_get_contents (path));
    
    path = filter_url_create_path ({testing_directory, "a"});
    EXPECT_EQ (true, file_or_dir_exists (path));
    EXPECT_EQ (true, filter_url_is_dir (path));
    
    filter_url_rmdir (path);
    EXPECT_EQ (false, file_or_dir_exists (path));
    EXPECT_EQ (false, filter_url_is_dir (path));
}


TEST_F (filter_url, escape_shell_argument)
{
  // Test filter_url_escape_shell_argument.
  EXPECT_EQ ("'argument'", filter_url_escape_shell_argument ("argument"));
  EXPECT_EQ ("'arg\\'ument'", filter_url_escape_shell_argument ("arg'ument"));
}


TEST_F (filter_url, decode)
{
  // Test URL decoder.
  EXPECT_EQ ("Store settings", filter_url_urldecode ("Store+settings"));
  EXPECT_EQ ("test@mail", filter_url_urldecode ("test%40mail"));
  EXPECT_EQ ("ᨀab\\d@a", filter_url_urldecode ("%E1%A8%80ab%5Cd%40a"));
  EXPECT_EQ ("\xFF", filter_url_urldecode ("%FF"));
  EXPECT_EQ ("\xFF", filter_url_urldecode ("%ff"));
}


TEST_F (filter_url, encode)
{
  // Test URL encoder.
  EXPECT_EQ ("Store%20settings", filter_url_urlencode ("Store settings"));
  EXPECT_EQ ("test%40mail", filter_url_urlencode ("test@mail"));
  EXPECT_EQ ("%E1%A8%80ab%5Cd%40a", filter_url_urlencode ("ᨀab\\d@a"));
  EXPECT_EQ ("foo%3Dbar%26baz%3D", filter_url_urlencode ("foo=bar&baz="));
  EXPECT_EQ ("%D7%91%D6%BC%D6%B0%D7%A8%D6%B5%D7%90%D7%A9%D7%81%D6%B4%D6%96%D7%99%D7%AA", filter_url_urlencode ("בְּרֵאשִׁ֖ית"));
  EXPECT_EQ ("ABC", filter_url_urlencode ("ABC"));
  EXPECT_EQ ("%FF", filter_url_urlencode ("\xFF"));
}


TEST_F (filter_url, encode_decode_roundtrip)
{
  // Test encode and decode round trip.
  std::string original ("\0\1\2", 3);
  std::string encoded ("%00%01%02");
  EXPECT_EQ (encoded, filter_url_urlencode (original));
  EXPECT_EQ (original, filter_url_urldecode (encoded));
}


TEST_F (filter_url, encode_decode_unsafe_chars)
{
  // Test encode and decode unsafe chars, RFC1738.
  std::string unsafe (" <>#{}|\\^~[]`");
  std::string unsafe_encoded = filter_url_urlencode (unsafe);
  EXPECT_EQ (true, unsafe_encoded.find_first_of (unsafe) == std::string::npos);
  EXPECT_EQ (unsafe, filter_url_urldecode (unsafe_encoded));
}


TEST_F (filter_url, dirname_basename)
{
  // Test dirname and basename functions.
  EXPECT_EQ (".", filter_url_dirname (std::string()));
  EXPECT_EQ (".", filter_url_dirname ("/"));
  EXPECT_EQ (".", filter_url_dirname ("dir/"));
  EXPECT_EQ (".", filter_url_dirname ("/dir"));
  EXPECT_EQ ("foo", filter_url_dirname ("foo/bar"));
  EXPECT_EQ ("/foo", filter_url_dirname ("/foo/bar"));
  EXPECT_EQ ("/foo", filter_url_dirname ("/foo/bar/"));
  EXPECT_EQ ("a.txt", filter_url_basename ("/a.txt"));
  EXPECT_EQ ("txt", filter_url_basename ("/txt/"));
  EXPECT_EQ ("foo.bar", filter_url_basename ("/path/to/foo.bar"));
  EXPECT_EQ ("foo.bar", filter_url_basename ("foo.bar"));
}


TEST_F (filter_url, get_post)
{
  // Test http GET and POST
  std::string result, error;
  result = filter_url_http_get ("http://localhost/none", error, false);
#ifdef HAVE_CLOUD
  EXPECT_EQ ("Could not connect to server", error);
#endif
  EXPECT_EQ (std::string(), result);
  const std::map <std::string, std::string> values = {std::pair ("a", "value1"), std::pair ("b", "value2")};
  result = filter_url_http_post ("http://localhost/none", std::string(), values, error, false, false, {});
#ifdef HAVE_CLOUD
  EXPECT_EQ ("Could not connect to server", error);
#endif
  EXPECT_EQ (std::string(), result);
}


TEST_F (filter_url, error_unknown_host)
{
  // Test low-level http(s) client error for unknown host.
  std::string result, error;
  result = filter_url_http_request_mbed ("http://unknownhost", error, {}, "", false);
  EXPECT_EQ ("", result);
  EXPECT_EQ ("Internet connection failure: unknownhost: nodename nor servname provided, or not known", error);
}


TEST_F (filter_url, error_closed_port)
{
  // Test low-level http(s) client error for closed port.
  std::string result, error;
  result = filter_url_http_request_mbed ("http://bibledit.org:8086/non-existing", error, {}, std::string(), false);
  EXPECT_EQ (std::string(), result);
  EXPECT_EQ ("bibledit.org:8086: Connection refused", error);
}


TEST_F (filter_url, client_result_fixed_ip)
{
  // Test low-level http(s) client result.
  std::string error;
  const std::string result = filter_url_http_request_mbed ("http://212.132.105.87", error, {}, "", false);
  EXPECT_EQ (true, result.find ("God") != std::string::npos);
  EXPECT_EQ (true, result.find ("bless") != std::string::npos);
  EXPECT_EQ (true, result.find ("you") != std::string::npos);
  EXPECT_EQ (std::string(), error);
}


TEST_F (filter_url, client_result_bibledit_org)
{
  // Test low-level http(s) client result.
  std::string error;
  const std::string result = filter_url_http_request_mbed ("https://bibledit.org", error, {}, "", false);
  EXPECT_EQ (true, result.find ("Bibledit") != std::string::npos);
  EXPECT_EQ (true, result.find ("Linux") != std::string::npos);
  EXPECT_EQ (true, result.find ("Cloud") != std::string::npos);
  EXPECT_EQ (true, result.find ("Android") != std::string::npos);
  EXPECT_EQ (std::string(), error);
}


TEST_F (filter_url, remove_credentials)
{
  // Test removing credentials from a URL.
  std::string url = "https://username:password@github.com/username/repository.git";
  url = filter_url_remove_username_password (url);
  EXPECT_EQ ("https://github.com/username/repository.git", url);
}


TEST_F (filter_url, copy_file)
{
  // Test copying an existing file.
  {
    constexpr auto output {"/tmp/test_copy_file"};
    filter_url_rmdir (output);
    filter_url_unlink (output);
    bool success = filter_url_file_cp (__FILE__, output);
    EXPECT_TRUE (file_or_dir_exists (output));
    EXPECT_TRUE (success);
    EXPECT_NE (0, filter_url_filesize (__FILE__));
    EXPECT_EQ (filter_url_filesize (__FILE__), filter_url_filesize (output));
  }
  // Test copying a non-existing file. To run for std::filesystem only.
  {
//    constexpr auto output {"/tmp/test_copy_file"};
//    filter_url_unlink (output);
//    bool success = filter_url_file_cp ("non_existing_file", output);
//    EXPECT_FALSE (file_or_dir_exists (output));
//    EXPECT_FALSE (success);
//    EXPECT_NE (0, filter_url_filesize (__FILE__));
//    EXPECT_NE (filter_url_filesize (__FILE__), filter_url_filesize (output));
//    refresh_sandbox (false);
  }
}


TEST_F (filter_url, copy_recursively)
{
  // Test recursively copying a directory.
  std::string input = filter_url_create_root_path ({"unittests"});
  std::string output = "/tmp/test_copy_directory";
  filter_url_rmdir (output);
  filter_url_dir_cp (input, output);
  std::string path = filter_url_create_path ({output, "tests", "basic.css"});
  EXPECT_EQ (true, file_or_dir_exists (path));
}


TEST_F (filter_url, https)
{
  // Secure communications.
  filter_url_ssl_tls_initialize ();
  
  std::string url;
  std::string error;
  std::string result;
  
  url = filter_url_set_scheme (" localhost ", false);
  EXPECT_EQ ("http://localhost", url);
  url = filter_url_set_scheme ("httpx://localhost", false);
  EXPECT_EQ ("http://localhost", url);
  url = filter_url_set_scheme ("http://localhost", true);
  EXPECT_EQ ("https://localhost", url);
  
  result = filter_url_http_request_mbed ("http://httpforever.com/", error, {}, "", false);
  EXPECT_TRUE (result.find ("insecure") != std::string::npos);
  EXPECT_TRUE (result.find ("WiFi") != std::string::npos);
  EXPECT_EQ (std::string(), error);

  result = filter_url_http_request_mbed ("https://www.google.com", error, {}, "", false);
  EXPECT_TRUE (result.find ("google") != std::string::npos);
  EXPECT_TRUE (result.find ("search") != std::string::npos);
  EXPECT_TRUE (result.find ("background") != std::string::npos);
  EXPECT_EQ (std::string(), error);

  result = filter_url_http_request_mbed ("https://bibledit.org:8091/index/index", error, {}, "", false);
  EXPECT_EQ (std::string(), error);
  EXPECT_TRUE (result.find ("Enable JavaScript to work with Bibledit") != std::string::npos);
  
  filter_url_ssl_tls_finalize ();
}


TEST_F (filter_url, is_dir)
{
  // Testing is_dir.
  std::string path = filter_url_create_root_path ({"git"});
  EXPECT_EQ (true, filter_url_is_dir (path));
  path = filter_url_create_root_path ({"setup", "index.html"});
  EXPECT_EQ (false, filter_url_is_dir (path));
}


TEST_F (filter_url, write_permissions)
{
  // Testing checking for and setting write permissions.
  std::string directory = filter_url_create_root_path ({filter_url_temp_dir ()});
  std::string file1 = filter_url_create_path ({directory, "1"});
  std::string file2 = filter_url_create_path ({directory, "2"});
  filter_url_file_put_contents (file1, "x");
  filter_url_file_put_contents (file2, "x");
  
  EXPECT_EQ (true, filter_url_get_write_permission (directory));
  EXPECT_EQ (true, filter_url_get_write_permission (file1));
  EXPECT_EQ (true, filter_url_get_write_permission (file2));
  
  chmod (directory.c_str(), S_IRUSR);
  chmod (file1.c_str(), S_IRUSR);
  chmod (file2.c_str(), S_IRUSR);
  
  EXPECT_EQ (false, filter_url_get_write_permission (directory));
  EXPECT_EQ (false, filter_url_get_write_permission (file1));
  EXPECT_EQ (false, filter_url_get_write_permission (file2));
  
  filter_url_set_write_permission (directory);
  filter_url_set_write_permission (file1);
  filter_url_set_write_permission (file2);
  
  EXPECT_EQ (true, filter_url_get_write_permission (directory));
  EXPECT_EQ (true, filter_url_get_write_permission (file1));
  EXPECT_EQ (true, filter_url_get_write_permission (file2));
}


TEST_F (filter_url, email_validity)
{
  // Email address validity.
  EXPECT_EQ (true, filter_url_email_is_valid ("user@web.site"));
  EXPECT_EQ (false, filter_url_email_is_valid ("user@website"));
  EXPECT_EQ (false, filter_url_email_is_valid (" user@web.site"));
  EXPECT_EQ (false, filter_url_email_is_valid ("user @ web.site"));
}


TEST_F (filter_url, file_extension)
{
  // Getting the file extension.
  EXPECT_EQ ("txt", filter_url_get_extension ("foo/bar.txt"));
  // For std::filesystem only EXPECT_EQ (std::string(), filter_url_get_extension (".hidden"));
  EXPECT_EQ (std::string(), filter_url_get_extension (""));
}


TEST_F (filter_url, scandir)
{
  // Reading the directory content.
  std::string directory = filter_url_create_root_path ({filter_url_temp_dir (), "dirtest"});
  filter_url_mkdir(directory);
  std::string file1 = filter_url_create_path ({directory, "1"});
  std::string file2 = filter_url_create_path ({directory, "2"});
  filter_url_file_put_contents (file1, "1");
  filter_url_file_put_contents (file2, "2");
  std::vector <std::string> files = filter_url_scandir (directory);
  EXPECT_EQ ((std::vector <std::string>{"1", "2"}), files);
}


TEST_F (filter_url, file_modification_time)
{
  // Test the file modification time.
  const std::string directory = filter_url_create_root_path ({filter_url_temp_dir (), "timetest"});
  filter_url_mkdir(directory);
  const std::string file = filter_url_create_path ({directory, "file.txt"});
  filter_url_file_put_contents (file, "file.txt");
  const int mod_time = filter_url_file_modification_time (file);
  const int ref_time = filter::date::seconds_since_epoch ();
  EXPECT_NEAR (mod_time, ref_time, 1);
  // Test the modification time returned if the file does not exist.
  filter_url_unlink (file);
  // const int mod_time_deleted = filter_url_file_modification_time (file);
  // For std::filesystem only EXPECT_EQ (0, mod_time_deleted);
}


TEST_F (filter_url, split_schem_host_port)
{
  // Testing the splitting of scheme and host and port.
  std::string scheme {};
  std::string host {};
  int port {0};
  
  filter_url_get_scheme_host_port ("https://bibledit.org:8080", scheme, host, port);
  EXPECT_EQ ("https", scheme);
  EXPECT_EQ ("bibledit.org", host);
  EXPECT_EQ (8080, port);
  
  filter_url_get_scheme_host_port ("bibledit.org:8080", scheme, host, port);
  EXPECT_EQ (std::string(), scheme);
  EXPECT_EQ ("bibledit.org", host);
  EXPECT_EQ (8080, port);
  
  filter_url_get_scheme_host_port ("bibledit.org", scheme, host, port);
  EXPECT_EQ (std::string(), scheme);
  EXPECT_EQ ("bibledit.org", host);
  EXPECT_EQ (0, port);
}


TEST_F (filter_url, rename)
{
  const std::string filename {"/tmp/בוקר טוב"};
  const std::string contents {"בוקר טוב בוקר טוב"};
  filter_url_file_put_contents (filename, contents);
  EXPECT_TRUE (file_or_dir_exists (filename));
  const std::string newfile {filename + "abc"};
  filter_url_rename (filename, newfile);
  EXPECT_FALSE (file_or_dir_exists (filename));
  EXPECT_TRUE (file_or_dir_exists (newfile));
}


TEST_F (filter_url, create_path)
{
  const std::vector <std::string> parts {"a", "b", "c"};
  std::string path = filter_url_create_path (parts);
  EXPECT_EQ ("a/b/c", path);
}


TEST_F (filter_url, build_http_query)
{
  // Empty URL in, empty URL out.
  EXPECT_EQ(std::string(), filter_url_build_http_query(std::string(), {}));
  EXPECT_EQ(std::string(), filter_url_build_http_query(std::string(), {{"foo", "bar"}}));

  // No parameters/values given: URL in -> URL out.
  EXPECT_EQ("url", filter_url_build_http_query("url", {}));

  // One parameter/value test.
  EXPECT_EQ("url?foo=bar", filter_url_build_http_query("url", {{"foo", "bar"}}));
  EXPECT_EQ("url?foo=bar&foo=bar", filter_url_build_http_query("url?foo=bar", {{"foo", "bar"}}));

  // Multiple parameters/values test.
  EXPECT_EQ("url?a=b&c=d", filter_url_build_http_query("url", {{"a","b"}, {"c","d"}}));
}


#endif

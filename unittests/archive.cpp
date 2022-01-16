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


#include <unittests/archive.h>
#include <unittests/utilities.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/string.h>
#include <filter/shell.h>


void test_archive ()
{
  trace_unit_tests (__func__);
  refresh_sandbox (true);
  
  // Prepare data structure for testing.
  
  string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);

  string file1 = "testarchive1";
  string file2 = "testarchive2";
  string path1 = filter_url_create_path_cpp17 ({directory, file1});
  string path2 = filter_url_create_path_cpp17 ({directory, file2});
  string data1;
  string data2;
  for (unsigned int i = 0; i < 1000; i++) {
    data1.append ("Data One\n");
    data2.append ("Data Two\n");
  }
  filter_url_file_put_contents (path1, data1);
  filter_url_file_put_contents (path2, data2);
  vector <string> files12 = { file1, file2 };

  for (int i = 0; i < 5; i++) {
    string path = filter_url_create_path_cpp17 ({directory, "testdata" + convert_to_string (i)});
    string data = convert_to_string (filter_string_rand (1000000, 2000000));
    for (int i2 = 0; i2 <= i; i2++) data.append (data);
    filter_url_file_put_contents (path, data);
    path = filter_url_create_path_cpp17 ({directory, convert_to_string (i), convert_to_string (i)});
    filter_url_mkdir (path);
    path = filter_url_create_path_cpp17 ({path, "data"});
    filter_url_file_put_contents (path, data);
  }
  
  // Test zip entire folder.
  {
    // Zip existing folder through the shell.
    string zipfile = filter_archive_zip_folder_shell_internal (directory);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    int size = filter_url_filesize (zipfile);
    int min = 3328;
    if (size < min) evaluate (__LINE__, __func__, "Should be at least " + convert_to_string (min) + " bytes", convert_to_string (size));
    int max = 3334;
    if (size > max) evaluate (__LINE__, __func__, "Should be no larger than " + convert_to_string (max) + " bytes", convert_to_string (size));

    // Zip existing folder through the miniz library.
    zipfile = filter_archive_zip_folder_miniz_internal (directory);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    size = filter_url_filesize (zipfile);
    if (size < 2437) evaluate (__LINE__, __func__, "Should be at least 2437 bytes", "");
    if (size > 2445) evaluate (__LINE__, __func__, "Should be no larger than 2445 bytes", "");

    // Zipping non-existing folder through the shell fails.
    zipfile = filter_archive_zip_folder_shell_internal ("xxx");
    evaluate (__LINE__, __func__, "", zipfile);

    // Zipping non-existing folder through the miniz library fails.
    zipfile = filter_archive_zip_folder_miniz_internal ("xxx");
    evaluate (__LINE__, __func__, "", zipfile);
  }
  
  // Test unzip through the shell.
  {
    // Create zip file through the shell.
    string zipfile = filter_archive_zip_folder_shell_internal (directory);
    // Test unzip through shell.
    string folder = filter_archive_unzip_shell_internal (zipfile);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    evaluate (__LINE__, __func__, 9000, filter_url_filesize (folder + "/testarchive1"));
    // Test that unzipping a non-existing zipfile returns nothing.
    folder = filter_archive_unzip_shell_internal ("xxxxx");
    evaluate (__LINE__, __func__, "", folder);
  }
  
  // Test unzip through the miniz library.
  {
    // Create a zipfile with test data through the shell.
    string zipfile = filter_archive_zip_folder_shell_internal (directory);
    // Unzip it through miniz and then check it.
    string folder = filter_archive_unzip_miniz_internal (zipfile);
    evaluate (__LINE__, __func__, false, folder.empty ());
    string out_err;
    int result = filter_shell_run ("diff -r " + directory + " " + folder, out_err);
    evaluate (__LINE__, __func__, "", out_err);
    evaluate (__LINE__, __func__, 0, result);
    // Test that unzipping a non-existing file returns nothing.
    folder = filter_archive_unzip_miniz_internal ("xxxxx");
    evaluate (__LINE__, __func__, "", folder);
  }
  
  // Test unzipping OpenDocument file through the miniz library.
  {
    string zipfile = filter_url_create_root_path_cpp17_Todo ({"odf", "template.odt"});
    string folder = filter_archive_unzip_miniz_internal (zipfile);
    evaluate (__LINE__, __func__, false, folder.empty ());
  }

  // Test tar gzip file.
  {
    // Test gzipped tarball compression.
    string tarball = filter_archive_tar_gzip_file (path1);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (tarball));
    size_t size = filter_url_filesize (tarball);
    size_t min = 155;
    size_t max = 181;
    if ((size < min) || (size > max)) evaluate (__LINE__, __func__, "between " + to_string (min) + " and " + to_string (max), convert_to_string (size));
    // Test that compressing a non-existing file returns NULL.
    tarball = filter_archive_tar_gzip_file ("xxxxx");
    evaluate (__LINE__, __func__, "", tarball);
  }
  
  // Test tar gzip folder.
  {
    // Test compress.
    string tarball = filter_archive_tar_gzip_folder (directory);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (tarball));
    int size = filter_url_filesize (tarball);
    int min = 618;
    int max = 634;
    if ((size < min) || (size > max)) evaluate (__LINE__, __func__, "between " + to_string (min) + " and " + to_string (max), convert_to_string (size));
    // Test that compressing a non-existing folder returns nothing.
    //tarball = filter_archive_tar_gzip_folder (directory + "/x");
    //evaluate (__LINE__, __func__, "", tarball);
  }
  
  // Test untargz.
  {
    // Create tarball.
    string tarball = filter_archive_tar_gzip_file (path1);
    // Test decompression.
    string folder = filter_archive_untar_gzip (tarball);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (folder));
    folder = filter_archive_uncompress (tarball);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (folder));
    evaluate (__LINE__, __func__, 9000, filter_url_filesize (folder + "/testarchive1"));
    // Test that unzipping garbage returns NULL.
    folder = filter_archive_untar_gzip ("xxxxx");
    evaluate (__LINE__, __func__, "", folder);
  }
  
  // Test embedded tar and untar routines.
  {
    string tarball = filter_url_tempfile () + ".tar";
    string folder;
    string result;
    int exitcode;

    // Fail to open empty tarball.
    result = filter_archive_microtar_pack ("", "", {});
    evaluate (__LINE__, __func__, "could not open", result);
    
    // Fail to unpack empty tarball.
    folder = filter_url_tempfile ();
    result = filter_archive_microtar_unpack ("", folder);
    evaluate (__LINE__, __func__, "could not open", result);
    
    // Pack files into a tarball.
    result = filter_archive_microtar_pack (tarball, directory, files12);
    evaluate (__LINE__, __func__, "", result);
    
    // Unpack the tarball created just before.
    folder = filter_url_tempfile ();
    result = filter_archive_microtar_unpack (tarball, folder);
    evaluate (__LINE__, __func__, "", result);

    // Check the untarred files.
    for (size_t i = 0; i < files12.size (); i++) {
      string file = files12 [i];
      string content = filter_url_file_get_contents (filter_url_create_path_cpp17 ({directory, file}));
      string data = filter_url_file_get_contents (filter_url_create_path_cpp17 ({folder, file}));
      evaluate (__LINE__, __func__, content, data);
    }
    
    // Pack files in a deep directory structure.
    vector <string> paths;
    filter_url_recursive_scandir (directory, paths);
    for (auto & path : paths) {
      path.erase (0, directory.length () + 1);
    }
    result = filter_archive_microtar_pack (tarball, directory, paths);
    evaluate (__LINE__, __func__, "", result);
    
    // Unpack the tarball with the deep directory structure.
    folder = filter_url_tempfile ();
    result = filter_archive_microtar_unpack (tarball, folder);
    evaluate (__LINE__, __func__, "", result);

    // Check the unpacked result.
    string out_err;
    exitcode = filter_shell_run ("diff -r " + directory + " " + folder, out_err);
    evaluate (__LINE__, __func__, "", out_err);
    evaluate (__LINE__, __func__, 0, exitcode);
  }

  // Clear up data used for the archive tests.
  refresh_sandbox (false);
}

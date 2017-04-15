/*
Copyright (©) 2003-2017 Teus Benschop.

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
  
  // Prepare for testing the archive functions.
  string file1 = "/tmp/testarchive1";
  string file2 = "/tmp/testarchive2";
  string data1;
  string data2;
  for (unsigned int i = 0; i < 1000; i++) {
    data1.append ("Data One\n");
    data2.append ("Data Two\n");
  }
  filter_url_file_put_contents (file1, data1);
  filter_url_file_put_contents (file2, data2);

  string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);
  for (int i = 0; i < 5; i++) {
    string path = filter_url_create_path (directory, "testdata" + convert_to_string (i));
    string data = convert_to_string (filter_string_rand (1000000, 2000000));
    for (int i2 = 0; i2 <= i; i2++) data.append (data);
    filter_url_file_put_contents (path, data);
    path = filter_url_create_path (directory, convert_to_string (i), convert_to_string (i));
    filter_url_mkdir (path);
    path = filter_url_create_path (path, "data");
    filter_url_file_put_contents (path, data);
  }
  
  // Test zip compression of one file.
  {
    string zipfile = filter_archive_zip_file (file1);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    evaluate (__LINE__, __func__, 223, filter_url_filesize (zipfile));
    filter_url_unlink (zipfile);
    // Test compressing a non-existing file.
    zipfile = filter_archive_zip_file ("xxxxx");
    evaluate (__LINE__, __func__, "", zipfile);
  }

  // Test zip folder.
  {
    string folder = filter_url_tempfile();
    filter_url_mkdir (folder);
    filter_url_file_put_contents (folder + "/file1", data1);
    filter_url_file_put_contents (folder + "/file2", data2);
    // Test zip compression.
    string zipfile = filter_archive_zip_folder (folder);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    evaluate (__LINE__, __func__, 396, filter_url_filesize (zipfile));
    // Clean up the mess.
    filter_url_unlink (zipfile);
    filter_url_rmdir (folder);
  }
  
  // Test unzip through the shell.
  {
    string zipfile = filter_archive_zip_file (file1);
    // Test unzip.
    string folder = filter_archive_unzip_shell (zipfile);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    evaluate (__LINE__, __func__, 9000, filter_url_filesize (folder + "/testarchive1"));
    filter_url_unlink (zipfile);
    filter_url_rmdir (folder);
    // Test that unzipping garbage returns NULL.
    folder = filter_archive_unzip_shell ("xxxxx");
    evaluate (__LINE__, __func__, "", folder);
  }
  
  // Test unzip through the miniz library.
  {
    // Zip a sample.
    string zipfile = filter_archive_zip_folder (directory);
    // Unzip it and check it.
    string folder = filter_archive_unzip_miniz (zipfile);
    evaluate (__LINE__, __func__, false, folder.empty ());
    string out_err;
    int result = filter_shell_run ("diff -r " + directory + " " + folder, out_err);
    evaluate (__LINE__, __func__, "", out_err);
    evaluate (__LINE__, __func__, 0, result);
    // Test that unzipping a non-existing file returns nothing.
    folder = filter_archive_unzip_miniz ("xxxxx");
    evaluate (__LINE__, __func__, "", folder);
  }
  
  // Test tar gzip file.
  {
    // Test gzipped tarball compression.
    string tarball = filter_archive_tar_gzip_file (file1);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (tarball));
    int size = filter_url_filesize (tarball);
    if ((size < 155) || (size > 180)) evaluate (__LINE__, __func__, "between 155 and 180", convert_to_string (size));
    // Clean up tarball from /tmp folder.
    filter_url_unlink (tarball);
    // Test that compressing a non-existing file returns NULL.
    tarball = filter_archive_tar_gzip_file ("xxxxx");
    evaluate (__LINE__, __func__, "", tarball);
  }
  
  // Test tar gzip Folder.
  {
    string folder = filter_url_tempfile ();
    filter_url_mkdir (folder);
    filter_url_file_put_contents (folder + "/file1", data1);
    filter_url_file_put_contents (folder + "/file2", data2);
    // Test compression.
    string tarball = filter_archive_tar_gzip_folder (folder);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (tarball));
    int size = filter_url_filesize (tarball);
    if ((size < 235) || (size > 260)) evaluate (__LINE__, __func__, "between 235 and 260", convert_to_string (size));
    // Clean up.
    filter_url_unlink (tarball);
    filter_url_rmdir (folder);
    // Test that compressing a non-existing folder returns NULL.
    //tarball = filter_archive_tar_gzip_folder (folder + "/x");
    //evaluate (__LINE__, __func__, "", tarball);
  }

  // Test untargz.
  {
    string tarball = filter_archive_tar_gzip_file (file1);
    // Test decompression.
    string folder = filter_archive_untar_gzip (tarball);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (folder));
    filter_url_rmdir (folder);
    folder = filter_archive_uncompress (tarball);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (folder));
    evaluate (__LINE__, __func__, 9000, filter_url_filesize (folder + "/testarchive1"));
    filter_url_rmdir (folder);
    filter_url_unlink (tarball);
    // Test that unzipping garbage returns NULL.
    folder = filter_archive_untar_gzip ("xxxxx");
    evaluate (__LINE__, __func__, "", folder);
  }

  {
    evaluate (__LINE__, __func__, true, filter_archive_can_zip ());
    evaluate (__LINE__, __func__, true, filter_archive_can_unzip ());
  }

  // Test embedded tar and untar routines.
  {
    string tarball;
    string directory;
    vector <string> files;
    vector <string> contents;
    string result;
    
    // Fail to open empty tarball.
    result = filter_archive_microtar_pack ("", "", {});
    evaluate (__LINE__, __func__, "could not open", result);

    // Store test data in files.
    directory = filter_url_tempfile ();
    filter_url_mkdir (directory);
    int max = 10;
    for (int i = 1; i <= max; i++) {
      string file = "file" + convert_to_string (i) + ".txt";
      files.push_back (file);
      string data;
      for (unsigned int i2 = 0; i2 < 100; i2++) {
        data.append ("Data " + convert_to_string (i) + "\n");
      }
      filter_url_file_put_contents (filter_url_create_path (directory, file), data);
      contents.push_back (data);
    }

    // Pack files into a tarball.
    tarball = filter_url_tempfile () + ".tar";
    result = filter_archive_microtar_pack (tarball, directory, files);
    evaluate (__LINE__, __func__, "", result);

    // Fail to unpack empty tarball.
    result = filter_archive_microtar_unpack ("", directory);
    evaluate (__LINE__, __func__, "could not open", result);

    // Unpack the tarball created just before.
    directory = filter_url_tempfile ();
    result = filter_archive_microtar_unpack (tarball, directory);
    evaluate (__LINE__, __func__, "", result);
    
    // Test the unpacked files.
    for (size_t i = 0; i < files.size (); i++) {
      string file = files [i];
      string content = contents [i];
      string data = filter_url_file_get_contents (filter_url_create_path (directory, file));
      evaluate (__LINE__, __func__, content, data);
    }
  }
  
  // Clear up data used for the archive tests.
  refresh_sandbox (false);
  filter_url_unlink (file1);
  filter_url_unlink (file2);
}

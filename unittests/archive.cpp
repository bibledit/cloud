/*
Copyright (©) 2003-2016 Teus Benschop.

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

  {
    // Test zip compression of one file.
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
  
  // Test unzip.
  {
    string zipfile = filter_archive_zip_file (file1);
    // Test unzip.
    string folder = filter_archive_unzip (zipfile);
    evaluate (__LINE__, __func__, true, file_or_dir_exists (zipfile));
    evaluate (__LINE__, __func__, 9000, filter_url_filesize (folder + "/testarchive1"));
    filter_url_unlink (zipfile);
    filter_url_rmdir (folder);
    // Test that unzipping garbage returns NULL.
    folder = filter_archive_unzip ("xxxxx");
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
    string plain = "This is data that is data to be compressed.";
    string compressed = filter_archive_compress (plain);
    string output = filter_archive_decompress (compressed);
    evaluate (__LINE__, __func__, plain, output);
    
    evaluate (__LINE__, __func__, true, filter_archive_can_zip ());
    evaluate (__LINE__, __func__, true, filter_archive_can_unzip ());
  }
  
  // Clear up data used for the archive tests.
  refresh_sandbox (false);
  filter_url_unlink (file1);
  filter_url_unlink (file2);
}

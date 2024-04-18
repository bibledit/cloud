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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <filter/url.h>
#include <filter/archive.h>
#include <filter/string.h>
#include <filter/shell.h>
using namespace std;


TEST (filter, archive)
{
  refresh_sandbox (false);
  
  // Prepare data structure for testing.
  
  std::string directory = filter_url_tempfile ();
  filter_url_mkdir (directory);

  std::string file1 = "testarchive1";
  std::string file2 = "testarchive2";
  std::string path1 = filter_url_create_path ({directory, file1});
  std::string path2 = filter_url_create_path ({directory, file2});
  std::string data1;
  std::string data2;
  for (unsigned int i = 0; i < 1000; i++) {
    data1.append ("Data One\n");
    data2.append ("Data Two\n");
  }
  filter_url_file_put_contents (path1, data1);
  filter_url_file_put_contents (path2, data2);
  std::vector <std::string> files12 = { file1, file2 };

  for (int i = 0; i < 5; i++) {
    std::string path = filter_url_create_path ({directory, "testdata" + filter::strings::convert_to_string (i)});
    std::string data = filter::strings::convert_to_string (filter::strings::rand (1000000, 2000000));
    for (int i2 = 0; i2 <= i; i2++) data.append (data);
    filter_url_file_put_contents (path, data);
    path = filter_url_create_path ({directory, filter::strings::convert_to_string (i), filter::strings::convert_to_string (i)});
    filter_url_mkdir (path);
    path = filter_url_create_path ({path, "data"});
    filter_url_file_put_contents (path, data);
  }
  
  // Test zip entire folder.
  {
    // Zip existing folder through the shell.
    std::string zipfile = filter_archive_zip_folder_shell_internal (directory);
    EXPECT_EQ (true, file_or_dir_exists (zipfile));
    int size = filter_url_filesize (zipfile);
    int min = 3328;
    if (size < min) EXPECT_EQ ("Should be at least " + filter::strings::convert_to_string (min) + " bytes", filter::strings::convert_to_string (size));
    int max = 3334;
    if (size > max) EXPECT_EQ ("Should be no larger than " + filter::strings::convert_to_string (max) + " bytes", filter::strings::convert_to_string (size));

    // Zip existing folder through the miniz library.
    zipfile = filter_archive_zip_folder_miniz_internal (directory);
    EXPECT_EQ (true, file_or_dir_exists (zipfile));
    size = filter_url_filesize (zipfile);
    if (size < 2433) EXPECT_EQ ("Should be at least 2433 bytes", to_string(size));
    if (size > 2445) EXPECT_EQ ("Should be no larger than 2445 bytes", to_string(size));

    // Zipping non-existing folder through the shell fails.
    zipfile = filter_archive_zip_folder_shell_internal ("xxx");
    EXPECT_EQ ("", zipfile);

    // Zipping non-existing folder through the miniz library fails.
    zipfile = filter_archive_zip_folder_miniz_internal ("xxx");
    EXPECT_EQ ("", zipfile);
  }
  
  // Test unzip through the shell.
  {
    // Create zip file through the shell.
    std::string zipfile = filter_archive_zip_folder_shell_internal (directory);
    // Test unzip through shell.
    std::string folder = filter_archive_unzip_shell_internal (zipfile);
    EXPECT_EQ (true, file_or_dir_exists (zipfile));
    EXPECT_EQ (9000, filter_url_filesize (folder + "/testarchive1"));
    // Test that unzipping a non-existing zipfile returns nothing.
    folder = filter_archive_unzip_shell_internal ("xxxxx");
    EXPECT_EQ ("", folder);
  }
  
  // Test unzip through the miniz library.
  {
    // Create a zipfile with test data through the shell.
    std::string zipfile = filter_archive_zip_folder_shell_internal (directory);
    // Unzip it through miniz and then check it.
    std::string folder = filter_archive_unzip_miniz_internal (zipfile);
    EXPECT_EQ (false, folder.empty ());
    std::string out_err;
    int result = filter_shell_run ("diff -r " + directory + " " + folder, out_err);
    EXPECT_EQ ("", out_err);
    EXPECT_EQ (0, result);
    // Test that unzipping a non-existing file returns nothing.
    folder = filter_archive_unzip_miniz_internal ("xxxxx");
    EXPECT_EQ ("", folder);
  }
  
  // Test unzipping OpenDocument file through the miniz library.
  {
    std::string zipfile = filter_url_create_root_path ({"odf", "template.odt"});
    std::string folder = filter_archive_unzip_miniz_internal (zipfile);
    EXPECT_EQ (false, folder.empty ());
  }

  // Test tar gzip file.
  {
    // Test gzipped tarball compression.
    std::string tarball = filter_archive_tar_gzip_file (path1);
    EXPECT_EQ (true, file_or_dir_exists (tarball));
    int size = filter_url_filesize (tarball);
    int min = 155;
    int max = 181;
    if ((size < min) || (size > max)) EXPECT_EQ ("between " + to_string (min) + " and " + to_string (max), filter::strings::convert_to_string (size));
    // Test that compressing a non-existing file returns NULL.
    tarball = filter_archive_tar_gzip_file ("xxxxx");
    EXPECT_EQ ("", tarball);
  }
  
  // Test tar gzip folder.
  {
    // Test compress.
    std::string tarball = filter_archive_tar_gzip_folder (directory);
    EXPECT_EQ (true, file_or_dir_exists (tarball));
    int size = filter_url_filesize (tarball);
    int min = 618;
    int max = 634;
    if ((size < min) || (size > max)) EXPECT_EQ ("between " + to_string (min) + " and " + to_string (max), filter::strings::convert_to_string (size));
    // Test that compressing a non-existing folder returns nothing.
    //tarball = filter_archive_tar_gzip_folder (directory + "/x");
    //EXPECT_EQ ("", tarball);
  }
  
  // Test untargz.
  {
    // Create tarball.
    std::string tarball = filter_archive_tar_gzip_file (path1);
    // Test decompression.
    std::string folder = filter_archive_untar_gzip (tarball);
    EXPECT_EQ (true, file_or_dir_exists (folder));
    folder = filter_archive_uncompress (tarball);
    EXPECT_EQ (true, file_or_dir_exists (folder));
    EXPECT_EQ (9000, filter_url_filesize (folder + "/testarchive1"));
    // Test that unzipping garbage returns NULL.
    folder = filter_archive_untar_gzip ("xxxxx");
    EXPECT_EQ ("", folder);
  }
  
  // Test embedded tar and untar routines.
  {
    std::string tarball = filter_url_tempfile () + ".tar";
    std::string folder;
    std::string result;
    int exitcode;

    // Fail to open empty tarball.
    result = filter_archive_microtar_pack ("", "", {});
    EXPECT_EQ ("could not open", result);
    
    // Fail to unpack empty tarball.
    folder = filter_url_tempfile ();
    result = filter_archive_microtar_unpack ("", folder);
    EXPECT_EQ ("could not open", result);
    
    // Pack files into a tarball.
    result = filter_archive_microtar_pack (tarball, directory, files12);
    EXPECT_EQ ("", result);
    
    // Unpack the tarball created just before.
    folder = filter_url_tempfile ();
    result = filter_archive_microtar_unpack (tarball, folder);
    EXPECT_EQ ("", result);

    // Check the untarred files.
    for (size_t i = 0; i < files12.size (); i++) {
      std::string file = files12 [i];
      std::string content = filter_url_file_get_contents (filter_url_create_path ({directory, file}));
      std::string data = filter_url_file_get_contents (filter_url_create_path ({folder, file}));
      EXPECT_EQ (content, data);
    }
    
    // Pack files in a deep directory structure.
    std::vector <std::string> paths;
    filter_url_recursive_scandir (directory, paths);
    for (auto & path : paths) {
      path.erase (0, directory.length () + 1);
    }
    result = filter_archive_microtar_pack (tarball, directory, paths);
    EXPECT_EQ ("", result);
    
    // Unpack the tarball with the deep directory structure.
    folder = filter_url_tempfile ();
    result = filter_archive_microtar_unpack (tarball, folder);
    EXPECT_EQ ("", result);

    // Check the unpacked result.
    std::string out_err;
    exitcode = filter_shell_run ("diff -r " + directory + " " + folder, out_err);
    EXPECT_EQ ("", out_err);
    EXPECT_EQ (0, exitcode);
  }

  // Clear up data used for the archive tests.
  refresh_sandbox (false);
}


#endif


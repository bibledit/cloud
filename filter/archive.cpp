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


#include <filter/archive.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <filter/string.h>
#include <database/logs.h>
#include <microtar/microtar.h>
#include <miniz/miniz.h>
using namespace std;


// Work around old Microsoft macro definitions.
#undef max
#undef min


// Compresses a $folder into zip format.
// Returns the path to the compressed archive it created.
string filter_archive_zip_folder (string folder)
{
#ifdef HAVE_CLOUD
  return filter_archive_zip_folder_shell_internal (folder);
#endif
#ifdef HAVE_CLIENT
  return filter_archive_zip_folder_miniz_internal (folder);
#endif
}


// Compresses a $folder into zip format.
// Returns the path to the compressed archive it created.
string filter_archive_zip_folder_shell_internal (string folder)
{
  if (!file_or_dir_exists (folder)) return string();
  string zippedfile = filter_url_tempfile () + ".zip";
#ifdef HAVE_CLOUD
  string logfile = filter_url_tempfile () + ".log";
  folder = filter_url_escape_shell_argument (folder);
  string command = "cd " + folder + " && zip -r " + zippedfile + " * > " + logfile + " 2>&1";
  int return_var;
  // Run the command.
  return_var = system (command.c_str());
  if (return_var != 0) {
    filter_url_unlink (zippedfile);
    zippedfile.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  }
#endif
  return zippedfile;
}


// Compresses a $folder into zip format.
// Returns the path to the compressed archive it created.
string filter_archive_zip_folder_miniz_internal (string folder)
{
  if (!file_or_dir_exists (folder)) {
    return "";
  }
  string zippedfile = filter_url_tempfile () + ".zip";
  vector <string> paths;
  filter_url_recursive_scandir (folder, paths);
  for (auto path : paths) {
    bool is_dir = filter_url_is_dir (path);
    string file = path.substr (folder.size () + 1);
#ifdef HAVE_WINDOWS
    // The file names in Windows will be backslashes (\) at this point.
    // But the mzip library, in its current configuration, works with forward slashes (/).
    // So the code below, in case of Windows, updates the type of slashes.
    file = filter::strings::replace (DIRECTORY_SEPARATOR, "/", file);
#endif
    mz_bool status;
    if (is_dir) {
      file.append ("/");
      status = mz_zip_add_mem_to_archive_file_in_place(zippedfile.c_str(), file.c_str(), nullptr, 0, "", 0, MZ_DEFAULT_LEVEL);
    } else {
      string contents = filter_url_file_get_contents (path);
      status = mz_zip_add_mem_to_archive_file_in_place (zippedfile.c_str(), file.c_str(), contents.c_str(), contents.size(), "", 0, MZ_DEFAULT_LEVEL);
    }
    if (!status) {
      Database_Logs::log ("mz_zip_add_mem_to_archive_file_in_place failed for " + path);
      return "";
    }
  }
  return zippedfile;
}


// Uncompresses a zip archive identified by $file.
// Returns the path to the folder it created.
string filter_archive_unzip (string file)
{
#ifdef HAVE_CLOUD
  return filter_archive_unzip_shell_internal (file);
#endif
#ifdef HAVE_CLIENT
  return filter_archive_unzip_miniz_internal (file);
#endif
}


// Uncompresses a zip archive identified by $file.
// Returns the path to the folder it created.
string filter_archive_unzip_shell_internal ([[maybe_unused]] string file)
{
  string folder = filter_url_tempfile ();
#ifdef HAVE_CLOUD
  filter_url_mkdir (folder);
  folder.append (DIRECTORY_SEPARATOR);
  string logfile = filter_url_tempfile () + ".log";
  file = filter_url_escape_shell_argument (file);
  string command = "unzip -o -d " + folder + " " + file + " > " + logfile + " 2>&1";
  // Run the command.
  int return_var = system (command.c_str());
  if (return_var != 0) {
    filter_url_rmdir (folder);
    folder.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  } else {
    // Set free permissions after unzipping.
    command = "chmod -R 0777 " + folder;
    [[maybe_unused]] int result = system (command.c_str ());
  }
#endif
  return folder;
}


// Uncompresses the $zipfile.
// Returns the path to the folder it created.
string filter_archive_unzip_miniz_internal (string zipfile)
{
  // Directory where to unzip the archive.
  string folder = filter_url_tempfile ();
  filter_url_mkdir (folder);

  // Covers the entire process.
  bool error = false;

  // Open the zip archive.
  mz_bool status;
  mz_zip_archive zip_archive;
  memset (&zip_archive, 0, sizeof (zip_archive));
  status = mz_zip_reader_init_file(&zip_archive, zipfile.c_str(), 0);
  if (status) {
    
    // Iterate over the files in the archive.
    unsigned filecount = mz_zip_reader_get_num_files (&zip_archive);
    for (unsigned i = 0; i < filecount; i++) {

      // If there was an error, skip processing further files.
      if (error) continue;

      // Get information about this file.
      mz_zip_archive_file_stat file_stat;
      status = mz_zip_reader_file_stat (&zip_archive, i, &file_stat);
      if (status) {

        string filename = filter_url_create_path ({folder, file_stat.m_filename});
        // The miniz library returns Unix directory separators above.
        // So in case of Windows, convert them to Windows ones.
        string fixed_filename = filter_url_update_directory_separator_if_windows (filename);
        
        if (mz_zip_reader_is_file_a_directory (&zip_archive, i)) {
          // Create this directory.
          if (!file_or_dir_exists (fixed_filename)) filter_url_mkdir (fixed_filename);
        } else {
          /* Code that extracts file contents memory, if needed.
          size_t filesize = file_stat.m_uncomp_size;
          cout << filename << " " << filesize << endl;
          void * buff = operator new (filesize);
          if (buff) {
            status = mz_zip_reader_extract_to_mem (&zip_archive, i, buff, filesize, 0);
            if (status) {
              string contents (static_cast<const char*>(buff), filesize);
            } else {
              // "mz_zip_reader_extract_to_mem failure for " + filename + " in " + zipfile;
              error = true;
            }
            operator delete (buff);
          } else {
            // "failure to allocate buffer for file extraction";
            error = true;
          }
           */
          
          // Ensure this file's folder exists.
          string dirname = filter_url_dirname (fixed_filename);
          if (!file_or_dir_exists (dirname)) filter_url_mkdir (dirname);
          // Extract this file.
          status = mz_zip_reader_extract_to_file (&zip_archive, i, fixed_filename.c_str(), 0);
          if (!status) {
            Database_Logs::log ("mz_zip_reader_extract_to_file failure for file " + filename + " in " + zipfile);
            error = true;
          }
        }
        
      } else {
        Database_Logs::log ("mz_zip_reader_file_stat failed for " + zipfile);
        error = true;
      }
    }
    
    // Close the archive, freeing any resources it was using.
    mz_zip_reader_end (&zip_archive);
    
  } else {
    Database_Logs::log ("mz_zip_reader_init_file failed for " + zipfile);
    error = true;
  }

  // If there was an error, return nothing, to indicate that uncompression has failed.
  if (error) folder.clear ();

  // The folder where the files were unpacked.
  return folder;
}


// Compresses a file identified by $filename into gzipped tar format.
// Returns the path to the compressed archive it created.
string filter_archive_tar_gzip_file (string filename)
{
  string tarball = filter_url_tempfile () + ".tar.gz";
  string dirname = filter_url_escape_shell_argument (filter_url_dirname (filename));
  string basename = filter_url_escape_shell_argument (filter_url_basename (filename));
  string logfile = filter_url_tempfile () + ".log";
  string command = "cd " + dirname + " && tar -czf " + tarball + " " + basename + " > " + logfile + " 2>&1";
  int return_var;
#ifdef HAVE_IOS
  // Crashes on iOS.
  return_var = 1;
#else
  // Run the command.
  return_var = system (command.c_str());
#endif
  if (return_var != 0) {
    filter_url_unlink (tarball);
    tarball.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  }
  return tarball;
}


// Compresses a $folder into gzipped tar format.
// Returns the path to the compressed archive it created.
string filter_archive_tar_gzip_folder (string folder)
{
  string tarball = filter_url_tempfile () + ".tar.gz";
  folder = filter_url_escape_shell_argument (folder);
  string logfile = filter_url_tempfile () + ".log";
  string command = "cd " + folder + " && tar -czf " + tarball + " . > " + logfile + " 2>&1";
  int return_var;
#ifdef HAVE_IOS
  // Crashes on iOS.
  return_var = 1;
#else
  // Run the command.
  return_var = system (command.c_str());
#endif
  if (return_var != 0) {
    filter_url_unlink (tarball);
    tarball.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  }
  return tarball;
}


// Uncompresses a .tar.gz archive identified by $file.
// Returns the path to the folder it created.
string filter_archive_untar_gzip (string file)
{
  file = filter_url_escape_shell_argument (file);
  string folder = filter_url_tempfile ();
  filter_url_mkdir (folder);
  folder.append (DIRECTORY_SEPARATOR);
  string logfile = filter_url_tempfile () + ".log";
  string command = "cd " + folder + " && tar zxf " + file + " > " + logfile + " 2>&1";
  int return_var;
#ifdef HAVE_IOS
  // Crashes on iOS.
  return_var = 1;
#else
  // Run the command.
  return_var = system (command.c_str());
#endif
  if (return_var != 0) {
    filter_url_rmdir (folder);
    folder.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  }
  return folder;
}


// Uncompresses a known archive identified by $file.
// Returns the path to the folder it created.
string filter_archive_uncompress (string file)
{
  int type = filter_archive_is_archive (file);
  if (type == 1) {
    return filter_archive_untar_gzip (file);
  }
  if (type == 2) {
    return filter_archive_unzip (file);
  }
  return "";
}


// Returns 0 if it is not an archive that Bibledit supports.
// Else returns 1, 2, 3... depending on the type of archive.
int filter_archive_is_archive (string file)
{
  // Tar (.tar) archives, including those compressed with gzip (.tar.gz, .tgz), bzip (.tar.bz, .tbz), bzip2 (.tar.bz2, .tbz2), compress (.tar.Z, .taz), lzop (.tar.lzo, .tzo) and lzma (.tar.lzma)
  // Zip archives (.zip)
  // Jar archives (.jar, .ear, .war)
  // 7z archives (.7z)
  // iso9660 CD images (.iso)
  // Lha archives (.lzh)
  // Single files compressed with gzip (.gz), bzip (.bz), bzip2 (.bz2), compress (.Z), lzop (.lzo) and lzma (.lzma)
  string suffix = filter_url_get_extension (file);
  if ((suffix == "tar.gz") || (suffix == "gz") || (suffix == "tgz")) {
    return 1;
  }
  if ((suffix == "zip")) {
    return 2;
  }
  if ((suffix == "tar.bz") || (suffix == "tbz") || (suffix == "tar.bz2") || (suffix == "tbz2")) {
    return 0;
  }
  return 0;
}


// Create a tarball at $tarpath with input $files from $directory.
string filter_archive_microtar_pack (string tarpath, string directory, vector <string> files)
{
  mtar_t tar;
  int res;
  
  // Open archive for writing.
  res = mtar_open (&tar, tarpath.c_str(), "w");
  if (res != MTAR_ESUCCESS) return mtar_strerror (res);
  
  // Iterate over the files.
  for (auto file : files) {
    // Full path.
    string path = filter_url_create_path ({directory, file});
    // Skip directories.
    if (filter_url_is_dir (path)) continue;
    // Read the file's data.
    string data = filter_url_file_get_contents (path);
    // Write the file's name to the tarball.
    res = mtar_write_file_header(&tar, file.c_str(), static_cast<unsigned> (data.length ()));
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
    // Write the file's data to the tarball.
    res = mtar_write_data(&tar, data.c_str(), static_cast<unsigned> (data.length ()));
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
  }
  
  // Finalize: This needs to be the last thing done before closing.
  res = mtar_finalize(&tar);
  if (res != MTAR_ESUCCESS) return mtar_strerror (res);
  
  // Close archive.
  res = mtar_close(&tar);
  if (res != MTAR_ESUCCESS) return mtar_strerror (res);
  
  // OK, done :)
  return "";
}


// Unpack the tarball at $tarpath and store the individual files at $outputpath.
string filter_archive_microtar_unpack (string tarball, string directory)
{
  mtar_t tar;
  mtar_header_t h;
  int res;
  
  // Open archive for reading.
  res = mtar_open (&tar, tarball.c_str(), "r");
  if (res != MTAR_ESUCCESS) return mtar_strerror (res);
  
  // Read all file names.
  vector <string> files;
  while ((mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD) {
    files.push_back (h.name);
    mtar_next(&tar);
  }
  
  // Create directory if needed.
  if (!file_or_dir_exists (directory)) filter_url_mkdir (directory);

  // Unpack all files and save them.
  for (auto file : files) {
    // Find the file's information.
    res = mtar_find (&tar, file.c_str(), &h);
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
    // Read the file's data.
    char *p = static_cast<char *> (calloc(1, h.size + 1));
    res = mtar_read_data(&tar, p, h.size);
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
    string data (p, h.size);
    free(p);
    // If the file contains a directory, ensure that directory exists.
    string dirname = filter_url_dirname (file);
    if (dirname != ".") {
      dirname = filter_url_create_path ({directory, dirname});
      if (!file_or_dir_exists (dirname)) filter_url_mkdir (dirname);
    }
    // Write the file's data.
    filter_url_file_put_contents (filter_url_create_path ({directory, file}), data);
  }
  
  // Close archive.
  res = mtar_close(&tar);
  if (res != MTAR_ESUCCESS) return mtar_strerror (res);

  // Done, hallelujah :)
  return "";
}

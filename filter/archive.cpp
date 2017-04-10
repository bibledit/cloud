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


#include <filter/archive.h>
#include <filter/url.h>
#include <filter/shell.h>
#include <database/logs.h>
#include <microtar/microtar.h>


// Work around old Microsoft macro definitions.
#undef max
#undef min


// Whether the operating system can zip data.
bool filter_archive_can_zip ()
{
  return filter_shell_is_present ("zip");
}


// Compresses a file identified by $filename into zip format.
// Returns the path to the zipfile it created.
string filter_archive_zip_file (string filename)
{
  if (!file_or_dir_exists (filename)) return "";
  string zippedfile = filter_url_tempfile () + ".zip";
  string logfile = filter_url_tempfile () + ".log";
  string dirname = filter_url_escape_shell_argument (filter_url_dirname (filename));
  string basename = filter_url_escape_shell_argument (filter_url_basename (filename));
  string command = "cd " + dirname + " && zip " + zippedfile + " " + basename + " > " + logfile + " 2>&1";
  int return_var = system (command.c_str());
  if (return_var != 0) {
    filter_url_unlink (zippedfile);
    zippedfile.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  }
  return zippedfile;
}
 

// Compresses a $folder into zip format.
// Returns the path to the compressed archive it created.
string filter_archive_zip_folder (string folder)
{
  if (!file_or_dir_exists (folder)) return "";
  string zippedfile = filter_url_tempfile () + ".zip";
  string logfile = filter_url_tempfile () + ".log";
  folder = filter_url_escape_shell_argument (folder);
  string command = "cd " + folder + " && zip -r " + zippedfile + " * > " + logfile + " 2>&1";
  int return_var = system (command.c_str());
  if (return_var != 0) {
    filter_url_unlink (zippedfile);
    zippedfile.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  }
  return zippedfile;
}


// Whether the operating system can unzip data.
bool filter_archive_can_unzip ()
{
  return filter_shell_is_present ("unzip");
}


// Uncompresses a zip archive identified by $file.
// Returns the path to the folder it created.
string filter_archive_unzip (string file)
{
  string folder = filter_url_tempfile ();
  filter_url_mkdir (folder);
  folder.append (DIRECTORY_SEPARATOR);
  string logfile = filter_url_tempfile () + ".log";
  file = filter_url_escape_shell_argument (file);
  string command = "unzip -o -d " + folder + " " + file + " > " + logfile + " 2>&1";
  int return_var = system (command.c_str());
  if (return_var != 0) {
    filter_url_rmdir (folder);
    folder.clear();
    string errors = filter_url_file_get_contents (logfile);
    Database_Logs::log (errors);
  } else {
    // Set free permissions after unzipping.
    command = "chmod -R 0777 " + folder;
    int result = system (command.c_str ());
    (void) result;
  }
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
  int return_var = system (command.c_str());
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
  int return_var = system (command.c_str());
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
  int return_var = system (command.c_str());
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


// Returns 0 is not an archive that Bibledit supports.
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
    // Read the file's data.
    string data = filter_url_file_get_contents (filter_url_create_path (directory, file));
    // Write the file's name to the tarball.
    res = mtar_write_file_header(&tar, file.c_str(), data.length ());
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
    // Write the file's data to the tarball.
    res = mtar_write_data(&tar, data.c_str(), data.length ());
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
    res = mtar_find (&tar, file.c_str(), &h);
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
    char *p = (char *)calloc(1, h.size + 1);
    res = mtar_read_data(&tar, p, h.size);
    if (res != MTAR_ESUCCESS) return mtar_strerror (res);
    string data (p, h.size);
    free(p);
    filter_url_file_put_contents (filter_url_create_path (directory, file), data);
  }
  
  // Close archive.
  res = mtar_close(&tar);
  if (res != MTAR_ESUCCESS) return mtar_strerror (res);

  // Done, hallelujah :)
  return "";
}

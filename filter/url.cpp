/*
Copyright (©) 2003-2026 Teus Benschop.

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


#include <filter/url.h>
#include <webserver/http.h>
#include <webserver/request.h>
#include <config/globals.h>
#include <config/config.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <filter/UriCodec.hpp>
#pragma GCC diagnostic pop
#include <filter/string.h>
#include <filter/date.h>
#include <database/books.h>
#include <database/logs.h>
#ifdef HAVE_CLOUD
#include <curl/curl.h>
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wswitch-enum"
#include <mbedtls/version.h>
#include <mbedtls/platform.h>
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#pragma GCC diagnostic pop
#ifdef HAVE_WINDOWS
#pragma comment(lib, "bcrypt.lib")
#endif

// Static check on required definitions, taken from the ssl_client1.c example.
#ifndef MBEDTLS_BIGNUM_C
static_assert (false, "MBEDTLS_BIGNUM_C should be defined");
#endif
#ifndef MBEDTLS_ENTROPY_C
static_assert (false, "MBEDTLS_ENTROPY_C should be defined");
#endif
#ifndef MBEDTLS_SSL_TLS_C
static_assert (false, "MBEDTLS_SSL_TLS_C should be defined");
#endif
#ifndef MBEDTLS_SSL_CLI_C
static_assert (false, "MBEDTLS_SSL_CLI_C should be defined");
#endif
#ifndef MBEDTLS_NET_C
static_assert (false, "MBEDTLS_NET_C should be defined");
#endif
#ifndef MBEDTLS_RSA_C
static_assert (false, "MBEDTLS_RSA_C should be defined");
#endif
#ifndef MBEDTLS_PEM_PARSE_C
static_assert (false, "MBEDTLS_PEM_PARSE_C should be defined");
#endif
#ifndef MBEDTLS_CTR_DRBG_C
static_assert (false, "MBEDTLS_CTR_DRBG_C should be defined");
#endif
#ifndef MBEDTLS_X509_CRT_PARSE_C
static_assert (false, "MBEDTLS_X509_CRT_PARSE_C should be defined");
#endif
#ifndef MBEDTLS_DEBUG_C
static_assert (false, "MBEDTLS_DEBUG_C should be defined");
#endif
//#ifndef MBEDTLS_USE_PSA_CRYPTO
//static_assert (false, "MBEDTLS_USE_PSA_CRYPTO should be defined");
//#endif
#ifdef MBEDTLS_X509_REMOVE_INFO
static_assert (false, "MBEDTLS_X509_REMOVE_INFO should not be defined");
#endif


#if MBEDTLS_VERSION_MAJOR == 2
#elif MBEDTLS_VERSION_MAJOR == 3
#else
static_assert (false, "MbedTLS version other than 2 or 3");
#endif


// SSL/TLS variables.
static mbedtls_x509_crt x509_ca_cert;
static mbedtls_ctr_drbg_context ctr_drbg_context;
static mbedtls_entropy_context entropy_context;


static std::vector <std::string> filter_url_scandir_internal (std::string folder, bool include_hidden = false)
{
  std::vector <std::string> files;
  
#ifdef HAVE_WINDOWS
  
  if (!folder.empty()) {
    if (folder[folder.size() - 1] == '\\') {
      folder = folder.substr(0, folder.size() - 1);
    }
    folder.append("\\*");
    std::wstring wfolder = filter::string::string2wstring(folder);
    WIN32_FIND_DATA fdata;
    HANDLE hFind = FindFirstFileW(wfolder.c_str(), &fdata);
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        std::wstring wfilename(fdata.cFileName);
        std::string name = filter::string::wstring2string (wfilename);
        if (name.substr(0, 1) != ".") {
          files.push_back(name);
        }
      } while (FindNextFileW(hFind, &fdata) != 0);
    }
    FindClose(hFind);
  }
  
#else
  
  DIR * dir = opendir (folder.c_str());
  if (dir) {
    dirent * direntry;
    while ((direntry = readdir (dir)) != nullptr) {
      const std::string name = direntry->d_name;
      // Exclude short-hand directory names.
      if (name == ".") continue;
      if (name == "..") continue;
      if (!include_hidden) {
        // Exclude developer temporal files.
        if (name == ".deps") continue;
        if (name == ".dirstamp") continue;
        // Exclude macOS files.
        if (name == ".DS_Store") continue;
      }
      // Store the name.
      files.push_back (name);
    }
    closedir (dir);
  }
  sort (files.begin(), files.end());
  
#endif
  
  // Remove . and ..
  files = filter::string::array_diff (files, {".", ".."});
  
  return files;
}


// Gets the base URL of current Bibledit installation.
std::string get_base_url (const Webserver_Request& webserver_request)
{
  const bool secure {webserver_request.secure || config_globals_enforce_https_browser};
  const std::string scheme {secure ? "https" : "http"};
  const std::string port {secure ? config::logic::https_network_port () : config::logic::http_network_port ()};
  const std::string url {scheme + "://" + webserver_request.host + ":" + port + "/"};
  return url;
}


// This function redirects the browser to "path".
// The "path" is an absolute value.
void redirect_browser (Webserver_Request& webserver_request, std::string path)
{
  // A location header should contain an absolute url, like http://localhost/some/path.
  // See 14.30 in the specification https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html.
  
  // The absolute location contains the user-facing URL, when the administrator entered it.
  // This is needed in case of a proxy server,
  // where Bibledit may not be able to obtain the user-facing URL of the website.
  std::string location = config::logic::site_url (webserver_request);
  
  // If the request was secure, or supposed to be secure,
  // ensure the location contains https rather than plain http,
  // plus the correct secure port.
  if (webserver_request.secure || config_globals_enforce_https_browser) {
    location = filter::string::replace ("http:", "https:", location);
    std::string plainport = config::logic::http_network_port ();
    std::string secureport = config::logic::https_network_port ();
    location = filter::string::replace (":" + plainport, ":" + secureport, location);
  }
  
  location.append (path);

  // If the page contains the topbar suppressing query,
  // the same query will be appended to the URL of the redirected page.
  if (webserver_request.query.count ("topbar") || webserver_request.post_count("topbar")) {
    location = filter_url_build_http_query(location, {{"topbar", "0"}});
  }

  webserver_request.header = "Location: " + location;
  webserver_request.response_code = 302;
}


// C++ equivalent for the dirname function, see https://linux.die.net/man/3/dirname.
// The BSD dirname is not thread-safe, see the implementation notes on $ man 3 dirname,
// therefore it is not used here.
// It uses the defined slash as the separator.
// The std::filesystem could be used, but then the behaviour changes, so that is not done.
std::string filter_url_dirname (std::string url)
#ifdef USE_STD_FILESYSTEM
{
  // Remove trailing slash if there.
  if (!url.empty ()) {
    if (url.find_last_of (std::filesystem::path::preferred_separator) == url.length () - 1) {
      url = url.substr (0, url.length () - 1);
    }
  }
  // Not using the standard library call for getting parent path because of different behaviour.
  const size_t pos = url.find_last_of (std::filesystem::path::preferred_separator);
  if (pos != std::string::npos)
    url = url.substr (0, pos);
  else
    url.clear();
  // The . is important in a few cases rather than an empty string.
  if (url.empty ())
    url = ".";
  // Done.
  return url;
}
#else
{
  if (!url.empty ()) {
    if (url.find_last_of (DIRECTORY_SEPARATOR) == url.length () - 1) {
      // Remove trailing slash.
      url = url.substr (0, url.length () - 1);
    }
    const size_t pos = url.find_last_of (DIRECTORY_SEPARATOR);
    if (pos != std::string::npos)
      url = url.substr (0, pos);
    else 
      url.clear();
  }
  if (url.empty ()) 
    url = ".";
  return url;
}
#endif


// Dirname routine for the web.
// It uses the forward slash as the separator.
std::string filter_url_dirname_web (std::string url)
{
  constexpr const auto separator {"/"};
  if (!url.empty ()) {
    // Remove trailing slash.
    if (url.find_last_of (separator) == url.length () - 1) {
      url = url.substr (0, url.length () - 1);
    }
    // Get dirname or empty string.
    const size_t pos = url.find_last_of (separator);
    if (pos != std::string::npos)
      url = url.substr (0, pos);
    else 
      url.clear();
  }
  // Done.
  return url;
}


// Basename routine for the operating system.
// It uses the defined slash as the separator.
std::string filter_url_basename (std::string url)
#ifdef USE_STD_FILESYSTEM
{
  // Remove possible trailing path slash.
  if (!url.empty ()) {
    if (url.find_last_of (std::filesystem::path::preferred_separator) == url.length () - 1) {
      url = url.substr (0, url.length () - 1);
    }
  }
  // Standard library call for getting base name path.
  url = std::filesystem::path(url).filename().string();
  // Done
  return url;
}
#else
{
  if (!url.empty ()) {
    if (url.find_last_of (DIRECTORY_SEPARATOR) == url.length () - 1) {
      // Remove trailing slash.
      url = url.substr (0, url.length () - 1);
    }
    size_t pos = url.find_last_of (DIRECTORY_SEPARATOR);
    if (pos != std::string::npos) url = url.substr (pos + 1);
  }
  return url;
}
#endif


// Basename routine for the web.
// It uses the forward slash as the separator.
std::string filter_url_basename_web (std::string url)
{
  if (!url.empty ()) {
    // Remove trailing slash.
    constexpr const auto separator {"/"};
    if (url.find_last_of (separator) == url.length () - 1) {
      url = url.substr (0, url.length () - 1);
    }
    // Keep last element: the base name.
    size_t pos = url.find_last_of (separator);
    if (pos != std::string::npos) url = url.substr (pos + 1);
  }
  // Done.
  return url;
}


void filter_url_unlink (const std::string& filename)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::path path (filename);
    std::filesystem::remove (path);
  } catch (...) { }
}
#else
{
  unlink (filename.c_str ());
}
#endif


void filter_url_rename (const std::string& oldfilename, const std::string& newfilename)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::path oldpath (oldfilename);
    std::filesystem::path newpath (newfilename);
    std::filesystem::rename(oldpath, newpath);
  } catch (...) { }
}
#else
{
  rename (oldfilename.c_str (), newfilename.c_str ());
}
#endif


// Creates a file path out of the components.
std::string filter_url_create_path (const std::vector <std::string>& parts)
#ifdef USE_STD_FILESYSTEM
{
  // Empty path.
  std::filesystem::path path;
  for (size_t i = 0; i < parts.size(); i++) {
    if (i == 0) path += parts.at(i); // Append the part without directory separator.
    else path /= parts.at(i); // Append the directory separator and then the part.
  }
  // Done.
  return path.string();
}
#else
{
  // Empty path.
  std::string path;
  for (size_t i = 0; i < parts.size(); i++) {
    // Initially append the first part without directory separator.
    if (i == 0) path += parts[i];
    else {
      // Other parts: Append the directory separator and then the part.
      path += DIRECTORY_SEPARATOR;
      path += parts[i];
    }
  }
  // Done.
  return path;
}
#endif


// Creates a web path out of the components.
std::string filter_url_create_path_web (const std::vector <std::string>& parts)
{
  // Empty path.
  std::string path;
  for (size_t i = 0; i < parts.size(); i++) {
    // Initially append the first part without directory separator.
    if (i == 0)
      path.append(parts.at(i));
    else {
      // Other parts: Append the web directory separator and then the part.
      path.append("/");
      path.append(parts.at(i));
    }
  }
  // Done.
  return path;
}


// Creates a file path out of the variable list of components,
// relative to the server's document root.
std::string filter_url_create_root_path (const std::vector <std::string>& parts)
#ifdef USE_STD_FILESYSTEM
{
  // Construct a path from the document root.
  std::filesystem::path path (config_globals_document_root);
  // Add the bits.
  for (size_t i = 0; i < parts.size(); i++) {
    std::string part = parts[i];
    // At times a path is created from a URL.
    // The URL likely starts with a slash, like this: /css/mouse.css
    // When creating a path out of that, the path will become this: /css/mouse.css
    // Such a path does not exist.
    // The path that is wanted is something like this:
    // /home/foo/bar/bibledit/css/mouse.css
    // So remove that starting slash.
    if (!part.empty()) if (part[0] == '/') part = part.erase(0, 1);
    // Add the part, with a preceding path separator.
    path /= part;
  }
  // Done.
  return path.string();
}
#else
{
  // Construct path from the document root.
  std::string path (config_globals_document_root);
  // Add the bits.
  for (size_t i = 0; i < parts.size(); i++) {
    std::string part = parts[i];
    // At times a path is created from a URL.
    // The URL likely starts with a slash, like this: /css/mouse.css
    // When creating a path out of that, the path will become this: /css/mouse.css
    // Such a path does not exist.
    // The path that is wanted is something like this:
    // /home/foo/bar/bibledit/css/mouse.css
    // So remove that starting slash.
    if (!part.empty()) if (part[0] == '/') part = part.erase(0, 1);
    // Add the part, with a preceding path separator.
    path += DIRECTORY_SEPARATOR;
    path += part;
  }
  // Done.
  return path;
}
#endif


// Gets the file / url extension, e.g. /home/joe/file.txt returns "txt".
std::string filter_url_get_extension (const std::string& url)
#ifdef USE_STD_FILESYSTEM
{
  std::filesystem::path path (url);
  std::string extension;
  if (path.has_extension()) {
    // Get the extension with the dot, e.g. ".txt".
    extension = path.extension().string();
    // Wanted is the extension without the dot, e.g. "txt".
    extension.erase (0, 1);
  }
  return extension;
}
#else
{
  std::string extension;
  size_t pos = url.find_last_of (".");
  if (pos != std::string::npos) {
    extension = url.substr (pos + 1);
  }
  return extension;
}
#endif


// Returns true if the file at $url exists.
bool file_or_dir_exists (const std::string& url)
#ifdef USE_STD_FILESYSTEM
{
  std::filesystem::path path (url);
  return std::filesystem::exists (path);
}
#else
{
  // The 'stat' function works as expected on Linux.
  struct stat buffer;
  return (stat (url.c_str(), &buffer) == 0);
}
#endif


// Makes a directory.
// Creates parents where needed.
void filter_url_mkdir (std::string directory)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::path path (directory);
    std::filesystem::create_directories(path);
  } catch (...) { }
}
#else
{
  const int status = mkdir (directory.c_str(), 0777);
  if (status != 0) {
    std::vector <std::string> paths;
    paths.push_back (directory);
    directory = filter_url_dirname (directory);
    while (directory.length () > 2) {
      paths.push_back (directory);
      directory = filter_url_dirname (directory);
    }
    reverse (paths.begin (), paths.end ());
    for (unsigned int i = 0; i < paths.size (); i++) {
      mkdir (paths[i].c_str (), 0777);
    }
  }
}
#endif


// Removes directory recursively.
void filter_url_rmdir (const std::string& directory)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::path path (directory);
    std::filesystem::remove_all(path);
  } 
  catch (const std::exception& exception)
  {
    Database_Logs::log(exception.what());
  }
}
#else
{
  // List the files in this directory, include the hidden files.
  // Reason for including hidden files: https://github.com/bibledit/cloud/issues/1002
  std::vector <std::string> files = filter_url_scandir_internal (directory, true);
  for (auto& path : files) {
    path = filter_url_create_path ({directory, path});
    if (filter_url_is_dir(path)) {
      filter_url_rmdir(path);
    }
    // On Linux remove the directory or the file.
    remove(path.c_str());
  }
  remove(directory.c_str());
}
#endif


// Returns true is $path points to a directory.
bool filter_url_is_dir (const std::string& path)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::path p (path);
    return std::filesystem::is_directory(p);
  } catch (...) { }
  return false;
}
#else
{
  struct stat sb;
  stat (path.c_str (), &sb);
  return (sb.st_mode & S_IFMT) == S_IFDIR;
}
#endif


bool filter_url_get_write_permission (const std::string& path)
// It would be good if std::filesystem could be used to check on write permissions.
// But currently the std::filesystem does not have this facility.
// The only known option is to use the "access" call.
{
#ifdef HAVE_WINDOWS
  std::wstring wpath = filter::string::string2wstring (path);
  int result = _waccess (wpath.c_str (), 06);
#else
  int result = access (path.c_str(), W_OK);
#endif
  return (result == 0);
}


void filter_url_set_write_permission (const std::string& path)
#ifdef USE_STD_FILESYSTEM
{
  std::filesystem::path p (path);
  std::filesystem::permissions(p, std::filesystem::perms::owner_all | std::filesystem::perms::group_all | std::filesystem::perms::others_all);
}
#else
{
  chmod (path.c_str (), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
}
#endif


// Get and returns the contents of $filename.
std::string filter_url_file_get_contents(const std::string& filename)
{
  if (!file_or_dir_exists (filename)) return std::string();
  try {
#ifdef HAVE_WINDOWS
    std::wstring wfilename = filter::string::string2wstring(filename);
    std::ifstream ifs(wfilename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
#else
    std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
#endif
    std::streamoff filesize = ifs.tellg();
    if (filesize == 0) return std::string();
    ifs.seekg(0, std::ios::beg);
    std::vector <char> bytes(static_cast<size_t> (filesize));
    ifs.read(&bytes[0], static_cast<int> (filesize));
    return std::string(&bytes[0], static_cast<size_t> (filesize));
  }
  catch (...) {
    return std::string();
  }
}


// Puts the $contents into $filename.
void filter_url_file_put_contents (const std::string& filename, const std::string& contents)
{
  try {
    std::ofstream file;
#ifdef HAVE_WINDOWS
    std::wstring wfilename = filter::string::string2wstring(filename);
    file.open(wfilename, std::ios::binary | std::ios::trunc);
#else
    file.open(filename, std::ios::binary | std::ios::trunc);
#endif
    file << contents;
    file.close ();
  } catch (...) {
  }
}


// C++ rough equivalent for PHP's file_put_contents.
// Appends the data if the file exists.
void filter_url_file_put_contents_append (const std::string& filename, const std::string& contents)
{
  try {
    std::ofstream file;
#ifdef HAVE_WINDOWS
    std::wstring wfilename = filter::string::string2wstring (filename);
    file.open (wfilename, std::ios::binary | std::ios::app);
#else
    file.open (filename, std::ios::binary | std::ios::app);
#endif
    file << contents;
    file.close ();
  } catch (...) {
  }
}


// Copies the contents of file named "input" to file named "output".
// It is assumed that the folder where "output" will reside exists.
bool filter_url_file_cp (const std::string& input, const std::string& output)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::copy(input, output, std::filesystem::copy_options::overwrite_existing);
  }
  catch (const std::exception& exception) {
    Database_Logs::log (exception.what());
    return false;
  }
  return true;
}
#else
{
  try {
    std::ifstream source (input, std::ios::binary);
    std::ofstream dest (output, std::ios::binary | std::ios::trunc);
    dest << source.rdbuf();
    source.close();
    dest.close();
  } catch (...) {
    return false;
  }
  return true;
}
#endif


// Copies the entire directory $input to a directory named $output.
// It will recursively copy the inner directories also.
void filter_url_dir_cp (const std::string& input, const std::string& output)
{
  // Create the output directory.
  filter_url_mkdir (output);
  // Check on all files in the input directory.
  std::vector <std::string> files = filter_url_scandir (input);
  for (auto & file : files) {
    std::string input_path = filter_url_create_path ({input, file});
    std::string output_path = filter_url_create_path ({output, file});
    if (filter_url_is_dir (input_path)) {
      // Create output directory.
      filter_url_mkdir (output_path);
      // Handle the new input directory.
      filter_url_dir_cp (input_path, output_path);
    } else {
      // Copy input file to output.
      filter_url_file_cp (input_path, output_path);
    }
  }
}


// Get the file's size in bytes.
int filter_url_filesize (const std::string& filename)
#ifdef USE_STD_FILESYSTEM
{
  try {
    std::filesystem::path p (filename);
    return std::filesystem::file_size(p);
  } catch (...) { }
  return 0;
}
#else
{
  struct stat buf;
  const int rc = stat (filename.c_str (), &buf);
  return rc == 0 ? static_cast<int> (buf.st_size) : 0;
}
#endif


// Scans the directory for files it contains.
std::vector <std::string> filter_url_scandir (const std::string& folder)
#ifdef USE_STD_FILESYSTEM
{
  std::vector <std::string> files;
  try {
    std::filesystem::path dir_path (folder);
    for (const auto& directory_entry : std::filesystem::directory_iterator {dir_path})
    {
      // The full path.
      std::filesystem::path entry_path = directory_entry.path();
      // Get the path as relative to the directory.
      std::filesystem::path relative_path = std::filesystem::relative(entry_path, dir_path);
      // Get the name of the relative path.
      const std::string name = relative_path.string();
      // Exclude developer temporal files.
      if (name == ".deps") continue;
      if (name == ".dirstamp") continue;
      // Exclude macOS files.
      if (name == ".DS_Store") continue;
      // Exclude non-interesting files.
      if (name == "gitflag") continue;
      // Store the name.
      files.push_back (name);
    }
  } catch (...) { }
  sort (files.begin(), files.end());
  return files;
}
#else
{
  std::vector <std::string> files = filter_url_scandir_internal (folder);
  files = filter::string::array_diff (files, {"gitflag"});
  return files;
}
#endif


// Recursively scans a directory for directories and files.
void filter_url_recursive_scandir (const std::string& folder, std::vector <std::string> & paths)
{
  std::vector <std::string> files = filter_url_scandir (folder);
  for (const auto& file : files) {
    const std::string path = filter_url_create_path ({folder, file});
    paths.push_back (path);
    if (filter_url_is_dir (path)) {
      filter_url_recursive_scandir (path, paths);
    }
  }
}


// Get the file modification time.
int filter_url_file_modification_time (std::string filename)
#ifdef USE_STD_FILESYSTEM
{
  try {
    const std::filesystem::path path (filename);
    const std::filesystem::file_time_type ftime = std::filesystem::last_write_time(path);
    const int seconds = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();
    return seconds;
  } catch (...) { }
  return 0;
}
#else
{
  struct stat attributes;
  stat (filename.c_str (), &attributes);
  return static_cast<int> (attributes.st_mtime);
}
#endif


// A C++ near equivalent for PHP's urldecode function.
std::string filter_url_urldecode (std::string url)
{
  url = UriDecode (url);
  std::replace (url.begin (), url.end (), '+', ' ');
  return url;
}


// A C++ near equivalent for PHP's urlencode function.
std::string filter_url_urlencode (std::string url)
{
  url = UriEncode (url);
  return url;
}


// Returns the name of the temp directory.
const char * filter_url_temp_dir ()
{
  return "tmp";
}


// Returns the name of a temporary file.
std::string filter_url_tempfile (const char * directory)
{
  std::string filename = std::to_string (filter::date::seconds_since_epoch ()) + std::to_string (filter::date::numerical_microseconds ()) + std::to_string (filter::string::rand (10000000, 99999999));
  if (directory) {
    filename = filter_url_create_path ({directory, filename});
  } else {
    filename = filter_url_create_root_path ({filter_url_temp_dir (), filename});
  }
  return filename;
}


// C++ equivalent for PHP's escapeshellarg function.
std::string filter_url_escape_shell_argument (std::string argument)
{
  argument = filter::string::replace ("'", "\\'", argument);
  argument.insert (0, "'");
  argument.append ("'");
  return argument;
}


// The function accepts a $path.
// The function may add a numerical suffix 
// to ensure that the $path does not yet exist in the filesystem.
std::string filter_url_unique_path (std::string path)
{
  if (!file_or_dir_exists (path)) return path;
  for (size_t i = 1; i < 100; i++) {
    std::string uniquepath = path + "." + std::to_string (i);
    if (!file_or_dir_exists (uniquepath)) return uniquepath;
  }
  return path + "." + std::to_string (filter::string::rand (100, 1000));
}


// Returns true if the email address is valid.
bool filter_url_email_is_valid (std::string email)
{
  const std::string valid_set ("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._-");
  // The @ character should appear only once.
  std::vector <std::string> atbits = filter::string::explode (email, '@');
  if (atbits.size() != 2) return false;
  // The characters on the left of @ should be from the valid set.
  std::string left = atbits [0];
  for (unsigned int i = 0; i < left.size(); i++) {
    char c = left [i];
    if (valid_set.find (c) == std::string::npos) return false;
  }
  // The characters on the right of @ should be from the valid set.
  std::string right = atbits [1];
  for (unsigned int i = 0; i < right.size(); i++) {
    char c = right [i];
    if (valid_set.find (c) == std::string::npos) return false;
  }
  // The character . should appear at least once to the right of @.
  std::vector <std::string> dotbits = filter::string::explode (right, '.');
  if (dotbits.size () < 2) return false;
  // The email address is valid.
  return true;
}


std::string filter_url_build_http_query (std::string url, const std::vector<std::pair<std::string,std::string>> parameters_values)
{
  for (const auto& [parameter, value] : parameters_values) {
    url.append ((url.find ("?") == std::string::npos) ? "?" : "&");
    url.append (parameter);
    url.append ("=");
    url.append (value);
  }
  return url;
}


static size_t filter_url_curl_write_function (void *ptr, size_t size, size_t count, void *stream)
{
  static_cast<std::string *>(stream)->append (static_cast<char *>(ptr), 0, size * count);
  return size * count;
}


// Sends a http GET request to the $url.
// It returns the response from the server.
// It writes any error to $error.
std::string filter_url_http_get (std::string url, std::string& error, [[maybe_unused]] bool check_certificate)
{
  std::string response;
#ifdef HAVE_CLIENT
  response = filter_url_http_request_mbed (url, error, {}, "", check_certificate);
#else
  CURL *curl = curl_easy_init ();
  if (curl) {
    curl_easy_setopt (curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, filter_url_curl_write_function);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
    //curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
    // Because a Bibledit client should work even over very bad networks,
    // pass some timeout options to curl so it properly deals with such networks.
    filter_url_curl_set_timeout (curl);
    // Some websites may prevent simple scrapers from getting their content.
    // If the request comes from a real browser, they may accept the request and give an appropriate response.
    // Here is how to mimic a request coming from a real browser:
    // https://stackoverflow.com/questions/28760694/how-to-use-curl-to-get-a-get-request-exactly-same-as-using-chrome
    // The code below mimics the Chrome browser in October 2024.
    curl_slist* extra_headers {nullptr};
    extra_headers = curl_slist_append (extra_headers, "accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    extra_headers = curl_slist_append (extra_headers, "accept-language: en-US,en;q=0.9");
    extra_headers = curl_slist_append (extra_headers, "cache-control: max-age=0");
    extra_headers = curl_slist_append (extra_headers, "cookie: PHPSESSID=9jin0fmqnb03np12o8v5dv4jf1");
    extra_headers = curl_slist_append (extra_headers, "priority: u=0, i");
    extra_headers = curl_slist_append (extra_headers, R"(sec-ch-ua: "Google Chrome";v="129", "Not=A?Brand";v="8", "Chromium";v="129")");
    extra_headers = curl_slist_append (extra_headers, R"(sec-ch-ua-mobile: ?0)");
    extra_headers = curl_slist_append (extra_headers, R"(sec-ch-ua-platform: "macOS")");
    extra_headers = curl_slist_append (extra_headers, R"(sec-fetch-dest: document)");
    extra_headers = curl_slist_append (extra_headers, R"(sec-fetch-mode: navigate)");
    extra_headers = curl_slist_append (extra_headers, R"(sec-fetch-site: none)");
    extra_headers = curl_slist_append (extra_headers, R"(sec-fetch-user: ?1)");
    extra_headers = curl_slist_append (extra_headers, R"(upgrade-insecure-requests: 1)");
    extra_headers = curl_slist_append (extra_headers, R"(user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/129.0.0.0 Safari/537.36)");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, extra_headers);
    CURLcode res = curl_easy_perform (curl);
    if (res == CURLE_OK) {
      error.clear ();
      long http_code = 0;
      curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
      if (http_code != 200) {
        response.append ("http code " + std::to_string (http_code));
      }
    } else {
      response.clear ();
      error = curl_easy_strerror (res);
    }
    curl_slist_free_all (extra_headers);
    curl_easy_cleanup (curl);
  }
#endif
  return response;
}


// The debug function for libcurl, it dumps the data as specified.
static void filter_url_curl_debug_dump (const char *text, FILE *stream, unsigned char *ptr, size_t size)
{
  size_t i;
  size_t c;
  unsigned int width = 0x10;
  
  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, static_cast<long> (size), static_cast<long> (size));
  
  for (i = 0; i < size; i += width) {
    fprintf(stream, "%4.4lx: ", static_cast<long> (i));
    
    // Show hex to the left.
    for (c = 0; c < width; c++) {
      if (i + c < size) fprintf (stream, "%02x ", ptr[i + c]);
      else fputs("   ", stream);
    }
    
    // Show data on the right.
    for (c = 0; (c < width) && (i + c < size); c++) {
      unsigned char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
      fputc (x, stream);
    }

    // Newline.
    fputc ('\n', stream);
  }
}


// The trace function for libcurl.
#ifdef HAVE_CLOUD
[[maybe_unused]]
static int filter_url_curl_trace (CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
  const char* text { nullptr };

  // Prevent compiler warnings.
  (void)handle;
  (void)userp;
  
  switch (type) {
    case CURLINFO_TEXT:
      fprintf(stderr, "== Info: %s", data);
      return 0;
    case CURLINFO_HEADER_OUT:
      text = "=> Send header";
      break;
    case CURLINFO_DATA_OUT:
      text = "=> Send data";
      break;
    case CURLINFO_SSL_DATA_OUT:
      text = "=> Send SSL data";
      break;
    case CURLINFO_HEADER_IN:
      text = "<= Recv header";
      break;
    case CURLINFO_DATA_IN:
      text = "<= Recv data";
      break;
    case CURLINFO_SSL_DATA_IN:
      text = "<= Recv SSL data";
      break;
    case CURLINFO_END:
    default: 
      return 0;
  }
  
  filter_url_curl_debug_dump(text, stderr, reinterpret_cast<unsigned char *> (data), size);
  return 0;
}
#endif


// Sends a http POST request to $url.
// burst: Set connection timing for burst mode, where the response comes after a relatively long silence.
// It posts the $post_data as-t.
// It appends the $values to the post data.
// It returns the response from the server.
// It writes any error to $error.
std::string filter_url_http_post (const std::string& url, [[maybe_unused]] std::string post_data, const std::map <std::string, std::string> & post_values, std::string& error, [[maybe_unused]] bool burst, [[maybe_unused]] bool check_certificate, [[maybe_unused]] const std::vector <std::pair <std::string, std::string> > & headers)
{
  std::string response;
#ifdef HAVE_CLIENT
  response = filter_url_http_request_mbed (url, error, post_values, "", check_certificate);
#else
  // Get a curl handle.
  CURL *curl = curl_easy_init ();
  if (curl) {
    // First set the URL that is about to receive the POST.
    // This can be http or https.
    curl_easy_setopt (curl, CURLOPT_URL, url.c_str());
    // Generate the post data, add it to the plain post data.
    for (auto & element : post_values) {
      if (!post_data.empty ()) post_data.append ("&");
      post_data.append (element.first);
      post_data.append ("=");
      post_data.append (filter_url_urlencode (element.second));
    }
    // Specify the POST data to curl, e.g.: "name=foo&project=bar"
    curl_easy_setopt (curl, CURLOPT_POSTFIELDS, post_data.c_str());
    // Callback for the server response.
    curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, filter_url_curl_write_function);
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, &response);
    // Further options.
    curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
    // Enable the trace function.
    // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, filter_url_curl_trace);
    // The DEBUGFUNCTION has no effect until we enable VERBOSE.
    // curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
    // Timeouts for very bad networks, see the GET routine above for an explanation.
    filter_url_curl_set_timeout (curl, burst);
    // Whether to check the secure certificate.
    // If the configuration of the site is not right, the certificate cannot be verified.
    // That would result in resources not being fetched anymore.
    if (!check_certificate) curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    // Optional extra headers.
    curl_slist *list {nullptr};
    for (auto header : headers) {
      std::string line = header.first + ": " + header.second;
      list = curl_slist_append (list, line.c_str ());
    }
    if (list) {
      curl_easy_setopt (curl, CURLOPT_HTTPHEADER, list);
    }
    // Perform the request.
    CURLcode res = curl_easy_perform (curl);
    // Result check.
    if (res == CURLE_OK) {
      error.clear ();
      long http_code = 0;
      curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
      if (http_code != 200) {
        error.append ("Server response " + filter_url_http_response_code_text (static_cast<int>(http_code)));
      }
    } else {
      error = curl_easy_strerror (res);
    }
    // Always cleanup.
    if (list) curl_slist_free_all (list);
    curl_easy_cleanup (curl);
  }
#endif
  return response;
}


std::string filter_url_http_response_code_text (int code)
{
  std::string text = std::to_string (code);
  text.append (" ");
  switch (code) {
    case 100: text += "Continue"; break;
    case 101: text += "Switching Protocols"; break;
    case 200: text += "OK"; break;
    case 201: text += "Created"; break;
    case 202: text += "Accepted"; break;
    case 203: text += "Non-Authoritative Information"; break;
    case 204: text += "No Content"; break;
    case 205: text += "Reset Content"; break;
    case 206: text += "Partial Content"; break;
    case 300: text += "Multiple Choices"; break;
    case 301: text += "Moved Permanently"; break;
    case 302: text += "Found"; break;
    case 303: text += "See Other"; break;
    case 304: text += "Not Modified"; break;
    case 305: text += "Use Proxy"; break;
    case 307: text += "Temporary Redirect"; break;
    case 308: text += "Permanent Redirect"; break;
    case 400: text += "Bad Request"; break;
    case 401: text += "Unauthorized"; break;
    case 402: text += "Payment Required"; break;
    case 403: text += "Forbidden"; break;
    case 404: text += "Not Found"; break;
    case 405: text += "Method Not Allowed"; break;
    case 406: text += "Not Acceptable"; break;
    case 407: text += "Proxy Authentication Required"; break;
    case 408: text += "Request Timeout"; break;
    case 409: text += "Conflict"; break;
    case 410: text += "Gone"; break;
    case 411: text += "Length Required"; break;
    case 412: text += "Precondition Failed"; break;
    case 413: text += "Request Entity Too Large"; break;
    case 414: text += "Request-URI Too Long"; break;
    case 415: text += "Unsupported Media Type"; break;
    case 416: text += "Requested Range Not Satisfiable"; break;
    case 417: text += "Expectation Failed"; break;
    case 426: text += "Upgrade Required"; break;
    case 428: text += "Precondition Required"; break;
    case 429: text += "Too Many Requests"; break;
    case 431: text += "Request Header Fields Too Large"; break;
    case 500: text += "Internal Server Error"; break;
    case 501: text += "Not Implemented"; break;
    case 502: text += "Bad Gateway"; break;
    case 503: text += "Service Unavailable"; break;
    case 504: text += "Gateway Timeout"; break;
    case 505: text += "HTTP Version Not Supported"; break;
    case 511: text += "Network Authentication Required"; break;
    default:  text += "Error"; break;
  }
  return text;
}


// Downloads the file at $url, and stores it at $filename.
void filter_url_download_file (std::string url, std::string filename, std::string& error,
                               [[maybe_unused]] bool check_certificate)
{
#ifdef HAVE_CLIENT
  filter_url_http_request_mbed (url, error, {}, filename, check_certificate);
#else
  CURL *curl = curl_easy_init ();
  if (curl) {
    curl_easy_setopt (curl, CURLOPT_URL, url.c_str());
    FILE* file = fopen (filename.c_str(), "w");
    curl_easy_setopt (curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
    // curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
    filter_url_curl_set_timeout (curl);
    CURLcode res = curl_easy_perform (curl);
    if (res == CURLE_OK) {
      error.clear ();
      long http_code = 0;
      curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
      if (http_code != 200) {
        error.append ("http code " + std::to_string (http_code));
      }
    } else {
      error = curl_easy_strerror (res);
    }
    curl_easy_cleanup (curl);
    fclose (file);
  }
#endif
}


/*
 * The function returns the filename for a html file for a Bible.
 * $path    - The path where to store the files.
 * $book    - The book identifier.
 * $chapter - The chapter number.
 */
std::string filter_url_html_file_name_bible (std::string path, int book, int chapter)
{
  std::string filename;
  
  // If a path is given, prefix it.
  if (path != "") {
    filename = path + "/";
  }
  
  // No book ID given: Return the name for the index file for the Bible.
  if (book == 0) {
    filename += "index.html";
    return filename;
  }
  
  // Add the name for the book. No spaces.
  filename += filter::string::fill (std::to_string (book), 2, '0');
  std::string sbook = database::books::get_english_from_id (static_cast<book_id>(book));
  sbook = filter::string::replace (" ", "", sbook);
  filename += '-' + sbook;
  
  // Chapter given: Provide name for the chaper.
  if (chapter >= 0) {
    filename += '-' + filter::string::fill (std::to_string (chapter), 3, '0');
  }
  
  filename += ".html";
  
  return filename;
}


// Callback function for logging cURL debug information.
#ifdef HAVE_CLIENT
#else
int filter_url_curl_debug_callback (void *curl_handle, int curl_info_type, char *data, size_t size, void *userptr)
{
  if (curl_handle && userptr) {};
  bool log = true;
  curl_infotype type = static_cast<curl_infotype>(curl_info_type);
  if (type == CURLINFO_SSL_DATA_OUT) log = false;
  if (type == CURLINFO_SSL_DATA_OUT) log = false;
  if (log) {
    std::string message (data, size);
    Database_Logs::log (message);
  }
  return 0;
}
#endif


// Sets timeouts for cURL operations.
// burst: When true, the server gives a burst response, that is, all data arrives at once after a delay.
//        When false, the data is supposed to be downloaded gradually.
// Without these timeouts, the Bibledit client will hang on stalled sync operations.
#ifdef HAVE_CLIENT
#else
void filter_url_curl_set_timeout (void *curl_handle, bool burst)
{
  CURL * handle = curl_handle;
  
  // There is a timeout on establishing a connection.
  curl_easy_setopt (handle, CURLOPT_CONNECTTIMEOUT, 10);
  
  // There is a also a transfer timeout for normal speeds.
  curl_easy_setopt (handle, CURLOPT_TIMEOUT, 600);
  
  // There is also a shorter transfer timeout for low speeds,
  // because low speeds indicate a stalled connection.
  // But when the server needs to do a lot of processing and then sends then response at once,
  // the low speed timeouts should be disabled,
  // else it times out before the response has come.
  if (!burst) {
    curl_easy_setopt (handle, CURLOPT_LOW_SPEED_LIMIT, 100);
    curl_easy_setopt (handle, CURLOPT_LOW_SPEED_TIME, 10);
  }
  
  // Timing out may use signals, which is not what we want.
  curl_easy_setopt (handle, CURLOPT_NOSIGNAL, 1L);
}
#endif



// When the client POSTs + sign to the server,
// the + sign is replaced with a space in the process.
// Therefore first convert the + to a TAG before sending it off.
std::string filter_url_plus_to_tag (std::string data)
{
  return filter::string::replace ("+", "PLUSSIGN", data);
}


// When POSTing a + sign via jQuery to the server,
// the + sign is replaced with a space in the process.
// Javascript first converts the + to a TAG before sending it off.
// This function reverts the TAG to the original + sign.
std::string filter_url_tag_to_plus (std::string data)
{
  return filter::string::replace ("PLUSSIGN", "+", data);
}


// This filter removes the username and password components from the $url.
std::string filter_url_remove_username_password (std::string url)
{
  std::string slashes = "//";
  size_t pos = url.find (slashes);

  // Consider the following URL for github:
  // https://username:password@github.com/username/repository.git
  if (filter::string::replace_between (url, slashes, ":", "")) {
    if (pos != std::string::npos) url.insert (pos, slashes);
  }
  if (filter::string::replace_between (url, slashes, "@", "")) {
    if (pos != std::string::npos) url.insert (pos, slashes);
  }
  
  return url;
}


// A very simple function that sends a HTTP GET or POST request and reads the response.
// It handles plain and secure http.
// $url: The URL including host / port / path.
// $error: To store any error messages.
// $post: Value pairs for a POST request.
// $filename: The filename to save the data to.
// $check_certificate: Whether to check the server certificate in case of secure http.
std::string filter_url_http_request_mbed (std::string url, std::string& error, 
                                          const std::map <std::string, std::string>& post,
                                          const std::string& filename, bool check_certificate)
{
  // The "http" scheme is used to locate network resources via the HTTP protocol.
  // url = "http(s):" "//" host [ ":" port ] [ abs_path [ "?" query ]]


  // Whether this is a secure http request.
  const bool secure = url.find ("https:") != std::string::npos;
  
  
  // Remove the scheme: http(s).
  size_t pos = url.find ("://");
  if (pos != std::string::npos) {
    url.erase (0, pos + 3);
  }

  
  // Extract the host.
  pos = url.find (":");
  if (pos == std::string::npos) pos = url.find ("/");
  if (pos == std::string::npos) pos = url.length () + 1;
  std::string hostname = url.substr (0, pos);
  url.erase (0, hostname.length ());

  
  // Default port numbers for plain or secure http.
  int port = 80;
  if (secure) port = 443;
  // Extract the port number if any.
  pos = url.find (":");
  if (pos != std::string::npos) {
    url.erase (0, 1);
    size_t pos2 = url.find ("/");
    if (pos2 == std::string::npos) pos2 = url.length () + 1;
    const std::string p = url.substr (0, pos2);
    port = filter::string::convert_to_int (p);
    url.erase (0, p.length ());
  }
  
  
  // Empty URL results in a slash.
  if (url.empty ()) url = "/";
  
  
  // The absolute path plus optional query remain after extracting the preceding stuff.
  

  bool connection_healthy = true;
  
  
  // SSL/TLS configuration and context.
  // The configuration is local, because options may not be the same for every website.
  // On Windows, threading has been disabled in the mbedTLS library.
  // On the server, this will lead to undefined crashes which are hard to find.
  // On a client, since the TLS context is not shared, there won't be any crashes.
  mbedtls_ssl_context ssl {};
  mbedtls_ssl_config conf {};
  if (secure) {
    mbedtls_ssl_init (&ssl);
    mbedtls_ssl_config_init (&conf);
  }
  
  
  // Resolve the host.
  addrinfo hints;
  addrinfo * address_results {nullptr};
  bool address_info_resolved {false};
  if (!secure) {
    memset (&hints, 0, sizeof (addrinfo));
    // Allow IPv4 and IPv6.
    hints.ai_family = AF_UNSPEC;
    // TCP/IP socket.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    // Select protocol that matches with the socket type.
    hints.ai_protocol = 0;
    // The 'service' is actually the port number.
    const std::string service = std::to_string (port);
    // Get a list of address structures. There can be several of them.
    const int res = getaddrinfo (hostname.c_str(), service.c_str (), &hints, &address_results);
    if (res != 0) {
      error = "Internet connection failure: " + hostname + ": ";
#ifdef HAVE_WINDOWS
      wchar_t * err = gai_strerrorW (res);
      error.append (filter::string::wstring2string (err));
#else
      error.append (gai_strerror (res));
#endif
      connection_healthy = false;
    } else {
      address_info_resolved = true;
    }
  }
  
  
  // Secure connection setup.
  mbedtls_net_context fd {};
  if (secure) {

    // Secure socket setup.
    if (connection_healthy) {
      mbedtls_net_init (&fd);
    }

    // SSL/TLS connection configuration.
    if (connection_healthy) {
      int ret = mbedtls_ssl_config_defaults (&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
      if (ret != 0) {
        filter_url_display_mbed_tls_error (ret, &error, false, std::string());
        connection_healthy = false;
      }
      mbedtls_ssl_conf_authmode (&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
      mbedtls_ssl_conf_ca_chain (&conf, &x509_ca_cert, nullptr);
      mbedtls_ssl_conf_rng (&conf, mbedtls_ctr_drbg_random, &ctr_drbg_context);
      ret = mbedtls_ssl_setup (&ssl, &conf);
      if (ret != 0) {
        filter_url_display_mbed_tls_error (ret, &error, false, std::string());
        connection_healthy = false;
      }
      // The hostname it connects to, and verifies the certificate for.
      ret = mbedtls_ssl_set_hostname (&ssl, hostname.c_str ());
      if (ret != 0) {
        filter_url_display_mbed_tls_error (ret, &error, false, std::string());
        connection_healthy = false;
      }
      mbedtls_ssl_set_bio (&ssl, &fd, mbedtls_net_send, mbedtls_net_recv, nullptr);
    }
    
    // Secure connect to host.
    if (connection_healthy) {
      // It used to pass the "server_port" to the connect routine:
      // const char * server_port = filter::string::convert_to_string (port).c_str ();
      // But MSVC optimized this variable away before it could be passed to that routine.
      // The code was updated to work around that.
      int ret = mbedtls_net_connect (&fd, hostname.c_str(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_TCP);
      if (ret != 0) {
        filter_url_display_mbed_tls_error (ret, &error, false, std::string());
        connection_healthy = false;
      }
    }
  }
  
  
  // Plain connection setup.
  int sock {0};
  if (!secure) {
    
    // Iterate over the list of address structures.
    std::vector <std::string> errors {};
    addrinfo * rp {nullptr};
    if (connection_healthy) {
      for (rp = address_results; rp != nullptr; rp = rp->ai_next) {
        // Try to get a socket for this address structure.
        sock = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        // If it fails, try the next one.
        if (sock < 0) {
          std::string err = "Creating socket: ";
          err.append (strerror (errno));
          errors.push_back (err);
          continue;
        }
        // Try to connect.
        int res = connect (sock, rp->ai_addr, rp->ai_addrlen);
        // Test and record error.
        if (res < 0) {
          std::string err = hostname + ":" + std::to_string (port) + ": ";
          err.append (strerror (errno));
          errors.push_back (err);
        }
        // If success: Done.
        if (res != -1) break;
        // Failure: Socket should be closed.
        if (sock) {
#ifdef HAVE_WINDOWS
          closesocket (sock);
#else
          close (sock);
#endif
        }
        sock = 0;
      }
    }
    
    // Check whether no address succeeded.
    if (connection_healthy) {
      if (rp == nullptr) {
        error = filter::string::implode (errors, " | ");
        connection_healthy = false;
      }
    }
    
    // No longer needed: Only to be freed when the address was resolved.
    if (address_info_resolved) freeaddrinfo (address_results);
  }
  
  
  // A Bibledit client should work even over very bad networks,
  // so set a timeout on the network connection.
  if (connection_healthy) {
    // Socket, whether plain or secure http.
    int comm_sock = sock;
    if (secure) comm_sock = fd.fd;
    // Make the timeout not too short, so it can support very slow networks.
#ifdef HAVE_WINDOWS
    // Windows: Timeout value is a DWORD in milliseconds, address passed to setsockopt() is const char *
    const char * tv = "600000";
#else
    // Linux: Timeout value is a struct timeval, address passed to setsockopt() is const void *
    timeval tv;
    tv.tv_sec = 600;
    tv.tv_usec = 0;
#endif
    // Check on the result of setting the socket options.
    // If it cannot be set, record it in the journal,
    // but still proceed with the connection, because this is not fatal.
    int ret;
#ifdef HAVE_WINDOWS
    ret = setsockopt (comm_sock, SOL_SOCKET, SO_RCVTIMEO, tv, sizeof (tv));
#else
    ret = setsockopt (comm_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
#endif
    if (ret != 0) Database_Logs::log (strerror (errno));
#ifdef HAVE_WINDOWS
    ret = setsockopt (comm_sock, SOL_SOCKET, SO_SNDTIMEO, tv, sizeof (tv));
#else
    ret = setsockopt (comm_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(timeval));
#endif
    if (ret != 0) Database_Logs::log (strerror (errno));
  }
  
  
  // SSL/TLS handshake.
  if (secure) {
    int ret {};
    while (connection_healthy && ((ret = mbedtls_ssl_handshake (&ssl)) != 0)) {
      // The connection requires reading more data.
      if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
      // The connection requires writing more data.
      if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
      // Received NewSessionTicket Post Handshake Message.
      if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) continue;
      // Handle all other error codes.
      filter_url_display_mbed_tls_error (ret, &error, false, std::string());
      connection_healthy = false;
    }
  }
  
  
  // Optionally verify the server certificate.
  if (connection_healthy && secure && check_certificate) {
    uint32_t flags = mbedtls_ssl_get_verify_result (&ssl);
    if (flags != 0) {
      char vrfy_buf [512];
      mbedtls_x509_crt_verify_info (vrfy_buf, sizeof (vrfy_buf), " ! ", flags);
      error = vrfy_buf;
      connection_healthy = false;
    }
  }

  
  // Assemble the data to POST, if any.
  std::string postdata;
  for (const auto& element : post) {
    if (!postdata.empty ()) postdata.append ("&");
    postdata.append (element.first);
    postdata.append ("=");
    postdata.append (filter_url_urlencode (element.second));
  }
  
  
  // Send the request.
  if (connection_healthy) {
    std::string request = "GET";
    if (!post.empty ()) request = "POST";
    request.append (" ");
    request.append (url);
    request.append (" ");
    request.append ("HTTP/1.1");
    request.append ("\r\n");
    request.append ("Host: ");
    request.append (hostname);
    request.append ("\r\n");
    // Close connection, else it's harder to locate the end of the response.
    request.append ("Connection: close");
    request.append ("\r\n");
    if (!post.empty ()) {
      request.append ("Content-Type: application/x-www-form-urlencoded");
      request.append ("\r\n");
      request.append ("Content-Length: " + std::to_string (postdata.length()));
      request.append ("\r\n");
    }
    request.append ("\r\n");
    request.append (postdata);
    if (secure) {
      
      // Write the secure http request to the server.
      const char * output = request.c_str();
      const unsigned char * buf = reinterpret_cast<const unsigned char *>(output);
      // The C function strlen () fails on null characters in the request, so take string::size()
      size_t len = request.size ();
      while (connection_healthy && (len > 0)) {
        // Function
        // int ret = mbedtls_ssl_write (&ssl, buf, len)
        // will do partial writes in some cases.
        // If the return value is non-negative but less than length,
        // the function must be called again with updated arguments:
        // buf + ret, len - ret
        // until it returns a value equal to the last 'len' argument.
        int ret = mbedtls_ssl_write (&ssl, buf, len);
        if (ret > 0) {
          buf += ret;
          len -= static_cast<size_t>(ret);
        } else {
          // When it returns MBEDTLS_ERR_SSL_WANT_WRITE/READ,
          // it must be called later with the *same* arguments,
          // until it returns a positive value.
          if (ret == MBEDTLS_ERR_SSL_WANT_READ) continue;
          if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
          if (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET) continue;
          filter_url_display_mbed_tls_error (ret, &error, false, std::string());
          connection_healthy = false;
        }
      }
      
    } else {

      // Send plain http.
      if (send (sock, request.c_str(), request.length(), 0) != static_cast<int>(request.length ())) {
        error = "Sending request: ";
        error.append (strerror (errno));
        connection_healthy = false;
      }
      
    }
  }
  
  
  // Read the response headers and body.
  std::string headers;
  std::string response;
  if (connection_healthy) {

    bool reading {true};
    bool reading_body {false};
    char prev {0};
    char cur;
    FILE* file {nullptr};
    if (!filename.empty ()) {
#ifdef HAVE_WINDOWS
      std::wstring wfilename = filter::string::string2wstring (filename);
      file = _wfopen (wfilename.c_str (), L"w");
#else
      file = fopen (filename.c_str (), "w");
#endif
    }

    do {
      int ret {0};
      if (secure) {
        unsigned char buffer [1];
        memset (&buffer, 0, 1);
        ret = mbedtls_ssl_read (&ssl, buffer, 1);
        cur = static_cast<char>(buffer [0]);
      } else {
#ifdef HAVE_WINDOWS
        ret = (int)recv(sock, &cur, 1, 0);
#else
        ret = static_cast<int>(read(sock, &cur, 1));
#endif
      }
      if (ret > 0) {
        if (reading_body) {
          if (file) {
            fwrite (&cur, 1, 1, file);
          }
          else response += cur;
        } else {
          if (cur == '\r') continue;
          headers += cur;
          if ((cur == '\n') && (prev == '\n')) reading_body = true;
          prev = cur;
        }
      } else if (!secure && (ret < 0)) {
        error = "Receiving: ";
        error.append (strerror (errno));
        connection_healthy = false;
      } else if (secure && (ret == MBEDTLS_ERR_SSL_WANT_READ)) {
      } else if (secure && (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
      } else if (secure && (ret == MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET)) {
      } else if (secure && (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)) {
      } else if (secure && (ret < 0)) {
        filter_url_display_mbed_tls_error (ret, &error, false, std::string());
        connection_healthy = false;
      } else {
        // Probably EOF.
        reading = false;
      }
    } while (reading && connection_healthy);

    if (file) fclose (file);
  }
  
  
  if (secure) {
    mbedtls_ssl_close_notify (&ssl);
    mbedtls_net_free (&fd);
    mbedtls_ssl_free (&ssl);
    mbedtls_ssl_config_free (&conf);
  } else {
    // Only close the socket if it was open.
    // It used to close (0) in error,
    // and on Android and iOS, when this was done a couple of times, it would crash the app.
    if (sock > 0) {
#ifdef HAVE_WINDOWS
      closesocket(sock);
#else
      close (sock);
#endif
    }
  }

  
  // Check the response headers.
  std::vector <std::string> lines = filter::string::explode (headers, '\n');
  for (auto & line : lines) {
    if (line.empty ()) continue;
    if (line.find ("HTTP") != std::string::npos) {
      const size_t pos2 = line.find (" ");
      if (pos2 != std::string::npos) {
        line.erase (0, pos2 + 1);
        int response_code = filter::string::convert_to_int (line);
        if (response_code != 200) {
          error = "Response code: " + line;
          return std::string();
        }
      } else {
        error = "Invalid response: " + line;
        return std::string();
      }
    }
  }
  
  
  // Done.
  if (!connection_healthy) response.clear ();
  return response;
}


// Initialize the SSL/TLS system once.
void filter_url_ssl_tls_initialize ()
{
  // No debug output.
  mbedtls_debug_set_threshold(0);

  int ret = 0;

  // Initialize the certificate store.
  mbedtls_x509_crt_init(&x509_ca_cert);

  // Random number generator.
  mbedtls_ctr_drbg_init (&ctr_drbg_context);
  mbedtls_entropy_init (&entropy_context);

  // Initialize the Platform Security Architecture that MbedTLS version 3 introduces.
  psa_status_t status = psa_crypto_init();
  filter_url_display_mbed_tls_error (status, nullptr, false, std::string());

  // Seed the random number generator.
  constexpr const auto pers = "Client";
  ret = mbedtls_ctr_drbg_seed (&ctr_drbg_context, mbedtls_entropy_func, &entropy_context, reinterpret_cast <const unsigned char *> (pers), strlen (pers));
  filter_url_display_mbed_tls_error (ret, nullptr, false, std::string());

  // Wait until the Certificate Authority root certificate exist.
  // This is necessary as there's cases that the data is still being installed at this point.
  std::string path = filter_url_create_root_path ({"filter", "cas.crt"});
  while (!file_or_dir_exists (path)) std::this_thread::sleep_for (std::chrono::milliseconds (100));
  
  // Initialize the certificate store and read the Certificate Authority root certificates.
  ret = mbedtls_x509_crt_parse_file (&x509_ca_cert, path.c_str ());
  filter_url_display_mbed_tls_error (ret, nullptr, false, std::string());
}


// Finalize the SSL/TLS system once.
void filter_url_ssl_tls_finalize ()
{
  mbedtls_ctr_drbg_free (&ctr_drbg_context);
  mbedtls_entropy_free (&entropy_context);
  mbedtls_x509_crt_free (&x509_ca_cert);
  mbedtls_psa_crypto_free();
}


// This logs the $ret (return) value, converted to readable text, to the journal.
// If $error is given, it is stored there instead.
// It $server is true, it suppresses additional error codes.
void filter_url_display_mbed_tls_error (int& ret, std::string* error, bool server, const std::string& remote_ip_address)
{
  // Local copy of the return value, and clear the original return value.
  int local_return = ret;
  ret = 0;
  
  // Everything OK
  if (local_return == 0) return;

  // The server suppresses a couple of error messages caused by rogue clients or spiders.
  // The reason for suppressing them is to prevent them from flooding the Journal.
  if (server) {
    // SSL - Processing of the ClientHello handshake message failed (-30976)
    // No longer in MbedTLS 3.x if (local_return == MBEDTLS_ERR_SSL_BAD_HS_CLIENT_HELLO) return;
    // SSL - The connection indicated an EOF (-29312)
    if (local_return == MBEDTLS_ERR_SSL_CONN_EOF) return;
    // NET - Reading information from the socket failed (-76)
    if (local_return == MBEDTLS_ERR_NET_RECV_FAILED) return;
    // NET - Connection was reset by peer (-80)
    if (local_return == MBEDTLS_ERR_NET_CONN_RESET) return;
  }

  // There's an error: Display it.
  char error_buf [100];
  mbedtls_strerror (local_return, error_buf, 100);
  std::string msg = error_buf;
  msg.append (" (");
  msg.append (std::to_string (local_return));
  msg.append (")");
  // Add the remote IP address if available.
  if (!remote_ip_address.empty()) {
    msg.append (" (IP address ");
    msg.append (remote_ip_address);
    msg.append (")");
  }
  if (error) {
    error->assign (msg);
  } else {
    Database_Logs::log (msg);
  }
}


// This takes $url, removes any scheme (http / https) it has,
// then sets the correct scheme based on $secure,
// and returns the URL, e.g. as http://localhost or https://localhost.
std::string filter_url_set_scheme (std::string url, bool secure)
{
  // Remove whitespace.
  url = filter::string::trim (url);
  // Remove amy existing scheme: http(s) or whatever.
  size_t pos = url.find ("://");
  if (pos != std::string::npos) {
    url.erase (0, pos + 3);
  }
  // Produce the correct scheme.
  std::string scheme = "http";
  if (secure) scheme.append ("s");
  scheme.append ("://");
  // Insert the scheme.
  url = scheme + url;
  // Done.
  return url;
}


// Replace invalid characters in Windows filenames with valid abbreviations.
std::string filter_url_clean_filename (std::string name)
{
  name = filter::string::replace ("\\", "b2", name);
  name = filter::string::replace ("/",  "sl", name);
  name = filter::string::replace (":",  "co", name);
  name = filter::string::replace ("*",  "as", name);
  name = filter::string::replace ("?",  "qu", name);
  name = filter::string::replace ("\"", "ba", name);
  name = filter::string::replace ("<",  "sm", name);
  name = filter::string::replace (">",  "la", name);
  name = filter::string::replace ("|",  "ve", name);
  return name;
}


// Replace invalid characters in Windows filenames with valid abbreviations.
// In contrast with the above function, the $name in this function can be "uncleaned" again.
// The next function does the "unclean" operation, to get the original $name back.
std::string filter_url_filename_clean (std::string name)
{
  name = filter::string::replace ("\\", "___b2___", name);
  name = filter::string::replace ("/",  "___sl___", name);
  name = filter::string::replace (":",  "___co___", name);
  name = filter::string::replace ("*",  "___as___", name);
  name = filter::string::replace ("?",  "___qu___", name);
  name = filter::string::replace ("\"", "___ba___", name);
  name = filter::string::replace ("<",  "___sm___", name);
  name = filter::string::replace (">",  "___la___", name);
  name = filter::string::replace ("|",  "___ve___", name);
  return name;
}


// Take $name, and undo the "clean" function in the above.
std::string filter_url_filename_unclean (std::string name)
{
  name = filter::string::replace ("___b2___", "\\", name);
  name = filter::string::replace ("___sl___", "/",  name);
  name = filter::string::replace ("___co___", ":",  name);
  name = filter::string::replace ("___as___", "*",  name);
  name = filter::string::replace ("___qu___", "?",  name);
  name = filter::string::replace ("___ba___", "\"", name);
  name = filter::string::replace ("___sm___", "<",  name);
  name = filter::string::replace ("___la___", ">",  name);
  name = filter::string::replace ("___ve___", "|",  name);
  return name;
}


// Changes a Unix directory separator to a Windows one.
// Works on Windows only.
std::string filter_url_update_directory_separator_if_windows (std::string filename)
{
#ifdef HAVE_WINDOWS
  filename = filter::string::replace ("/", DIRECTORY_SEPARATOR, filename);
#endif
  return filename;
}


// Returns true if it is possible to connect to port $port on $hostname.
bool filter_url_port_can_connect (std::string hostname, int port)
{
  // Resolve the host.
  addrinfo hints;
  addrinfo * address_results {nullptr};
  bool address_info_resolved {false};
  memset (&hints, 0, sizeof (addrinfo));
  // Allow IPv4 and IPv6.
  hints.ai_family = AF_UNSPEC;
  // TCP/IP socket.
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  // Select protocol that matches with the socket type.
  hints.ai_protocol = 0;
  // The 'service' is actually the port number.
  std::string service = std::to_string (port);
  // Get a list of address structures. There can be several of them.
  int res = getaddrinfo (hostname.c_str(), service.c_str (), &hints, &address_results);
  if (res != 0) return false;
  // Result of the text.
  bool connected {false};
  // Iterate over the list of address structures.
  std::vector <std::string> errors {};
  addrinfo * rp {nullptr};
  for (rp = address_results; rp != nullptr; rp = rp->ai_next) {
    // Try to get a socket for this address structure.
    int sock = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    // If it fails, try the next one.
    if (sock < 0) continue;
    // Try to connect.
    int res2 = connect (sock, rp->ai_addr, rp->ai_addrlen);
    // If connected, set the flag.
    if (res2 != -1) connected = true;
    // Socket should be closed.
    if (sock) {
  #ifdef HAVE_WINDOWS
      closesocket (sock);
  #else
      close (sock);
  #endif
    }
    // If connected: Done.
    if (connected) break;
  }
  // No longer needed: Only to be freed when the address was resolved.
  if (address_info_resolved) freeaddrinfo (address_results);
  // Done.
  return connected;
}


bool filter_url_is_image (std::string extension)
{
  if (extension == "png") return true;
  
  if (extension == "gif") return true;

  if (extension == "jpg") return true;
  
  if (extension == "svg") return true;
  
  // Default: It is not an image.
  return false;
}


// Source:
// https://svn.apache.org/repos/asf/httpd/httpd/trunk/docs/conf/mime.types
// See also:
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
std::string filter_url_get_mime_type (std::string extension)
{
  static std::map <std::string, std::string> mime_types = {
    {"jar", "application/java-archive"},
    {"js", "application/javascript"},
    {"json", "application/json"},
    {"pdf", "application/pdf"},
    {"odt", "application/vnd.oasis.opendocument.text"},
    {"xml", "application/xml"},
    {"zip", "application/zip"},
    {"otf", "font/otf"},
    {"ttf", "application/font-sfnt"},
    {"woff", "application/font-woff"},
    {"bmp", "image/bmp"}, // Windows OS/2 Bitmap Graphics
    {"gif", "image/gif"}, // Graphics Interchange Format
    {"jpe", "image/jpeg"}, // JPEG images
    {"jpg", "image/jpeg"}, // JPEG images
    {"jpeg", "image/jpeg"}, // JPEG images
    {"png", "image/png"}, // Portable Network Graphics
    {"svgz", "image/svg+xml"},
    {"svg", "image/svg+xml"}, // Scalable Vector Graphics
    {"tif", "image/tiff"}, // Tagged Image File Format
    {"tiff", "image/tiff"}, // Tagged Image File Format
    {"ico", "image/vnd.microsoft.icon"}, // Icon format
    {"css", "text/css"},
    {"csv", "text/csv"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"txt", "text/plain"},
    {"usfm", "text/plain"},
    {"webp", "image/webp"}, // WEBP image
  };
  return mime_types [extension];
}
    

// Read the URL, and split it up in three parts: The scheme, the host, and the port.
// For example: "https://bibledit.org:8080" will be split up into this:
// - https
// - bibledit.org
// - 8080
// If any of these three parts is not found, then the part is left empty, or the port remains 0.
void filter_url_get_scheme_host_port (std::string url, std::string & scheme, std::string & host, int & port)
{
  // Clear the values that are going to be detected.
  scheme.clear();
  host.clear();
  port = 0;

  // Extract the scheme: http(s).
  size_t pos = url.find ("://");
  if (pos != std::string::npos) {
    scheme = url.substr(0, pos);
    url.erase (0, pos + 3);
  }
  
  // Extract the host.
  pos = url.find (":");
  if (pos == std::string::npos) pos = url.find ("/");
  if (pos == std::string::npos) pos = url.length () + 1;
  host = url.substr (0, pos);
  url.erase (0, host.length ());
  
  // Extract the port number if any.
  pos = url.find (":");
  if (pos != std::string::npos) {
    url.erase (0, 1);
    size_t pos2 = url.find ("/");
    if (pos2 == std::string::npos) pos2 = url.length () + 1;
    std::string p = url.substr (0, pos2);
    port = filter::string::convert_to_int (p);
    url.erase (0, p.length ());
  }
}

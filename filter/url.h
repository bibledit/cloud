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


#ifndef INCLUDED_FILTER_URL_H
#define INCLUDED_FILTER_URL_H


#include <config/libraries.h>


string get_base_url (void * webserver_request);
void redirect_browser (void * webserver_request, string url);
string filter_url_dirname (string url);
string filter_url_dirname_web (string url);
string filter_url_basename (string url);
string filter_url_basename_web (string url);
void filter_url_unlink (string filename);
void filter_url_rename (const string& oldfilename, const string& newfilename);
string filter_url_create_path (string part1 = "", string part2 = "", string part3 = "", string part4 = "", string part5 = "", string part6 = "");
string filter_url_create_root_path (string part1 = "", string part2 = "", string part3 = "", string part4 = "", string part5 = "");
string filter_url_remove_root_path (string path);
string filter_url_get_extension (string url);
bool file_or_dir_exists (string url);
void filter_url_mkdir (string directory);
void filter_url_rmdir (string directory);
bool filter_url_is_dir (string path);
bool filter_url_get_write_permission (string path);
void filter_url_set_write_permission (string path);
string filter_url_file_get_contents (string filename);
void filter_url_file_put_contents (string filename, string contents);
void filter_url_file_put_contents_append (string filename, string contents);
bool filter_url_file_cp (string input, string output);
void filter_url_dir_cp (const string & input, const string & output);
int filter_url_filesize (string filename);
vector <string> filter_url_scandir (string folder);
void filter_url_recursive_scandir (string folder, vector <string> & paths);
int filter_url_file_modification_time (string filename);
string filter_url_urldecode (string url);
string filter_url_urlencode (string url);
const char * filter_url_temp_dir ();
string filter_url_tempfile (const char * directory = NULL);
string filter_url_escape_shell_argument (string argument);
string filter_url_unique_path (string path);
bool filter_url_email_is_valid (string email);
string filter_url_build_http_query (string url, const string& parameter, const string& value);
string filter_url_http_get (string url, string& error, bool check_certificate);
string filter_url_http_post (string url, map <string, string> values, string& error, bool burst, bool check_certificate);
string filter_url_http_upload (string url, map <string, string> values, string filename, string& error);
string filter_url_http_response_code_text (int code);
void filter_url_download_file (string url, string filename, string& error, bool check_certificate);
string filter_url_html_file_name_bible (string path = "", int book = 0, int chapter = -1);
int filter_url_curl_debug_callback (void *curl_handle, int curl_info_type, char *data, size_t size, void *userptr);
void filter_url_curl_set_timeout (void *curl_handle, bool burst = false);
string filter_url_plus_to_tag (string data);
string filter_url_tag_to_plus (string data);
string filter_url_remove_username_password (string url);
string filter_url_http_request_mbed (string url, string& error, const map <string, string>& post, const string& filename, bool check_certificate);
void filter_url_ssl_tls_initialize ();
void filter_url_ssl_tls_finalize ();
void filter_url_display_mbed_tls_error (int & ret, string * error, bool server);
string filter_url_set_scheme (string url, bool secure);
string filter_url_clean_filename (string name);
string filter_url_filename_clean (string name);
string filter_url_filename_unclean (string name);


#endif

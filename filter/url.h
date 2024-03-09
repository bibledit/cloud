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


#pragma once

#include <config/libraries.h>

class Webserver_Request;

std::string get_base_url (Webserver_Request& webserver_request);
void redirect_browser (Webserver_Request& webserver_request, std::string url);
std::string filter_url_dirname (std::string url);
std::string filter_url_dirname_web (std::string url);
std::string filter_url_basename (std::string url);
std::string filter_url_basename_web (std::string url);
void filter_url_unlink (std::string filename);
void filter_url_rename (const std::string& oldfilename, const std::string& newfilename);
std::string filter_url_create_path (const std::vector<std::string>& parts);
std::string filter_url_create_root_path (const std::vector<std::string>& parts);
std::string filter_url_get_extension (std::string url);
bool file_or_dir_exists (std::string url);
void filter_url_mkdir (std::string directory);
void filter_url_rmdir (std::string directory);
bool filter_url_is_dir (std::string path);
bool filter_url_get_write_permission (std::string path);
void filter_url_set_write_permission (std::string path);
std::string filter_url_file_get_contents (std::string filename);
void filter_url_file_put_contents (std::string filename, std::string contents);
void filter_url_file_put_contents_append (std::string filename, std::string contents);
bool filter_url_file_cp (std::string input, std::string output);
void filter_url_dir_cp (const std::string & input, const std::string & output);
int filter_url_filesize (std::string filename);
std::vector <std::string> filter_url_scandir (std::string folder);
void filter_url_recursive_scandir (std::string folder, std::vector <std::string> & paths);
int filter_url_file_modification_time (std::string filename);
std::string filter_url_urldecode (std::string url);
std::string filter_url_urlencode (std::string url);
const char * filter_url_temp_dir ();
std::string filter_url_tempfile (const char * directory = nullptr);
std::string filter_url_escape_shell_argument (std::string argument);
std::string filter_url_unique_path (std::string path);
bool filter_url_email_is_valid (std::string email);
std::string filter_url_build_http_query (std::string url, const std::string& parameter, const std::string& value);
std::string filter_url_http_get (std::string url, std::string& error, bool check_certificate);
std::string filter_url_http_post (const std::string & url, std::string post_data, const std::map <std::string, std::string> & post_values, std::string& error, bool burst, bool check_certificate, const std::vector <std::pair <std::string, std::string> > & headers);
std::string filter_url_http_upload (std::string url, std::map <std::string, std::string> values, std::string filename, std::string& error);
std::string filter_url_http_response_code_text (int code);
void filter_url_download_file (std::string url, std::string filename, std::string& error, bool check_certificate);
std::string filter_url_html_file_name_bible (std::string path = "", int book = 0, int chapter = -1);
int filter_url_curl_debug_callback (void *curl_handle, int curl_info_type, char *data, size_t size, void *userptr);
void filter_url_curl_set_timeout (void *curl_handle, bool burst = false);
std::string filter_url_plus_to_tag (std::string data);
std::string filter_url_tag_to_plus (std::string data);
std::string filter_url_remove_username_password (std::string url);
std::string filter_url_http_request_mbed (std::string url, std::string& error, const std::map <std::string, std::string>& post, const std::string& filename, bool check_certificate);
void filter_url_ssl_tls_initialize ();
void filter_url_ssl_tls_finalize ();
void filter_url_display_mbed_tls_error (int& ret, std::string* error, bool server, const std::string& remote_ip_address);
std::string filter_url_set_scheme (std::string url, bool secure);
std::string filter_url_clean_filename (std::string name);
std::string filter_url_filename_clean (std::string name);
std::string filter_url_filename_unclean (std::string name);
std::string filter_url_update_directory_separator_if_windows (std::string filename);
bool filter_url_port_can_connect (std::string hostname, int port);
bool filter_url_is_image (std::string extension);
std::string filter_url_get_mime_type (std::string extension);
void filter_url_get_scheme_host_port (std::string url, std::string & scheme, std::string & host, int & port);

